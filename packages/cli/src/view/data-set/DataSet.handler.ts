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
import { IReadDatasetRequest, IReadDatasetResponse, ZSshClient } from "zowe-native-proto-sdk";
import { SshBaseHandler } from "../../SshBaseHandler";

export default class ViewDataSetHandler extends SshBaseHandler {
    public async processWithSession(params: IHandlerParameters, session: SshSession): Promise<IReadDatasetResponse> {
        using client = await ZSshClient.create(session);
        const request: IReadDatasetRequest = {
            command: "readDataset",
            dataset: params.arguments.dataSet,
            // binary: params.arguments.binary,
            encoding: params.arguments.encoding
        };
        const response = await client.request<IReadDatasetResponse>(request);
        const content = Buffer.from(response.data as unknown as string, "base64").toString();
        params.response.data.setMessage("Successfully downloaded %d bytes of content from %s", content.length, params.arguments.dataSet);
        params.response.data.setObj(content);
        params.response.console.log(content);
        return response;
    }
}
