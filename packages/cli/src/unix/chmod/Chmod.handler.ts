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

export const ChmodDefinition: ICommandDefinition = {
    handler: `${__dirname}/Chmod.handler`,
    description: "Change the permissions of a UNIX file or directory",
    type: "command",
    name: "chmod",
    aliases: ["mod"],
    summary: "Change the permissions of a UNIX file or directory",
    examples: [
        {
            description: "Change the permissions of a UNIX file",
            options: '"755 /path/to/file.txt"',
        },
        {
            description: "Change the permissions of a UNIX directory",
            options: '"644 /path/to/directory"',
        },
        {
            description: "Change the permissions of all files/folders in a UNIX directory",
            options: '"755 /path/to/directory --recursive"',
        },
    ],
    positionals: [
        {
            name: "mode",
            description: 'The permissions to set (represented as an octal number, e.g. "755")',
            type: "string",
            required: true,
        },
        {
            name: "path",
            description: "The UNIX file or directory to change the permissions of",
            type: "string",
            required: true,
        },
    ],
    options: [
        {
            name: "recursive",
            description: "Change the permissions of all inner files and directories",
            type: "boolean",
            required: false,
        },
    ],
    profile: { optional: ["ssh"] },
};
