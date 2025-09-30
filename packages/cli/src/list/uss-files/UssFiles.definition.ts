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

export const ListUssFilesDefinition: ICommandDefinition = {
    handler: `${__dirname}/UssFiles.handler`,
    type: "command",
    name: "uss-files",
    aliases: ["uf", "uss"],
    summary: "List USS files in a directory",
    description:
        "List USS files and subdirectories in a directory. Optional file name pattern like " +
        '"prefix*", "*suffix", or "prefix*suffix" can be specified at the end of directory. ' +
        "See EXAMPLES section.\n\n" +
        "The following values can be used with the --response-format-filter (--rff) argument to " +
        "display more data from the data sets: " +
        "name, size, owner, group, and permissions.",
    examples: [
        {
            description: 'List USS files in the directory "/u/users/ibmuser/"',
            options: '"/u/users/ibmuser"',
        },
        {
            description: 'List USS files with suffix of ".txt" in the directory "/u/users/ibmuser/"',
            options: '"/u/users/ibmuser/*.txt"',
        },
        {
            description: 'List USS files in the directory "/u/users/ibmuser/" and show only the file name',
            options: '"/u/users/ibmuser/" --rff name',
        },
        {
            description: 'List all USS files (including hidden files) in the directory "/u/users/ibmuser/"',
            options: '"/u/users/ibmuser/" --all',
        },
        {
            description:
                'List USS files in long format, showing detailed information in the directory "/u/users/ibmuser/"',
            options: '"/u/users/ibmuser/" --long',
        },
        {
            description: 'List all USS files in long format in the directory "/u/users/ibmuser/"',
            options: '"/u/users/ibmuser/" --all --long',
        },
    ],
    options: [
        {
            name: "all",
            description: "Show all files",
            type: "boolean",
            required: false,
            aliases: ["a"],
        },
        {
            name: "long",
            description: "Show long format",
            type: "boolean",
            required: false,
            aliases: ["l"],
        },
        {
            name: "depth",
            description: "Depth of subdirectories to list",
            type: "number",
            required: false,
            aliases: ["d"],
            defaultValue: 1,
        },
    ],
    positionals: [
        {
            name: "directory",
            description: "The USS directory to list files in, or the directory with file name pattern",
            type: "string",
            required: true,
        },
    ],
    profile: { optional: ["ssh"] },
    outputFormatOptions: true,
};
