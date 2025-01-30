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
import { SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { DataSetDefinition } from "./data-set/DataSet.definition";

const RestoreDefinition: ICommandDefinition = {
    name: "restore",
    summary: "Restore datasets",
    description: "Restore datasets that have been archived/migrated",
    type: "group",
    aliases: ["r"],
    children: [DataSetDefinition],
    passOn: [
        {
            property: "options",
            value: SshSession.SSH_CONNECTION_OPTIONS,
            merge: true,
            ignoreNodes: [{ type: "group" }],
        },
    ],
};

export = RestoreDefinition;
