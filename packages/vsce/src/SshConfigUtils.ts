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
import {
    FileManagement,
    Gui,
    type IZoweTree,
    type IZoweTreeNode,
    ZoweVsCodeExtension,
    imperative,
} from "@zowe/zowe-explorer-api";
import { Client, type ClientChannel } from "ssh2";
import * as vscode from "vscode";
import { type ISshConfigExt, ZClientUtils, ZSshClient } from "zowe-native-proto-sdk";

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

        // Choose between adding a new SSH host, an existing team config profile, and migrating from config.
        const qpItems: vscode.QuickPickItem[] = [
            { label: "$(plus) Add New SSH Host..." },
            ...sshProfiles.map(({ name, profile }) => ({
                label: name!,
                description: profile!.host!,
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

        // Function to show the QuickPick with dynamic top option
        async function showQuickPickWithCustomInput(): Promise<vscode.QuickPickItem | undefined> {
            const quickPick = vscode.window.createQuickPick();
            quickPick.items = qpItems;
            quickPick.placeholder = "Select configured SSH host or enter user@host";

            let value: undefined;

            // Add the custom entry
            const customItem = {
                label: `> ${value}`, // Using ">" as a visual cue for custom input
                description: "Custom SSH Host",
                alwaysShow: true,
            };
            quickPick.onDidChangeValue((value) => {
                if (value) {
                    // Update custom entry when something is typed in search bar
                    customItem.label = `> ${value}`;
                    // Update the QuickPick items with the custom entry at the top, if not already added
                    quickPick.items = [customItem, ...qpItems.filter((item) => item.label !== customItem.label)];
                } else {
                    // Remove the custom entry if the search bar is cleared
                    quickPick.items = [...qpItems];
                }
            });

            // Show the QuickPick
            quickPick.show();

            // Wait for selection
            const result = await new Promise<vscode.QuickPickItem | undefined>((resolve) => {
                quickPick.onDidAccept(() => {
                    resolve(quickPick.selectedItems[0]);
                    quickPick.hide();
                });
            });

            if (result?.label.startsWith(">")) result.label = result.label.replace(">", "").trim();

            return result;
        }
        const result = await showQuickPickWithCustomInput();

        // If nothing selected, return
        if (!result) return;

        // If result is add new SSH host then create a new config, if not use migrated configs
        let selectedProfile = filteredMigratedConfigs.find(
            ({ name, hostname }) => result.label === name && result.description === hostname,
        );

        if (result.description === "Custom SSH Host") {
            selectedProfile = await SshConfigUtils.createNewConfig(result.label, false);
        } else if (result.label === "$(plus) Add New SSH Host...") {
            selectedProfile = await SshConfigUtils.createNewConfig();
        }

        if (!selectedProfile) return sshProfiles.find(({ name }) => name === result.label);

        const workspaceDir = ZoweVsCodeExtension.workspaceRoot;

        // Prioritize creating a team config in the local workspace if it exists even if a global config exists
        // TODO: This behavior is only for the POC phase
        ////////////////////////////////////////////////////////////////////////////////////////////////////////
        if (workspaceDir?.uri.fsPath !== undefined && !profInfo.getTeamConfig().layerExists(workspaceDir!.uri.fsPath)) {
            await SshConfigUtils.createZoweSchema(false);
        } else {
            await SshConfigUtils.createZoweSchema(true);
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////

        // Prompt for a new profile name with the hostname (for adding a new config) or host value (for migrating from a config)
        selectedProfile = await SshConfigUtils.getNewProfileName(selectedProfile, profInfo.getTeamConfig());
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
                        const testValidation: ISshConfigExt = profile;

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
                            const testValidation: ISshConfigExt = selectedProfile!;
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
                        // vscode.window.showWarningMessage("Password Authentication Failed");
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

    public static async showSessionInTree(profileName: string, visible: boolean): Promise<void> {
        // This method is a hack until the ZE API offers a method to show/hide profile in tree
        // See https://github.com/zowe/zowe-explorer-vscode/issues/3506
        const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi().getExplorerExtenderApi();
        const treeProviders = ["datasetProvider", "ussFileProvider", "jobsProvider"].map(
            // biome-ignore lint/suspicious/noExplicitAny: Accessing internal properties
            (prop) => (zoweExplorerApi as any)[prop] as IZoweTree<IZoweTreeNode>,
        );
        const localStorage = zoweExplorerApi.getLocalStorage?.();
        for (const provider of treeProviders) {
            // Show or hide profile in active window
            const sessionNode = provider.mSessionNodes.find((node) => node.getProfileName() === profileName);
            if (visible && sessionNode == null) {
                await provider.addSession({ sessionName: profileName, profileType: "ssh" });
            } else if (!visible && sessionNode != null) {
                provider.deleteSession(sessionNode);
            }
            // Update tree session history to persist
            const settingName = provider.getTreeType();
            if (localStorage != null) {
                const treeHistory = localStorage.getValue<{ sessions: string[] }>(settingName);
                treeHistory.sessions = treeHistory.sessions.filter((session: string) => session !== profileName);
                if (visible) {
                    treeHistory.sessions.push(profileName);
                }
                localStorage.setValue(settingName, treeHistory);
            }
        }
    }

    private static async createNewConfig(
        knownConfigOpts?: string,
        acceptFlags = true,
    ): Promise<ISshConfigExt | undefined> {
        const sshRegex = /^ssh\s+([a-zA-Z0-9_-]+)@([a-zA-Z0-9.-]+)/;
        const flagRegex = /-(\w+)(?:\s+("[^"]+"|'[^']+'|\S+))?/g;
        const SshProfile: ISshConfigExt = {};
        const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
        const profInfo = await zoweExplorerApi.getExplorerExtenderApi().getProfilesCache().getProfileInfo();

        //check if project layer exists, if it doesnt create one, but if no workspace then create it as global

        const workspaceDir = ZoweVsCodeExtension.workspaceRoot;
        if (workspaceDir?.uri.fsPath !== undefined && !profInfo.getTeamConfig().layerExists(workspaceDir!.uri.fsPath)) {
            await SshConfigUtils.createZoweSchema(false);
        }

        let sshResponse: string | undefined;

        // KnownConfigOpts is defined if a custom option is selected via the first quickpick (ex: user@host is entered in search bar)
        if (!knownConfigOpts)
            sshResponse = await vscode.window.showInputBox({
                prompt: "Enter SSH connection command",
                placeHolder: "E.g. ssh user@example.com",
                ignoreFocusOut: true,
            });
        else {
            sshResponse = `ssh ${knownConfigOpts}`;
            if (!sshResponse.match(sshRegex))
                vscode.window.showErrorMessage(
                    "Invalid custom connection format. Ensure it matches the expected pattern.",
                );
        }

        if (sshResponse === undefined) {
            vscode.window.showWarningMessage("SSH setup cancelled.");
            return undefined;
        }

        const sshMatch = sshResponse.match(sshRegex);
        if (!sshMatch) {
            vscode.window.showErrorMessage("Invalid SSH command format. Ensure it matches the expected pattern.");
            return undefined;
        }
        SshProfile.user = sshMatch[1];
        SshProfile.hostname = sshMatch[2];

        let flagMatch: RegExpExecArray | null;

        if (acceptFlags) {
            // biome-ignore lint/suspicious/noAssignInExpressions: We just want to use the regex array in the loop
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
                        vscode.window.showErrorMessage(
                            `Invalid value for flag --${flag}. Port must be a valid number.`,
                        );
                        return undefined;
                    }
                    SshProfile.port = portNumber;
                } else if (flag === "i" || flag.toLowerCase() === "identity_file") {
                    SshProfile.privateKey = unquotedValue;
                }

                // Validate if quotes are required
                if (/\s/.test(unquotedValue) && !/^["'].*["']$/.test(value)) {
                    vscode.window.showErrorMessage(
                        `Invalid value for flag --${flag}. Values with spaces must be quoted.`,
                    );
                    return undefined;
                }
            }
        }
        return SshProfile;
    }

    private static async getNewProfileName(
        selectedProfile: ISshConfigExt,
        configApi: imperative.Config,
    ): Promise<ISshConfigExt | undefined> {
        let isUniqueName = false;

        // If no name option set then use hostname with all "." replaced with "_"
        if (!selectedProfile.name) selectedProfile.name = selectedProfile.hostname!.replace(/\./g, "_");

        // If selectedProfile already has a name, return it unless an existing profile is found
        if (selectedProfile.name) {
            const existingProfile = configApi.layerActive().properties.profiles[selectedProfile.name];
            if (existingProfile) {
                const overwriteResponse = await vscode.window.showQuickPick(["Yes", "No"], {
                    placeHolder: `A profile with the name "${selectedProfile.name}" already exists. Do you want to overwrite it?`,
                });
                if (overwriteResponse === "Yes") return selectedProfile;
            } else return selectedProfile;
        }

        // If no name set or overwriting, proceed with the input loop
        while (!isUniqueName) {
            selectedProfile.name = await vscode.window.showInputBox({
                prompt: "Enter a name for the SSH config",
                value: selectedProfile.name!.replace(/\./g, "_"),
                validateInput: (input) => (input.includes(".") ? "Name cannot contain '.'" : null),
            });
            if (!selectedProfile.name) return;
            const existingProfile = configApi.layerActive().properties.profiles[selectedProfile.name];
            if (existingProfile) {
                const overwriteResponse = await vscode.window.showQuickPick(["Yes", "No"], {
                    placeHolder: `A profile with the name "${selectedProfile.name}" already exists. Do you want to overwrite it?`,
                });
                if (overwriteResponse === "Yes") {
                    isUniqueName = true;
                }
            } else {
                isUniqueName = true;
            }
        }
        return selectedProfile;
    }

    private static async setProfile(selectedConfig: ISshConfigExt | undefined): Promise<void> {
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

        if (!configApi.profiles.defaultGet("ssh")) configApi.profiles.defaultSet("ssh", selectedConfig?.name!);

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

            const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
            const profCache = zoweExplorerApi.getExplorerExtenderApi().getProfilesCache();
            const knownCliConfig: imperative.ICommandProfileTypeConfiguration[] = [
                // biome-ignore lint/suspicious/noExplicitAny: Accessing protected method
                ...(profCache as any).getCoreProfileTypes(),
                ...profCache.getConfigArray(),
                ProfileConstants.BaseProfile,
            ];

            config.setSchema(imperative.ConfigSchema.buildSchema(knownCliConfig));

            // Note: IConfigBuilderOpts not exported
            // const opts: IConfigBuilderOpts = {
            const opts: imperative.IConfigBuilderOpts = {
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
    private static async validateConfig(newConfig: ISshConfigExt): Promise<boolean | string> {
        const attemptConnection = async (config: ISshConfigExt): Promise<boolean> => {
            return new Promise((resolve, reject) => {
                const sshClient = new Client();
                const testConnection = { ...config }; // Create a shallow copy

                // Parse privateKey if provided
                if (testConnection.privateKey && typeof testConnection.privateKey === "string") {
                    testConnection.privateKey = readFileSync(path.normalize(testConnection.privateKey), "utf8");
                }

                // Test credentials
                sshClient
                    .connect({ ...testConnection, passphrase: testConnection.keyPassphrase })
                    .on("error", (err) => {
                        reject(err); // Reject if connection fails
                    })
                    .on("ready", () => {
                        sshClient.shell((err, stream: ClientChannel) => {
                            if (err) {
                                reject(err); // Reject if shell command fails
                                return;
                            }

                            stream.on("data", (data: Buffer | string) => {
                                const dataStr = data.toString();

                                // If password expired error is detected
                                if (dataStr.startsWith("FOTS1668")) {
                                    reject(new Error(dataStr));
                                }
                            });
                            stream.on("end", () => {
                                resolve(true);
                            });
                            sshClient.end();
                        });
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
            const errorMessage = `${err}`;

            // Private key authentication failed (but not due to missing passphrase)
            if (
                newConfig.privateKey &&
                !newConfig.password &&
                errorMessage.includes("All configured authentication methods failed")
            ) {
                return false;
            }

            // Case: Private key requires a passphrase
            if (errorMessage.includes("but no passphrase given")) {
                let passphraseAttempts = 0;
                while (passphraseAttempts < 3) {
                    newConfig.keyPassphrase = await vscode.window.showInputBox({
                        title: `Enter passphrase for key '${newConfig.privateKey}'`,
                        password: true,
                        placeHolder: "Enter passphrase for key",
                        ignoreFocusOut: true,
                    });

                    try {
                        await attemptConnection(newConfig);
                        return newConfig.keyPassphrase || true;
                    } catch (error) {
                        if (!`${error}`.includes("integrity check failed")) break;
                        passphraseAttempts++;
                        vscode.window.showErrorMessage(`Passphrase Authentication Failed (${passphraseAttempts}/3)`);
                    }
                }

                // Max attempts reached, clean up and return false
                newConfig.keyPassphrase = undefined;
                newConfig.privateKey = undefined;
                return false;
            }

            // Case: Password authentication failed
            if (errorMessage.includes("All configured authentication methods failed")) {
                let passwordAttempts = 1;
                vscode.window.showErrorMessage(`Password Authentication Failed (${passwordAttempts}/3)`);

                while (passwordAttempts < 3) {
                    newConfig.password = await vscode.window.showInputBox({
                        title: `${newConfig.user}@${newConfig.hostname}'s password:`,
                        password: true,
                        placeHolder: "Enter your password",
                        ignoreFocusOut: true,
                    });

                    if (!newConfig.password) break; // Do not retry if user leaves input blank

                    try {
                        await attemptConnection(newConfig);
                        return newConfig.password as string;
                    } catch (passwordError) {
                        passwordAttempts++;
                        if (`${passwordError}`.includes("FOTS1668")) {
                            vscode.window.showErrorMessage("Password Expired on Target System");
                            return false;
                        }
                        vscode.window.showErrorMessage(`Password Authentication Failed (${passwordAttempts}/3)`);
                    }
                }
            }
            return false;
        }
        return true;
    }
}
