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

import type { IHandlerParameters } from "@zowe/imperative";
import type { ListDatasets, ZSshClient } from "zowe-native-proto-sdk";
import { SshBaseHandler } from "../../SshBaseHandler";

export default class ListDataSetsHandler extends SshBaseHandler {
    public async processWithClient(params: IHandlerParameters, client: ZSshClient): Promise<ListDatasets.Response> {
        const response = await client.ds.listDatasets({
            pattern: params.arguments.pattern,
        });
        params.response.data.setMessage(
            "Successfully listed %d matching data sets for pattern '%s'",
            response.returnedRows,
            params.arguments.pattern,
        );
        params.response.format.output({
            output: response.items,
            format: "table",
            fields: ["name", "dsorg", "volser"],
        });
        return response;
    }
}
