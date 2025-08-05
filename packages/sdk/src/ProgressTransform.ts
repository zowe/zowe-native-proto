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

import { Transform, type TransformCallback } from "node:stream";
import type { CallbackInfo } from "./doc/types";

export class ProgressTransform extends Transform {
    private mBytesProcessed = 0;
    private mPercentReported = 0;

    public constructor(
        private mCallbackInfo?: CallbackInfo,
        private onDataCallback?: () => void,
    ) {
        super();
        this.mCallbackInfo?.callback(0);
    }

    /**
     * Get the total number of bytes written so far
     */
    public get bytesProcessed(): number {
        return this.mBytesProcessed;
    }

    /**
     * Override the transform method to count bytes and invoke progress callback
     */
    public _transform(chunk: Buffer | string, encoding: BufferEncoding, callback: TransformCallback): void {
        if (chunk != null) {
            this.mBytesProcessed += typeof chunk === "string" ? Buffer.byteLength(chunk, encoding) : chunk.length;
            if (this.mCallbackInfo != null) {
                const percent = Math.min(100, Math.round((this.mBytesProcessed / this.mCallbackInfo.totalBytes) * 100));
                if (percent !== this.mPercentReported) {
                    this.mCallbackInfo.callback(percent);
                    this.mPercentReported = percent;
                }
            }
        }
        this.onDataCallback?.();
        callback(null, chunk);
    }
}
