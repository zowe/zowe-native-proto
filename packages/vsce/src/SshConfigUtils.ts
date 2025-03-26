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

import {
    FileManagement,
    Gui,
    type IZoweTree,
    type IZoweTreeNode,
    ZoweVsCodeExtension,
    type imperative,
} from "@zowe/zowe-explorer-api";
import { Client, type ClientChannel } from "ssh2";
import * as vscode from "vscode";
import { type ISshConfigExt, ZClientUtils, ZSshClient } from "zowe-native-proto-sdk";
import { AbstractConfigManager, MESSAGE_TYPE, type inputBoxOpts } from "../../sdk/src/ZSshAuthUtils";

// biome-ignore lint/complexity/noStaticOnlyClass: Utilities class has static methods
export class SshConfigUtils {
    public static migratedConfigs: ISshConfigExt[];
    public static filteredMigratedConfigs: ISshConfigExt[];
    public static validationResult: ISshConfigExt | undefined;
    public static selectedProfile: ISshConfigExt | undefined;
    public static sshProfiles: imperative.IProfileLoaded[];

    public static getServerPath(profile?: imperative.IProfile): string {
        const serverPathMap: Record<string, string> =
            vscode.workspace.getConfiguration("zowe-native-proto-vsce").get("serverInstallPath") ?? {};
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
        SshConfigUtils.validationResult = undefined;
        if (profileName) {
            return profCache.getLoadedProfConfig(profileName, "ssh");
        }

        SshConfigUtils.sshProfiles = (await profCache.fetchAllProfilesByType("ssh")).filter(
            ({ name, profile }) => name && profile?.host,
        );
        // Get configs from ~/.ssh/config
        SshConfigUtils.migratedConfigs = await ZClientUtils.migrateSshConfig();

        // Parse to remove migratable configs that already exist on the team config
        SshConfigUtils.filteredMigratedConfigs = SshConfigUtils.migratedConfigs.filter(
            (migratedConfig) =>
                !SshConfigUtils.sshProfiles.some((sshProfile) => sshProfile.profile?.host === migratedConfig.hostname),
        );

        // Prompt user for ssh (new config, existing, migrating)
        const result = await SshConfigUtils.showQuickPickWithCustomInput();

        // If nothing selected, return
        if (!result) return;

        // If result is add new SSH host then create a new config, if not use migrated configs
        SshConfigUtils.selectedProfile = SshConfigUtils.filteredMigratedConfigs.find(
            ({ name, hostname }) => result?.label === name && result?.description === hostname,
        );

        if (result.description === "Custom SSH Host") {
            const createNewConfig = await SshConfigUtils.createNewConfig(result.label);
            if (!createNewConfig) return undefined;
            SshConfigUtils.selectedProfile = createNewConfig;
        } else if (result.label === "$(plus) Add New SSH Host...") {
            const createNewConfig = await SshConfigUtils.createNewConfig();
            if (!createNewConfig) return undefined;
            SshConfigUtils.selectedProfile = createNewConfig;
        }

        // If an existing team config profile was selected
        if (!SshConfigUtils.selectedProfile) {
            const foundProfile = SshConfigUtils.sshProfiles.find(({ name }) => name === result.label);
            if (foundProfile) {
                const validConfig = await SshConfigUtils.validateConfig({
                    hostname: foundProfile?.profile?.host,
                    port: foundProfile?.profile?.port,
                    privateKey: foundProfile?.profile?.privateKey,
                    keyPassphrase: foundProfile?.profile?.keyPassphrase,
                    user: foundProfile?.profile?.user,
                    password: foundProfile?.profile?.password,
                });
                if (validConfig === undefined) return;

                await SshConfigUtils.setProfile(validConfig, foundProfile.name);
                return { ...foundProfile, profile: { ...foundProfile.profile, ...validConfig } };
            }
        }

        // Current directory open in vscode window
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
        SshConfigUtils.selectedProfile = await SshConfigUtils.getNewProfileName(
            SshConfigUtils.selectedProfile!,
            profInfo.getTeamConfig(),
        );

        if (!SshConfigUtils.selectedProfile?.name) {
            vscode.window.showWarningMessage("SSH setup cancell1ed.");
            return;
        }

        if (SshConfigUtils.validationResult === undefined) {
            await SshConfigUtils.validateFoundPrivateKeys();
        }

        if (SshConfigUtils.validationResult === undefined) {
            // Attempt to validate with given URL/creds
            SshConfigUtils.validationResult = await SshConfigUtils.validateConfig(SshConfigUtils.selectedProfile);
        }
        // If validateConfig returns a string, that string is the correct keyPassphrase
        if (SshConfigUtils.validationResult && Object.keys(SshConfigUtils.validationResult).length >= 1) {
            SshConfigUtils.selectedProfile = { ...SshConfigUtils.selectedProfile, ...SshConfigUtils.validationResult };
        }

        // If no private key or password is on the profile then there is no possible validation combination, thus return
        if (!SshConfigUtils.selectedProfile?.privateKey && !SshConfigUtils.selectedProfile?.password) {
            vscode.window.showWarningMessage("SSH setup cancelled.");
            return;
        }

        await SshConfigUtils.setProfile(SshConfigUtils.selectedProfile);

        return {
            name: SshConfigUtils.selectedProfile.name,
            message: "",
            failNotFound: false,
            type: "ssh",
            profile: {
                host: SshConfigUtils.selectedProfile.hostname,
                name: SshConfigUtils.selectedProfile.name,
                password: SshConfigUtils.selectedProfile.password,
                user: SshConfigUtils.selectedProfile.user,
                privateKey: SshConfigUtils.selectedProfile.privateKey,
                handshakeTimeout: SshConfigUtils.selectedProfile.handshakeTimeout,
                port: SshConfigUtils.selectedProfile.port,
                keyPassphrase: SshConfigUtils.selectedProfile.keyPassphrase,
            },
        };
    }

    // Function to show the QuickPick with dynamic top option
    private static async showQuickPickWithCustomInput(): Promise<vscode.QuickPickItem | undefined> {
        // Choose between adding a new SSH host, an existing team config profile, and migrating from config.
        const qpItems: vscode.QuickPickItem[] = [
            { label: "$(plus) Add New SSH Host..." },
            ...SshConfigUtils.sshProfiles.map(({ name, profile }) => ({
                label: name!,
                description: profile!.host!,
            })),
            {
                label: "Migrate From SSH Config",
                kind: vscode.QuickPickItemKind.Separator,
            },
            ...SshConfigUtils.filteredMigratedConfigs.map(({ name, hostname }) => ({
                label: name!,
                description: hostname,
            })),
        ];

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

    private static async setProfile(selectedConfig: ISshConfigExt | undefined, updatedProfile?: string): Promise<void> {
        // Profile information
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

        if (updatedProfile) {
            for (const key of Object.keys(selectedConfig!)) {
                const validKey = key as keyof ISshConfigExt;

                // Get the location of the property being modified
                profCache.getDefaultProfile("ssh");
                const propertyLocation = profInfo
                    .mergeArgsForProfile({
                        profName: updatedProfile,
                        profType: "ssh",
                        isDefaultProfile: profCache.getDefaultProfile("ssh").name === updatedProfile,
                        profLoc: { locType: 1 },
                    })
                    .knownArgs.find((obj) => obj.argName === key)?.argLoc.jsonLoc;

                let allowBaseModification: string | undefined;

                if (propertyLocation) {
                    const profileName = configApi.profiles.getProfileNameFromPath(propertyLocation);

                    // Check to see if the property being modified comes from the service profile to handle possibly breaking configuration changes
                    const doesPropComeFromProfile = profileName === updatedProfile;

                    if (!doesPropComeFromProfile) {
                        const quickPick = vscode.window.createQuickPick();
                        quickPick.items = [
                            { label: "Yes", description: "Proceed with modification" },
                            { label: "No", description: "Modify SSH profile instead" },
                        ];
                        quickPick.title = `Property: "${key}" found in a possibly shared configuration and may break others, continue?`;
                        quickPick.placeholder = "Select an option";
                        quickPick.ignoreFocusOut = true;

                        allowBaseModification = await new Promise<string | undefined>((resolve) => {
                            quickPick.onDidAccept(() => {
                                resolve(quickPick.selectedItems[0]?.label);
                                quickPick.hide();
                            });
                            quickPick.onDidHide(() => resolve(undefined)); // Handle case when user cancels
                            quickPick.show();
                        });
                    }
                }

                profInfo.updateProperty({
                    profileName: updatedProfile,
                    profileType: "ssh",
                    property: validKey,
                    value: selectedConfig![validKey],
                    forceUpdate: allowBaseModification !== "Yes",
                    setSecure: profInfo.isSecured(),
                });
            }
        } else {
            if (!configApi.profiles.defaultGet("ssh")) configApi.profiles.defaultSet("ssh", selectedConfig?.name!);
            configApi.profiles.set(selectedConfig?.name!, config);
        }

        await profInfo.getTeamConfig().save();
    }

    private static async validateFoundPrivateKeys() {
        // Create a progress bar using the custom Gui.withProgress
        await Gui.withProgress(
            {
                location: vscode.ProgressLocation.Notification,
                title: "Validating SSH Configurations...",
                cancellable: false,
            },

            async (progress, token) => {
                // Find private keys located at ~/.ssh/ and attempt to connect with them
                if (!SshConfigUtils.validationResult) {
                    const foundPrivateKeys = await ZClientUtils.findPrivateKeys();
                    for (const privateKey of foundPrivateKeys) {
                        const testValidation: ISshConfigExt = SshConfigUtils.selectedProfile!;
                        testValidation.privateKey = privateKey;
                        const result = await SshConfigUtils.validateConfig(testValidation);
                        progress.report({ increment: 100 / foundPrivateKeys.length });

                        if (result) {
                            SshConfigUtils.validationResult = {};
                            if (Object.keys(result).length >= 1) {
                                SshConfigUtils.selectedProfile = { ...SshConfigUtils.selectedProfile, ...result };
                            }
                            return;
                        }
                    }
                }

                // Match hostname to configurations from ~/.ssh/config file
                let validationAttempts = SshConfigUtils.migratedConfigs.filter(
                    (config) => config.hostname === SshConfigUtils.selectedProfile?.hostname,
                );

                // If multiple matches exist, narrow down by user
                if (validationAttempts.length > 1 && SshConfigUtils.selectedProfile?.user) {
                    validationAttempts = validationAttempts.filter(
                        (config) => config.user === SshConfigUtils.selectedProfile?.user,
                    );
                } else {
                    // If no user is specified, allow all configs where the hostname matches
                    validationAttempts = validationAttempts.filter(
                        (config) =>
                            !SshConfigUtils.selectedProfile?.user ||
                            config.user === SshConfigUtils.selectedProfile?.user,
                    );
                }

                for (const profile of validationAttempts) {
                    const testValidation: ISshConfigExt = profile;
                    const result = await SshConfigUtils.validateConfig(testValidation);
                    progress.report({ increment: 100 / validationAttempts.length });
                    if (result !== undefined) {
                        SshConfigUtils.validationResult = {};
                        if (Object.keys(result).length >= 1) {
                            SshConfigUtils.selectedProfile = {
                                ...SshConfigUtils.selectedProfile,
                                ...result,
                                privateKey: testValidation.privateKey,
                            };
                        }
                        return;
                    }
                }
            },
        );
    }
}
class VscePromptApi extends AbstractConfigManager {
    protected showMessage(message: string, messageType: MESSAGE_TYPE): void {
        switch (messageType) {
            case MESSAGE_TYPE.INFORMATION:
                vscode.window.showInformationMessage(message);
                break;
            case MESSAGE_TYPE.WARNING:
                vscode.window.showWarningMessage(message);
                break;
            case MESSAGE_TYPE.ERROR:
                vscode.window.showErrorMessage(message);
                break;
            default:
                break;
        }
    }
    protected async showInputBox(opts: inputBoxOpts): Promise<string | undefined> {
        return vscode.window.showInputBox(opts);
    }
}
