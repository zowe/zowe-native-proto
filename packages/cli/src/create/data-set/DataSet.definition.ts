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

export const DataSetDefinition: ICommandDefinition = {
    handler: `${__dirname}/DataSet.handler`,
    description: "Create a data set",
    type: "command",
    name: "data-set",
    aliases: ["ds"],
    summary: "Create a data set",
    examples: [
        {
            description: "Create a data set",
            options: '"ibmuser.test.cntl"',
        },
    ],
    positionals: [
        {
            name: "name",
            description: "The data set to create",
            type: "string",
            required: true,
        },
    ],
    profile: { optional: ["ssh"] },
};
