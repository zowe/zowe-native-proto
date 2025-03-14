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
import { B64String, type ZSshClient, type jobs } from "zowe-native-proto-sdk";
import { SshBaseHandler } from "../../SshBaseHandler";
import path = require("node:path");

export default class DownloadJobJclHandler extends SshBaseHandler {
    public async processWithClient(params: IHandlerParameters, client: ZSshClient): Promise<jobs.ReadSpoolResponse> {
        const response = await client.jobs.readSpool({
            spoolId: params.arguments.dsnKey,
            jobId: params.arguments.jobId,
            encoding: params.arguments.encoding,
        });
        const content = B64String.decode(response.data);
        const localFilePath: string = path.join(
            params.arguments.directory ?? process.cwd(),
            `${params.arguments.jobId}.txt`,
        );

        params.response.console.log(
            "Downloading spool '%s' from job ID '%s' to local file '%s'",
            params.arguments.dsnKey,
            params.arguments.jobId,
            localFilePath,
        );
        IO.createDirsSyncFromFilePath(localFilePath);
        fs.writeFileSync(localFilePath, content, params.arguments.binary ? "binary" : "utf8");
        params.response.data.setMessage("Successfully downloaded content to %s", localFilePath);
        return response;
    }
}
