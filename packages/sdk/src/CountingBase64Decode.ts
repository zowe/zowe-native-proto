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

import { Base64Decode } from "base64-stream";

export class CountingBase64Decode extends Base64Decode {
    private mBytesWritten = 0;

    /**
     * Get the total number of bytes written so far
     */
    public get bytesWritten(): number {
        return this.mBytesWritten;
    }

    /**
     * Reset the byte counter to zero
     */
    public resetCounter(): void {
        this.mBytesWritten = 0;
    }

    /**
     * Override the push method to count bytes as they are written
     */
    public push(chunk: Buffer | string | Uint8Array | null, encoding?: BufferEncoding): boolean {
        if (chunk != null) {
            this.mBytesWritten += chunk.length;
        }

        return super.push(chunk, encoding);
    }
}