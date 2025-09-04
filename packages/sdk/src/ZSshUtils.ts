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
            handshakeTimeout: args.handshakeTimeout,
        };
        return new SshSession(sshSessCfg);
    }

    public static buildSshConfig(session: SshSession, configProps?: ConnectConfig): ConnectConfig {
        return {
            host: session.ISshSession.hostname,
            port: session.ISshSession.port,
            username: session.ISshSession.user,
            password: session.ISshSession.password,
            privateKey: session.ISshSession.privateKey
                ? fs.readFileSync(session.ISshSession.privateKey, "utf-8")
                : undefined,
            passphrase: session.ISshSession.keyPassphrase,
            readyTimeout: session.ISshSession.handshakeTimeout,
            // ssh2 debug messages are extremely verbose so log at TRACE level
            debug: (msg) => Logger.getAppLogger().trace(msg),
            ...configProps,
        };
    }

    public static async installServer(
        session: SshSession,
        serverPath: string,
        localDir: string,
        options?: {
            onProgress?: (increment: number) => void; // Callback to report incremental progress
            onError?: (error: Error, context: string) => Promise<boolean>; // Callback to handle errors, returns true to continue/retry
        },
    ): Promise<void> {
        Logger.getAppLogger().debug(`Installing server to ${session.ISshSession.hostname} at path: ${serverPath}`);
        const remoteDir = serverPath.replace(/^~/, ".");
        return ZSshUtils.sftp(session, async (sftp, ssh) => {
            await promisify(sftp.mkdir.bind(sftp))(remoteDir, { mode: 0o700 }).catch((err: SftpError) => {
                if (err.code !== 4) throw err;
                Logger.getAppLogger().debug(`Remote directory already exists: ${serverPath}`);
            });

            // Track the previous progress percentage
            let previousPercentage = 0;

            // Create the progress callback for tracking the upload progress
            const progressCallback = options?.onProgress
                ? (progress: number, _chunk: number, total: number) => {
                      const percentage = Math.floor((progress / total) * 100); // Calculate percentage
                      const increment = percentage - previousPercentage;

                      if (increment > 0) {
                          options.onProgress!(increment);
                          previousPercentage = percentage;
                      }
                  }
                : undefined;

            try {
                await promisify(sftp.fastPut.bind(sftp))(
                    path.join(localDir, ZSshUtils.SERVER_PAX_FILE),
                    path.posix.join(remoteDir, ZSshUtils.SERVER_PAX_FILE),
                    { step: progressCallback },
                );
            } catch (err) {
                const errMsg = `Failed to upload server PAX file${(err as SftpError).code ? ` with RC ${(err as SftpError).code}` : ""}: ${err}`;
                Logger.getAppLogger().error(errMsg);

                if (options?.onError) {
                    const shouldContinue = await options.onError(new Error(errMsg), "upload");
                    if (!shouldContinue) {
                        throw new Error(errMsg);
                    }
                    // If shouldContinue is true, the error handler might have resolved the issue
                    // or the user chose to continue despite the error
                } else {
                    throw new Error(errMsg);
                }
            }

            const result = await ssh.execCommand(`pax -rzf ${ZSshUtils.SERVER_PAX_FILE}`, { cwd: remoteDir });
            if (result.code === 0) {
                Logger.getAppLogger().debug(`Extracted server binaries with response: ${result.stdout}`);
            } else {
                const errMsg = `Failed to extract server binaries with RC ${result.code}: ${result.stderr}`;
                Logger.getAppLogger().error(errMsg);

                if (options?.onError) {
                    const shouldContinue = await options.onError(new Error(errMsg), "extract");
                    if (!shouldContinue) {
                        throw new Error(errMsg);
                    }
                } else {
                    throw new Error(errMsg);
                }
            }

            try {
                await promisify(sftp.unlink.bind(sftp))(path.posix.join(remoteDir, ZSshUtils.SERVER_PAX_FILE));
            } catch (err) {
                const errMsg = `Failed to cleanup server PAX file: ${err}`;
                Logger.getAppLogger().warn(errMsg);

                if (options?.onError) {
                    await options.onError(new Error(errMsg), "cleanup");
                    // Don't throw for cleanup errors, just log them
                } else {
                    // Non-fatal cleanup error, just log it
                    Logger.getAppLogger().debug("Cleanup error is non-fatal, continuing...");
                }
            }
        });
    }

    public static async uninstallServer(
        session: SshSession,
        serverPath: string,
        options?: {
            onError?: (error: Error, context: string) => Promise<boolean>; // Callback to handle errors, returns true to continue/retry
        },
    ): Promise<void> {
        Logger.getAppLogger().debug(`Uninstalling server from ${session.ISshSession.hostname} at path: ${serverPath}`);
        const remoteDir = serverPath.replace(/^~/, ".");
        return ZSshUtils.sftp(session, async (sftp, _ssh) => {
            for (const file of ZSshUtils.SERVER_BIN_FILES) {
                try {
                    await promisify(sftp.unlink.bind(sftp))(path.posix.join(remoteDir, file));
                } catch (err) {
                    const sftpErr = err as SftpError;
                    if (sftpErr.code === 2) {
                        // File not found - this is expected and not an error
                        Logger.getAppLogger().info(`Remote file does not exist: ${serverPath}/${file}`);
                    } else {
                        const errMsg = `Failed to remove server file ${file}: ${err}`;
                        Logger.getAppLogger().error(errMsg);

                        if (options?.onError) {
                            const shouldContinue = await options.onError(new Error(errMsg), "unlink");
                            if (!shouldContinue) {
                                throw new Error(errMsg);
                            }
                        } else {
                            throw new Error(errMsg);
                        }
                    }
                }
            }

            try {
                await promisify(sftp.rmdir.bind(sftp))(remoteDir);
            } catch (err) {
                const sftpErr = err as SftpError;
                if (sftpErr.code === 4) {
                    // Directory not found - this is expected and not an error
                    Logger.getAppLogger().info(`Remote directory does not exist: ${serverPath}`);
                } else {
                    const errMsg = `Failed to remove server directory: ${err}`;
                    Logger.getAppLogger().error(errMsg);

                    if (options?.onError) {
                        const shouldContinue = await options.onError(new Error(errMsg), "rmdir");
                        if (!shouldContinue) {
                            throw new Error(errMsg);
                        }
                    } else {
                        throw new Error(errMsg);
                    }
                }
            }
        });
    }

    public static async checkIfOutdated(localFile: string, remoteChecksums?: Record<string, string>): Promise<boolean> {
        if (remoteChecksums == null) {
            Logger.getAppLogger().warn("Checksums not found, could not verify server");
            return false;
        }
        const localChecksums: Record<string, string> = {};
        for (const line of fs.readFileSync(localFile, "utf-8").trimEnd().split("\n")) {
            const [checksum, file] = line.split(/\s+/);
            localChecksums[file] = checksum;
        }
        return JSON.stringify(localChecksums) !== JSON.stringify(remoteChecksums);
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
