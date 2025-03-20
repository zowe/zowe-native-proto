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
import type { SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { imperative } from "@zowe/zowe-explorer-api";
import * as vscode from "vscode";
import { ZSshClient, ZSshUtils } from "zowe-native-proto-sdk";
import { SshConfigUtils } from "./SshConfigUtils";

export class SshClientCache extends vscode.Disposable {
    private static mInstance: SshClientCache;
    private mClientMap: Map<string, ZSshClient> = new Map();

    private constructor(private mContext: vscode.ExtensionContext) {
        super(() => this.dispose());
    }

    public dispose(): void {
        for (const client of this.mClientMap.values()) {
            client.dispose();
        }
    }

    public static initialize(context: vscode.ExtensionContext): SshClientCache {
        SshClientCache.mInstance = new SshClientCache(context);
        return SshClientCache.mInstance;
    }

    public static get inst(): SshClientCache {
        return SshClientCache.mInstance;
    }

    public async connect(profile: imperative.IProfileLoaded, restart = false): Promise<ZSshClient> {
        const clientId = this.getClientId(profile.profile!);
        if (restart) {
            this.end(clientId);
        }

        if (!this.mClientMap.has(clientId)) {
            const session = ZSshUtils.buildSession(profile.profile!);
            const serverPath = SshConfigUtils.getServerPath(profile.profile);
            const localDir = path.join(this.mContext.extensionPath, "bin");

            if (
                vscode.workspace.getConfiguration("zowe-native-proto-vsce").get("serverAutoUpdate", true) &&
                (await ZSshUtils.checkIfOutdated(session, serverPath, path.join(localDir, "checksums.asc")))
            ) {
                // Server is out of date so re-deploy it
                await ZSshUtils.installServer(session, serverPath, localDir);
            }

            let newClient: ZSshClient;
            try {
                newClient = await this.buildClient(session, serverPath, clientId);
            } catch (err) {
                if (err instanceof imperative.ImperativeError && err.errorCode === "ENOTFOUND") {
                    // Server is missing so deploy it and try again
                    await ZSshUtils.installServer(session, serverPath, localDir);
                    newClient = await this.buildClient(session, serverPath, clientId);
                } else {
                    throw err;
                }
            }
            this.mClientMap.set(clientId, newClient);
        }

        return this.mClientMap.get(clientId) as ZSshClient;
    }

    public end(hostOrProfile: string | imperative.IProfile): void {
        const clientId = typeof hostOrProfile === "string" ? hostOrProfile : this.getClientId(hostOrProfile);
        this.mClientMap.get(clientId)?.dispose();
        this.mClientMap.delete(clientId);
    }

    private getClientId(profile: imperative.IProfile): string {
        return `${profile.host}:${profile.port ?? 22}`;
    }

    private buildClient(session: SshSession, serverPath: string, clientId: string): Promise<ZSshClient> {
        return ZSshClient.create(session, {
            serverPath,
            onClose: () => {
                this.end(clientId);
            },
            onError: (err: Error) => {
                vscode.window.showErrorMessage(err.toString());
            },
        });
    }
}
