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

import { Readable, type Stream, Transform, Writable } from "node:stream";
import { pipeline } from "node:stream/promises";
import { Logger } from "@zowe/imperative";
import { Base64Encode } from "base64-stream";
import type { Client, ClientChannel } from "ssh2";
import { CountingBase64Decode } from "./CountingBase64Decode";
import type { CommandResponse, RpcNotification, RpcPromise, RpcRequest } from "./doc";

type callbackInfo = {
    callback?: (percent: number) => void;
    fsize?: number;
};

type StreamMode = "r" | "w";

export class RpcNotificationManager {
    private mPendingStreamMap: Map<number, { stream: Stream; callbackInfo?: callbackInfo }> = new Map();

    public constructor(private mSshClient: Client) {}

    public registerStream(
        request: RpcRequest,
        stream: Stream,
        timeoutId?: NodeJS.Timeout,
        callbackInfo?: callbackInfo,
    ): void {
        this.mPendingStreamMap.set(request.id, { stream, callbackInfo });
        request.params.stream = request.id;
    }

    public handleNotification(notif: RpcNotification, promise?: RpcPromise): void {
        switch (notif.method) {
            case "sendStream":
                this.linkStreamToPromise(promise, this.uploadStream(notif.params), "r");
                break;
            case "receiveStream":
                this.linkStreamToPromise(promise, this.downloadStream(notif.params), "w");
                break;
            case "updateProgress":
                this.mPendingStreamMap.get(notif.params.id)?.callbackInfo?.callback?.(notif.params.progress);
                break;
            default:
                throw new Error(`Unknown RPC notification type: ${notif.method}`);
        }
    }

    private async uploadStream(params: { id: number; pipePath: string }): Promise<number> {
        const { stream: readStream, callbackInfo } = this.mPendingStreamMap.get(params.id)!;
        if (!(readStream instanceof Readable)) {
            throw new Error(`No stream found for request ID: ${params.id}`);
        }
        if (!callbackInfo?.callback || !callbackInfo.fsize) {
            throw new Error(`Progress info callback missing for request ID: ${params.id}`);
        }
        this.mPendingStreamMap.delete(params.id);
        callbackInfo.callback(0);

        const sshStream = await new Promise<ClientChannel>((resolve, reject) => {
            this.mSshClient.exec(`cat > ${params.pipePath}`, (err, stream) => (err ? reject(err) : resolve(stream)));
        });

        let totalBytes = 0;

        const progressTransform = new Transform({
            transform(chunk: Buffer, _, callback) {
                totalBytes += chunk.length;
                const percent = Math.min(100, Math.round((totalBytes / callbackInfo.fsize!) * 100));
                callbackInfo.callback!(percent);
                readStream.emit("keepAlive");
                callback(null, chunk);
            },
        });

        await pipeline(readStream, progressTransform, new Base64Encode(), sshStream.stdin);

        return totalBytes;
    }

    private async downloadStream(params: { id: number; pipePath: string }): Promise<number> {
        const { stream: writeStream } = this.mPendingStreamMap.get(params.id)!;
        if (writeStream == null || !(writeStream instanceof Writable)) {
            throw new Error(`No stream found for request ID: ${params.id}`);
        }

        this.mPendingStreamMap.delete(params.id);

        const sshStream = await new Promise<ClientChannel>((resolve, reject) => {
            this.mSshClient.exec(`cat ${params.pipePath}`, (err, stream) => (err ? reject(err) : resolve(stream)));
        });
        sshStream.stdout.on("data", () => writeStream.emit("keepAlive"));
        const decoder = new CountingBase64Decode();

        await pipeline(sshStream.stdout, decoder, writeStream);
        return decoder.bytesWritten;
    }

    private linkStreamToPromise(rpcPromise: RpcPromise, streamPromise: Promise<number>, mode: StreamMode): void {
        const { reject, resolve } = rpcPromise;
        rpcPromise.resolve = async (response: CommandResponse) => {
            const contentLen = await streamPromise;
            try {
                this.expectContentLengthMatches(response, contentLen, mode);
                resolve(response);
            } catch (err) {
                reject(err);
            }
        };
    }

    private expectContentLengthMatches(response: CommandResponse, clientLen: number, mode: StreamMode): void {
        if ("contentLen" in response && response.contentLen != null && response.contentLen !== clientLen) {
            const expectedLen = mode === "r" ? clientLen : response.contentLen;
            const actualLen = mode === "r" ? response.contentLen : clientLen;
            const errMsg = Logger.getAppLogger().error(
                "Content length mismatch: expected %d, got %d",
                expectedLen,
                actualLen,
            );
            throw new Error(errMsg);
        }
    }
}
