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
import { join } from 'path';
import { ISshSession } from "@zowe/zos-uss-for-zowe-sdk";

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
        const profInfo = await profCache.getProfileInfo();

        if (profileName != null) {
            return profCache.getLoadedProfConfig(profileName, "ssh");
        }

        const sshProfiles = (await profCache.fetchAllProfilesByType("ssh"))
            .filter(({ name, profile }) => name != null && profile?.host);
        const migratedConfigs = await SshConfigUtils.migrateSshConfig();

        const qpItems: vscode.QuickPickItem[] = [
            { label: "$(plus) Add New SSH Host..." },
            ...sshProfiles.map((prof) => ({ label: prof.name!, description: prof.profile?.host })),
            { label: "Merge From SSH Config", kind: vscode.QuickPickItemKind.Separator },
            ...migratedConfigs.map((config) => ({ label: config.name!, description: config.hostname }))
        ];

        const result = await Gui.showQuickPick(qpItems, { title: "Choose an SSH host" });

        if (result === qpItems[0]) {
            if (await SshConfigUtils.createTeamConfig()) {
                return profCache.getDefaultProfile("ssh");
            }
        } else if (migratedConfigs.some((config) => result?.label === config.name!)) {
            const selectedConfig = migratedConfigs.find((config) => result!.label === config.name!);
            if (selectedConfig) {
                const newName = await vscode.window.showInputBox({
                    prompt: "Enter a name for the SSH config",
                    value: selectedConfig.name!.replace(/\./g, "_"),
                    validateInput: (input) => input.includes(".") ? "Name cannot contain '.'" : null
                });

                if (newName) {
                    const configApi = profInfo.getTeamConfig().api;
                    configApi.profiles.set(newName, {
                        type: "ssh",
                        properties: {
                            user: selectedConfig.user,
                            host: selectedConfig.hostname,
                            privateKey: selectedConfig.privateKey,
                            port: selectedConfig.port || 22,
                            keyPassphrase: selectedConfig.keyPassphrase
                        }
                    });
                    vscode.window.showInformationMessage(`SSH config '${newName}' saved successfully.`);
                }
            }
        } else if (result != null) {
            return sshProfiles.find((prof) => prof.name === result.label);
        }
    }

    private static async migrateSshConfig(): Promise<sshConfigExt[]> {
        const filePath = join(homedir(), '.ssh', 'config');
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
                                    session.port = value;
                                    break;
                                case 'user':
                                    session.user = value;
                                    break;
                                case 'password':
                                    session.password = value;
                                    break;
                                case 'keypassphrase':
                                    session.keyPassphrase = value;
                                    break;
                                case 'privatekey':
                                    session.privateKey = value;
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

    private static async createTeamConfig(): Promise<object[]> {
        throw new Error("Not yet implemented");
    }
}
