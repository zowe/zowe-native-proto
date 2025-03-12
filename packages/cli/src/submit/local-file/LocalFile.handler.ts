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

import { readFileSync } from "node:fs";
import { type IHandlerParameters, ImperativeError } from "@zowe/imperative";
import { type ZSshClient, ZSshUtils, type jobs } from "zowe-native-proto-sdk";
import { SshBaseHandler } from "../../SshBaseHandler";

export default class SubmitLocalFileHandler extends SshBaseHandler {
    public async processWithClient(params: IHandlerParameters, client: ZSshClient): Promise<jobs.SubmitJclResponse> {
        const fspath = params.arguments.fspath;
        let JclString: string;
        try {
            JclString = readFileSync(fspath).toString();
        } catch (err) {
            throw new ImperativeError({
                msg: "Failed to read local file",
                additionalDetails: err.toString(),
                causeErrors: err,
            });
        }
        const response = await client.jobs.submitJcl({ jcl: ZSshUtils.encodeByteArray(Buffer.from(JclString)) });

        const msg = `Job submitted: ${response.jobId}`;
        params.response.data.setMessage(msg);
        params.response.data.setObj(response);
        if (response.success) {
            params.response.console.log(msg);
        }
        return response;
    }
}
