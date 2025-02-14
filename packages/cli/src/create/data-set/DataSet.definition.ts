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
    description: "Create a data set or data set member",
    type: "command",
    name: "data-set",
    aliases: ["ds"],
    summary: "Create a data set or data set member",
    examples: [
        {
            description: "Create a data set",
            options: '"ibmuser.test.cntl"',
        },
        {
            description: "Create a data set member",
            options: '"ibmuser.test.cntl(member)"',
        },
    ],
    positionals: [
        {
            name: "name",
            description: "The name of the data set to create",
            type: "string",
            required: true,
        },
    ],
    profile: { optional: ["ssh"] },
    options: [
        // {
        //     name: "type",
        //     description: "The type of the data set. Required for creating a data set",
        //     type: "string",
        //     allowableValues: { values: ["PDS", "PDSE"], caseSensitive: false },
        // },
        // {
        //     name: "record-format",
        //     description: "The record format of the data set. Required for creating a data set",
        //     type: "string",
        //     allowableValues: { values: ["FB", "VB", "FS", "VS", "U"], caseSensitive: false },
        // },
        // {
        //     name: "block-size",
        //     description: "The block size of the data set. Required for creating a data set",
        //     type: "number",
        // },
        // {
        //     name: "record-length",
        //     description: "The record length of the data set. Required for creating a data set",
        //     type: "number",
        // },
    ],
};
