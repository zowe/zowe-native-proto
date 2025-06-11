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
import { Base64Decode, Base64Encode } from "base64-stream";
import type { Client, ClientChannel } from "ssh2";
import {
    B64String,
    type CommandRequest,
    type CommandResponse,
    type RpcNotification,
    type RpcRequest,
    type StreamMode,
} from "./doc";

export class RpcStreamManager {
    private readonly CHUNK_SIZE = 32768;

    private mPendingStreamMap: Map<number, Stream> = new Map();

    public constructor(private mSshClient: Client) {}

    public registerStream(request: RpcRequest, stream: Stream, timeoutId?: NodeJS.Timeout): void {
        this.mPendingStreamMap.set(
            request.id,
            stream.on("keepAlive", () => timeoutId?.refresh()),
        );
        request.params.stream = request.id;
    }

    public async handleRequest<T extends CommandResponse>(
        request: CommandRequest & { stream?: Stream },
        handler: (request: CommandRequest) => Promise<T>,
        mode: StreamMode,
    ): Promise<T> {
        if ("stream" in request && request.stream instanceof Readable && mode === "r") {
            await this.handleReadStream(request as CommandRequest & { stream: Readable });
        }
        const response = await handler(request);
        if ("stream" in request && request.stream instanceof Writable && mode === "w") {
            this.handleWriteStream(request as CommandRequest & { stream: Writable }, response);
        }
        return response;
    }

    private handleReadStream(request: CommandRequest & { stream: Readable; data?: B64String }): Promise<void> {
        return new Promise((resolve) => {
            const chunks: Buffer[] = [];
            let bytesRead = 0;
            request.stream.once("readable", () => {
                let chunk: Buffer;
                do {
                    chunk = request.stream.read();
                    if (chunk != null) {
                        chunks.push(chunk);
                        bytesRead += chunk.length;
                        if (bytesRead > this.CHUNK_SIZE) {
                            break;
                        }
                    }
                } while (chunk != null);
                if (bytesRead > 0) {
                    if (bytesRead <= this.CHUNK_SIZE) {
                        request.data = B64String.encode(Buffer.concat(chunks));
                        request.stream = undefined;
                    } else {
                        request.stream.unshift(Buffer.concat(chunks));
                    }
                }
                resolve();
            });
        });
    }

    private handleWriteStream(
        request: CommandRequest & { stream: Writable },
        response: CommandResponse & { data?: B64String },
    ): void {
        if ("data" in response && response.data != null) {
            request.stream.end(B64String.decodeBytes(response.data));
        }
    }

    public handleNotification(response: RpcNotification): Promise<void> {
        const responseId = response.params?.id as number;
        const pendingStream = this.mPendingStreamMap.get(responseId);
        let streamPromise: Promise<void> | undefined;
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

    private async uploadStream(readStream: Readable, params: { pipePath: string }): Promise<void> {
        const sshStream = await new Promise<ClientChannel>((resolve, reject) => {
            this.mSshClient.exec(`cat > ${params.pipePath}`, (err, stream) => (err ? reject(err) : resolve(stream)));
        });
        readStream.on("data", () => readStream.emit("keepAlive"));
        return pipeline(readStream, new Base64Encode(), sshStream.stdin);
    }

    private async downloadStream(writeStream: Writable, params: { pipePath: string }): Promise<void> {
        const sshStream = await new Promise<ClientChannel>((resolve, reject) => {
            this.mSshClient.exec(`cat ${params.pipePath}`, (err, stream) => (err ? reject(err) : resolve(stream)));
        });
        sshStream.stdout.on("data", () => writeStream.emit("keepAlive"));
        return pipeline(sshStream.stdout, new Base64Decode(), writeStream);
    }
}
