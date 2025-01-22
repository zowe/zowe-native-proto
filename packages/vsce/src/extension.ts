import * as vscode from "vscode";
import { ZoweVsCodeExtension } from "@zowe/zowe-explorer-api";
import { SshJesApi, SshMvsApi, SshUssApi } from "./api";
import { SshClientCache } from "./SshClientCache";

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
