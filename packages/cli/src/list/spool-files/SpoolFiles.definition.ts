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

export const ListSpoolFilesDefinition: ICommandDefinition = {
    handler: `${__dirname}/SpoolFiles.handler`,
    type: "command",
    name: "spool-files-by-job-id",
    aliases: ["sfbj", "job-spool-file"],
    summary: "List spool files by job id",
    description: "List all spool files for a given job id. ",
    positionals: [
        {
            name: "job-id",
            description: "The ID of the job which you would like to view the contents of.",
            type: "string",
            required: true,
        },
    ],
    examples: [
        {
            description: "List all spool files for TSU00403",
            options: "TSU00403",
        },
    ],
    profile: { optional: ["ssh"] },
    outputFormatOptions: true,
};
