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
import type { ZSshClient, jobs } from "zowe-native-proto-sdk";
import { SshBaseHandler } from "../../SshBaseHandler";

export default class ViewJobStatusHandler extends SshBaseHandler {
    public async processWithClient(params: IHandlerParameters, client: ZSshClient): Promise<jobs.GetStatusResponse> {
        const response = await client.jobs.getStatus({
            jobId: params.arguments.jobId,
        });
        params.response.data.setMessage("Successfully downloaded status for %s", params.arguments.jobId);
        params.response.format.output({
            output: response.items,
            format: "table",
            fields: ["id", "name", "status", "retcode"],
        });
        return response;
    }
}
