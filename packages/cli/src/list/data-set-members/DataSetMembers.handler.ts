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
import type { ListDsMembers, ZSshClient } from "zowe-native-proto-sdk";
import { SshBaseHandler } from "../../SshBaseHandler";

export default class ListDataSetMembersHandler extends SshBaseHandler {
    public async processWithClient(params: IHandlerParameters, client: ZSshClient): Promise<ListDsMembers.Response> {
        const response = await client.ds.listDsMembers({
            dataset: params.arguments.dsname,
        });
        params.response.data.setMessage(
            "Successfully listed %d members in data sets %s",
            response.returnedRows,
            params.arguments.dsname,
        );
        params.response.format.output({
            output: response.items,
            format: "table",
            fields: ["name"],
        });
        return response;
    }
}
