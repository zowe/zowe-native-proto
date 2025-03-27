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
import * as vscode from "vscode";
import { type ISshConfigExt, ZClientUtils, ZSshClient } from "zowe-native-proto-sdk";
import {
    AbstractConfigManager,
    MESSAGE_TYPE,
    type inputBoxOpts,
    type ProgressCallback,
    type qpItem,
    type qpOpts,
} from "../../sdk/src/ZSshAuthUtils";

// biome-ignore lint/complexity/noStaticOnlyClass: Utilities class has static methods
export class SshConfigUtils {
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
}
export class VscePromptApi extends AbstractConfigManager {
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

    protected async withProgress<T>(message: string, task: (progress: ProgressCallback) => Promise<T>): Promise<T> {
        return await Gui.withProgress(
            {
                location: vscode.ProgressLocation.Notification,
                title: message,
                cancellable: false,
            },
            async (progress) => {
                return await task((percent) => progress.report({ increment: percent }));
            },
        );
    }
    protected async showQuickPick(opts: qpOpts): Promise<string | undefined> {
        const quickPick = vscode.window.createQuickPick();
        quickPick.items = opts.items;
        quickPick.title = opts.title;
        quickPick.placeholder = opts.placeholder;
        quickPick.ignoreFocusOut = opts.ignoreFocusOut ?? false;

        return await new Promise<string | undefined>((resolve) => {
            quickPick.onDidAccept(() => {
                resolve(quickPick.selectedItems[0]?.label);
                quickPick.hide();
            });
            quickPick.onDidHide(() => resolve(undefined)); // Handle case when user cancels
            quickPick.show();
        });
    }

    protected async showCustomQuickPick(opts: qpOpts): Promise<qpItem | undefined> {
        const quickPick = vscode.window.createQuickPick();
        quickPick.items = opts.items.map((item) => {
            if (item.separator) {
                return {
                    ...item,
                    kind: vscode.QuickPickItemKind.Separator,
                };
            }
            return item;
        });
        quickPick.title = opts.title;
        quickPick.placeholder = opts.placeholder;
        quickPick.ignoreFocusOut = opts.ignoreFocusOut ?? false;

        const customItem = {
            label: ">", // Using ">" as a visual cue for custom input
            description: "Custom SSH Host",
            alwaysShow: true,
        };

        quickPick.onDidChangeValue((value) => {
            if (value) {
                customItem.label = `> ${value}`;
                quickPick.items = [customItem, ...opts.items];
            } else {
                quickPick.items = opts.items;
            }
        });

        return new Promise<qpItem | undefined>((resolve) => {
            quickPick.onDidAccept(() => {
                const selection = quickPick.selectedItems[0];
                if (selection) {
                    if (selection.label.startsWith(">")) {
                        resolve({
                            label: selection.label.replace(">", "").trim(),
                            description: "Custom SSH Host",
                        });
                    } else {
                        resolve(selection);
                    }
                }
                quickPick.hide();
            });
            quickPick.onDidHide(() => resolve(undefined));
            quickPick.show();
        });
    }
}
