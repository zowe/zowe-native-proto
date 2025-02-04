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

import type { SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import * as vscode from "vscode";
import { ZSshClient } from "zowe-native-proto-sdk";
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

    public async connect(session: SshSession, restart = false): Promise<ZSshClient> {
        const clientKey = session.ISshSession.hostname!;
        if (restart) {
            this.end(clientKey);
        }
        if (!this.mClientMap.has(clientKey)) {
            const serverPath = SshConfigUtils.getServerPath(clientKey);
            this.mClientMap.set(
                clientKey,
                await ZSshClient.create(session, serverPath, (session) => {
                    this.end(session.ISshSession.hostname!);
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
