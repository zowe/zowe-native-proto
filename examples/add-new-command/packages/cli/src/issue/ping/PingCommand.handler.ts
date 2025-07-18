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
import type { ZSshClient } from "zowe-native-proto-sdk";
import { SshBaseHandler } from "../../SshBaseHandler";

export default class PingCommandHandler extends SshBaseHandler {
    public async processWithClient(commandParameters: IHandlerParameters, client: ZSshClient) {
        const message = commandParameters.arguments.message;

        const response = await client.cmds.ping({
            message: message,
        });

        commandParameters.response.console.log(`Server Response: ${response.data}`);
        commandParameters.response.console.log(`Timestamp: ${response.timestamp}`);

        return response;
    }
}
