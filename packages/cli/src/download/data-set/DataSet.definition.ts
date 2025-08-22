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

export const DownloadDataSetDefinition: ICommandDefinition = {
    handler: `${__dirname}/DataSet.handler`,
    description: "Download the contents of a z/OS data set",
    type: "command",
    name: "data-set",
    aliases: ["ds"],
    summary: "Download data set content",
    examples: [
        {
            description: 'Download the content of the data set "ibmuser.cntl(iefbr14)"',
            options: '"ibmuser.cntl(iefbr14)"',
        },
        {
            description:
                'Download the content of the data set "ibmuser.loadlib(main)" and pipe it into the hex viewer program xxd',
            options: '"ibmuser.loadlib(main)" -b | xxd',
        },
    ],
    positionals: [
        {
            name: "dataSet",
            description:
                "The data set (PDS member or physical sequential data set) which you would like to download the contents of.",
            type: "string",
            required: true,
        },
    ],
    options: [
        {
            name: "binary",
            aliases: ["b"],
            description: "Download content in binary form without converting to ASCII text",
            type: "boolean",
        },
        {
            name: "encoding",
            aliases: ["ec"],
            description: "The desired encoding for the z/OS data set.",
            defaultValue: null,
            type: "string",
        },
        {
            name: "local-encoding",
            aliases: ["lec"],
            description: "The source encoding of the z/OS data set content (defaults to UTF-8).",
            defaultValue: null,
            type: "string",
        },
        {
            name: "directory",
            aliases: ["d"],
            description: "The directory for the downloaded data set.",
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
        {
            name: "volume-serial",
            aliases: ["vs"],
            description: "The volume serial (VOLSER) where the data set resides.",
            type: "string",
        },
    ],
    profile: { optional: ["ssh"] },
};
