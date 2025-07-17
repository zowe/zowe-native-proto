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

import { Readable, type Stream, Writable } from "node:stream";
import { pipeline } from "node:stream/promises";
import { Logger } from "@zowe/imperative";
import { Base64Encode } from "base64-stream";
import type { Client, ClientChannel } from "ssh2";
import { CountingBase64Decode } from "./CountingBase64Decode";
import type { CommandResponse, RpcNotification, RpcPromise, RpcRequest } from "./doc";

export class RpcNotificationManager {
    private mPendingStreamMap: Map<number, Stream> = new Map();

    public constructor(private mSshClient: Client) {}

    public registerStream(request: RpcRequest, stream: Stream, timeoutId?: NodeJS.Timeout): void {
        this.mPendingStreamMap.set(
            request.id,
            stream.on("keepAlive", () => timeoutId?.refresh()),
        );
        request.params.stream = request.id;
    }

    public handleNotification(notif: RpcNotification, promise?: RpcPromise): void {
        switch (notif.method) {
            case "sendStream":
                this.finishStream(promise, this.uploadStream(notif.params));
                break;
            case "receiveStream":
                this.finishStream(promise, this.downloadStream(notif.params));
                break;
            default:
                throw new Error(`Unknown RPC notification type: ${notif.method}`);
        }
    }

    private async uploadStream(params: { id: number; pipePath: string }): Promise<number> {
        const readStream = this.mPendingStreamMap.get(params.id);
        if (readStream == null || !(readStream instanceof Readable)) {
            throw new Error(`No stream found for request ID: ${params.id}`);
        }
        this.mPendingStreamMap.delete(params.id);

        const sshStream = await new Promise<ClientChannel>((resolve, reject) => {
            this.mSshClient.exec(`cat > ${params.pipePath}`, (err, stream) => (err ? reject(err) : resolve(stream)));
        });
        let totalBytes = 0;
        readStream.on("data", (chunk: Buffer) => {
            totalBytes += chunk.length;
            readStream.emit("keepAlive");
        });

        await pipeline(readStream, new Base64Encode(), sshStream.stdin);
        return totalBytes;
    }

    private async downloadStream(params: { id: number; pipePath: string }): Promise<number> {
        const writeStream = this.mPendingStreamMap.get(params.id);
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

    private finishStream(rpcPromise: RpcPromise, streamPromise: Promise<number>): void {
        const { reject, resolve } = rpcPromise;
        rpcPromise.resolve = async (response: CommandResponse) => {
            const contentLen = await streamPromise;
            try {
                this.expectContentLengthMatches(response, contentLen);
                resolve(response);
            } catch (err) {
                reject(err);
            }
        };
    }

    private expectContentLengthMatches(response: CommandResponse, expectedLen: number): void {
        if ("contentLen" in response && response.contentLen != null && response.contentLen !== expectedLen) {
            const errMsg = Logger.getAppLogger().error(
                "Content length mismatch: expected %d, got %d",
                expectedLen,
                response.contentLen,
            );
            throw new Error(errMsg);
        }
    }
}
