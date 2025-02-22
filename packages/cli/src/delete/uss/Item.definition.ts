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

export const ItemDefinition: ICommandDefinition = {
    handler: `${__dirname}/Item.handler`,
    description: "Delete a UNIX file or directory",
    type: "command",
    name: "uss",
    aliases: ["uf", "ud", "uss-file", "uss-dir"],
    summary: "Delete a UNIX file or directory",
    examples: [
        {
            description: "Delete a file",
            options: '"/path/to/file.txt"',
        },
        {
            description: "Delete a directory",
            options: '"/path/to/directory"',
        },
        {
            description: "Delete a directory recursively",
            options: '"/path/to/directory" --recursive',
        },
    ],
    positionals: [
        {
            name: "path",
            description: "The path to the file or directory to delete",
            type: "string",
            required: true,
        },
    ],
    options: [
        {
            name: "recursive",
            description: "Delete the directory even if it is not empty",
            type: "boolean",
            required: false,
            aliases: ["r"],
        },
    ],
    profile: { optional: ["ssh"] },
};
