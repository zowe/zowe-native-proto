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

export const ChownDefinition: ICommandDefinition = {
    handler: `${__dirname}/Chown.handler`,
    description: "Change the owner of a UNIX file or directory",
    type: "command",
    name: "chown",
    aliases: ["own"],
    summary: "Change the owner (or owner:group) of a UNIX file or directory",
    examples: [
        {
            description: "Change the owner of a UNIX file",
            options: '"bob /path/to/file.txt"',
        },
        {
            description: "Change the owner and group of a UNIX file",
            options: '"bob:users /path/to/file.txt"',
        },
        {
            description: "Change the owner of a UNIX directory",
            options: '"root /path/to/directory"',
        },
        {
            description: "Change the owner of all files/folders in a UNIX directory",
            options: '"bob /path/to/directory --recursive"',
        },
    ],
    positionals: [
        {
            name: "owner",
            description: "The owner (or owner:group) to set",
            type: "string",
            required: true,
        },
        {
            name: "path",
            description: "The UNIX file or directory to change the owner of",
            type: "string",
            required: true,
        },
    ],
    options: [
        {
            name: "recursive",
            description: "Change the owner of all inner files and directories",
            type: "boolean",
            required: false,
        },
    ],
    profile: { optional: ["ssh"] },
};
