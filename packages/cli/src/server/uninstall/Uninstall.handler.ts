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

import { ConfigUtils, type ICommandHandler, type IHandlerParameters } from "@zowe/imperative";
import { ZSshClient, ZSshUtils } from "zowe-native-proto-sdk";

export default class ServerUninstallHandler implements ICommandHandler {
    public async process(params: IHandlerParameters): Promise<void> {
        const session = ZSshUtils.buildSession(params.arguments);
        const serverPath = params.arguments.serverPath ?? ZSshClient.DEFAULT_SERVER_PATH;
        await ZSshUtils.uninstallServer(session, serverPath);
        params.response.console.log(
            `Uninstalled Zowe SSH server from ${ConfigUtils.getActiveProfileName("ssh", params.arguments)}`,
        );
    }
}
