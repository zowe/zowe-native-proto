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

export const CopyDefinition: ICommandDefinition = {
    handler: `${__dirname}/Copy.handler`,
    description: "Copy a UNIX file or directory to a UNIX destination",
    type: "command",
    name: "copy",
    aliases: ["cp"],
    summary: "Copy a UNIX file or directory",
    examples: [
        {
            description: "Copy a file",
            options: '"/path/to/src.txt" "/path/to/dst.txt"',
        },
        {
            description: "Copy a directory",
            options: '"/path/name" "/other_path" --recursive',
        },
        {
            description: "Copy a file to a directory",
            options: '"/path/to/src.txt" "/other_path"',
        },
        {
            description: "Copy a directory and follow symlinks",
            options: '"/path/name" "/other_path" --recursive --follow-symlinks',
        },
        {
            description: "Copy a file and preserve file attributes",
            options: '"/path/to/src.txt" "/path/to/dst.txt" --preserve-attributes',
        },
        {
            description: "Copy a file, and remove/replace target files without prompts.",
            options: '"/path/to/src.txt" "/path/to/dst.txt" --force',
        },
    ],
    positionals: [
        {
            name: "srcFsPath",
            description: "The source UNIX file or directory to be copied",
            type: "string",
            required: true,
        },
        {
            name: "dstFsPath",
            description: "The target UNIX file or directory to copy to",
            type: "string",
            required: true,
        },
    ],
    options: [
        {
            name: "recursive",
            description: "Copy directory and sub-directory contents. Required when copying directories.",
            type: "boolean",
            required: false,
        },
        {
            name: "followSymlinks",
            description: "Copy symlink contents rather than the symlink file itself. Requires --recursive.",
            type: "boolean",
            required: false,
        },
        {
            name: "preserveAttributes",
            description:
                "Preserves the modification time, access times, file mode, file format, owner, group owner, and extended attributes of copied files. Retaining some of these attributes may require additional authorizations.",
            type: "boolean",
            required: false,
        },
        {
            name: "force",
            description: "For each destination path, attempts to remove and replace the file.",
            type: "boolean",
            required: false,
        },
    ],
    profile: { optional: ["ssh"] },
};
