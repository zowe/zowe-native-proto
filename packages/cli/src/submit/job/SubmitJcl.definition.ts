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

export const SubmitJclDefinition: ICommandDefinition = {
    handler: `${__dirname}/SubmitJcl.handler`,
    description: "Submit JCL contents",
    type: "command",
    name: "jcl",
    summary: "Submit JCL contents as a new job",
    examples: [
        {
            description: "Submit a job",
            options: '"//FIRSTJOB JOB (IZUACCT),CLASS=A,MSGLEVEL=(1,1),NOTIFY=&SYSUID"',
        },
    ],
    positionals: [
        {
            name: "jcl",
            description: "The JCL contents to submit as a new job",
            type: "string",
            required: true,
        },
    ],
    profile: { optional: ["ssh"] },
};
