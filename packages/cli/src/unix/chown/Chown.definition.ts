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

export default class ChownHandler extends SshBaseHandler {
    public async processWithClient(params: IHandlerParameters, client: ZSshClient): Promise<uss.ChownFileResponse> {
        const response = await client.uss.chownFile({
            recursive: params.arguments.recursive,
            owner: params.arguments.owner,
            fspath: params.arguments.path,
        });
        params.response.data.setMessage(
            "Successfully changed owner of %s to %s",
            params.arguments.path,
            params.arguments.owner,
        );
        params.response.format.output({
            output: response,
            format: "table",
            fields: ["success", "fspath"],
        });
        return response;
    }
}
