/*
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

import { IHandlerParameters } from "@zowe/imperative";
import { SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { IListDatasetRequest, IListDatasetResponse, ZSshClient } from "zowe-native-proto-sdk";
import { SshBaseHandler } from "../../SshBaseHandler";

export default class ListDataSetsHandler extends SshBaseHandler {
    public async processWithSession(params: IHandlerParameters, session: SshSession): Promise<IListDatasetResponse> {
        using client = await ZSshClient.create(session);
        const request: IListDatasetRequest = { command: "listDataset", pattern: params.arguments.pattern };
        const response = await client.request<IListDatasetResponse>(request);
        params.response.data.setMessage("Successfully listed %d matching data sets for pattern '%s'",
            response.returnedRows, params.arguments.pattern);
        params.response.format.output({
            output: response.items,
            format: "table",
            fields: ["name", "dsorg"]
        });
        return response;
    }
}
