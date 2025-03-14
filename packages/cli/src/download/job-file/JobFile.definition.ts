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

export const DownloadJobFileDefinition: ICommandDefinition = {
    handler: `${__dirname}/JobFile.handler`,
    description: "Download the contents of a job spool file",
    type: "command",
    name: "spool-files-by-id",
    aliases: ["jsf", "spool", "job-spool-file"],
    summary: "Download job spool file",
    examples: [
        {
            description: "Download the contents of a job spool file",
            options: "TSU00296 2",
        },
    ],
    positionals: [
        {
            name: "job-id",
            description: "The ID of the job which you would like to download the contents of.",
            type: "string",
            required: true,
        },
        {
            name: "dsn-key",
            description: "The key of the job which you would like to download the contents of.",
            type: "number",
            required: true,
        },
    ],
    options: [
        {
            name: "encoding",
            description: "The encoding for the job spool file.",
            type: "string",
            required: false,
        },
        {
            name: "directory",
            aliases: ["d"],
            description: "The directory for the downloaded job spool file.",
            defaultValue: null,
            type: "string",
        },
    ],
    profile: { optional: ["ssh"] },
};
