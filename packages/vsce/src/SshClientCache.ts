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
        const clientKey = profile.profile!.host;
        if (restart) {
            this.end(clientKey);
        }
        if (!this.mClientMap.has(clientKey)) {
            const session = ZSshUtils.buildSession(profile.profile!);
            const serverPath = SshConfigUtils.getServerPath(profile);
            this.mClientMap.set(
                clientKey,
                await ZSshClient.create(session, serverPath, () => {
                    this.end(clientKey);
                }),
            );
        }
        return this.mClientMap.get(clientKey) as ZSshClient;
    }

    public end(hostname: string): void {
        this.mClientMap.get(hostname)?.dispose();
        this.mClientMap.delete(hostname);
    }
}
