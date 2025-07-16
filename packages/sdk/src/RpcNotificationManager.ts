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
import { Base64Encode } from "base64-stream";
import type { Client, ClientChannel } from "ssh2";
import { CountingBase64Decode } from "./CountingBase64Decode";
import type { RpcNotification, RpcRequest } from "./doc";

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

    public handleNotification(response: RpcNotification): Promise<number> {
        const responseId = response.params?.id as number;
        const pendingStream = this.mPendingStreamMap.get(responseId);
        let streamPromise: Promise<number> | undefined;
        if (response.method === "sendStream" && pendingStream instanceof Readable) {
            streamPromise = this.uploadStream(pendingStream, response.params);
        } else if (response.method === "receiveStream" && pendingStream instanceof Writable) {
            streamPromise = this.downloadStream(pendingStream, response.params);
        }
        if (streamPromise != null) {
            this.mPendingStreamMap.delete(responseId);
        }
        return streamPromise;
    }

    private async uploadStream(readStream: Readable, params: { pipePath: string }): Promise<number> {
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

    private async downloadStream(writeStream: Writable, params: { pipePath: string }): Promise<number> {
        const sshStream = await new Promise<ClientChannel>((resolve, reject) => {
            this.mSshClient.exec(`cat ${params.pipePath}`, (err, stream) => (err ? reject(err) : resolve(stream)));
        });
        sshStream.stdout.on("data", () => writeStream.emit("keepAlive"));
        const decoder = new CountingBase64Decode();
        await pipeline(sshStream.stdout, decoder, writeStream);
        return decoder.bytesWritten;
    }
}
