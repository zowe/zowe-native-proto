/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

import { readFileSync } from "node:fs";
import * as path from "node:path";
import { ProfileConstants } from "@zowe/core-for-zowe-sdk";
import { ZosTsoProfile } from "@zowe/zos-tso-for-zowe-sdk";
import { ZosUssProfile } from "@zowe/zos-uss-for-zowe-sdk";
import { ZosmfProfile } from "@zowe/zosmf-for-zowe-sdk";
import { FileManagement, Gui, ZoweVsCodeExtension, imperative } from "@zowe/zowe-explorer-api";
import { Client } from "ssh2";
import * as vscode from "vscode";
import { ZClientUtils, ZSshClient } from "zowe-native-proto-sdk";
import type { sshConfigExt } from "zowe-native-proto-sdk";

// biome-ignore lint/complexity/noStaticOnlyClass: Utilities class has static methods
export class SshConfigUtils {
    public static getServerPath(profile?: imperative.IProfile): string {
        const serverPathMap: Record<string, string> =
            vscode.workspace.getConfiguration("zowe-native-proto-vsce").get("serverPath") ?? {};
        return (
            (profile && serverPathMap[profile?.host]) ??
            process.env.ZOWE_OPT_SERVER_PATH ??
            profile?.serverPath ??
            ZSshClient.DEFAULT_SERVER_PATH
        );
    }

    public static async promptForProfile(profileName?: string): Promise<imperative.IProfileLoaded | undefined> {
        const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
        const profCache = zoweExplorerApi.getExplorerExtenderApi().getProfilesCache();
        const profInfo = await profCache.getProfileInfo();

        if (profileName) {
            return profCache.getLoadedProfConfig(profileName, "ssh");
        }

        const sshProfiles = (await profCache.fetchAllProfilesByType("ssh")).filter(
            ({ name, profile }) => name && profile?.host,
        );

        // Get configs from ~/.ssh/config
        const migratedConfigs = await ZClientUtils.migrateSshConfig();

        // Parse to remove migratable configs that already exist on the team config
        const filteredMigratedConfigs = migratedConfigs.filter(
            (migratedConfig) => !sshProfiles.some((sshProfile) => sshProfile.profile?.host === migratedConfig.hostname),
        );

        // Choose between adding a new ssh host, an existing team config profile, and migrating from config.
        const qpItems: vscode.QuickPickItem[] = [
            { label: "$(plus) Add New SSH Host..." },
            ...sshProfiles.map(({ name, profile }) => ({
                label: name!,
                description: profile?.host,
            })),
            {
                label: "Migrate From SSH Config",
                kind: vscode.QuickPickItemKind.Separator,
            },

            ...filteredMigratedConfigs.map(({ name, hostname }) => ({
                label: name!,
                description: hostname,
            })),
        ];

        const result = await Gui.showQuickPick(qpItems, {
            title: "Choose an SSH host",
        });

        // If nothing selected, return
        if (!result) return;

        if (result === qpItems[0] || filteredMigratedConfigs.some(({ name }) => result.label === name)) {
            const isNewConfig = result === qpItems[0];

            // If result is add new SSH host then create a new config, if not use migrated configs
            let selectedProfile = isNewConfig
                ? await SshConfigUtils.createNewConfig()
                : filteredMigratedConfigs.find(({ name }) => result.label === name);

            if (!selectedProfile) return;

            const workspaceDir = ZoweVsCodeExtension.workspaceRoot;

            // Prioritize creating a team config in the local workspace if it exists even if a global config exists
            if (
                workspaceDir?.uri.fsPath !== undefined &&
                !profInfo.getTeamConfig().layerExists(workspaceDir!.uri.fsPath)
            ) {
                await SshConfigUtils.createZoweSchema(false);
            } else {
                await SshConfigUtils.createZoweSchema(true);
            }

            //If no profile name found, prompt for a new one with hostname as placeholder
            selectedProfile = await SshConfigUtils.getNewProfileName(selectedProfile);
            if (!selectedProfile?.name) {
                vscode.window.showWarningMessage("SSH setup cancelled.");
                return;
            }

            // Attempt to validate with given URL/creds
            let validationResult = await SshConfigUtils.validateConfig(selectedProfile);

            // If validateConfig returns a string, that string is the correct keyPassphrase
            if (typeof validationResult === "string") selectedProfile.keyPassphrase = validationResult;

            if (!validationResult) {
                // Create a progress bar using the custom Gui.withProgress
                await Gui.withProgress(
                    {
                        location: vscode.ProgressLocation.Notification,
                        title: "Validating SSH Configurations...",
                        cancellable: false,
                    },
                    async (progress, token) => {
                        let validationAttempts = migratedConfigs.filter(
                            (config) => config.hostname === selectedProfile?.hostname,
                        );

                        // If multiple matches exist, narrow down by user
                        if (validationAttempts.length > 1 && selectedProfile?.user) {
                            validationAttempts = validationAttempts.filter(
                                (config) => config.user === selectedProfile?.user,
                            );
                        } else {
                            // If no user is specified, allow all configs where the hostname matches
                            validationAttempts = validationAttempts.filter(
                                (config) => !selectedProfile?.user || config.user === selectedProfile?.user,
                            );
                        }

                        // Iterate over filtered validation attempts
                        let step = 0;
                        for (const profile of validationAttempts) {
                            const testValidation: sshConfigExt = profile;

                            const result = await SshConfigUtils.validateConfig(testValidation);
                            step++;
                            progress.report({ increment: (step / validationAttempts.length) * 100 });

                            if (result) {
                                validationResult = true;
                                if (typeof result === "string") {
                                    testValidation.keyPassphrase = result;
                                }
                                selectedProfile = testValidation;
                                break;
                            }
                        }

                        // Find private keys located at ~/.ssh/ and attempt to connect with them
                        if (!validationResult) {
                            const foundPrivateKeys = await ZClientUtils.findPrivateKeys();
                            for (const privateKey of foundPrivateKeys) {
                                const testValidation: sshConfigExt = selectedProfile!;
                                testValidation.privateKey = privateKey;

                                const result = await SshConfigUtils.validateConfig(testValidation);
                                step++;
                                progress.report({ increment: (step / foundPrivateKeys.length) * 100 });

                                if (result) {
                                    validationResult = true;
                                    if (typeof result === "string") {
                                        testValidation.keyPassphrase = result;
                                        selectedProfile = testValidation;
                                    }
                                    break;
                                }
                            }
                        }
                    },
                );
            }

            // Password loading bar
            await Gui.withProgress(
                {
                    location: vscode.ProgressLocation.Notification,
                    title: "Validating Password...",
                    cancellable: false,
                },
                async (progress) => {
                    // If not validated, remove private key from profile and get the password
                    if (!validationResult) {
                        selectedProfile!.privateKey = undefined;

                        // Show the password input prompt
                        const passwordPrompt = await vscode.window.showInputBox({
                            title: `${selectedProfile?.user}@${selectedProfile?.hostname}'s password:`,
                            password: true,
                            placeHolder: "Enter your password",
                            ignoreFocusOut: true,
                        });

                        if (!passwordPrompt) return;

                        if (!selectedProfile) return;

                        // Validate the password
                        const validatePassword = await SshConfigUtils.validateConfig({
                            ...selectedProfile,
                            password: passwordPrompt,
                        });

                        if (validatePassword && typeof validatePassword === "boolean") {
                            selectedProfile.password = passwordPrompt;
                        } else if (validatePassword && typeof validatePassword === "string") {
                            selectedProfile.password = validatePassword;
                        } else {
                            vscode.window.showWarningMessage("Password Authentication Failed");
                            return;
                        }
                    }
                },
            );

            // If no private key or password is on the profile then there is no possible validation combination, thus return
            if (!selectedProfile?.privateKey && !selectedProfile?.password) {
                vscode.window.showWarningMessage("SSH setup cancelled.");
                return;
            }

            await SshConfigUtils.setProfile(selectedProfile);
            return {
                name: selectedProfile.name,
                message: "",
                failNotFound: false,
                type: "ssh",
                profile: {
                    host: selectedProfile.hostname,
                    name: selectedProfile.name,
                    password: selectedProfile.password,
                    user: selectedProfile.user,
                    privateKey: selectedProfile.privateKey,
                    handshakeTimeout: selectedProfile.handshakeTimeout,
                    port: selectedProfile.port,
                    keyPassphrase: selectedProfile.keyPassphrase,
                },
            };
        }

        return sshProfiles.find(({ name }) => name === result.label);
    }

    public static showSessionInTree(profileName: string, visible: boolean): void {
        const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
        for (const setting of ["zowe.ds.history", "zowe.uss.history", "zowe.jobs.history"]) {
            const localStorage = (zoweExplorerApi.getExplorerExtenderApi() as any).getLocalStorage();
            const treeHistory = localStorage.getValue(setting);
            treeHistory.sessions = treeHistory.sessions.filter((session: any) => session !== profileName);
            if (visible) {
                treeHistory.sessions.push(profileName);
            }
            localStorage.setValue(setting, treeHistory);
        }
        zoweExplorerApi.getExplorerExtenderApi().reloadProfiles("ssh");
    }

    private static async createNewConfig(): Promise<sshConfigExt | undefined> {
        const SshProfile: sshConfigExt = {};
        const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
        const profInfo = await zoweExplorerApi.getExplorerExtenderApi().getProfilesCache().getProfileInfo();

        //check if project layer exists, if it doesnt create one, but if no workspace then create it as global

        const workspaceDir = ZoweVsCodeExtension.workspaceRoot;
        if (workspaceDir?.uri.fsPath !== undefined && !profInfo.getTeamConfig().layerExists(workspaceDir!.uri.fsPath)) {
            await SshConfigUtils.createZoweSchema(false);
        }

        const sshResponse = await vscode.window.showInputBox({
            prompt: "Enter SSH connection command",
            placeHolder: "E.g. ssh user@example.com",
        });
        if (sshResponse === undefined) {
            vscode.window.showWarningMessage("SSH setup cancelled.");
            return undefined;
        }
        const sshRegex = /^ssh\s+([a-zA-Z0-9_-]+)@([a-zA-Z0-9.-]+)/;
        const flagRegex = /-(\w+)(?:\s+("[^"]+"|'[^']+'|\S+))?/g;

        const sshMatch = sshResponse.match(sshRegex);
        if (!sshMatch) {
            vscode.window.showErrorMessage("Invalid SSH command format. Ensure it matches the expected pattern.");
            return undefined;
        }
        SshProfile.user = sshMatch[1];
        SshProfile.hostname = sshMatch[2];

        let flagMatch: RegExpExecArray | null;
        while ((flagMatch = flagRegex.exec(sshResponse)) !== null) {
            const [, flag, value] = flagMatch;
            // Check for missing value
            if (!value) {
                vscode.window.showErrorMessage(`Missing value for flag -${flag}.`);
                return undefined;
            }

            const unquotedValue = value.replace(/^["']|["']$/g, ""); // Remove surrounding quotes

            // Map aliases to consistent keys
            if (flag === "p" || flag.toLowerCase() === "port") {
                const portNumber = Number.parseInt(unquotedValue, 10);
                if (Number.isNaN(portNumber)) {
                    vscode.window.showErrorMessage(`Invalid value for flag --${flag}. Port must be a valid number.`);
                    return undefined;
                }
                SshProfile.port = portNumber;
            } else if (flag === "i" || flag.toLowerCase() === "identity_file") {
                SshProfile.privateKey = unquotedValue;
            }

            // Validate if quotes are required
            if (/\s/.test(unquotedValue) && !/^["'].*["']$/.test(value)) {
                vscode.window.showErrorMessage(`Invalid value for flag --${flag}. Values with spaces must be quoted.`);
                return undefined;
            }
        }

        return SshProfile;
    }

    private static async getNewProfileName(selectedProfile: sshConfigExt): Promise<sshConfigExt | undefined> {
        const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
        const profCache = zoweExplorerApi.getExplorerExtenderApi().getProfilesCache();
        const profiles = await profCache.fetchAllProfiles();
        let isUniqueName = false;

        if (!selectedProfile.name) selectedProfile.name = selectedProfile.hostname;
        while (!isUniqueName) {
            selectedProfile.name = await vscode.window.showInputBox({
                prompt: "Enter a name for the SSH config",
                value: selectedProfile.name!.replace(/\./g, "_"),
                validateInput: (input) => (input.includes(".") ? "Name cannot contain '.'" : null),
            });
            const existingProfile = profiles.find((profile) => profile.name === selectedProfile.name);
            if (existingProfile) {
                const overwriteResponse = await vscode.window.showQuickPick(["Yes", "No"], {
                    placeHolder: `A profile with the name "${selectedProfile.name}" already exists. Do you want to overwrite it?`,
                });
                if (overwriteResponse === "Yes") {
                    isUniqueName = true;
                } else {
                }
            } else {
                isUniqueName = true;
            }
        }
        return selectedProfile;
    }

    private static async setProfile(selectedConfig: sshConfigExt | undefined): Promise<void> {
        //Profile information
        const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
        const profCache = zoweExplorerApi.getExplorerExtenderApi().getProfilesCache();
        const profInfo = await profCache.getProfileInfo();
        const configApi = profInfo.getTeamConfig().api;
        // Create the base config object
        const config = {
            type: "ssh",
            properties: {
                user: selectedConfig?.user,
                host: selectedConfig?.hostname,
                privateKey: selectedConfig?.privateKey,
                port: selectedConfig?.port || 22,
                keyPassphrase: selectedConfig?.keyPassphrase,
                password: selectedConfig?.password,
            },
            secure: [],
        };
        //if password or KP is defined, make them secure
        if (selectedConfig?.password) config.secure.push("password" as never);
        if (selectedConfig?.keyPassphrase) config.secure.push("keyPassphrase" as never);

        configApi.profiles.defaultSet("ssh", selectedConfig?.name!);
        configApi.profiles.set(selectedConfig?.name!, config);
        await profInfo.getTeamConfig().save();
    }

    // Cloned method
    public static async createZoweSchema(global: boolean): Promise<void> {
        try {
            const user = false;
            const workspaceDir = ZoweVsCodeExtension.workspaceRoot;

            const config = await imperative.Config.load("zowe", {
                homeDir: FileManagement.getZoweDir(),
                projectDir: workspaceDir?.uri.fsPath,
            });

            config.api.layers.activate(user, global);

            const knownCliConfig: imperative.ICommandProfileTypeConfiguration[] = [
                ZosUssProfile,
                ProfileConstants.BaseProfile,
            ];

            if (global) knownCliConfig.push(ZosmfProfile, ZosTsoProfile);

            config.setSchema(imperative.ConfigSchema.buildSchema(knownCliConfig));

            // Note: IConfigBuilderOpts not exported
            // const opts: IConfigBuilderOpts = {
            const opts: any = {
                // getSecureValue: this.promptForProp.bind(this),
                populateProperties: true,
            };
            // Build new config and merge with existing layer
            const impConfig: Partial<imperative.IImperativeConfig> = {
                profiles: [ProfileConstants.BaseProfile],
                baseProfile: ProfileConstants.BaseProfile,
            };
            const newConfig: imperative.IConfig = await imperative.ConfigBuilder.build(impConfig, global, opts);
            config.api.layers.merge(newConfig);
            await config.save(false);
        } catch (err) {}
    }
    private static async validateConfig(newConfig: sshConfigExt): Promise<boolean | string> {
        const attemptConnection = async (config: sshConfigExt) => {
            return new Promise((resolve, reject) => {
                const sshClient = new Client();
                const testConnection = { ...config }; // Create a shallow copy

                // Parse privateKey
                if (testConnection.privateKey && typeof testConnection.privateKey === "string") {
                    testConnection.privateKey = readFileSync(path.normalize(testConnection.privateKey), "utf8");
                }
                // Test credentials
                sshClient
                    .connect(testConnection)
                    .on("error", (err) => {
                        if (err) {
                            reject(err);
                        }
                    })
                    .on("ready", () => {
                        resolve(null);
                    });
            });
        };

        try {
            // If private key cant be opened or found and there is no password, return false validation
            if (
                (!newConfig?.privateKey || !readFileSync(path.normalize(newConfig?.privateKey!), "utf-8")) &&
                !newConfig?.password
            )
                return false;
            await attemptConnection(newConfig);
        } catch (err) {
            // Case in which the private key requires a passphrase
            if ((err as any).message.includes("but no passphrase given")) {
                let passphraseAttempts = 0;
                while (passphraseAttempts < 3) {
                    (newConfig as any).passphrase = await vscode.window.showInputBox({
                        title: `Enter passphrase for key '${newConfig.privateKey}'`,
                        password: true,
                        placeHolder: "Enter passphrase for key",
                        ignoreFocusOut: true,
                    });

                    try {
                        await attemptConnection(newConfig);
                        return (newConfig as any).passphrase;
                    } catch (error) {
                        if (!(error as any).message.includes("integrity check failed")) break;
                        passphraseAttempts++;
                        vscode.window.showErrorMessage(`Passphrase Authentication Failed (${passphraseAttempts}/3)`);
                    }
                }

                // Max attempts reached, clean up and return false
                (newConfig as any).passphrase = undefined;
                (newConfig as any).privateKey = undefined;
                return false;
            }
            // Case in which password was given but failed
            if ((err as any).message.includes("All configured authentication methods failed")) {
                // 1 attempt has already occured
                let passwordAttempts = 1;
                vscode.window.showErrorMessage(`Password Authentication Failed (${passwordAttempts}/3)`);

                while (passwordAttempts <= 3) {
                    newConfig.password = await vscode.window.showInputBox({
                        title: `${newConfig.user}@${newConfig.hostname}'s password:`,
                        password: true,
                        placeHolder: "Enter your password",
                        ignoreFocusOut: true,
                    });
                    try {
                        // If password is blank then do not attempt and break out and return false;
                        if (newConfig.password === "") break;
                        await attemptConnection(newConfig);
                        return (newConfig as any).password;
                    } catch {
                        passwordAttempts++;
                        vscode.window.showErrorMessage(`Password Authentication Failed (${passwordAttempts}/3)`);
                    }
                }
            }
            return false;
        }
        return true;
    }
}
