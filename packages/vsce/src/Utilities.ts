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
import * as path from "node:path";
import { Gui, imperative } from "@zowe/zowe-explorer-api";
import * as vscode from "vscode";
import { ZSshUtils } from "zowe-native-proto-sdk";
import { SshClientCache } from "./SshClientCache";
import { SshConfigUtils } from "./SshConfigUtils";

export function initLogger(context: vscode.ExtensionContext): void {
    const loggerConfigPath = path.join(context.extensionPath, "log4jsconfig.json");
    const loggerConfig = JSON.parse(fs.readFileSync(loggerConfigPath, "utf-8"));
    const logLevel = vscode.workspace.getConfiguration("zowe-native-proto-vsce").get<string>("logLevel");

    for (const appenderName of Object.keys(loggerConfig.log4jsConfig.appenders)) {
        loggerConfig.log4jsConfig.appenders[appenderName].filename = path.join(
            context.extensionPath,
            loggerConfig.log4jsConfig.appenders[appenderName].filename,
        );
        if (logLevel != null) {
            loggerConfig.log4jsConfig.categories[appenderName].level = logLevel;
        }
    }

    console.log(loggerConfig);
    imperative.Logger.initLogger(loggerConfig);
    imperative.Logger.getAppLogger().debug("Initialized logger for VSCode extension");
}

export function registerCommands(context: vscode.ExtensionContext): vscode.Disposable[] {
    return [
        vscode.commands.registerCommand("zowe-native-proto-vsce.connect", async (profName?: string) => {
            imperative.Logger.getAppLogger().trace("Running connect command for profile %s", profName);
            const profile = await SshConfigUtils.promptForProfile(profName);
            if (!profile?.profile) return;

            const sshSession = ZSshUtils.buildSession(profile.profile);
            const serverPath = SshConfigUtils.getServerPath(profile.profile);
            const localDir = path.join(context.extensionPath, "bin");
            await Gui.withProgress(
                {
                    location: vscode.ProgressLocation.Notification,
                    title: "Deploying Zowe SSH server...",
                },
                () => ZSshUtils.installServer(sshSession, serverPath, localDir),
            );

            SshConfigUtils.showSessionInTree(profile.name!, true);
            const infoMsg = `Installed Zowe SSH server on ${profile.profile?.host ?? profile.name}`;
            imperative.Logger.getAppLogger().info(infoMsg);
            await Gui.showMessage(infoMsg);
        }),
        vscode.commands.registerCommand("zowe-native-proto-vsce.restart", async (profName?: string) => {
            imperative.Logger.getAppLogger().trace("Running restart command for profile %s", profName);
            const profile = await SshConfigUtils.promptForProfile(profName);
            if (!profile?.profile) return;

            await SshClientCache.inst.connect(profile, true);

            imperative.Logger.getAppLogger().info(
                `Restarted Zowe SSH server on ${profile.profile?.host ?? profile.name}`,
            );
            const statusMsg = Gui.setStatusBarMessage("Restarted Zowe SSH server");
            setTimeout(() => statusMsg.dispose(), 5000);
        }),
        vscode.commands.registerCommand("zowe-native-proto-vsce.uninstall", async (profName?: string) => {
            imperative.Logger.getAppLogger().trace("Running uninstall command for profile %s", profName);
            const profile = await SshConfigUtils.promptForProfile(profName);
            if (!profile?.profile) return;

            SshClientCache.inst.end(profile.profile);
            const serverPath = SshConfigUtils.getServerPath(profile.profile);
            await ZSshUtils.uninstallServer(ZSshUtils.buildSession(profile.profile), serverPath);

            SshConfigUtils.showSessionInTree(profile.name!, false);
            const infoMsg = `Uninstalled Zowe SSH server from ${profile.profile.host ?? profile.name}`;
            imperative.Logger.getAppLogger().info(infoMsg);
            await Gui.showMessage(infoMsg);
        }),
    ];
}
