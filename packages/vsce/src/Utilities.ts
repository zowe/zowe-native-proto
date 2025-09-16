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
import type { SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { Gui, imperative, ZoweVsCodeExtension } from "@zowe/zowe-explorer-api";
import * as vscode from "vscode";
import { ZSshUtils } from "zowe-native-proto-sdk";
import { SshClientCache } from "./SshClientCache";
import { SshConfigUtils, VscePromptApi } from "./SshConfigUtils";

const EXTENSION_NAME = "zowe-native-proto-vsce";

export function deployWithProgress(session: SshSession, serverPath: string, localDir: string): Thenable<void> {
    return Gui.withProgress(
        {
            location: vscode.ProgressLocation.Notification,
            title: "Deploying Zowe SSH server...",
        },
        async (progress) => {
            // Pass a callback function that will update the progress object
            await ZSshUtils.installServer(session, serverPath, localDir, (progressIncrement) => {
                progress.report({ increment: progressIncrement });
            });
        },
    );
}

export function getVsceConfig(): vscode.WorkspaceConfiguration {
    return vscode.workspace.getConfiguration(EXTENSION_NAME);
}

export function initLogger(context: vscode.ExtensionContext): void {
    const loggerConfigPath = path.join(context.extensionPath, "log4jsconfig.json");
    const loggerConfig = JSON.parse(fs.readFileSync(loggerConfigPath, "utf-8"));
    const logLevel = getVsceConfig().get<string>("logLevel");

    for (const appenderName of Object.keys(loggerConfig.log4jsConfig.appenders)) {
        loggerConfig.log4jsConfig.appenders[appenderName].filename = path.join(
            context.logUri.fsPath,
            path.basename(loggerConfig.log4jsConfig.appenders[appenderName].filename),
        );
        if (logLevel != null) {
            loggerConfig.log4jsConfig.categories[appenderName].level = logLevel;
        }
    }

    imperative.Logger.initLogger(loggerConfig);
    imperative.Logger.getAppLogger().debug("Initialized logger for VSCode extension");
}

export function registerCommands(context: vscode.ExtensionContext): vscode.Disposable[] {
    const profCache = ZoweVsCodeExtension.getZoweExplorerApi().getExplorerExtenderApi().getProfilesCache();
    return [
        vscode.commands.registerCommand(`${EXTENSION_NAME}.connect`, async (profName?: string) => {
            imperative.Logger.getAppLogger().trace("Running connect command for profile %s", profName);
            const vscePromptApi = new VscePromptApi(await profCache.getProfileInfo());
            const profile = await vscePromptApi.promptForProfile(profName);
            if (!profile?.profile) return;
            const defaultServerPath = SshConfigUtils.getServerPath(profile.profile);
            const deployDirectory = await vscePromptApi.promptForDeployDirectory(defaultServerPath);

            const sshSession = ZSshUtils.buildSession(profile.profile);
            const localDir = path.join(context.extensionPath, "bin");
            await deployWithProgress(sshSession, deployDirectory, localDir);

            await SshConfigUtils.showSessionInTree(profile.name!, true);
            const infoMsg = `Installed Zowe SSH server on ${profile.profile.host ?? profile.name}`;
            imperative.Logger.getAppLogger().info(infoMsg);
            await Gui.showMessage(infoMsg);
        }),
        vscode.commands.registerCommand(`${EXTENSION_NAME}.restart`, async (profName?: string) => {
            imperative.Logger.getAppLogger().trace("Running restart command for profile %s", profName);
            const vscePromptApi = new VscePromptApi(await profCache.getProfileInfo());
            const profile = await vscePromptApi.promptForProfile(profName);
            if (!profile?.profile) return;

            await SshClientCache.inst.connect(profile, true);

            imperative.Logger.getAppLogger().info(
                `Restarted Zowe SSH server on ${profile.profile?.host ?? profile.name}`,
            );
            const statusMsg = Gui.setStatusBarMessage("Restarted Zowe SSH server");
            setTimeout(() => statusMsg.dispose(), 5000);
        }),
        vscode.commands.registerCommand(`${EXTENSION_NAME}.showLog`, async () => {
            imperative.Logger.getAppLogger().trace("Running showLog command");
            await vscode.commands.executeCommand(
                "vscode.open",
                vscode.Uri.file(path.join(context.logUri.fsPath, "zowe-native-proto.log")),
            );
            await vscode.commands.executeCommand("workbench.action.files.setActiveEditorReadonlyInSession");
        }),
        vscode.commands.registerCommand(`${EXTENSION_NAME}.uninstall`, async (profName?: string) => {
            imperative.Logger.getAppLogger().trace("Running uninstall command for profile %s", profName);
            const vscePromptApi = new VscePromptApi(await profCache.getProfileInfo());
            const profile = await vscePromptApi.promptForProfile(profName);
            if (!profile?.profile) return;

            SshClientCache.inst.end(profile.profile);
            const serverPath = SshConfigUtils.getServerPath(profile.profile);
            await SshConfigUtils.showSessionInTree(profile.name!, false);
            await ZSshUtils.uninstallServer(ZSshUtils.buildSession(profile.profile), serverPath);

            const infoMsg = `Uninstalled Zowe SSH server from ${profile.profile.host ?? profile.name}`;
            imperative.Logger.getAppLogger().info(infoMsg);
            await Gui.showMessage(infoMsg);
        }),
    ];
}
