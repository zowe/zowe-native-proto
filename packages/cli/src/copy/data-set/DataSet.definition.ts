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

export const CopyDataSetDefinition: ICommandDefinition = {
    handler: `${__dirname}/DataSet.handler`,
    description:
        "Copy a data set or member to another data set or member. " +
        "Supports PDS-to-PDS, member-to-member, and sequential-to-sequential copies. " +
        "Note: RECFM=U data sets are not supported.",
    type: "command",
    name: "data-set",
    aliases: ["ds"],
    summary: "Copy a data set",
    examples: [
        {
            description: "Copy a sequential data set to a new sequential data set",
            options: '"ibmuser.source.seq" "ibmuser.target.seq"',
        },
        {
            description: "Copy a PDS to a new PDS (copies all members)",
            options: '"ibmuser.source.pds" "ibmuser.target.pds"',
        },
        {
            description: "Copy a single member to another PDS",
            options: '"ibmuser.source.pds(member)" "ibmuser.target.pds(member)"',
        },
        {
            description: "Copy a PDS and replace existing members in the target",
            options: '"ibmuser.source.pds" "ibmuser.target.pds" --replace',
        },
        {
            description: "Copy a PDS and delete all target members before copying (makes target match source exactly)",
            options: '"ibmuser.source.pds" "ibmuser.target.pds" --delete-target-members',
        },
    ],
    positionals: [
        {
            name: "fromDataset",
            description: "The source data set to copy from (can include member name in parentheses)",
            type: "string",
            required: true,
        },
        {
            name: "toDataset",
            description: "The target data set to copy to (can include member name in parentheses)",
            type: "string",
            required: true,
        },
    ],
    options: [
        {
            name: "replace",
            aliases: ["r"],
            description:
                "Replace existing data. For PDS-to-PDS: replaces matching members, preserves target-only members. " +
                "For sequential or member-to-member: overwrites the target.",
            type: "boolean",
            defaultValue: false,
        },
        {
            name: "delete-target-members",
            aliases: ["d"],
            description:
                "Delete all members from target PDS before copying (PDS-to-PDS copy only). " +
                "Makes the target match the source exactly.",
            type: "boolean",
            defaultValue: false,
        },
    ],
    profile: { optional: ["ssh"] },
};
