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

import { homedir } from "os";
import * as path from "node:path";
import * as fs from "fs";
import { ISshSession } from "@zowe/zos-uss-for-zowe-sdk";
import * as sshConfig from "ssh-config";

export interface sshConfigExt extends ISshSession {
    name?: string;
}
export class ZClientUtils{
    public static async findPrivateKey(privateKeyPath: string)
    {
        for (const algo of ["id_ed25519", "id_rsa"]) {
            const tempPath = path.resolve(homedir(), ".ssh", algo);
            if (fs.existsSync(tempPath)) {
                privateKeyPath = path.resolve(homedir(), ".ssh", algo);
                break;
            }
        }
        if (privateKeyPath == null) {
            throw Error("Failed to discover an ssh private key inside `~/.ssh`.");
        }
        return privateKeyPath;
    }
    public static async migrateSshConfig(): Promise<sshConfigExt[]> {
        const filePath = path.join(homedir(), '.ssh', 'config');
        let fileContent: string;
        try {
            fileContent = fs.readFileSync(filePath, "utf-8");
        } catch (error) {
            return [];
        }

        const parsedConfig = sshConfig.parse(fileContent);
        const SSHConfigs: sshConfigExt[] = [];

        for (const config of parsedConfig) {
            if (config.type === sshConfig.LineType.DIRECTIVE) {
                const session: sshConfigExt = {};
                session.name = (config as any).value;

                if (Array.isArray((config as any).config)) {
                    for (const subConfig of (config as any).config) {
                        if (typeof subConfig === 'object' && 'param' in subConfig && 'value' in subConfig) {
                            const param = (subConfig as any).param.toLowerCase();
                            const value = (subConfig as any).value;

                            switch (param) {
                                case 'hostname':
                                    session.hostname = value;
                                    break;
                                case 'port':
                                    session.port = parseInt(value);
                                    break;
                                case 'user':
                                    session.user = value;
                                    break;
                                case 'identityfile':
                                    session.privateKey = value;
                                    break;
                                case 'connecttimeout':
                                    session.handshakeTimeout = parseInt(value);
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                }
                SSHConfigs.push(session);
            }
        }
        return SSHConfigs;
    }
}