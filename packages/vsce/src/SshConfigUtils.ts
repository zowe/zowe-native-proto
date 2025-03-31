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

import { ProfileConstants } from "@zowe/core-for-zowe-sdk";
import { Gui, type IZoweTree, type IZoweTreeNode, ZoweVsCodeExtension, type imperative } from "@zowe/zowe-explorer-api";
import * as vscode from "vscode";
import {
    AbstractConfigManager,
    MESSAGE_TYPE,
    type ProgressCallback,
    ZSshClient,
    type inputBoxOpts,
    type qpItem,
    type qpOpts,
} from "zowe-native-proto-sdk";

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
        Object.assign(quickPick, {
            items: opts.items,
            title: opts.title,
            placeholder: opts.placeholder,
            ignoreFocusOut: true,
        });

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
        const quickPick = vscode.window.createQuickPick<vscode.QuickPickItem>();

        const mappedItems = opts.items.map((item) =>
            item.separator
                ? { label: item.label, kind: vscode.QuickPickItemKind.Separator }
                : { label: item.label, description: item.description },
        );

        quickPick.items = mappedItems;

        Object.assign(quickPick, {
            title: opts.title,
            placeholder: opts.placeholder,
            ignoreFocusOut: true,
        });

        const customItem = {
            label: ">",
            description: "Custom SSH Host",
        };

        quickPick.onDidChangeValue((value) => {
            if (value) {
                customItem.label = `> ${value}`;
                quickPick.items = [{ label: customItem.label, description: customItem.description }, ...mappedItems];
            } else {
                quickPick.items = mappedItems;
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
                        resolve({
                            label: selection.label,
                            description: selection.description,
                        });
                    }
                }
                quickPick.hide();
            });
            quickPick.onDidHide(() => resolve(undefined));
            quickPick.show();
        });
    }

    protected getCurrentDir(): string | undefined {
        return ZoweVsCodeExtension.workspaceRoot?.uri.fsPath;
    }

    protected getProfileType(): imperative.IProfileTypeConfiguration[] {
        const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
        const profCache = zoweExplorerApi.getExplorerExtenderApi().getProfilesCache();

        return [
            // biome-ignore lint/suspicious/noExplicitAny: Accessing protected method
            ...(profCache as any).getCoreProfileTypes(),
            ...profCache.getConfigArray(),
            ProfileConstants.BaseProfile,
        ];
    }
}
