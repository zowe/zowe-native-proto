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
    summary: "Create a data set with attributes",
    examples: [
        {
            description: "Create a data set with attributes",
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
    options: [
        {
            name: "template",
            description: "Template to use the default attributes of. Ex: PARTITIONED (DEFAULT), SEQUENTIAL, CLASSIC, C, BINARY",
            type: "string",
        },
        {
            name: "alcunit",
            description: "Allocation Unit",
            type: "string",
        },
        {
            name: "blksize",
            description: "Block Size",
            type: "number",
        },
        {
            name: "dirblk",
            description: "Directory Blocks",
            type: "number",
        },
        {
            name: "dsorg",
            description: "Data Set Organization",
            type: "string",
        },
        {
            name: "primary",
            description: "Primary Space",
            type: "number",
        },
        {
            name: "recfm",
            description: "Record Format",
            type: "string",
        },
        {
            name: "lrecl",
            description: "Record Length",
            type: "number",
        },
        {
            name: "dataclass",
            description: "Data Class",
            type: "string",
        },
        {
            name: "dev",
            description: "Device Type",
            type: "string",
        },
        {
            name: "dsntype",
            description: "Data Set Type",
            type: "string",
        },
        {
            name: "mgntclass",
            description: "Management Class",
            type: "string",
        },
        {
            name: "dsname",
            description: "Data Set Name",
            type: "string",
        },
        {
            name: "avgblk",
            description: "Average Block Length",
            type: "number",
        },
        {
            name: "secondary",
            description: "Secondary Space",
            type: "number",
        },
        {
            name: "size",
            description: "Size",
            type: "number",
        },
        {
            name: "storclass",
            description: "Storage Class",
            type: "string",
        },
        {
            name: "vol",
            description: "Volume Serial",
            type: "string",
        },
    ],
    profile: { optional: ["ssh"] },
};
