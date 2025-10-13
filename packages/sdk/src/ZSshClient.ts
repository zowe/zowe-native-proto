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
import { Stream } from "node:stream";
import { ImperativeError, Logger } from "@zowe/imperative";
import type { SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { Client, type ClientChannel } from "ssh2";
import { AbstractRpcClient } from "./AbstractRpcClient";
import type {
    ClientOptions,
    CommandRequest,
    CommandResponse,
    RpcNotification,
    RpcPromise,
    RpcRequest,
    RpcResponse,
    StatusMessage,
} from "./doc";
import { RpcApiBuilder, type ZSshRpcApi } from "./RpcApiBuilder";
import { RpcStreamManager } from "./RpcStreamManager";
import { ZSshUtils } from "./ZSshUtils";

export class ZSshClient extends AbstractRpcClient implements Disposable {
    public static readonly DEFAULT_SERVER_PATH = "~/.zowe-server";

    private mApi: ZSshRpcApi;
    private mErrHandler: ClientOptions["onError"];
    private mResponseTimeout: number;
    private mServerInfo: { checksums?: Record<string, string> };
    private mSshClient: Client;
    private mSshStream: ClientChannel;
    private mStreamMgr: RpcStreamManager;
    private mPartialStderr = "";
    private mPartialStdout = "";
    private readonly mPromiseMap: Map<number, RpcPromise> = new Map();
    // biome-ignore lint/correctness/noUnusedPrivateClassMembers: Linter Error: this.mRequestId is used
    private mRequestId = 0;

    private constructor() {
        super();
        this.mApi = RpcApiBuilder.build(this.request.bind(this));
    }

    private static defaultErrHandler(err: Error): void {
        const errMsg = err.toString();
        console.error(errMsg);
        Logger.getAppLogger().error(errMsg);
    }

    public static async create(session: SshSession, opts: ClientOptions = {}): Promise<ZSshClient> {
        Logger.getAppLogger().debug("Starting SSH client");
        const client = new ZSshClient();
        client.mErrHandler = opts.onError ?? ZSshClient.defaultErrHandler;
        client.mResponseTimeout = opts.responseTimeout ? opts.responseTimeout * 1000 : 60e3;
        client.mSshClient = new Client();
        client.mSshStream = await new Promise((resolve, reject) => {
            client.mSshClient.on("error", (err) => {
                Logger.getAppLogger().error(`Error connecting to SSH: ${err}`);
                reject(err);
            });
            client.mSshClient.on("ready", async () => {
                const zowedBin = posix.join(opts.serverPath ?? ZSshClient.DEFAULT_SERVER_PATH, "zowed");
                const zowedArgs = ["-num-workers", `${opts.numWorkers ?? 3}`];
                client.execAsync(zowedBin, ...zowedArgs).then(resolve, reject);
            });
            client.mSshClient.on("close", () => {
                Logger.getAppLogger().debug("Client disconnected");
                opts.onClose?.();
            });
            const keepAliveMsec = opts.keepAliveInterval != null ? opts.keepAliveInterval * 1000 : 30e3;
            client.mSshClient.connect(ZSshUtils.buildSshConfig(session, { keepaliveInterval: keepAliveMsec }));
        });
        client.mStreamMgr = new RpcStreamManager(client.mSshClient);
        return client;
    }

    public dispose(): void {
        Logger.getAppLogger().debug("Stopping SSH client");
        this.mSshClient?.end();
    }

    public [Symbol.dispose](): void {
        this.dispose();
    }

    public get cmds() {
        return this.mApi.cmds;
    }

    public get ds() {
        return this.mApi.ds;
    }

    public get jobs() {
        return this.mApi.jobs;
    }

    public get uss() {
        return this.mApi.uss;
    }

    public get serverChecksums(): Record<string, string> | undefined {
        return this.mServerInfo?.checksums;
    }

    public async request<T extends CommandResponse>(
        request: CommandRequest,
        progressCallback?: (percent: number) => void,
    ): Promise<T> {
        let timeoutId: NodeJS.Timeout;
        return new Promise<T>((resolve, reject) => {
            const { command, ...rest } = request;
            const rpcRequest: RpcRequest = {
                jsonrpc: "2.0",
                method: command,
                params: rest,
                id: ++this.mRequestId,
            };
            timeoutId = setTimeout(() => {
                this.mPromiseMap.delete(rpcRequest.id);
                reject(new ImperativeError({ msg: "Request timed out", errorCode: "ETIMEDOUT" }));
            }, this.mResponseTimeout);
            if ("stream" in request && request.stream instanceof Stream) {
                this.mStreamMgr.registerStream(
                    rpcRequest,
                    request.stream,
                    timeoutId,
                    progressCallback && {
                        callback: progressCallback,
                        // If stream is a ReadStream use the size of the localFile in bytes
                        // If stream is a WriteStream, set undefined because the size progress will be provided by a notification
                        totalBytes: "contentLen" in request ? (request.contentLen as number) : undefined,
                    },
                );
            }
            this.mPromiseMap.set(rpcRequest.id, { resolve, reject });
            const requestStr = JSON.stringify(rpcRequest);
            Logger.getAppLogger().trace(`Sending request: ${requestStr}`);
            this.mSshStream.stdin.write(`${requestStr}\n`);
        }).finally(() => clearTimeout(timeoutId));
    }

    protected handleNotification(notif: RpcNotification): void {
        const streamPromise = this.mPromiseMap.get(notif.params?.id as number);
        try {
            switch (notif.method) {
                case "sendStream":
                    this.mStreamMgr.linkStreamToPromise(streamPromise, notif, "r");
                    break;
                case "receiveStream":
                    this.mStreamMgr.linkStreamToPromise(streamPromise, notif, "w");
                    break;
                default:
                    throw new Error(`unknown method ${notif.method}`);
            }
        } catch (err) {
            const errMsg = Logger.getAppLogger().error("Failed to handle RPC notification: %s", err.message);
            this.mErrHandler(new Error(errMsg));
        }
    }

    protected handleResponse(response: RpcResponse): void {
        if (response.error != null) {
            Logger.getAppLogger().error(`Error for response ID: ${response.id}\n${JSON.stringify(response.error)}`);
            this.mPromiseMap.get(response.id).reject(
                new ImperativeError({
                    msg: response.error.message,
                    errorCode: response.error.code.toString(),
                    additionalDetails: response.error.data,
                }),
            );
        } else {
            this.mPromiseMap.get(response.id).resolve(response.result);
        }
        this.mPromiseMap.delete(response.id);
    }

    private execAsync(...args: string[]): Promise<ClientChannel> {
        return new Promise((resolve, reject) => {
            this.mSshClient.exec(args.join(" "), (err, stream) => {
                if (err) {
                    Logger.getAppLogger().error(`Error running SSH command: ${err}`);
                    reject(err);
                } else {
                    const onData = (data: Buffer) => {
                        const removeListeners = () => {
                            stream.stderr.removeListener("data", onData);
                            stream.stdout.removeListener("data", onData);
                        };
                        try {
                            this.mServerInfo = this.getServerStatus(stream, data.toString());
                            if (this.mServerInfo) {
                                removeListeners();
                                resolve(stream);
                            }
                        } catch (err) {
                            removeListeners();
                            reject(err);
                        }
                    };
                    stream.stderr.on("data", onData);
                    stream.stdout.on("data", onData);
                }
            });
        });
    }

    private getServerStatus(stream: ClientChannel, data: string): StatusMessage["data"] | undefined {
        Logger.getAppLogger().debug(`Received SSH data: ${data}`);
        let response: StatusMessage;
        try {
            response = JSON.parse(data);
        } catch {
            const errMsg = Logger.getAppLogger().error("Error starting Zowe server: %s", data);
            if (data.includes("FSUM7351")) {
                throw new ImperativeError({
                    msg: "Server not found",
                    errorCode: "ENOTFOUND",
                    additionalDetails: data,
                });
            }
            if (data.includes("FOTS1681")) {
                // non-fatal chdir error, display the error but continue waiting for ready message
                this.mErrHandler(new Error(errMsg));
                return;
            }
            throw new Error(errMsg);
        }
        if (response?.status === "ready") {
            stream.stderr.on("data", this.onErrData.bind(this));
            stream.stdout.on("data", this.onOutData.bind(this));
            Logger.getAppLogger().debug("Client is ready");
            return response.data;
        }
    }

    private onErrData(chunk: Buffer) {
        if (this.mPromiseMap.size === 0) {
            const errMsg = Logger.getAppLogger().error("Error from server: %s", chunk.toString());
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
            if (responses[i].length === 0) {
                continue;
            }

            Logger.getAppLogger().trace(`Received response: ${responses[i]}`);
            let response: RpcResponse | RpcNotification;
            try {
                response = JSON.parse(responses[i]);
            } catch {
                const errMsg = Logger.getAppLogger().error("Invalid JSON response: %s", responses[i]);
                this.mErrHandler(new Error(errMsg));
                continue;
            }

            const responseId: number = "id" in response ? response.id : response.params?.id;
            if (!this.mPromiseMap.has(responseId)) {
                const errMsg = Logger.getAppLogger().error("Missing promise for response ID: %d", responseId);
                this.mErrHandler(new Error(errMsg));
                continue;
            }

            if ("method" in response) {
                this.handleNotification(response);
            } else {
                this.handleResponse(response);
            }
        }
        return responses[responses.length - 1];
    }
}
