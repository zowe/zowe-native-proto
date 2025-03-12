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

export type B64String = string & { __brand: "B64String" };

export namespace B64String {
    export function decode(data: B64String): string {
        return B64String.decodeBytes(data).toString();
    }

    export function decodeBytes(data: B64String): Uint8Array {
        return Buffer.from(data, "base64");
    }

    export function encode(data: Uint8Array | string): B64String {
        return (typeof data === "string" ? Buffer.from(data) : data).toString("base64") as B64String;
    }
}
