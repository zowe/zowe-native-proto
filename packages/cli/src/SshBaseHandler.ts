/*
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

import { ICommandHandler, IHandlerParameters } from "@zowe/imperative";
import { SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { IRpcResponse, ZSshUtils } from "zowe-native-proto-sdk";

export abstract class SshBaseHandler implements ICommandHandler {
    public async process(commandParameters: IHandlerParameters) {
        const session = ZSshUtils.buildSession(commandParameters.arguments);

        const response = await this.processWithSession(commandParameters, session);

        commandParameters.response.progress.endBar(); // end any progress bars

        // Return as an object when using --response-format-json
        commandParameters.response.data.setObj(response);
    }

    public abstract processWithSession(commandParameters: IHandlerParameters, session: SshSession): Promise<IRpcResponse>;
}
