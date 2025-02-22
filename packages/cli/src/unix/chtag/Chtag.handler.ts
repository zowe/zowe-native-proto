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

export const ChtagDefinition: ICommandDefinition = {
    handler: `${__dirname}/Chtag.handler`,
    description: "Change the tag of a UNIX file or directory",
    type: "command",
    name: "chtag",
    aliases: ["tag"],
    summary: "Change the tag of a UNIX file or directory",
    examples: [
        {
            description: "Change the tag of a UNIX file to ISO8859-1",
            options: '"819 /path/to/file.txt"',
        },
        {
            description: "Change the tag of a UNIX directory to EBCDIC",
            options: '"1047 /path/to/directory"',
        },
        {
            description: "Change the tag of a UNIX directory",
            options: '"819 /path/to/directory" --recursive',
        },
        {
            description: "Change the tag of all files/folders in a UNIX directory",
            options: '"939 /path/to/directory --recursive"',
        },
    ],
    positionals: [
        {
            name: "tag",
            description: "The tag to set",
            type: "string",
            required: true,
        },
        {
            name: "path",
            description: "The UNIX file or directory to change the tag of",
            type: "string",
            required: true,
        },
    ],
    options: [
        {
            name: "recursive",
            description: "Change the tag for all inner files and directories",
            type: "boolean",
            required: false,
        },
    ],
    profile: { optional: ["ssh"] },
};
