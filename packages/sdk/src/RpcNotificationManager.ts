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
import { Base64Decode, Base64Encode } from "base64-stream";
import type { Client, ClientChannel } from "ssh2";
import type { CallbackInfo, CommandResponse, RpcNotification, RpcPromise, RpcRequest } from "./doc";
import type { ReadDatasetResponse } from "./doc/gen/ds";
import type { ReadFileResponse } from "./doc/gen/uss";
import { ProgressTransform } from "./ProgressTransform";

type StreamMode = "r" | "w";

export class RpcNotificationManager {
    private readonly mPendingStreamMap: Map<number, { stream: Stream; callbackInfo?: CallbackInfo }> = new Map();

    public constructor(private readonly mSshClient: Client) {}

    public registerStream(
        request: RpcRequest,
        stream: Stream,
        timeoutId?: NodeJS.Timeout,
        callbackInfo?: CallbackInfo,
    ): void {
        this.mPendingStreamMap.set(request.id, {
            stream: stream.on("keepAlive", () => timeoutId?.refresh()),
            callbackInfo,
        });
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
            default:
                throw new Error(`Unknown RPC notification type: ${notif.method}`);
        }
    }

    private async uploadStream(params: { id: number; pipePath: string }): Promise<number> {
        const { stream: readStream, callbackInfo } = this.mPendingStreamMap.get(params.id)!;
        if (readStream == null || !(readStream instanceof Readable)) {
            throw new Error(`No stream found for request ID: ${params.id}`);
        }
        this.mPendingStreamMap.delete(params.id);

        const sshStream = await new Promise<ClientChannel>((resolve, reject) => {
            this.mSshClient.exec(`cat > ${params.pipePath}`, (err, stream) => (err ? reject(err) : resolve(stream)));
        });
        const progressTransform = new ProgressTransform(callbackInfo, () => readStream.emit("keepAlive"));

        await pipeline(readStream, progressTransform, new Base64Encode(), sshStream.stdin);
        return progressTransform.bytesProcessed;
    }

    private async downloadStream(params: { id: number; pipePath: string; contentLen?: number }): Promise<number> {
        const { stream: writeStream, callbackInfo } = this.mPendingStreamMap.get(params.id)!;
        if (writeStream == null || !(writeStream instanceof Writable)) {
            throw new Error(`No stream found for request ID: ${params.id}`);
        }
        this.mPendingStreamMap.delete(params.id);
        if (callbackInfo != null && params.contentLen != null) {
            callbackInfo.totalBytes = params.contentLen;
        }

        const sshStream = await new Promise<ClientChannel>((resolve, reject) => {
            this.mSshClient.exec(`cat ${params.pipePath}`, (err, stream) => (err ? reject(err) : resolve(stream)));
        });
        const progressTransform = new ProgressTransform(callbackInfo, () => writeStream.emit("keepAlive"));

        await pipeline(sshStream.stdout, new Base64Decode(), progressTransform, writeStream);
        return progressTransform.bytesProcessed;
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
            const resourceName = (response as ReadDatasetResponse).dataset ?? (response as ReadFileResponse).fspath;
            let errMsg = "Content length mismatch";
            if (resourceName != null) {
                errMsg += ` for ${resourceName}`;
            }
            const expectedLen = mode === "r" ? clientLen : response.contentLen;
            const actualLen = mode === "r" ? response.contentLen : clientLen;
            throw new Error(Logger.getAppLogger().error("%s: expected %d, got %d", errMsg, expectedLen, actualLen));
        }
    }
}
