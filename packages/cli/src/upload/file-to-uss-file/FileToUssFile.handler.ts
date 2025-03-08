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

import * as fs from "node:fs";
import type { IHandlerParameters } from "@zowe/imperative";
import { type ZSshClient, ZSshUtils, type uss } from "zowe-native-proto-sdk";
import { SshBaseHandler } from "../../SshBaseHandler";

export default class UploadFileToUssFileHandler extends SshBaseHandler {
    public async processWithClient(params: IHandlerParameters, client: ZSshClient): Promise<uss.WriteFileResponse> {
        const data = ZSshUtils.encodeByteArray(fs.readFileSync(params.arguments.file));
        const response = await client.uss.writeFile({
            data,
            fspath: params.arguments.ussFile,
            encoding: params.arguments.binary ? "binary" : params.arguments.encoding,
        });
        const uploadSource: string = `local file '${params.arguments.file}'`;
        const successMsg = params.response.console.log(
            "Uploaded from %s to %s ",
            uploadSource,
            params.arguments.ussFile,
        );
        params.response.data.setMessage(successMsg);
        return response;
    }
}
