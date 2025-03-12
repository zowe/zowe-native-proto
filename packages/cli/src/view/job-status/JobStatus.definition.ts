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

export const ViewJobStatusDefinition: ICommandDefinition = {
    handler: `${__dirname}/JobStatus.handler`,
    description: "View the status of a job",
    type: "command",
    name: "job-status-by-job-id",
    aliases: ["js", "job-status"],
    summary: "View job status",
    examples: [
        {
            description: "View the status of the job TSU00296",
            options: "TSU00296",
        },
    ],
    positionals: [
        {
            name: "job-id",
            description: "The ID of the job which you would like to view the status of.",
            type: "string",
            required: true,
        },
    ],
    profile: { optional: ["ssh"] },
    outputFormatOptions: true,
};
