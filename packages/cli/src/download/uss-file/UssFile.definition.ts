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

export const DownloadUssFileDefinition: ICommandDefinition = {
    handler: `${__dirname}/UssFile.handler`,
    description: "Download the contents of a USS file",
    type: "command",
    name: "uss-file",
    aliases: ["uf", "uss"],
    summary: "Download USS file content",
    examples: [
        {
            description: 'Download the content of the USS file "/u/tsu/jcl/test"',
            options: '"/u/tsu/jcl/test"',
        },
        {
            description:
                'Download the content of the USS file "/u/tsu/jcl/test" and pipe it into the hex viewer program xxd',
            options: '"/u/tsu/jcl/test" -b | xxd',
        },
    ],
    positionals: [
        {
            name: "file-path",
            description: "The USS file which you would like to download the contents of.",
            type: "string",
            required: true,
        },
    ],
    options: [
        {
            name: "binary",
            aliases: ["b"],
            description: "Download content in binary form without any conversions",
            type: "boolean",
        },
        {
            name: "encoding",
            aliases: ["ec"],
            description: "The encoding for the USS file.",
            defaultValue: null,
            type: "string",
        },
        {
            name: "source-encoding",
            aliases: ["sec"],
            description: "The source encoding of the USS file content (defaults to UTF-8).",
            defaultValue: null,
            type: "string",
        },
        {
            name: "directory",
            aliases: ["d"],
            description: "The directory for the downloaded USS file.",
            defaultValue: null,
            type: "string",
        },
        {
            name: "file",
            aliases: ["f"],
            description: "The full file path for the downloaded content.",
            defaultValue: null,
            type: "string",
            conflictsWith: ["directory"],
        },
    ],
    profile: { optional: ["ssh"] },
};
