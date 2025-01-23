import { type SshSession, ZosUssProfile } from "@zowe/zos-uss-for-zowe-sdk";
import type { MainframeInteraction, imperative } from "@zowe/zowe-explorer-api";
import * as vscode from "vscode";
import { type ZSshClient, ZSshUtils } from "zowe-native-proto-sdk";
import { SshClientCache } from "../SshClientCache";

export class SshCommonApi implements MainframeInteraction.ICommon {
    public constructor(public profile?: imperative.IProfileLoaded) {}

    public getProfileTypeName(): string {
        return ZosUssProfile.type;
    }

    public getSession(profile?: imperative.IProfileLoaded): imperative.Session {
        return this.getSshSession(profile) as any;
    }

    public async getStatus(profile: imperative.IProfileLoaded, profileType?: string): Promise<string> {
        if (profileType === ZosUssProfile.type) {
            try {
                await SshClientCache.inst.connect(this.getSshSession(profile));
                return Promise.resolve("active");
            } catch (err) {
                vscode.window.showErrorMessage((err as Error).toString());
                return Promise.resolve("inactive");
            }
        }
        return Promise.resolve("unverified");
    }

    public get client(): Promise<ZSshClient> {
        return SshClientCache.inst.connect(this.getSshSession());
    }

    public getSshSession(profile?: imperative.IProfileLoaded): SshSession {
        return ZSshUtils.buildSession((profile ?? this.profile)?.profile!);
    }
}
