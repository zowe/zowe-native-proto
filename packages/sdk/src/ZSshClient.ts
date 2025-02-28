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
import { ImperativeError } from "@zowe/imperative";
import type { SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { Client, type ClientChannel } from "ssh2";
import { AbstractRpcClient } from "./AbstractRpcClient";
import { ZSshUtils } from "./ZSshUtils";
import type { CommandRequest, CommandResponse, RpcRequest, RpcResponse } from "./doc";

export class ZSshClient extends AbstractRpcClient implements Disposable {
    public static readonly DEFAULT_SERVER_PATH = "~/.zowe-server";

    private mSshClient: Client;
    private mSshStream: ClientChannel;
    private mPartialStderr = "";
    private mPartialStdout = "";
    private mPromiseMap: Map<number, { resolve: typeof Promise.resolve; reject: typeof Promise.reject }> = new Map();
    private mRequestId = 0;

    private constructor() {
        super();
    }

    public static async create(session: SshSession, serverPath?: string, onClose?: () => void): Promise<ZSshClient> {
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
                            stream.stderr.on("data", client.onErrData.bind(client));
                            stream.stdout.on("data", client.onOutData.bind(client));
                            // console.log("client ready");
                            resolve(stream);
                        }
                    },
                );
            });
            client.mSshClient.on("close", () => {
                onClose?.();
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

    public async request<T extends CommandResponse>(request: CommandRequest): Promise<T> {
        return new Promise((resolve, reject) => {
            const { command, ...rest } = request;
            const rpcRequest: RpcRequest = {
                jsonrpc: "2.0",
                method: command,
                params: rest,
                id: ++this.mRequestId,
            };
            this.mPromiseMap.set(rpcRequest.id, { resolve, reject } as any);
            this.mSshStream.stdin.write(`${JSON.stringify(rpcRequest)}\n`);
        });
    }

    private onErrData(chunk: Buffer) {
        if (this.mRequestId === 0) {
            console.error("STDERR:", chunk.toString());
            return;
        }
        this.mPartialStderr = this.processResponses(this.mPartialStderr + chunk.toString());
    }

    private onOutData(chunk: Buffer) {
        this.mPartialStdout = this.processResponses(this.mPartialStdout + chunk.toString());
    }

    private processResponses(data: string): string {
        const responses = data.split("\n");
        for (let i = 0; i < responses.length - 1; i++) {
            this.requestEnd(responses[i], true);
        }
        return responses[responses.length - 1];
    }

    private requestEnd(data: string, success: boolean) {
        let response: RpcResponse;
        try {
            response = JSON.parse(data);
        } catch (err) {
            throw new Error(`Failed to parse response as JSON: ${err}`);
        }
        if (!this.mPromiseMap.has(response.id)) {
            throw new Error(`Missing promise for response ID: ${response.id}`);
        }
        if (response.error != null) {
            this.mPromiseMap.get(response.id).reject(
                new ImperativeError({
                    msg: response.error.message,
                    errorCode: response.error.code.toString(),
                    additionalDetails: response.error.data,
                }),
            );
        } else {
            this.mPromiseMap.get(response.id).resolve({ success, ...response.result });
        }
        this.mPromiseMap.delete(response.id);
    }
}
