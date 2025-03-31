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

export const ConfigSetupDefinition: ICommandDefinition = {
    handler: `${__dirname}/Setup.handler`,
    type: "command",
    name: "setup",
    aliases: ["su"],
    summary: "Setup SSH Configuration",
    description:
        "Setup SSH Configuration from an SSH Command, an existing team configuration file, or a local SSH Config",
    examples: [],
    profile: { optional: ["ssh"] },
};
