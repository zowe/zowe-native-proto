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

export const SubmitUssDefinition: ICommandDefinition = {
    handler: `${__dirname}/Uss.handler`,
    description: "Submit a job from JCL written in a USS file.",
    type: "command",
    name: "uss-file",
    aliases: ["uss"],
    summary: "Submit a job from USS files",
    examples: [
        {
            description: 'Submit a job from the USS file "my_jcl.txt"',
            options: '"my_jcl.txt"',
        },
    ],
    positionals: [
        {
            name: "fspath",
            description: "The USS path to the JCL to submit",
            type: "string",
            required: true,
        },
    ],
    profile: { optional: ["ssh"] },
    outputFormatOptions: true,
};
