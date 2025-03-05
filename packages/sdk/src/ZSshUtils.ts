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

import * as fs from "node:fs";
import * as path from "node:path";
import { promisify } from "node:util";
import type { IProfile } from "@zowe/imperative";
import { type ISshSession, SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { NodeSSH, type Config as NodeSSHConfig } from "node-ssh";
import type { ConnectConfig, SFTPWrapper } from "ssh2";

type SftpError = Error & { code?: number };

// biome-ignore lint/complexity/noStaticOnlyClass: Utilities class has static methods
export class ZSshUtils {
    private static readonly SERVER_BIN_FILES = ["ioserver", "zowex"];
    private static readonly SERVER_PAX_FILE = "server.pax.Z";

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

    public static buildSshConfig(session: SshSession): ConnectConfig {
        return {
            host: session.ISshSession.hostname,
            port: session.ISshSession.port,
            username: session.ISshSession.user,
            password: session.ISshSession.password,
            privateKey: session.ISshSession.privateKey ? fs.readFileSync(session.ISshSession.privateKey) : undefined,
            passphrase: session.ISshSession.keyPassphrase,
        };
    }

    public static decodeByteArray(data: Buffer | string): Buffer {
        return typeof data === "string" ? Buffer.from(data, "base64") : data;
    }

    public static encodeByteArray(data: Buffer | string): string {
        return typeof data !== "string" ? Buffer.from(data).toString("base64") : data;
    }

    public static async installServer(session: SshSession, serverPath: string, localDir: string): Promise<void> {
        const remoteDir = serverPath.replace(/^~/, ".");
        return ZSshUtils.sftp(session, async (sftp, ssh) => {
            await promisify(sftp.mkdir.bind(sftp))(remoteDir, { mode: 0o700 }).catch((err: SftpError) =>
                // Ignore if directory already exists
                err.code !== 4 ? Promise.reject(err) : Promise.resolve(),
            );
            await promisify(sftp.fastPut.bind(sftp))(
                path.join(localDir, ZSshUtils.SERVER_PAX_FILE),
                path.posix.join(remoteDir, ZSshUtils.SERVER_PAX_FILE),
            );
            await ssh.execCommand(`pax -rzf ${ZSshUtils.SERVER_PAX_FILE}`, { cwd: remoteDir });
            await promisify(sftp.unlink.bind(sftp))(path.posix.join(remoteDir, ZSshUtils.SERVER_PAX_FILE));
        });
    }

    public static async uninstallServer(session: SshSession, serverPath: string): Promise<void> {
        const remoteDir = serverPath.replace(/^~/, ".");
        return ZSshUtils.sftp(session, async (sftp, _ssh) => {
            for (const file of ZSshUtils.SERVER_BIN_FILES) {
                await promisify(sftp.unlink.bind(sftp))(path.posix.join(remoteDir, file)).catch((err: SftpError) =>
                    // Ignore if file does not exist
                    err.code !== 2 ? Promise.reject(err) : Promise.resolve(),
                );
            }
            await promisify(sftp.rmdir.bind(sftp))(remoteDir).catch((err: SftpError) =>
                // Ignore if directory is not empty
                err.code !== 4 ? Promise.reject(err) : Promise.resolve(),
            );
        });
    }

    private static async sftp<T>(
        session: SshSession,
        callback: (sftp: SFTPWrapper, ssh: NodeSSH) => Promise<T>,
    ): Promise<T> {
        const ssh = new NodeSSH();
        await ssh.connect(ZSshUtils.buildSshConfig(session) as NodeSSHConfig);
        try {
            return await ssh.requestSFTP().then((sftp) => callback(sftp, ssh));
        } finally {
            ssh.dispose();
        }
    }
}
