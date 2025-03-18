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

import {
    ConfigUtils,
    type ICommandHandler,
    type IHandlerParameters,
    type ITaskWithStatus,
    TaskStage,
} from "@zowe/imperative";
import { ZSshClient, ZSshUtils } from "zowe-native-proto-sdk";
import { Constants } from "../../Constants";

export default class ServerInstallHandler implements ICommandHandler {
    public async process(params: IHandlerParameters): Promise<void> {
        const session = ZSshUtils.buildSession(params.arguments);
        const serverPath = params.arguments.serverPath ?? ZSshClient.DEFAULT_SERVER_PATH;

        const task: ITaskWithStatus = {
            percentComplete: 0,
            statusMessage: "Deploying Zowe SSH server...",
            stageName: TaskStage.IN_PROGRESS,
        };

        params.response.progress.startBar({ task });
        try {
            await ZSshUtils.installServer(session, serverPath, Constants.ZSSH_BIN_DIR, (progressIncrement) => {
                task.percentComplete += progressIncrement;
            });
        } finally {
            task.stageName = TaskStage.COMPLETE;
            params.response.progress.endBar();
        }
        params.response.console.log(
            `Installed Zowe SSH server on ${ConfigUtils.getActiveProfileName("ssh", params.arguments)}`,
        );
    }
}
