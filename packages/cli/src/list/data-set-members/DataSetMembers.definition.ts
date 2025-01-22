/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0 & Apache-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

import type { ICommandDefinition } from "@zowe/imperative";

export const ListDataSetMembersDefinition: ICommandDefinition = {
    handler: __dirname + "/DataSetMembers.handler",
    type: "command",
    name: "data-set-members",
    aliases: ["members"],
    summary: "List PDS or PDSE data set members",
    description: "List all members of the specified PDS or PDSE data set.",
    examples: [
        {
            description: 'List all members in data set "ibmuser.test.cntl"',
            options: '"ibmuser.test.cntl"',
        },
    ],
    positionals: [
        {
            name: "dsname",
            description: "The PDS or PDSE data set name.",
            type: "string",
            required: true,
        },
    ],
    profile: { optional: ["ssh"] },
    outputFormatOptions: true,
};
