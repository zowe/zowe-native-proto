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

export const RenameMemberDefinition: ICommandDefinition = {
    handler: `${__dirname}/DataSetMembers.handler`,
    description: "Rename a PDS/PDSE member",
    type: "command",
    name: "data-set-member",
    aliases: ["member"],
    summary: "Rename a data set member",
    examples: [
        {
            description: "Rename a member",
            options: '"ibmuser.test.seq.cntl" "ibmuser.test.seq.cntl2"',
        },
    ],
    positionals: [
        {
            name: "dsname",
            description: "The data set name",
            type: "string",
            required: true,
        },
        {
            name: "memberBefore",
            description: "The new name for the member",
            type: "string",
            required: true,
        },
        {
            name: "memberAfter",
            description: "The new name for the member",
            type: "string",
            required: true,
        },
    ],
    profile: { optional: ["ssh"] },
};
