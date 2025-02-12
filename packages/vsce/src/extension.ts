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

import * as path from "node:path";
import { Gui, ZoweVsCodeExtension } from "@zowe/zowe-explorer-api";
import * as vscode from "vscode";
import { ZSshUtils } from "zowe-native-proto-sdk";
import { SshClientCache } from "./SshClientCache";
import { SshConfigUtils } from "./SshConfigUtils";
import { SshJesApi, SshMvsApi, SshUssApi } from "./api";

function registerCommands(context: vscode.ExtensionContext): vscode.Disposable[] {
    return [
        vscode.commands.registerCommand("zowe-native-proto-vsce.connect", async (profName?: string) => {
            const profile = await SshConfigUtils.promptForProfile(profName);
            if (!profile) return;
            console.debug();
            const serverPath = SshConfigUtils.getServerPath(profile.profile!.host);
            console.debug();
            const localDir = path.join(context.extensionPath, "bin");
            await Gui.withProgress(
                {
                    location: vscode.ProgressLocation.Notification,
                    title: "Deploying Zowe SSH server...",
                },
                () => {
                    return ZSshUtils.installServer(ZSshUtils.buildSession(profile.profile!), serverPath, localDir);
                },
            );
            SshConfigUtils.showSessionInTree(profile.name!, true);
            await Gui.showMessage(`Installed Zowe SSH server on ${profile.name}`);
        }),
        vscode.commands.registerCommand("zowe-native-proto-vsce.restart", async (profName?: string) => {
            const profile = await SshConfigUtils.promptForProfile(profName);
            if (!profile) return;
            console.debug();
            await SshClientCache.inst.connect(ZSshUtils.buildSession(profile.profile!), true);
            console.debug();
            const statusMsg = Gui.setStatusBarMessage("Restarted Zowe SSH server");
            setTimeout(() => statusMsg.dispose(), 5000);
        }),
        vscode.commands.registerCommand("zowe-native-proto-vsce.uninstall", async (profName?: string) => {
            const profile = await SshConfigUtils.promptForProfile(profName);
            if (!profile) return;
            SshClientCache.inst.end(profile.profile!.host);
            const serverPath = SshConfigUtils.getServerPath(profile.profile!.host);
            await ZSshUtils.uninstallServer(ZSshUtils.buildSession(profile.profile!), serverPath);
            SshConfigUtils.showSessionInTree(profile.name!, false);
            await Gui.showMessage(`Uninstalled Zowe SSH server from ${profile.name}`);
        }),
    ];
}

// This method is called when your extension is activated
export function activate(context: vscode.ExtensionContext) {
    const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
    if (zoweExplorerApi == null) {
        vscode.window.showErrorMessage(
            "Could not access Zowe Explorer API. Please check that the latest version of Zowe Explorer is installed.",
        );
        return;
    }

    context.subscriptions.push(...registerCommands(context));
    context.subscriptions.push(SshClientCache.inst);
    zoweExplorerApi.registerMvsApi(new SshMvsApi());
    zoweExplorerApi.registerUssApi(new SshUssApi());
    zoweExplorerApi.registerJesApi(new SshJesApi());
    zoweExplorerApi.getExplorerExtenderApi().reloadProfiles("ssh");
}

// This method is called when your extension is deactivated
export function deactivate() {}
