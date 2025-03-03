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

import { text } from "node:stream/consumers";
import type { IHandlerParameters } from "@zowe/imperative";
import type { ZSshClient, jobs } from "zowe-native-proto-sdk";
import { SshBaseHandler } from "../../SshBaseHandler";

export default class SubmitJclHandler extends SshBaseHandler {
    public async processWithClient(params: IHandlerParameters, client: ZSshClient): Promise<jobs.DeleteJobResponse> {
        const jcl = await text(params.stdin);
        const response = await client.jobs.submitJcl({ jcl });

        const msg = `Job submitted: ${response.jobId}`;
        params.response.data.setMessage(msg);
        params.response.data.setObj(response);
        if (response.success) {
            params.response.console.log(msg);
        }
        return response;
    }
}
