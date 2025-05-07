import type { IHandlerParameters } from "@zowe/imperative";
import type { ZSshClient, ds } from "zowe-native-proto-sdk";
import type { DatasetAttr } from "zowe-native-proto-sdk/src/doc/gen/common";
import { SshBaseHandler } from "../../SshBaseHandler";

export default class CreateDatasetAttrHandler extends SshBaseHandler {
    public async processWithClient(params: IHandlerParameters, client: ZSshClient): Promise<ds.CreateDatasetResponse> {
        const dsname = params.arguments.name;
        const attributes: Partial<DatasetAttr> = {};

        const attrKeys: (keyof DatasetAttr)[] = [
            "alcunit",
            "blksz",
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

        const args = params.arguments as Record<string, unknown>;

        for (const key of attrKeys) {
            const value = args[key];
            if (value !== undefined) {
                // biome-ignore lint/suspicious/noExplicitAny: as any required for attribute mapping
                (attributes as any)[key] = value;
            }
        }

        const response = await client.ds.createDatasetAttr({ dsname, attributes });

        const dsMessage = `Dataset "${dsname}" created`;
        params.response.data.setMessage(dsMessage);
        params.response.data.setObj(response);
        if (response.success) {
            params.response.console.log(dsMessage);
        }
        return response;
    }
}
