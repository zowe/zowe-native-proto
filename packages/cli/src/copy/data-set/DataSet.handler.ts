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
import type { ds, ZSshClient } from "zowe-native-proto-sdk";
import { SshBaseHandler } from "../../SshBaseHandler";

export default class CopyDataSetHandler extends SshBaseHandler {
    public async processWithClient(params: IHandlerParameters, client: ZSshClient): Promise<ds.CopyDatasetResponse> {
        const fromDataset = params.arguments.fromDataset;
        const toDataset = params.arguments.toDataset;
        const replace = params.arguments.replace ?? false;
        const overwrite = params.arguments.overwrite ?? false;

        const response = await client.ds.copyDataset({ fromDataset, toDataset, replace, overwrite });

        let dsMessage: string;
        if (response.targetCreated) {
            dsMessage = `Data set "${toDataset}" created and copied from "${fromDataset}"`;
        } else if (overwrite) {
            dsMessage = `Data set "${toDataset}" overwritten with contents of "${fromDataset}"`;
        } else if (replace) {
            dsMessage = `Data set "${toDataset}" updated with contents of "${fromDataset}"`;
        } else {
            dsMessage = `Data set "${fromDataset}" copied to "${toDataset}"`;
        }

        params.response.data.setMessage(dsMessage);
        params.response.data.setObj(response);
        if (response.success) {
            params.response.console.log(dsMessage);
        }
        return response;
    }
}
