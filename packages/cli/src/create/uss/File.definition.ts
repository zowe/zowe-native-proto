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

export const FileDefinition: ICommandDefinition = {
    handler: `${__dirname}/File.handler`,
    description: "Create a file",
    type: "command",
    name: "file",
    aliases: ["f"],
    summary: "Create a file",
    examples: [
        {
            description: "Create a file",
            options: '"/u/users/yourname/newFile.txt"',
        },
    ],
    positionals: [
        {
            name: "path",
            description: "The full path of the new file",
            type: "string",
            required: true,
        },
    ],
    options: [
        {
            name: "permissions",
            description: "The permissions for the new file as an octal value (e.g. 755)",
            type: "string",
        },
    ],
    profile: { optional: ["ssh"] },
};
