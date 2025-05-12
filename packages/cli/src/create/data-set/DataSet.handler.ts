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
import { CreateDefaults } from "../CreateDefaults";
import type { ZSshClient, ds } from "zowe-native-proto-sdk";
import type { DatasetAttributes } from "zowe-native-proto-sdk/src/doc/gen/common";
import { SshBaseHandler } from "../../SshBaseHandler";

export default class CreateDatasetHandler extends SshBaseHandler {
    public async processWithClient(params: IHandlerParameters, client: ZSshClient): Promise<ds.CreateDatasetResponse> {
        const dsname = params.arguments.name;
        let attributes: Partial<DatasetAttributes> = {};

        const attrKeys: (keyof DatasetAttributes)[] = [
            "alcunit",
            "blksize",
            "dirblk",
            "dsorg",
            "primary",
            "recfm",
            "lrecl",
            "dataclass",
            "dev",
            "dsntype",
            "mgntclass",
            "dsname",
            "avgblk",
            "secondary",
            "size",
            "storclass",
            "vol",
        ];

        switch (params.arguments.template?.toLocaleUpperCase()) {
            case "PARTITIONED":
                attributes = CreateDefaults.DATA_SET.PARTITIONED;
                break;
            case "SEQUENTIAL":
                attributes = CreateDefaults.DATA_SET.SEQUENTIAL;
                break;
            case "DEFAULT":
                attributes = CreateDefaults.DATA_SET.SEQUENTIAL;
                break;
            case "CLASSIC":
                attributes = CreateDefaults.DATA_SET.CLASSIC;
                break;
            case "C":
                attributes = CreateDefaults.DATA_SET.C;
                break;
            case "BINARY":
                attributes = CreateDefaults.DATA_SET.BINARY;
                break;
            default:
                break;
        }

        const args = params.arguments as Record<string, unknown>;

        for (const key of attrKeys) {
            const value = args[key];
            if (value !== undefined) {
                // biome-ignore lint/suspicious/noExplicitAny: as any required for attribute mapping
                (attributes as any)[key] = value;
            }
        }

        const response = await client.ds.createDataset({ dsname, attributes });

        const dsMessage = `Dataset "${dsname}" created`;
        params.response.data.setMessage(dsMessage);
        params.response.data.setObj(response);
        if (response.success) {
            params.response.console.log(dsMessage);
        }
        return response;
    }
}
