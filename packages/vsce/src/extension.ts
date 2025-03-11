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

import { ZoweVsCodeExtension, imperative } from "@zowe/zowe-explorer-api";
import * as vscode from "vscode";
import { SshClientCache } from "./SshClientCache";
import { initLogger, registerCommands } from "./Utilities";
import { SshCommandApi, SshJesApi, SshMvsApi, SshUssApi } from "./api";

// This method is called when your extension is activated
export function activate(context: vscode.ExtensionContext) {
    initLogger(context);
    const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
    if (zoweExplorerApi == null) {
        const errMsg =
            "Could not access Zowe Explorer API. Please check that the latest version of Zowe Explorer is installed.";
        imperative.Logger.getAppLogger().fatal(errMsg);
        vscode.window.showErrorMessage(errMsg);
        return;
    }

    context.subscriptions.push(...registerCommands(context));
    context.subscriptions.push(SshClientCache.inst);
    zoweExplorerApi.registerMvsApi(new SshMvsApi());
    zoweExplorerApi.registerUssApi(new SshUssApi());
    zoweExplorerApi.registerJesApi(new SshJesApi());
    zoweExplorerApi.registerCommandApi(new SshCommandApi());
    zoweExplorerApi.getExplorerExtenderApi().reloadProfiles("ssh");
}

// This method is called when your extension is deactivated
export function deactivate() {}
