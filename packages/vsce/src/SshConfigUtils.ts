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

import * as fs from "node:fs";
import { homedir } from "node:os";
import * as path from "node:path";
import { Gui, ZoweVsCodeExtension, type imperative } from "@zowe/zowe-explorer-api";
import * as vscode from "vscode";
import { ZSshClient } from "zowe-native-proto-sdk";

// biome-ignore lint/complexity/noStaticOnlyClass: Utilities class has static methods
export class SshConfigUtils {
    public static getServerPath(hostname: string): string {
        const serverPathMap = vscode.workspace
            .getConfiguration("zowe-native-proto-vsce")
            .get<Record<string, string>>("serverPath");
        return serverPathMap?.[hostname] ?? ZSshClient.DEFAULT_SERVER_PATH;
    }

    public static async promptForProfile(profileName?: string): Promise<imperative.IProfileLoaded | undefined> {
        const profCache = ZoweVsCodeExtension.getZoweExplorerApi().getExplorerExtenderApi().getProfilesCache();
        if (profileName != null) {
            return profCache.getLoadedProfConfig(profileName, "ssh");
        }
        const sshProfiles = (await profCache.fetchAllProfilesByType("ssh")).filter(
            ({ name, profile }) => name != null && profile?.host,
        );
        const qpItems: vscode.QuickPickItem[] = [
            ...sshProfiles.map((prof) => ({ label: prof.name!, description: prof.profile?.host })),
            { label: "$(plus) Add New SSH Host..." },
        ];
        const result = await Gui.showQuickPick(qpItems, { title: "Choose an SSH host" });
        if (result === qpItems[qpItems.length - 1]) {
            if (await SshConfigUtils.createTeamConfig()) {
                await profCache.refresh();
                return profCache.getDefaultProfile("ssh");
            }
        } else if (result != null) {
            return sshProfiles.find((prof) => prof.name === result.label);
        }
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

    private static async createTeamConfig(): Promise<boolean> {
        let user: string;
        let host: string;
        let privateKey: string | undefined;
        let port: number | undefined;
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
            return false;
        }
        const sshRegex = /^ssh\s+([a-zA-Z0-9_-]+)@([a-zA-Z0-9.-]+)/;
        const flagRegex = /--(\w+)(?:\s+("[^"]+"|'[^']+'|\S+))?/g;

        const sshMatch = sshResponse.match(sshRegex);
        if (!sshMatch) {
            vscode.window.showErrorMessage("Invalid SSH command format. Ensure it matches the expected pattern.");
            return false;
        }
        user = sshMatch[1];
        host = sshMatch[2];
        const parsedCommand: { [key: string]: string | undefined | number } = { user, host };

        let flagMatch;
        while ((flagMatch = flagRegex.exec(sshResponse)) !== null) {
            const [, flag, value] = flagMatch;
            const normalizedFlag = flag.toLowerCase();

            // Check for missing value
            if (!value) {
                vscode.window.showErrorMessage(`Missing value for flag --${flag}.`);
                return false;
            }

            const unquotedValue = value.replace(/^["']|["']$/g, ""); // Remove surrounding quotes

            // Map aliases to consistent keys
            if (normalizedFlag === "p" || normalizedFlag === "port") {
                const portNumber = Number.parseInt(unquotedValue, 10);
                if (Number.isNaN(portNumber)) {
                    vscode.window.showErrorMessage(`Invalid value for flag --${flag}. Port must be a valid number.`);
                    return false;
                }
                parsedCommand.port = portNumber;
            } else if (normalizedFlag === "pk" || normalizedFlag === "privateKey") {
                parsedCommand.privateKey = unquotedValue;
            }

            // Validate if quotes are required
            if (/\s/.test(unquotedValue) && !/^["'].*["']$/.test(value)) {
                vscode.window.showErrorMessage(`Invalid value for flag --${flag}. Values with spaces must be quoted.`);
                return false;
            }
        }

        // Output the parsed result for debugging purposes
        console.log(`Parsed SSH Command: ${JSON.stringify(parsedCommand)}`);

        // Example of using the parsed data
        port = parsedCommand.port as number | undefined;

        privateKeyPath =
            typeof parsedCommand.privateKey === "number"
                ? parsedCommand.privateKey.toString()
                : parsedCommand.privateKey;

        console.log(
            `User: ${user}, Host: ${host}, Port: ${port || "Not Provided"}, Private Key: ${privateKey || "Not Provided"}`,
        );
        if (!privateKey) {
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
                if (privateKey == null) {
                    throw Error("Failed to discover an ssh private key inside `~/.ssh`.");
                }
            }
        } catch (err) {
            //do nuthin
        }
        let quickpickDefault = true;
        await profInfo.readProfilesFromDisk();
        if (profInfo.getAllProfiles("ssh").length > 0 && configExists) quickpickDefault = false;

        const nameResponse = await vscode.window.showInputBox({
            prompt: "Enter profile name",
            value: quickpickDefault ? "ssh" : undefined,
            placeHolder: quickpickDefault ? undefined : "Enter a profile name (E.g. ssh2)",
        });
        const configApi = profInfo.getTeamConfig().api;
        if (nameResponse && !nameResponse.includes(".")) {
            // await profInfo.addProfileToConfig("ssh");
            configApi.profiles.set(nameResponse!, {
                type: "ssh",
                properties: {
                    user: user,
                    host: host,
                    privateKey: privateKeyPath,
                    port: port || 22,
                },
            });
        } else {
            vscode.window.showErrorMessage("Invalid profile name");
            return false;
        }

        if (configApi.profiles.defaultGet("ssh") === null) configApi.profiles.defaultSet("ssh", nameResponse!);

        await profInfo.getTeamConfig().save();
        return true;
    }
}
