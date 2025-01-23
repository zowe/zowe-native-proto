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

export const ViewJobJclDefinition: ICommandDefinition = {
    handler: `${__dirname}/JobJcl.handler`,
    description: "View the JCL contents of a job",
    type: "command",
    name: "job-jcl",
    aliases: ["jj"],
    summary: "View job JCL",
    examples: [
        {
            description: "View the JCL contents of the job TSU00296",
            options: "TSU00296",
        },
    ],
    positionals: [
        {
            name: "jobId",
            description: "The ID of the job which you would like to view the JCL of.",
            type: "string",
            required: true,
        },
    ],
    profile: { optional: ["ssh"] },
};
