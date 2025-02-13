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

import { posix } from "node:path";
import type { Writable } from "node:stream";
import { DeferredPromise } from "@zowe/imperative";
import type { SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { Client, type ClientChannel } from "ssh2";
import { AbstractRpcClient } from "./AbstractRpcClient";
import { ZSshUtils } from "./ZSshUtils";
import type { IRpcRequest, IRpcResponse } from "./doc";

export class ZSshClient extends AbstractRpcClient implements Disposable {
    public static readonly DEFAULT_SERVER_PATH = "~/.zowe-server";

    private mSshClient: Client;
    private mSshStream: ClientChannel;
    private mResponse = "";
    private mResponseStream: Writable | undefined;
    private sshMutex: DeferredPromise<void> | undefined;

    private constructor() {
        super();
    }

    public static async create(
        session: SshSession,
        serverPath?: string,
        onClose?: (session: SshSession) => void,
    ): Promise<ZSshClient> {
        const client = new ZSshClient();
        client.mSshClient = new Client();
        client.mSshClient.connect(ZSshUtils.buildSshConfig(session));
        client.mSshStream = await new Promise((resolve, reject) => {
            client.mSshClient.on("error", reject).on("ready", () => {
                client.mSshClient.exec(
                    posix.join(serverPath ?? ZSshClient.DEFAULT_SERVER_PATH, "ioserver"),
                    (err, stream) => {
                        if (err) {
                            reject(err);
                        } else {
                            stream.stderr.on("data", (chunk: Buffer) => {
                                console.log("STDERR:", chunk.toString());
                            });
                            // console.log("client ready");
                            resolve(stream);
                        }
                    },
                );
            });
            client.mSshClient.on("close", () => {
                onClose?.(session);
            });
        });
        return client;
    }

    public dispose(): void {
        this.mSshClient?.end();
    }

    public [Symbol.dispose](): void {
        this.dispose();
    }

    public async request<T extends IRpcResponse>(request: IRpcRequest, stream?: Writable): Promise<T> {
        await this.sshMutex?.promise;
        this.sshMutex = new DeferredPromise();
        this.mResponse = "";
        this.mResponseStream = stream;

        return new Promise((resolve, reject) => {
            this.mSshStream.stdin.write(`${JSON.stringify(request)}\n`);
            this.mSshStream.stderr.on("data", this.onErrData.bind(this, reject));
            this.mSshStream.stdout.on(
                "data",
                this.onOutData.bind(this, (response: string) => {
                    try {
                        resolve(JSON.parse(response));
                    } catch {
                        reject(response);
                    }
                }),
            );
        });
    }

    private onErrData(reject: (typeof Promise)["reject"], chunk: Buffer) {
        const error = chunk.toString();
        console.error(error);
        this.requestEnd();
        reject(error);
    }

    private onOutData(resolve: (typeof Promise)["resolve"], chunk: Buffer) {
        const endsWithNewLine = chunk[chunk.length - 1] === 0x0a;
        const newChunk = endsWithNewLine ? chunk.subarray(0, chunk.length - 1) : chunk;

        if (this.mResponseStream != null) {
            this.mResponseStream.write(newChunk);
        } else {
            this.mResponse += newChunk;
        }
        if (endsWithNewLine) {
            this.requestEnd();
            resolve(this.mResponse);
        }
    }

    private requestEnd() {
        this.mSshStream.stderr.removeAllListeners();
        this.mSshStream.stdout.removeAllListeners();
        this.mResponseStream?.end();
        this.sshMutex?.resolve();
    }
}
