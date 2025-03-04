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

import { readFileSync } from "node:fs";
import { homedir } from "node:os";
import * as path from "node:path";
import type { ISshSession } from "@zowe/zos-uss-for-zowe-sdk";
import * as sshConfig from "ssh-config";

export interface ISshConfigExt extends ISshSession {
    name?: string;
}
// biome-ignore lint/complexity/noStaticOnlyClass: Utilities class has static methods
export class ZClientUtils {
    public static async findPrivateKeys(): Promise<string[]> {
        const keyNames = ["id_ed25519", "id_rsa", "id_ecdsa", "id_dsa"];
        const privateKeyPaths: Set<string> = new Set();

        // Check standard ~/.ssh private keys
        for (const algo of keyNames) {
            const keyPath = path.resolve(homedir(), ".ssh", algo);
            try {
                if (readFileSync(keyPath)) privateKeyPaths.add(keyPath);
            } catch {
                // Ignore missing keys
            }
        }
        return Array.from(privateKeyPaths);
    }

    public static async migrateSshConfig(): Promise<ISshConfigExt[]> {
        const filePath = path.join(homedir(), ".ssh", "config");
        let fileContent: string;
        try {
            fileContent = readFileSync(filePath, "utf-8");
        } catch (error) {
            return [];
        }

        const parsedConfig = sshConfig.parse(fileContent);
        const SSHConfigs: ISshConfigExt[] = [];

        for (const config of parsedConfig) {
            if (config.type === sshConfig.LineType.DIRECTIVE) {
                const session: ISshConfigExt = {};
                session.name = config.value as string;

                if (Array.isArray((config as any).config)) {
                    for (const subConfig of (config as any).config) {
                        if (typeof subConfig === "object" && "param" in subConfig && "value" in subConfig) {
                            const param = (subConfig as any).param.toLowerCase();
                            const value = subConfig.value as string;

                            switch (param) {
                                case "hostname":
                                    session.hostname = value;
                                    break;
                                case "port":
                                    session.port = Number.parseInt(value);
                                    break;
                                case "user":
                                    session.user = value;
                                    break;
                                case "identityfile":
                                    session.privateKey = value;
                                    break;
                                case "connecttimeout":
                                    session.handshakeTimeout = Number.parseInt(value);
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
