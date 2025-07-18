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

import type { ICommandDefinition } from "@zowe/imperative";
import { SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { Constants } from "../Constants";
import { ConsoleCommand } from "./console/ConsoleCommand.definition";
import { PingCommand } from "./ping/PingCommand.definition";

const IssueDefinition: ICommandDefinition = {
    name: "issue",
    summary: "Issue commands",
    description: "Issue console, TSO, or Unix commands",
    type: "group",
    aliases: ["i"],
    children: [ConsoleCommand, PingCommand],
    passOn: [
        {
            property: "options",
            value: [...SshSession.SSH_CONNECTION_OPTIONS, Constants.OPT_SERVER_PATH],
            merge: true,
            ignoreNodes: [{ type: "group" }],
        },
    ],
};

export = IssueDefinition;
