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
import { type ReadFile, type ZSshClient, ZSshUtils } from "zowe-native-proto-sdk";
import { SshBaseHandler } from "../../SshBaseHandler";

export default class ViewUssFileHandler extends SshBaseHandler {
    public async processWithClient(params: IHandlerParameters, client: ZSshClient): Promise<ReadFile.Response> {
        const response = await client.uss.readFile({
            path: params.arguments.filePath,
            // binary: params.arguments.binary,
            encoding: params.arguments.binary ? "binary" : params.arguments.encoding,
        });
        const content = ZSshUtils.decodeByteArray(response.data).toString();
        params.response.data.setMessage(
            "Successfully downloaded %d bytes of content from %s",
            content.length,
            params.arguments.filePath,
        );
        params.response.data.setObj(content);
        params.response.console.log(content);
        return response;
    }
}
