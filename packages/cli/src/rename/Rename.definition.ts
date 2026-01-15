import type { ICommandDefinition } from "@zowe/imperative";
import { SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { Constants } from "../Constants";
import { RenameDataSetDefinition } from "./data-set/DataSet.definition";

const RenameDefinition: ICommandDefinition = {
    name: "rename",
    summary: "Rename a sequential or partitioned data set",
    description: "Rename a sequential or partitioned data set",
    type: "group",
    children: [RenameDataSetDefinition],
    passOn: [
        {
            property: "options",
            value: [...SshSession.SSH_CONNECTION_OPTIONS, Constants.OPT_SERVER_PATH],
            merge: true,
            ignoreNodes: [{ type: "group" }],
        },
    ],
};

export = RenameDefinition;
