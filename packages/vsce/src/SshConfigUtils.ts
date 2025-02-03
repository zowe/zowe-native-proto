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
            ({ name, profile }) => name != null && profile?.host != null,
        );
        const qpItems: vscode.QuickPickItem[] = [
            ...sshProfiles.map((prof) => ({ label: prof.name!, description: prof.profile?.host })),
            { label: "$(plus) Add New SSH Host..." },
        ];
        const result = await Gui.showQuickPick(qpItems, { title: "Choose an SSH host" });
        if (result === qpItems[qpItems.length - 1]) {
            if (await SshConfigUtils.createTeamConfig()) {
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
        throw new Error("Not yet implemented");
    }
}
