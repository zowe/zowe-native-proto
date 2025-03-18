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
import { type IProfile, Logger } from "@zowe/imperative";
import { type ISshSession, SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { NodeSSH, type Config as NodeSSHConfig } from "node-ssh";
import type { ConnectConfig, SFTPWrapper } from "ssh2";

type SftpError = Error & { code?: number };

// biome-ignore lint/complexity/noStaticOnlyClass: Utilities class has static methods
export class ZSshUtils {
    private static readonly SERVER_BIN_FILES = ["zowed", "zowex"];
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
            privateKey: session.ISshSession.privateKey
                ? fs.readFileSync(session.ISshSession.privateKey, "utf-8")
                : undefined,
            passphrase: session.ISshSession.keyPassphrase,
            debug: (msg: string) => Logger.getAppLogger().trace(msg),
        };
    }

    public static async installServer(
        session: SshSession,
        serverPath: string,
        localDir: string,
        onProgress?: (increment: number) => void, // Callback to report incremental progress
    ): Promise<void> {
        Logger.getAppLogger().debug(`Installing server to ${session.ISshSession.hostname} at path: ${serverPath}`);
        const remoteDir = serverPath.replace(/^~/, ".");
        return ZSshUtils.sftp(session, async (sftp, ssh) => {
            await promisify(sftp.mkdir.bind(sftp))(remoteDir, { mode: 0o700 }).catch((err: SftpError) => {
                if (err.code !== 4) throw err;
                Logger.getAppLogger().debug(`Remote directory already exists: ${remoteDir}`);
            });

            // Track the previous progress percentage
            let previousPercentage = 0;

            // Create the progress callback for tracking the upload progress
            const progressCallback = onProgress
                ? (progress: number, chunk: number, total: number) => {
                      const percentage = Math.floor((progress / total) * 100); // Calculate percentage
                      const increment = percentage - previousPercentage;

                      if (increment > 0) {
                          onProgress(increment);
                          previousPercentage = percentage;
                      }
                  }
                : undefined;

            try {
                await promisify(sftp.fastPut.bind(sftp))(
                    path.join(localDir, ZSshUtils.SERVER_PAX_FILE),
                    path.posix.join(remoteDir, ZSshUtils.SERVER_PAX_FILE),
                );
            } catch (err) {
                const errMsg = `Failed to upload server PAX file${err.code ? ` with RC ${err.code}` : ""}: ${err}`;
                Logger.getAppLogger().error(errMsg);
                throw new Error(errMsg);
            }

            const result = await ssh.execCommand(`pax -rzf ${ZSshUtils.SERVER_PAX_FILE}`, { cwd: remoteDir });
            if (result.code === 0) {
                Logger.getAppLogger().debug(`Extracted server binaries with response: ${result.stdout}`);
            } else {
                const errMsg = `Failed to extract server binaries with RC ${result.code}: ${result.stderr}`;
                Logger.getAppLogger().error(errMsg);
                throw new Error(errMsg);
            }
            await promisify(sftp.unlink.bind(sftp))(path.posix.join(remoteDir, ZSshUtils.SERVER_PAX_FILE));
        });
    }

    public static async uninstallServer(session: SshSession, serverPath: string): Promise<void> {
        Logger.getAppLogger().debug(`Uninstalling server from ${session.ISshSession.hostname} at path: ${serverPath}`);
        const remoteDir = serverPath.replace(/^~/, ".");
        return ZSshUtils.sftp(session, async (sftp, _ssh) => {
            for (const file of ZSshUtils.SERVER_BIN_FILES) {
                await promisify(sftp.unlink.bind(sftp))(path.posix.join(remoteDir, file)).catch((err: SftpError) => {
                    if (err.code !== 2) throw err;
                    Logger.getAppLogger().info(`Remote file does not exist: ${remoteDir}/${file}`);
                });
            }
            await promisify(sftp.rmdir.bind(sftp))(remoteDir).catch((err: SftpError) => {
                if (err.code !== 4) throw err;
                Logger.getAppLogger().info(`Remote directory does not exist: ${remoteDir}`);
            });
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
