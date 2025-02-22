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

export const DirectoryDefinition: ICommandDefinition = {
    handler: `${__dirname}/Directory.handler`,
    description: "Create a directory",
    type: "command",
    name: "directory",
    aliases: ["dir"],
    summary: "Create a directory",
    examples: [
        {
            description: "Create a directory",
            options: '"/u/users/yourname/newFolder"',
        },
    ],
    positionals: [
        {
            name: "path",
            description: "The full path of the new directory",
            type: "string",
            required: true,
        },
    ],
    options: [
        {
            name: "permissions",
            description: "The permissions for the new directory as an octal value (e.g. 755)",
            type: "string",
        },
    ],
    profile: { optional: ["ssh"] },
};
