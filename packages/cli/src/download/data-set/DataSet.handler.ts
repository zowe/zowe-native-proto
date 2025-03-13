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
import { homedir } from "node:os";
import type { IHandlerParameters } from "@zowe/imperative";
import { IO } from "@zowe/imperative";
import { B64String, type ZSshClient, type ds } from "zowe-native-proto-sdk";
import { SshBaseHandler } from "../../SshBaseHandler";
import path = require("node:path");

export default class DownloadDataSetHandler extends SshBaseHandler {
    public async processWithClient(params: IHandlerParameters, client: ZSshClient): Promise<ds.ReadDatasetResponse> {
        const response = await client.ds.readDataset({
            dsname: params.arguments.dataSet,
            encoding: params.arguments.binary ? "binary" : params.arguments.encoding,
        });

        const content = B64String.decode(response.data);
        const match = params.arguments.dataSet.match(/\(([^)]+)\)/);
        const localFilePath: string = path.join(homedir(), match ? match[1] : params.arguments.dataSet);

        console.log("Downloading data set '%s' to local file '%s'", params.arguments.dataSet, localFilePath);
        IO.createDirsSyncFromFilePath(localFilePath);
        fs.writeFileSync(localFilePath, content, params.arguments.binary ? "binary" : "utf8");
        params.response.data.setMessage("Successfully downloaded content to %s", localFilePath);

        return response;
    }
}
