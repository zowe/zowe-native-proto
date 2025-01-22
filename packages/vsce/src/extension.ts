/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0 & Apache-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
import * as vscode from "vscode";
import { ZoweVsCodeExtension } from "@zowe/zowe-explorer-api";
import { SshClientCache } from "./SshClientCache";
import { SshJesApi, SshMvsApi, SshUssApi } from "./api";

// This method is called when your extension is activated
export function activate(context: vscode.ExtensionContext) {
    const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
    if (zoweExplorerApi != null) {
        context.subscriptions.push(SshClientCache.inst);
        zoweExplorerApi.registerMvsApi(new SshMvsApi());
        zoweExplorerApi.registerUssApi(new SshUssApi());
        zoweExplorerApi.registerJesApi(new SshJesApi());
    } else {
        vscode.window.showErrorMessage(
            "Could not access Zowe Explorer API. Please check that the latest version of Zowe Explorer is installed.",
        );
    }
}

// This method is called when your extension is deactivated
export function deactivate() {}
