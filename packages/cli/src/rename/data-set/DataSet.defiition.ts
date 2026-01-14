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

export const RenameDataSetDefinition: ICommandDefinition = {
    handler: `${__dirname}/DataSet.handler`,
    description: "Rename a sequential or partitioned data set",
    type: "command",
    name: "data-set",
    aliases: ["ds"],
    summary: "Rename a data set",
    examples: [
        {
            description: "Rename a sequential data set",
            options: '"ibmuser.test.seq.cntl"',
        },
        {
            description: "Rename a partitioned data set",
            options: '"ibmuser.test.pds.cntl"',
        },
    ],
    positionals: [
        {
            name: "dsnameBefore",
            description: "The data set to rename",
            type: "string",
            required: true,
        },
        {
            name: "dsnameAfter",
            description: "The new name for the data set",
            type: "string",
            required: true,
        },
    ],
    profile: { optional: ["ssh"] },
};
