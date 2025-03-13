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
import { IO } from "@zowe/imperative";
import { B64String, type ZSshClient, type uss } from "zowe-native-proto-sdk";
import { SshBaseHandler } from "../../SshBaseHandler";
import * as fs from "node:fs";
import { homedir } from "node:os";
import path = require("node:path");

export default class DownloadUssFileHandler extends SshBaseHandler {
    public async processWithClient(params: IHandlerParameters, client: ZSshClient): Promise<uss.ReadFileResponse> {
        const response = await client.uss.readFile({
            fspath: params.arguments.filePath,
            encoding: params.arguments.binary ? "binary" : params.arguments.encoding,
        });
        const content = B64String.decode(response.data);
        const match = params.arguments.filePath.match(/[^/]+$/)?.[0] || "";
        const localFilePath: string = path.join(homedir(), match);

        console.log("Downloading USS file '%s' to local file '%s'", params.arguments.filePath, localFilePath);
        IO.createDirsSyncFromFilePath(localFilePath);
        fs.writeFileSync(localFilePath, content, params.arguments.binary ? "binary" : "utf8");
        params.response.data.setMessage("Successfully downloaded content to %s", localFilePath);

        return response;
    }
}
