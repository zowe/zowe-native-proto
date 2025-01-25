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
import type * as vscode from "vscode";

export class SshConfigManager {
    private static mInstance: SshConfigManager;

    public static get inst(): SshConfigManager {
        SshConfigManager.mInstance ??= new SshConfigManager();
        return SshConfigManager.mInstance;
    }

    public async createTeamConfig(): Promise<boolean> {
        throw new Error("Not yet implemented");
    }

    public async promptForProfile(): Promise<imperative.IProfileLoaded | undefined> {
        const profCache = ZoweVsCodeExtension.getZoweExplorerApi().getExplorerExtenderApi().getProfilesCache();
        const sshProfiles = (await profCache.fetchAllProfilesByType("ssh")).filter(
            ({ name, profile }) => name != null && profile?.host != null,
        );
        const qpItems: vscode.QuickPickItem[] = [
            ...sshProfiles.map((prof) => ({ label: prof.name!, description: prof.profile?.host })),
            { label: "$(plus) Add New SSH Host..." },
        ];
        const result = await Gui.showQuickPick(qpItems, { title: "Choose an SSH host" });
        if (result === qpItems[qpItems.length - 1]) {
            if (await this.createTeamConfig()) {
                return profCache.getDefaultProfile("ssh");
            }
        } else if (result != null) {
            return sshProfiles.find((prof) => prof.name === result.label);
        }
    }

    public showSessionInTree(profileName: string, visible: boolean): void {
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
}
