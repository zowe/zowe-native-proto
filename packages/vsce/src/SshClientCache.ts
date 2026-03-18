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
import { imperative } from "@zowe/zowe-explorer-api";
import * as vscode from "vscode";
import { type ClientOptions, ZSshClient, ZSshUtils } from "zowe-native-proto-sdk";
import { ConfigUtils } from "./ConfigUtils";
import { deployWithProgress, getVsceConfig } from "./Utilities";

class AsyncMutex extends imperative.DeferredPromise<void> implements Disposable {
    constructor(private onDispose?: () => void) {
        super();
    }

    public [Symbol.dispose](): void {
        this.resolve();
        this.onDispose?.();
    }
}

type ZSshClientSessions = {
    client: ZSshClient;
    profile: imperative.IProfileLoaded;
    status: ServerStatus;
    startTime: number;
    requestTimeout: number;
};

enum ServerStatus {
    UP,
    DOWN,
}

export class SshClientCache extends vscode.Disposable {
    private static mInstance: SshClientCache;
    private mClientSessionMap: Map<string, ZSshClientSessions> = new Map();
    private mMutexMap: Map<string, AsyncMutex> = new Map();
    private static readonly ERROR_SNIPPETS = {
        FATAL: ["CEE5207E", "CEE3204S", "at compile unit offset", "Fatal error encountered in zowex"],
        TIMEOUT: ["Request timed out"],
    };
    private static readonly ACTIONS = {
        RELOAD: "Reload ZRS",
        CLOSE: "Close",
    };

    private constructor() {
        super(() => this.dispose());
    }

    public dispose(): void {
        for (const session of this.mClientSessionMap.values()) {
            session.client.dispose();
        }
    }

    public static get inst(): SshClientCache {
        if (SshClientCache.mInstance == null) {
            SshClientCache.mInstance = new SshClientCache();
        }
        return SshClientCache.mInstance;
    }

    public async connect(profile: imperative.IProfileLoaded, restart = false): Promise<ZSshClient> {
        const clientId = this.getClientId(profile);
        await this.mMutexMap.get(clientId)?.promise;
        if (restart) {
            this.end(clientId);
        }

        if (!this.mClientSessionMap.has(clientId)) {
            using _lock = this.acquireProfileLock(clientId);
            const session = ZSshUtils.buildSession(profile.profile!);
            const serverPath = ConfigUtils.getServerPath(profile.profile);
            const vsceConfig = getVsceConfig();
            const keepAliveInterval = vsceConfig.get<number>("keepAliveInterval");
            const numWorkers = vsceConfig.get<number>("workerCount");
            const requestTimeout = vsceConfig.get<number>("requestTimeout");
            const autoUpdate = vsceConfig.get("serverAutoUpdate", true);

            let newClient: ZSshClient | undefined;
            try {
                newClient = await this.buildClient(session, clientId, {
                    serverPath,
                    keepAliveInterval,
                    numWorkers,
                    requestTimeout,
                });
                imperative.Logger.getAppLogger().debug(
                    `Server checksums: ${JSON.stringify(newClient.serverChecksums)}`,
                );
                if (await ZSshUtils.checkIfOutdated(newClient.serverChecksums)) {
                    if (autoUpdate) {
                        imperative.Logger.getAppLogger().info(`Server is out of date, deploying to ${profile.name}`);
                        newClient = undefined;
                    } else {
                        imperative.Logger.getAppLogger().warn(
                            `Server is out of date, skipping update for ${profile.name}`,
                        );
                    }
                }
            } catch (err) {
                if (err instanceof imperative.ImperativeError && err.errorCode === "ENOTFOUND") {
                    imperative.Logger.getAppLogger().info(`Server is missing, deploying to ${profile.name}`);
                } else {
                    throw err;
                }
            }
            if (newClient == null) {
                await deployWithProgress(session, serverPath);
                newClient = await this.buildClient(session, clientId, {
                    serverPath,
                    keepAliveInterval,
                    numWorkers,
                    requestTimeout,
                });
            }
            this.mClientSessionMap.set(clientId, {
                client: newClient,
                profile: profile,
                status: ServerStatus.UP,
                startTime: Date.now(),
                requestTimeout: 60e3, // TODO: can we sync this with client impl?
            });
        }

        return this.mClientSessionMap.get(clientId)?.client as ZSshClient;
    }

    public end(hostOrProfile: string | imperative.IProfileLoaded): void {
        const clientId = typeof hostOrProfile === "string" ? hostOrProfile : this.getClientId(hostOrProfile);
        this.mClientSessionMap.get(clientId)?.client.dispose();
        this.mClientSessionMap.delete(clientId);
    }

    private async reloadClient(clientId: string): Promise<void> {
        const clientStatus = this.mClientSessionMap.get(clientId);
        // TODO: can we reload the profile??
        this.connect(clientStatus!.profile, true);
    }

    private getClientId(profile: imperative.IProfileLoaded): string {
        return `${profile.name}_${profile.type}`;
    }

    private acquireProfileLock(clientId: string): AsyncMutex {
        const lock = new AsyncMutex(() => this.mMutexMap.delete(clientId));
        this.mMutexMap.set(clientId, lock);
        return lock;
    }

    private buildClient(session: SshSession, clientId: string, opts: ClientOptions): Promise<ZSshClient> {
        return ZSshClient.create(session, {
            ...opts,
            onClose: () => {
                this.end(clientId);
            },
            onError: (err: Error) => {
                this.handleClientError(clientId, err);
            },
        });
    }

    private handleClientError(clientId: string, err: Error): void {
        const errorMsg = err.toString();
        const clientSession = this.mClientSessionMap.get(clientId);

        // TODO: What does this case imply? No active session?
        if (clientSession == null) {
            return;
        }

        const isFatal = SshClientCache.ERROR_SNIPPETS.FATAL.some((item) => errorMsg.includes(item));
        const isTimeout = SshClientCache.ERROR_SNIPPETS.TIMEOUT.some((item) => errorMsg.includes(item));

        if (isFatal) {
            if (clientSession) {
                clientSession.status = ServerStatus.DOWN;
            }
            this.promptErrorAndReload(
                "Zowe Remote SSH has encountered an unrecoverable error. Reload the server component.",
                clientId,
            );
            return;
        }

        if (isTimeout) {
            // This came before the server started, implying a crash and reload, and so we should ignore the error.
            const isOldTimeout = Date.now() - clientSession.requestTimeout < clientSession.startTime;
            if (!isOldTimeout) {
                const isDown = clientSession?.status === ServerStatus.DOWN;
                const suffix = isDown
                    ? ', and the Zowe Remote SSH server is down. Click "Reload ZRS" to restart it.'
                    : ". If this continues, consider reloading the Zowe Remote SSH Server.";

                this.promptErrorAndReload(`Zowe Remote SSH request timed out${suffix}`, clientId);
            }

            return;
        }

        // Fallback for unclassified errors
        vscode.window.showErrorMessage(errorMsg);
    }

    private promptErrorAndReload(message: string, clientId: string): void {
        vscode.window
            .showErrorMessage(message, SshClientCache.ACTIONS.RELOAD, SshClientCache.ACTIONS.CLOSE)
            .then((selection) => {
                if (selection === SshClientCache.ACTIONS.RELOAD) {
                    this.reloadClient(clientId);
                }
            });
    }
}
