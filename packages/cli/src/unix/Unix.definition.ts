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
import { ChmodDefinition } from "./chmod/Chmod.definition";
import { ChownDefinition } from "./chown/Chown.definition";
import { ChtagDefinition } from "./chtag/Chtag.definition";

const UnixDefinition: ICommandDefinition = {
    name: "unix",
    aliases: ["uss"],
    summary: "UNIX-specific features",
    description: "UNIX-specific features",
    type: "group",
    children: [ChmodDefinition, ChownDefinition, ChtagDefinition],
    passOn: [
        {
            property: "options",
            value: [...SshSession.SSH_CONNECTION_OPTIONS, Constants.OPT_SERVER_PATH],
            merge: true,
            ignoreNodes: [{ type: "group" }],
        },
    ],
};

export = UnixDefinition;
