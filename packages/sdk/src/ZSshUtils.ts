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
import { ImperativeError, type IProfile, Logger } from "@zowe/imperative";
import { type ISshSession, SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { isEqual } from "es-toolkit";
import { NodeSSH, type Config as NodeSSHConfig } from "node-ssh";
import type { ConnectConfig, SFTPWrapper } from "ssh2";
import { PrivateKeyFailurePatterns } from "./SshErrors";

export interface ISshCallbacks {
    onProgress?: (increment: number) => void; // Callback to report incremental progress
    onError?: (error: Error, context: string) => Promise<boolean>; // Callback to handle errors, returns true to continue/retry
}

type SftpError = Error & { code?: number };

// biome-ignore lint/complexity/noStaticOnlyClass: Utilities class has static methods
export class ZSshUtils {
    private static readonly SERVER_PAX_FILE = "server.pax.Z";
    private static readonly EXPIRED_PASSWORD_CODES = ["FOTS1668", "FOTS1669"];

    /**
     * Throws an `ImperativeError` if the given text contains z/OS expired-password
     * indicators (FOTS1668/FOTS1669). Call this after any `execCommand` to surface
     * the real problem instead of returning a generic failure.
     */
    private static throwIfExpiredPassword(text: string): void {
        if (ZSshUtils.EXPIRED_PASSWORD_CODES.some((code) => text.includes(code))) {
            throw new ImperativeError({
                msg: "Password expired on the remote z/OS system. Change your password and retry.",
                errorCode: "EPASSWD_EXPIRED",
                additionalDetails: text,
            });
        }
    }

    /**
     * Checks if an error message indicates a private key authentication failure
     * @param errorMessage The error message to check
     * @param hasPrivateKey Optional flag to indicate if a private key is configured (for more accurate detection)
     * @returns True if the error indicates a private key authentication failure
     */
    public static isPrivateKeyAuthFailure(errorMessage: string, hasPrivateKey?: boolean): boolean {
        // If no private key is configured, this can't be a private key failure
        if (hasPrivateKey === false) {
            return false;
        }

        return PrivateKeyFailurePatterns.some((pattern) => errorMessage.includes(pattern));
    }

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
        options?: ISshCallbacks,
    ): Promise<boolean> {
        Logger.getAppLogger().info(`[ZSshUtils] installServer to ${session.ISshSession.hostname} at path: ${serverPath}`);
        const localDir = ZSshUtils.getBinDir(__dirname);
        const remoteDir = serverPath.replace(/^~/, ".");

        return ZSshUtils.sftp(session, async (sftp, ssh) => {
            Logger.getAppLogger().info(`[ZSshUtils] Step 1/4: Creating remote directory ${remoteDir}`);
            const execReturn = await ssh.execCommand(`mkdir -p ${remoteDir}`);
            ZSshUtils.throwIfExpiredPassword(execReturn.stderr ?? "");
            if (execReturn.code !== 0) {
                const errMsg = `[ZSshUtils] Step 1 FAILED: mkdir -p ${remoteDir} RC=${execReturn.code}: ${execReturn.stderr}`;
                Logger.getAppLogger().error(errMsg);
                if (options?.onError) {
                    const shouldRetry = await options.onError(new Error(errMsg), "deploy");
                    if (!shouldRetry) {
                        return false;
                    }
                    return ZSshUtils.installServer(session, serverPath, options);
                }
                return false;
            }

            const localPaxPath = path.join(localDir, ZSshUtils.SERVER_PAX_FILE);
            const remotePaxPath = path.posix.join(remoteDir, ZSshUtils.SERVER_PAX_FILE);

            let previousPercentage = 0;
            const progressCallback = options?.onProgress
                ? (progress: number, _chunk: number, total: number) => {
                      const percentage = Math.floor((progress / total) * 100);
                      const increment = percentage - previousPercentage;
                      if (increment > 0) {
                          options.onProgress!(increment);
                          previousPercentage = percentage;
                      }
                  }
                : undefined;

            Logger.getAppLogger().info(`[ZSshUtils] Step 2/4: Uploading ${ZSshUtils.SERVER_PAX_FILE} to ${remotePaxPath}`);
            try {
                await promisify(sftp.fastPut.bind(sftp))(localPaxPath, remotePaxPath, { step: progressCallback });
            } catch (err) {
                ZSshUtils.throwIfExpiredPassword(String(err));
                const codePart = (err as SftpError).code != null ? ` RC=${(err as SftpError).code}` : "";
                const errMsg = `[ZSshUtils] Step 2 FAILED: Upload ${ZSshUtils.SERVER_PAX_FILE}${codePart}: ${err}`;
                Logger.getAppLogger().error(errMsg);

                if (options?.onError) {
                    const shouldRetry = await options.onError(new Error(errMsg), "upload");
                    if (!shouldRetry) {
                        return false;
                    }
                    return ZSshUtils.installServer(session, serverPath, options);
                }
                return false;
            }

            Logger.getAppLogger().info(`[ZSshUtils] Step 3/4: Extracting PAX archive in ${remoteDir}`);
            const result = await ssh.execCommand(`pax -rzf ${ZSshUtils.SERVER_PAX_FILE}`, { cwd: remoteDir });
            ZSshUtils.throwIfExpiredPassword(result.stderr ?? "");
            if (result.code === 0) {
                Logger.getAppLogger().info(`[ZSshUtils] Step 3 OK: Extracted server binaries`);
            } else {
                const errMsg = `[ZSshUtils] Step 3 FAILED: pax -rzf RC=${result.code}: ${result.stderr}`;
                Logger.getAppLogger().error(errMsg);

                if (options?.onError) {
                    const shouldContinue = await options.onError(new Error(errMsg), "extract");
                    if (!shouldContinue) {
                        return false;
                    }
                } else {
                    return false;
                }
            }

            Logger.getAppLogger().info(`[ZSshUtils] Step 4/4: Cleaning up ${remotePaxPath}`);
            try {
                await promisify(sftp.unlink.bind(sftp))(remotePaxPath);
            } catch (err) {
                const errMsg = `[ZSshUtils] Step 4 WARNING: cleanup failed: ${err}`;
                Logger.getAppLogger().warn(errMsg);

                if (options?.onError) {
                    await options.onError(new Error(errMsg), "cleanup");
                } else {
                    Logger.getAppLogger().debug("Cleanup error is non-fatal, continuing...");
                }
            }
            Logger.getAppLogger().info("[ZSshUtils] installServer completed successfully");
            return true;
        });
    }

    public static async uninstallServer(
        session: SshSession,
        serverPath: string,
        options?: Omit<ISshCallbacks, "onProgress">,
    ): Promise<void> {
        Logger.getAppLogger().debug(`Uninstalling server from ${session.ISshSession.hostname} at path: ${serverPath}`);
        return ZSshUtils.sftp(session, async (_sftp, ssh) => {
            const result = await ssh.execCommand(`rm -rf ${serverPath}`);
            ZSshUtils.throwIfExpiredPassword(result.stderr ?? "");
            if (result.code === 0) {
                Logger.getAppLogger().debug(`Deleted directory ${serverPath} with response: ${result.stdout}`);
            } else {
                const errMsg = `Failed to delete directory ${serverPath} with RC ${result.code}: ${result.stderr}`;
                Logger.getAppLogger().error(errMsg);
                if (options?.onError) {
                    const shouldContinue = await options.onError(new Error(errMsg), "unlink");
                    if (!shouldContinue) {
                        throw new Error(errMsg);
                    }
                    return ZSshUtils.uninstallServer(session, serverPath, options);
                }
                throw new Error(errMsg);
            }
        });
    }

    public static async checkIfOutdated(remoteChecksums?: Record<string, string>): Promise<boolean> {
        if (remoteChecksums == null) {
            Logger.getAppLogger().warn("Checksums not found, could not verify server");
            return false;
        }
        const localFile = path.join(ZSshUtils.getBinDir(__dirname), "checksums.asc");
        const localChecksums: Record<string, string> = {};
        for (const line of fs.readFileSync(localFile, "utf-8").trimEnd().split("\n")) {
            const [checksum, file] = line.split(/\s+/);
            localChecksums[file] = checksum;
        }
        return !isEqual(localChecksums, remoteChecksums);
    }

    private static getBinDir(dir: string): string {
        if (!dir || fs.existsSync(path.join(dir, "package.json"))) {
            return path.join(dir, "bin");
        }

        const dirUp = path.normalize(path.join(dir, ".."));
        if (path.parse(dirUp).base.length > 0) {
            return ZSshUtils.getBinDir(dirUp);
        }

        throw new Error(`Failed to find bin directory in path ${dir}`);
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
