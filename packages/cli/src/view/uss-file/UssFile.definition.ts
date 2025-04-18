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

export const ViewUssFileDefinition: ICommandDefinition = {
    handler: `${__dirname}/UssFile.handler`,
    description: "View the contents of a USS file",
    type: "command",
    name: "uss-file",
    aliases: ["uf", "uss"],
    summary: "View USS file content",
    examples: [
        {
            description: 'View the content of the USS file "/u/tsu/jcl/test"',
            options: '"/u/tsu/jcl/test"',
        },
        {
            description:
                'View the content of the USS file "/u/tsu/jcl/test" and pipe it into the hex viewer program xxd',
            options: '"/u/tsu/jcl/test" -b | xxd',
        },
    ],
    positionals: [
        {
            name: "file-path",
            description: "The USS file which you would like to view the contents of.",
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
            description: "The encoding for viewing a USS file.",
            defaultValue: null,
            type: "string",
        },
    ],
    profile: { optional: ["ssh"] },
};
