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

export const TsoCommand: ICommandDefinition = {
    handler: `${__dirname}/TsoCommand.handler`,
    description: "Issue a TSO command",
    type: "command",
    name: "tso-command",
    aliases: ["tso", "tso-cmd"],
    summary: "Issue a TSO command",
    examples: [
        {
            description: "Issue a TSO command",
            options: '"TIME"',
        },
    ],
    positionals: [
        {
            name: "command",
            description: "The command to issue.",
            type: "string",
            required: true,
        },
    ],
    options: [],
    profile: { optional: ["ssh"] },
};
