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

import type { imperative } from "@zowe/zowe-explorer-api";
import * as vscode from "vscode";
import { ZSshClient, ZSshUtils } from "zowe-native-proto-sdk";
import { SshConfigUtils } from "./SshConfigUtils";

export class SshClientCache extends vscode.Disposable {
    private static mInstance: SshClientCache;
    private mClientMap: Map<string, ZSshClient> = new Map();

    private constructor() {
        super(() => this.dispose());
    }

    public dispose(): void {
        for (const client of this.mClientMap.values()) {
            client.dispose();
        }
    }

    public static get inst(): SshClientCache {
        SshClientCache.mInstance ??= new SshClientCache();
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
            this.mClientMap.set(
                clientId,
                await ZSshClient.create(session, {
                    serverPath,
                    onClose: () => {
                        this.end(clientId);
                    },
                    onError: (err: Error) => {
                        vscode.window.showErrorMessage(err.toString());
                    },
                }),
            );
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
}
