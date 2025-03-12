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
import { ImperativeError, Logger } from "@zowe/imperative";
import type { SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { Client, type ClientChannel } from "ssh2";
import { AbstractRpcClient } from "./AbstractRpcClient";
import { ZSshUtils } from "./ZSshUtils";
import type { ClientOptions, CommandRequest, CommandResponse, RpcRequest, RpcResponse } from "./doc";

type PromiseResolve<T> = (value: T | PromiseLike<T>) => void;
// biome-ignore lint/suspicious/noExplicitAny: Promise reject type uses any
type PromiseReject = (reason?: any) => void;

export class ZSshClient extends AbstractRpcClient implements Disposable {
    public static readonly DEFAULT_SERVER_PATH = "~/.zowe-server";

    private mErrHandler: ClientOptions["onError"];
    private mSshClient: Client;
    private mSshStream: ClientChannel;
    private mPartialStderr = "";
    private mPartialStdout = "";
    private mPromiseMap: Map<number, { resolve: PromiseResolve<CommandResponse>; reject: PromiseReject }> = new Map();
    private mRequestId = 0;

    private constructor() {
        super();
    }

    public static async create(session: SshSession, opts: ClientOptions = {}): Promise<ZSshClient> {
        Logger.getAppLogger().debug("Starting SSH client");
        const client = new ZSshClient();
        client.mErrHandler = opts.onError ?? console.error;
        client.mSshClient = new Client();
        client.mSshClient.connect(ZSshUtils.buildSshConfig(session));
        client.mSshStream = await new Promise((resolve, reject) => {
            client.mSshClient
                .on("error", (err) => {
                    Logger.getAppLogger().error("Error connecting to SSH: %s", err.toString());
                    reject(err);
                })
                .on("ready", () => {
                    client.mSshClient.exec(
                        posix.join(opts.serverPath ?? ZSshClient.DEFAULT_SERVER_PATH, "zowed"),
                        (err, stream) => {
                            if (err) {
                                Logger.getAppLogger().error("Error running SSH command: %s", err.toString());
                                reject(err);
                            } else {
                                stream.stderr.on("data", client.onErrData.bind(client));
                                stream.stdout.on("data", client.onOutData.bind(client));
                                Logger.getAppLogger().debug("Client is ready");
                                resolve(stream);
                            }
                        },
                    );
                });
            client.mSshClient.on("close", () => {
                Logger.getAppLogger().debug("Client disconnected");
                opts.onClose?.();
            });
        });
        return client;
    }

    public dispose(): void {
        Logger.getAppLogger().debug("Stopping SSH client");
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
            this.mPromiseMap.set(rpcRequest.id, { resolve, reject });
            const requestStr = JSON.stringify(rpcRequest);
            Logger.getAppLogger().trace("Sending request: %s", requestStr);
            this.mSshStream.stdin.write(`${requestStr}\n`);
        });
    }

    private onErrData(chunk: Buffer) {
        if (this.mRequestId === 0) {
            const errMsg = Logger.getAppLogger().error("Message received on stderr: %s", chunk.toString());
            this.mErrHandler(new Error(errMsg));
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
            this.requestEnd(responses[i]);
        }
        return responses[responses.length - 1];
    }

    private requestEnd(data: string, success = true) {
        Logger.getAppLogger().trace("Received response: %s", data);
        let response: RpcResponse;
        try {
            response = JSON.parse(data);
        } catch (err) {
            const errMsg = `Failed to parse response as JSON: ${err}`;
            Logger.getAppLogger().error(errMsg);
            throw new Error(errMsg);
        }
        if (!this.mPromiseMap.has(response.id)) {
            const errMsg = `Missing promise for response ID: ${response.id}`;
            Logger.getAppLogger().error(errMsg);
            throw new Error(errMsg);
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
