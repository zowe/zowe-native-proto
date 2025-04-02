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

export const ViewDataSetDefinition: ICommandDefinition = {
    handler: `${__dirname}/DataSet.handler`,
    description: "View the contents of a z/OS data set",
    type: "command",
    name: "data-set",
    aliases: ["ds"],
    summary: "View data set content",
    examples: [
        {
            description: 'View the content of the data set "ibmuser.cntl(iefbr14)"',
            options: '"ibmuser.cntl(iefbr14)"',
        },
        {
            description:
                'View the content of the data set "ibmuser.loadlib(main)" and pipe it into the hex viewer program xxd',
            options: '"ibmuser.loadlib(main)" -b | xxd',
        },
    ],
    positionals: [
        {
            name: "dataSet",
            description:
                "The data set (PDS member or physical sequential data set) which you would like to view the contents of.",
            type: "string",
            required: true,
        },
    ],
    options: [
        {
            name: "binary",
            aliases: ["b"],
            description: "View content in binary form without converting to ASCII text",
            type: "boolean",
        },
        {
            name: "encoding",
            aliases: ["ec"],
            description: "The encoding for viewing a z/OS data set.",
            defaultValue: null,
            type: "string",
        },
    ],
    profile: { optional: ["ssh"] },
};
