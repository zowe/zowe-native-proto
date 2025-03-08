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

export default class DeleteJobHandler extends SshBaseHandler {
    public async processWithClient(params: IHandlerParameters, client: ZSshClient): Promise<jobs.DeleteJobResponse> {
        const jobId = params.arguments.jobId;

        const response = await client.jobs.deleteJob({ jobId });

        const msg = `Job "${jobId}" deleted`;
        params.response.data.setMessage(msg);
        params.response.data.setObj(response);
        if (response.success) {
            params.response.console.log(msg);
        }
        return response;
    }
}
