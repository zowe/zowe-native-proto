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
import { SshConfigManager } from "./SshConfigManager";
import { SshJesApi, SshMvsApi, SshUssApi } from "./api";

// This method is called when your extension is activated
export function activate(context: vscode.ExtensionContext) {
    const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
    if (zoweExplorerApi == null) {
        vscode.window.showErrorMessage(
            "Could not access Zowe Explorer API. Please check that the latest version of Zowe Explorer is installed.",
        );
        return;
    }

    const connectCmd = vscode.commands.registerCommand("zowe-native-proto-vsce.connect", async () => {
        const profile = await SshConfigManager.inst.promptForProfile();
        if (!profile) return;
        const serverPath = SshClientCache.getServerPath(profile.profile!.host);
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
        SshConfigManager.inst.showSessionInTree(profile.name!, true);
        await Gui.showMessage(`Installed Zowe SSH server on ${profile.name}`);
    });
    const restartCmd = vscode.commands.registerCommand("zowe-native-proto-vsce.restart", async () => {
        const profile = await SshConfigManager.inst.promptForProfile();
        if (!profile) return;
        const serverPath = SshClientCache.getServerPath(profile.profile!.host);
        const client = await SshClientCache.inst.connect(ZSshUtils.buildSession(profile.profile!));
        client.stop();
        client.start(serverPath);
        const statusMsg = Gui.setStatusBarMessage("Restarted Zowe SSH server");
        setTimeout(() => statusMsg.dispose(), 5000);
    });
    const uninstallCmd = vscode.commands.registerCommand("zowe-native-proto-vsce.uninstall", async () => {
        const profile = await SshConfigManager.inst.promptForProfile();
        if (!profile) return;
        const serverPath = SshClientCache.getServerPath(profile.profile!.host);
        const localDir = path.join(context.extensionPath, "bin");
        await ZSshUtils.uninstallServer(ZSshUtils.buildSession(profile.profile!), serverPath, localDir);
        SshConfigManager.inst.showSessionInTree(profile.name!, false);
        await Gui.showMessage(`Uninstalled Zowe SSH server from ${profile.name}`);
    });
    context.subscriptions.push(connectCmd, restartCmd, uninstallCmd);

    context.subscriptions.push(SshClientCache.inst);
    zoweExplorerApi.registerMvsApi(new SshMvsApi());
    zoweExplorerApi.registerUssApi(new SshUssApi());
    zoweExplorerApi.registerJesApi(new SshJesApi());
    zoweExplorerApi.getExplorerExtenderApi().reloadProfiles("ssh");
}

// This method is called when your extension is deactivated
export function deactivate() {}
