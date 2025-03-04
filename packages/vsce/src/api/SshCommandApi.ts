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

import type { IConsoleResponse } from "@zowe/zos-console-for-zowe-sdk";
import type { IIssueResponse, IStartTsoParms } from "@zowe/zos-tso-for-zowe-sdk";
import type { SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import type { MainframeInteraction } from "@zowe/zowe-explorer-api";
import { SshCommonApi } from "./SshCommonApi";

export class SshCommandApi extends SshCommonApi implements MainframeInteraction.ICommand {
    public async issueTsoCommandWithParms?(command: string, parms?: IStartTsoParms): Promise<IIssueResponse> {
        throw new Error("Method not implemented.");
    }
    public async issueMvsCommand?(command: string, consoleName?: string): Promise<IConsoleResponse> {
        try {
            const user = this.profile?.profile?.user;
            const response = await (await this.client).cmds.issueConsole({
                commandText: command,
                consoleName: consoleName ?? (user ? `${user.slice(0, -2)}CN` : "ZOWE00CN"),
            });
            return {
                success: true,
                commandResponse: response.data,
                zosmfResponse: [{ "cmd-response": response.data }],
            };
        } catch (err) {
            return {
                success: false,
                commandResponse: `${err}`,
                zosmfResponse: [{ "cmd-response": `${err}` }],
            };
        }
    }
    public async issueUnixCommand?(command: string, cwd: string, sshSession?: SshSession): Promise<string> {
        throw new Error("Method not implemented.");
    }
    public sshProfileRequired?(): boolean {
        return true;
    }
}
