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

export const JobDefinition: ICommandDefinition = {
    handler: `${__dirname}/Job.handler`,
    description: "Cancel a job",
    type: "command",
    name: "job",
    aliases: ["j"],
    summary: "Cancel a job",
    examples: [
        {
            description: "Cancel a job",
            options: '"TSU00369"',
        },
    ],
    positionals: [
        {
            name: "job-id",
            description: "The ID of the job to cancel",
            type: "string",
            required: true,
        },
    ],
    profile: { optional: ["ssh"] },
};
