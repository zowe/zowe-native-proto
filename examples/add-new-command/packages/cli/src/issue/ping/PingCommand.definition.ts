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

export const PingCommand: ICommandDefinition = {
    handler: `${__dirname}/PingCommand.handler`,
    description: "Ping the server to test connectivity",
    type: "command",
    name: "ping",
    aliases: ["p"],
    summary: "Ping the server to test connectivity",
    examples: [
        {
            description: "Ping the server with default message",
            options: "",
        },
        {
            description: "Ping the server with custom message",
            options: '--message "Hello from CLI!"',
        },
    ],
    options: [
        {
            name: "message",
            aliases: ["m"],
            description: "Optional message to include in ping.",
            type: "string",
            required: false,
        },
    ],
    profile: { optional: ["ssh"] },
};
