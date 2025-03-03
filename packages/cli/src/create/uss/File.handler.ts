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
import type { ZSshClient, uss } from "zowe-native-proto-sdk";
import { SshBaseHandler } from "../../SshBaseHandler";

export default class CreateFileHandler extends SshBaseHandler {
    public async processWithClient(params: IHandlerParameters, client: ZSshClient): Promise<uss.CreateFileResponse> {
        const path = params.arguments.path;
        const permissions = params.arguments.permissions;
        const response = await client.uss.createFile({ fspath: path, permissions });

        const dsMessage = `File "${path}" created`;
        params.response.data.setMessage(dsMessage);
        params.response.data.setObj(response);
        if (response.success) {
            params.response.console.log(dsMessage);
        }
        return response;
    }
}
