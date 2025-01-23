/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0 & Apache-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

import type { IHandlerParameters } from "@zowe/imperative";
import { type ReadDataset, ZSshClient, ZSshUtils } from "zowe-native-proto-sdk";
import { SshBaseHandler } from "../../SshBaseHandler";

export default class ViewDataSetHandler extends SshBaseHandler {
    public async processWithClient(params: IHandlerParameters, client: ZSshClient): Promise<ReadDataset.Response> {
        const response = await client.ds.readDataset({
            dataset: params.arguments.dataSet,
            // binary: params.arguments.binary,
            encoding: params.arguments.encoding,
        });
        const content = ZSshUtils.decodeByteArray(response.data).toString();
        params.response.data.setMessage(
            "Successfully downloaded %d bytes of content from %s",
            content.length,
            params.arguments.dataSet,
        );
        params.response.data.setObj(content);
        params.response.console.log(content);
        return response;
    }
}
