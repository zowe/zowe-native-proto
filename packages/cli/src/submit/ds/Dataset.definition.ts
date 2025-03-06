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

export const SubmitDsDefinition: ICommandDefinition = {
    handler: `${__dirname}/Dataset.handler`,
    description: "Submit a job from JCL written in a dataset.",
    type: "command",
    name: "data-set",
    aliases: ["ds"],
    summary: "Submit a job from a dataset",
    examples: [
        {
            description: 'Submit a job from the dataset "IBMUSER.PUBLIC.CNTL(IEFBR14)"',
            options: '"IBMUSER.PUBLIC.CNTL(IEFBR14)"',
        },
    ],
    positionals: [
        {
            name: "name",
            description: "The dataset containing the JCL to submit",
            type: "string",
            required: true,
        },
    ],
    profile: { optional: ["ssh"] },
    outputFormatOptions: true,
};
