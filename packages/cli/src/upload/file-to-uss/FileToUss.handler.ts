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
import { type ITaskWithStatus, TaskStage } from "@zowe/imperative";
import type { ZSshClient, uss } from "zowe-native-proto-sdk";
import { SshBaseHandler } from "../../SshBaseHandler";

export default class UploadFileToUssFileHandler extends SshBaseHandler {
    public async processWithClient(params: IHandlerParameters, client: ZSshClient): Promise<uss.WriteFileResponse> {
        let encoding = params.arguments.encoding;
        const binary = params.arguments.binary;
        if (encoding == null && binary == null) {
            try {
                const fileResp = await client.uss.listFiles({
                    fspath: params.arguments.ussFile,
                    all: true,
                    long: true,
                });
                if (fileResp.success && fileResp.items.length > 0) {
                    const file = fileResp.items[0];
                    encoding = file.filetag;
                }
            } catch (error) {
                // Ignore as the file may not exist yet
            }
        }

        const task: ITaskWithStatus = {
            percentComplete: 0,
            statusMessage: "Uploading...",
            stageName: TaskStage.IN_PROGRESS,
        };
        params.response.progress.startBar({ task });
        const response = await client.uss.writeFile(
            {
                stream: fs.createReadStream(params.arguments.file),
                fspath: params.arguments.ussFile,
                encoding: binary ? "binary" : encoding,
                contentLen: fs.statSync(params.arguments.file).size,
            },
            (percent: number): void => {
                task.percentComplete = percent;
            },
        );

        task.stageName = TaskStage.COMPLETE;
        params.response.progress.endBar();

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
