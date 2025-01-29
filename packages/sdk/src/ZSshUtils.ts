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

import type { IProfile } from "@zowe/imperative";
import { type ISshSession, SshSession } from "@zowe/zos-uss-for-zowe-sdk";

// biome-ignore lint/complexity/noStaticOnlyClass: Utilities class has static methods
export class ZSshUtils {
    public static buildSession(args: IProfile): SshSession {
        const sshSessCfg: ISshSession = {
            hostname: args.host,
            port: args.port ?? 22,
            user: args.user,
            privateKey: args.privateKey,
            keyPassphrase: args.privateKey ? args.keyPassphrase : undefined,
            password: args.privateKey ? undefined : args.password,
        };
        return new SshSession(sshSessCfg);
    }

    public static decodeByteArray(data: Buffer | string): Buffer {
        return typeof data === "string" ? Buffer.from(data, "base64") : data;
    }

    public static encodeByteArray(data: Buffer | string): string {
        return typeof data !== "string" ? Buffer.from(data).toString("base64") : data;
    }

    public static deployServer(): void {}
}
