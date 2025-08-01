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
import { B64String, type ZSshClient, type uss } from "zowe-native-proto-sdk";
import { SshBaseHandler } from "../../SshBaseHandler";

export default class ViewUssFileHandler extends SshBaseHandler {
    public async processWithClient(params: IHandlerParameters, client: ZSshClient): Promise<uss.ReadFileResponse> {
        let encoding = params.arguments.encoding;
        const binary = params.arguments.binary;
        if (encoding == null && binary == null) {
            try {
                const fileResp = await client.uss.listFiles({
                    fspath: params.arguments.filePath,
                    all: true,
                    long: true,
                });
                if (fileResp.success && fileResp.items.length > 0) {
                    const file = fileResp.items[0];
                    encoding = file.filetag;
                }
            } catch (error) {
                params.response.console.error(
                    "Failed to auto-detect file encoding for %s: %s",
                    params.arguments.filePath,
                    error,
                );
            }
        }

        const response = await client.uss.readFile({
            fspath: params.arguments.filePath,
            encoding: binary ? "binary" : encoding,
        });
        const content = B64String.decode(response.data);
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
