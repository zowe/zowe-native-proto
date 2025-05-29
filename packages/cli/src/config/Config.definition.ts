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
import { ConfigSetupDefinition } from "./setup/Setup.definition";

const ConfigDefinition: ICommandDefinition = {
    name: "config",
    aliases: ["cfg"],
    summary: "Manage Configurations",
    description: "Manage Configurations of SSH Profiles",
    type: "group",
    children: [ConfigSetupDefinition],
};

export = ConfigDefinition;
