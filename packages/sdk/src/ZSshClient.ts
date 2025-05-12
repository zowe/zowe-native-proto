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
import { Readable, Stream, Writable } from "node:stream";
import { ImperativeError, Logger } from "@zowe/imperative";
import type { SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { Base64Decode, Base64Encode } from "base64-stream";
import { Client, type ClientChannel } from "ssh2";
import { AbstractRpcClient } from "./AbstractRpcClient";
import { ZSshUtils } from "./ZSshUtils";
import type {
    ClientOptions,
    CommandRequest,
    CommandResponse,
    RpcNotification,
    RpcRequest,
    RpcResponse,
    StatusMessage,
} from "./doc";

type PromiseResolve<T> = (value: T | PromiseLike<T>) => void;
// biome-ignore lint/suspicious/noExplicitAny: Promise reject type uses any
type PromiseReject = (reason?: any) => void;

export class ZSshClient extends AbstractRpcClient implements Disposable {
    public static readonly DEFAULT_SERVER_PATH = "~/.zowe-server";

    private mErrHandler: ClientOptions["onError"];
    private mResponseTimeout: number;
    private mServerInfo: { checksums?: Record<string, string> };
    private mSshClient: Client;
    private mSshStream: ClientChannel;
    private mPartialStderr = "";
    private mPartialStdout = "";
    private mPendingStreamMap: Map<number, Stream> = new Map();
    private mPromiseMap: Map<number, { resolve: PromiseResolve<CommandResponse>; reject: PromiseReject }> = new Map();
    private mRequestId = 0;

    private constructor() {
        super();
    }

    public static async create(session: SshSession, opts: ClientOptions = {}): Promise<ZSshClient> {
        Logger.getAppLogger().debug("Starting SSH client");
        const client = new ZSshClient();
        client.mErrHandler = opts.onError ?? console.error;
        client.mResponseTimeout = opts.responseTimeout ? opts.responseTimeout * 1000 : 60e3;
        client.mSshClient = new Client();
        client.mSshStream = await new Promise((resolve, reject) => {
            client.mSshClient.on("error", (err) => {
                Logger.getAppLogger().error("Error connecting to SSH: %s", err.toString());
                reject(err);
            });
            client.mSshClient.on("ready", async () => {
                const zowedBin = posix.join(opts.serverPath ?? ZSshClient.DEFAULT_SERVER_PATH, "zowed");
                const zowedArgs = ["-num-workers", `${opts.numWorkers ?? 10}`];
                client.execAsync(zowedBin, ...zowedArgs).then(resolve, reject);
            });
            client.mSshClient.on("close", () => {
                Logger.getAppLogger().debug("Client disconnected");
                opts.onClose?.();
            });
            const keepAliveMsec = opts.keepAliveInterval != null ? opts.keepAliveInterval * 1000 : 30e3;
            client.mSshClient.connect(ZSshUtils.buildSshConfig(session, { keepaliveInterval: keepAliveMsec }));
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

    public get serverChecksums(): Record<string, string> | undefined {
        return this.mServerInfo?.checksums;
    }

    public async request<T extends CommandResponse>(request: CommandRequest): Promise<T> {
        let timeoutId: NodeJS.Timeout;
        return new Promise<T>((resolve, reject) => {
            const { command, ...rest } = request;
            const rpcRequest: RpcRequest = {
                jsonrpc: "2.0",
                method: command,
                params: rest,
                id: ++this.mRequestId,
            };
            if ("stream" in request && request.stream instanceof Stream) {
                this.mPendingStreamMap.set(rpcRequest.id, request.stream);
                rpcRequest.params.stream = rpcRequest.id;
            }
            timeoutId = setTimeout(() => {
                this.mPromiseMap.delete(rpcRequest.id);
                reject(new ImperativeError({ msg: "Request timed out", errorCode: "ETIMEDOUT" }));
            }, this.mResponseTimeout);
            this.mPromiseMap.set(rpcRequest.id, { resolve, reject });
            const requestStr = JSON.stringify(rpcRequest);
            Logger.getAppLogger().trace("Sending request: %s", requestStr);
            this.mSshStream.stdin.write(`${requestStr}\n`);
        }).finally(() => clearTimeout(timeoutId));
    }

    private execAsync(...args: string[]): Promise<ClientChannel> {
        return new Promise((resolve, reject) => {
            this.mSshClient.exec(args.join(" "), (err, stream) => {
                if (err) {
                    Logger.getAppLogger().error("Error running SSH command: %s", err.toString());
                    reject(err);
                } else {
                    const onData = (data: Buffer) => {
                        try {
                            this.mServerInfo = this.onReady(stream, data.toString());
                            resolve(stream);
                        } catch (err) {
                            reject(err);
                        } finally {
                            stream.stderr.removeListener("data", onData);
                            stream.stdout.removeListener("data", onData);
                        }
                    };
                    stream.stderr.once("data", onData);
                    stream.stdout.once("data", onData);
                }
            });
        });
    }

    private onReady(stream: ClientChannel, data: string): StatusMessage["data"] {
        Logger.getAppLogger().debug("Received SSH data: %s", data);
        let response: StatusMessage;
        try {
            response = JSON.parse(data);
        } catch (err) {
            const errMsg = Logger.getAppLogger().error("Error starting Zowe server: %s", data);
            if (data.includes("FSUM7351")) {
                throw new ImperativeError({
                    msg: "Server not found",
                    errorCode: "ENOTFOUND",
                    additionalDetails: data,
                });
            }
            throw new Error(errMsg);
        }
        if (response.status === "ready") {
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
            if (responses[i].length > 0) {
                this.requestEnd(responses[i]);
            }
        }
        return responses[responses.length - 1];
    }

    private requestEnd(data: string, success = true) {
        Logger.getAppLogger().trace("Received response: %s", data);
        let response: RpcResponse | RpcNotification;
        try {
            response = JSON.parse(data);
        } catch (err) {
            const errMsg = `Invalid JSON response: ${data}`;
            Logger.getAppLogger().error(errMsg);
            this.mErrHandler(new Error(errMsg));
            return;
        }
        const responseId: number = "id" in response ? response.id : response.params?.id;
        if (!this.mPromiseMap.has(responseId)) {
            const errMsg = `Missing promise for response ID: ${responseId}`;
            Logger.getAppLogger().error(errMsg);
            this.mErrHandler(new Error(errMsg));
            return;
        }
        if ("method" in response) {
            const pendingStream = this.mPendingStreamMap.get(responseId);
            if (response.method === "sendStream" && pendingStream instanceof Readable) {
                this.uploadStream(pendingStream, response.params);
            } else if (response.method === "receiveStream" && pendingStream instanceof Writable) {
                this.downloadStream(pendingStream, response.params);
            }
            this.mPendingStreamMap.delete(responseId);
            return;
        }
        if (response.error != null) {
            Logger.getAppLogger().error(`Error for response ID: ${responseId}\n${JSON.stringify(response.error)}`);
            this.mPromiseMap.get(responseId).reject(
                new ImperativeError({
                    msg: response.error.message,
                    errorCode: response.error.code.toString(),
                    additionalDetails: response.error.data,
                }),
            );
        } else {
            this.mPromiseMap.get(responseId).resolve({ success, ...response.result });
        }
        this.mPromiseMap.delete(responseId);
    }

    private uploadStream(readStream: Readable, params: { pipePath: string }): Promise<void> {
        return new Promise((resolve, reject) => {
            this.mSshClient.exec(`cat > ${params.pipePath}`, (err, stream) => {
                if (err != null) {
                    reject(err);
                    return;
                }
                readStream.pipe(new Base64Encode()).pipe(stream.stdin);
                readStream.on("end", resolve);
            });
        });
    }

    private downloadStream(writeStream: Writable, params: { pipePath: string }): Promise<void> {
        return new Promise((resolve, reject) => {
            this.mSshClient.exec(`cat ${params.pipePath}`, (err, stream) => {
                if (err != null) {
                    reject(err);
                    return;
                }
                stream.stdout.pipe(new Base64Decode()).pipe(writeStream);
                stream.stdout.on("end", resolve);
            });
        });
    }
}
