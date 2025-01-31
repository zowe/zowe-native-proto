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
import type { IProfile } from "@zowe/imperative";
import { type ISshSession, SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { Client, type ConnectConfig, type SFTPWrapper } from "ssh2";

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
        const client = new Client();
        client.connect(ZSshUtils.buildSshConfig(session));
        return new Promise((resolve, reject) => {
            client.on("ready", () => {
                client.sftp((err, sftp) => {
                    if (err) {
                        reject(err);
                    } else {
                        ZSshUtils.uploadDir(sftp, localDir, serverPath.replace(/^~/, "."))
                            .then(resolve, reject)
                            .finally(() => client.end());
                    }
                });
            });
        });
    }

    public static async uninstallServer(session: SshSession, serverPath: string, localDir: string): Promise<void> {
        const client = new Client();
        client.connect(ZSshUtils.buildSshConfig(session));
        return new Promise((resolve, reject) => {
            client.on("ready", () => {
                client.sftp((err, sftp) => {
                    if (err) {
                        reject(err);
                    } else {
                        ZSshUtils.safeRemoveDir(sftp, localDir, serverPath.replace(/^~/, "."))
                            .then(resolve, reject)
                            .finally(() => client.end());
                    }
                });
            });
        });
    }

    private static async uploadDir(sftp: SFTPWrapper, localDir: string, remoteDir: string): Promise<void> {
        const _dirExists = await new Promise((resolve, reject) => {
            sftp.mkdir(remoteDir, { mode: 0o700 }, (err) => {
                if (err && (err as any).code !== 4) {
                    reject(err);
                } else {
                    resolve(err != null);
                }
            });
        });
        await Promise.all(
            fs.readdirSync(localDir).map((file) => {
                return new Promise<void>((resolve, reject) => {
                    sftp.fastPut(
                        path.join(localDir, file),
                        path.posix.join(remoteDir, file),
                        { mode: 0o700 },
                        (err) => {
                            if (err) {
                                reject(err);
                            } else {
                                resolve();
                            }
                        },
                    );
                });
            }),
        );
    }

    private static async safeRemoveDir(sftp: SFTPWrapper, localDir: string, remoteDir: string): Promise<void> {
        await Promise.all(
            fs.readdirSync(localDir).map((file) => {
                return new Promise<void>((resolve, reject) => {
                    sftp.unlink(path.posix.join(remoteDir, file), (err) => {
                        if (err) {
                            reject(err);
                        } else {
                            resolve();
                        }
                    });
                });
            }),
        );
        const _dirEmpty = await new Promise((resolve, reject) => {
            sftp.rmdir(remoteDir, (err) => {
                if (err && (err as any).code !== 4) {
                    reject(err);
                } else {
                    resolve(err == null);
                }
            });
        });
    }
}
