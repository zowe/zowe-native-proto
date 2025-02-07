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

import { Gui, ZoweVsCodeExtension, type imperative } from "@zowe/zowe-explorer-api";
import * as fs from "fs";
import * as vscode from "vscode";
import * as sshConfig from "ssh-config";
import { ZSshClient } from "zowe-native-proto-sdk";
import { homedir } from 'os';
import { ISshSession } from "@zowe/zos-uss-for-zowe-sdk";
import * as path from "node:path";

interface sshConfigExt extends ISshSession {
    name?: string;
}

// biome-ignore lint/complexity/noStaticOnlyClass: Utilities class has static methods
export class SshConfigUtils {
    public static getServerPath(hostname: string): string {
        const serverPathMap = vscode.workspace
            .getConfiguration("zowe-native-proto-vsce")
            .get<Record<string, string>>("serverPath");
        return serverPathMap?.[hostname] ?? ZSshClient.DEFAULT_SERVER_PATH;
    }

    public static async promptForProfile(profileName?: string): Promise<imperative.IProfileLoaded | undefined> {
        const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
        const profCache = zoweExplorerApi.getExplorerExtenderApi().getProfilesCache();

        if (profileName != null) {
            return profCache.getLoadedProfConfig(profileName, "ssh");
        }

        const sshProfiles = (await profCache.fetchAllProfilesByType("ssh"))
            .filter(({ name, profile }) => name != null && profile?.host);

        // get configs via migrateSshConfig() and remove options that have host name that already exists as an ssh profile on the config file
        const migratedConfigs = (await SshConfigUtils.migrateSshConfig()).filter(migratedConfig =>
            !sshProfiles.some(sshProfile => sshProfile.profile?.host === migratedConfig.hostname)
        );

        const qpItems: vscode.QuickPickItem[] = [
            { label: "$(plus) Add New SSH Host..." },
            ...sshProfiles.map((prof) => ({ label: prof.name!, description: prof.profile?.host })),
            { label: "Merge From SSH Config", kind: vscode.QuickPickItemKind.Separator },
            ...migratedConfigs.map((config) => ({ label: config.name!, description: config.hostname }))
        ];

        const result = await Gui.showQuickPick(qpItems, { title: "Choose an SSH host" });

        let selectedProfile: sshConfigExt | undefined;

        if (result === qpItems[0] || migratedConfigs.some((config) => result?.label === config.name!)) {
            if(result === qpItems[0]){
                // if (await SshConfigUtils.createTeamConfig()) {
                //     return profCache.getDefaultProfile("ssh");
                // }
                selectedProfile = await SshConfigUtils.createNewConfig();
                await profCache.refresh();
            }
            else if(migratedConfigs.some((config) => result?.label === config.name!)){
                selectedProfile = migratedConfigs.find((config) => result!.label === config.name!);
            }
            if (selectedProfile) {
                selectedProfile = await SshConfigUtils.getNewProfileName(selectedProfile);
                selectedProfile = await SshConfigUtils.promptForAuth(selectedProfile);
            }
        }
        else if (result != null) {
            return sshProfiles.find((prof) => prof.name === result.label);
        }
    }

    private static async migrateSshConfig(): Promise<sshConfigExt[]> {
        const filePath = path.join(homedir(), '.ssh', 'config');
        let fileContent: string;
        try {
            fileContent = fs.readFileSync(filePath, "utf-8");
        } catch (error) {
            vscode.window.showErrorMessage(`Error reading SSH config: ${error}`);
            return [];
        }

        const parsedConfig = sshConfig.parse(fileContent);
        const SSHConfigs: sshConfigExt[] = [];

        for (const config of parsedConfig) {
            if (config.type === sshConfig.LineType.DIRECTIVE) {
                const session: sshConfigExt = {};
                session.name = (config as any).value;

                if (Array.isArray((config as any).config)) {
                    for (const subConfig of (config as any).config) {
                        if (typeof subConfig === 'object' && 'param' in subConfig && 'value' in subConfig) {
                            const param = (subConfig as any).param.toLowerCase();
                            const value = (subConfig as any).value;

                            switch (param) {
                                case 'hostname':
                                    session.hostname = value;
                                    break;
                                case 'port':
                                    session.port = parseInt(value);
                                    break;
                                case 'user':
                                    session.user = value;
                                    break;
                                case 'identityfile':
                                    session.privateKey = value;
                                    break;
                                case 'connecttimeout':
                                    session.handshakeTimeout = parseInt(value);
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                }
                SSHConfigs.push(session);
            }
        }
        return SSHConfigs;
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

    private static async createNewConfig(): Promise<sshConfigExt | undefined>{
        let SshProfile: sshConfigExt = {};
        let privateKeyPath: string | boolean | undefined;
        const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
        const profInfo = await zoweExplorerApi.getExplorerExtenderApi().getProfilesCache().getProfileInfo();
        const configExists = profInfo.getTeamConfig().exists;

        if (!configExists) await vscode.commands.executeCommand("zowe.ds.addSession");

        const sshResponse = await vscode.window.showInputBox({
            prompt: "Enter SSH connection command",
            placeHolder:
                'Enter the SSH host address (E.g. ssh user@example.com --port 22 --privateKey "my passphrase")',
        });
        if (sshResponse === undefined) {
            vscode.window.showWarningMessage("SSH setup cancelled.");
            return undefined;
        }
        const sshRegex = /^ssh\s+([a-zA-Z0-9_-]+)@([a-zA-Z0-9.-]+)/;
        const flagRegex = /--(\w+)(?:\s+("[^"]+"|'[^']+'|\S+))?/g;

        const sshMatch = sshResponse.match(sshRegex);
        if (!sshMatch) {
            vscode.window.showErrorMessage("Invalid SSH command format. Ensure it matches the expected pattern.");
            return undefined;
        }
        SshProfile.user = sshMatch[1];
        SshProfile.hostname = sshMatch[2];

        let flagMatch;
        while ((flagMatch = flagRegex.exec(sshResponse)) !== null) {
            const [, flag, value] = flagMatch;
            const normalizedFlag = flag.toLowerCase();

            // Check for missing value
            if (!value) {
                vscode.window.showErrorMessage(`Missing value for flag --${flag}.`);
                return undefined;
            }

            const unquotedValue = value.replace(/^["']|["']$/g, ""); // Remove surrounding quotes

            // Map aliases to consistent keys
            if (normalizedFlag === "p" || normalizedFlag === "port") {
                const portNumber = Number.parseInt(unquotedValue, 10);
                if (Number.isNaN(portNumber)) {
                    vscode.window.showErrorMessage(`Invalid value for flag --${flag}. Port must be a valid number.`);
                    return undefined;
                }
                SshProfile.port = portNumber;
            } else if (normalizedFlag === "pk" || normalizedFlag === "privateKey") {
                SshProfile.privateKey = unquotedValue;
            }

            // Validate if quotes are required
            if (/\s/.test(unquotedValue) && !/^["'].*["']$/.test(value)) {
                vscode.window.showErrorMessage(`Invalid value for flag --${flag}. Values with spaces must be quoted.`);
                return undefined;
            }
        }

        if (!SshProfile.privateKey) {
            if (privateKeyPath == null) {
                privateKeyPath = true;
            }
        } else {
            privateKeyPath = undefined;
        }
        try {
            if (typeof privateKeyPath === "string") {
                let match;
                if ((match = /~(\/.*)/.exec(privateKeyPath))) {
                    privateKeyPath = path.join(homedir(), match[1]);
                }
            } else if (privateKeyPath === true) {
                for (const algo of ["id_ed25519", "id_rsa"]) {
                    const tempPath = path.resolve(homedir(), ".ssh", algo);
                    if (fs.existsSync(tempPath)) {
                        privateKeyPath = path.resolve(homedir(), ".ssh", algo);
                        break;
                    }
                }
                if (SshProfile.privateKey == null) {
                    throw Error("Failed to discover an ssh private key inside `~/.ssh`.");
                }
            }
        } catch (err) {
            //do nuthin
        }
        return SshProfile;
    }

    private static async promptForAuth(selectedConfig: sshConfigExt | undefined): Promise<sshConfigExt | undefined> {
        let qpItems: vscode.QuickPickItem[] = [
            { label: "$(lock) Password" },
            { label: "$(key) Private Key" },
            { label: "$(key) Private Key with Passphrase" }
            // { label: "$(lock) Password", description: "Authenticate using a password" },
            // { label: "$(key) Private Key", description: "Authenticate using a private key" },
            // { label: "$(key) Private Key with Passphrase", description: "Authenticate using a private key with passphrase" }
        ];

        //if PrivateKey/IdentityFile is passed, move password option to last priority
        //add description to show that the option was detected from config
        if(selectedConfig?.privateKey){
            qpItems.push(qpItems.shift()!);
            qpItems[0].description = " (detected from config)";
            qpItems[1].description = " (detected from config)";
        }
        qpItems.push({ label: "$(x) Skip Authentication"})

        const selectedOption = await vscode.window.showQuickPick(qpItems, { title: "Select an authentication method", ignoreFocusOut: true});

        // Priority for default Private Key URI
        // 1. IdentityFile/PrivateKey if passed from config
        // 2. .ssh directory within homedir()
        // 3. homedir()

        let defaultPrivateKeyUri = vscode.Uri.file(selectedConfig?.privateKey
            ? selectedConfig.privateKey
            : fs.existsSync(path.join(homedir(), '.ssh'))
            ? path.join(homedir(), '.ssh')
            : homedir());

        let privateKeyUri;
        if (selectedOption) {
            switch (selectedOption.label) {
                case "$(lock) Password":
                    selectedConfig!.password = await vscode.window.showInputBox({
                        title: "Enter Password",
                        password: true,
                        placeHolder: "Enter your password",
                        ignoreFocusOut: true,
                    });
                    break;

                case "$(key) Private Key":
                    privateKeyUri = await vscode.window.showOpenDialog({
                        canSelectFiles: true,
                        canSelectMany: false,
                        title: "Select Private Key File",
                        defaultUri: defaultPrivateKeyUri,
                    });
                    if (privateKeyUri && privateKeyUri.length > 0) {
                        selectedConfig!.privateKey = privateKeyUri[0].fsPath;
                    }
                    break;

                case "$(key) Private Key with Passphrase":
                    privateKeyUri = await vscode.window.showOpenDialog({
                        canSelectFiles: true,
                        canSelectMany: false,
                        title: "Select Private Key File",
                        defaultUri: defaultPrivateKeyUri
                    });
                    if (privateKeyUri && privateKeyUri.length > 0) {
                        selectedConfig!.privateKey = privateKeyUri[0].fsPath;
                        selectedConfig!.keyPassphrase = await vscode.window.showInputBox({
                            title: "Enter Passphrase",
                            password: true,
                            placeHolder: "Enter passphrase for private key",
                            ignoreFocusOut: true
                        });
                    }
                    break;
                default:
                    break;
            }

            //Profile information
            const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
            const profCache = zoweExplorerApi.getExplorerExtenderApi().getProfilesCache();
            const profInfo = await profCache.getProfileInfo();
            const configApi = profInfo.getTeamConfig().api;

            if (selectedConfig?.name) {
                // Create the base config object
                const config = {
                    type: "ssh",
                    properties: {
                        user: selectedConfig?.user,
                        host: selectedConfig?.hostname,
                        privateKey: selectedConfig?.privateKey,
                        port: selectedConfig?.port || 22,
                        keyPassphrase: selectedConfig?.keyPassphrase,
                        password: selectedConfig?.password
                    },
                    secure: []
                };

                //if password or KP is defined, make them secure
                if (selectedConfig?.password) config.secure.push("password" as never);
                if (selectedConfig?.keyPassphrase) config.secure.push("keyPassphrase" as never);

                configApi.profiles.set(selectedConfig.name, config);
                await profInfo.getTeamConfig().save();
            }

            return selectedConfig;
        }
    }

    private static async getNewProfileName(selectedProfile: sshConfigExt): Promise<sshConfigExt | undefined> {
        const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
        const profCache = zoweExplorerApi.getExplorerExtenderApi().getProfilesCache();
        const profiles = await profCache.fetchAllProfiles();
        let isUniqueName = false;
        while (!isUniqueName) {
            selectedProfile.name = await vscode.window.showInputBox({
                prompt: "Enter a name for the SSH config",
                value: selectedProfile.name!.replace(/\./g, "_"),
                validateInput: (input) => input.includes(".") ? "Name cannot contain '.'" : null
            });
            const existingProfile = profiles.find(profile => profile.name === selectedProfile.name);
            if (existingProfile) {
                const overwriteResponse = await vscode.window.showQuickPick(
                    ["Yes", "No"],
                    {
                        placeHolder: `A profile with the name "${selectedProfile.name}" already exists. Do you want to overwrite it?`
                    }
                );
                if (overwriteResponse === "Yes") {
                    isUniqueName = true;
                } else {}
            } else {
                isUniqueName = true;
            }
        }
        return selectedProfile
    }
}
