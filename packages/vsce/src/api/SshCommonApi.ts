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

import { type SshSession, ZosUssProfile } from "@zowe/zos-uss-for-zowe-sdk";
import { type imperative, type MainframeInteraction, ZoweExplorerApiType } from "@zowe/zowe-explorer-api";
import { type ZSshClient, ZSshUtils } from "zowe-native-proto-sdk";
import { SshClientCache } from "../SshClientCache";
import { SshErrorHandler } from "../SshErrorHandler";

export class SshCommonApi implements MainframeInteraction.ICommon {
    public constructor(public profile?: imperative.IProfileLoaded) {}

    public getProfileTypeName(): string {
        return ZosUssProfile.type;
    }

    public getSession(profile?: imperative.IProfileLoaded): imperative.Session {
        return this.getSshSession(profile) as unknown as imperative.Session;
    }

    public async getStatus(profile: imperative.IProfileLoaded, profileType?: string): Promise<string> {
        if (profileType === ZosUssProfile.type) {
            try {
                await SshClientCache.inst.connect(profile);
                return "active";
            } catch (err) {
                await SshErrorHandler.handleError(
                    err as Error,
                    ZoweExplorerApiType.All,
                    "SSH connection status check failed",
                    false,
                );
                return "inactive";
            }
        }
        return "unverified";
    }

    public get client(): Promise<ZSshClient> {
        if (this.profile == null) {
            throw new Error("Failed to create SSH client: no profile found");
        }
        return SshClientCache.inst.connect(this.profile);
    }

    public getSshSession(profile?: imperative.IProfileLoaded): SshSession {
        return ZSshUtils.buildSession((profile ?? this.profile)?.profile!);
    }
}
