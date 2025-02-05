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

import type { IRpcRequest, IRpcResponse } from "./doc";
import type * as cmds from "./doc/cmds";
import type * as ds from "./doc/ds";
import type * as jobs from "./doc/job";
import type * as uss from "./doc/uss";

export abstract class AbstractRpcClient {
    public abstract request<T extends IRpcResponse>(request: IRpcRequest): Promise<T>;

    public get ds() {
        return {
            listDatasets: (request: Omit<ds.ListDatasetsRequest, "command">): Promise<ds.ListDatasetsResponse> =>
                this.request({ command: "listDatasets", ...request }),
            listDsMembers: (request: Omit<ds.ListDsMembersRequest, "command">): Promise<ds.ListDsMembersResponse> =>
                this.request({ command: "listDsMembers", ...request }),
            readDataset: (request: Omit<ds.ReadDatasetRequest, "command">): Promise<ds.ReadDatasetResponse> =>
                this.request({ command: "readDataset", ...request }),
            writeDataset: (request: Omit<ds.WriteDatasetRequest, "command">): Promise<ds.WriteDatasetResponse> =>
                this.request({ command: "writeDataset", ...request }),
            restoreDataset: (request: Omit<ds.RestoreDatasetRequest, "command">): Promise<ds.RestoreDatasetResponse> =>
                this.request({ command: "restoreDataset", ...request }),
        };
    }

    public get jobs() {
        return {
            getJcl: (request: Omit<jobs.GetJclRequest, "command">): Promise<jobs.GetJclResponse> =>
                this.request({ command: "getJcl", ...request }),
            listJobs: (request: Omit<jobs.ListJobsRequest, "command">): Promise<jobs.ListJobsResponse> =>
                this.request({ command: "listJobs", ...request }),
            listSpools: (request: Omit<jobs.ListSpoolsRequest, "command">): Promise<jobs.ListSpoolsResponse> =>
                this.request({ command: "listSpools", ...request }),
            readSpool: (request: Omit<jobs.ReadSpoolRequest, "command">): Promise<jobs.ReadSpoolResponse> =>
                this.request({ command: "readSpool", ...request }),
            getStatus: (request: Omit<jobs.GetStatusRequest, "command">): Promise<jobs.GetStatusResponse> =>
                this.request({ command: "getStatus", ...request }),
        };
    }

    public get uss() {
        return {
            listFiles: (request: Omit<uss.ListFilesRequest, "command">): Promise<uss.ListFilesResponse> =>
                this.request({ command: "listFiles", ...request }),
            readFile: (request: Omit<uss.ReadFileRequest, "command">): Promise<uss.ReadFileResponse> =>
                this.request({ command: "readFile", ...request }),
            writeFile: (request: Omit<uss.WriteFileRequest, "command">): Promise<uss.WriteFileResponse> =>
                this.request({ command: "writeFile", ...request }),
        };
    }

    public get cmds() {
        return {
            issueConsole: (request: Omit<cmds.IssueConsoleRequest, "command">): Promise<cmds.IssueConsoleResponse> =>
                this.request({ command: "consoleCommand", ...request }),
            issueTso: (request: Omit<cmds.IssueTsoRequest, "command">): Promise<cmds.IssueTsoResponse> =>
                this.request({ command: "tsoCommand", ...request }),
            issueUnix: (request: Omit<cmds.IssueUnixRequest, "command">): Promise<cmds.IssueUnixResponse> =>
                this.request({ command: "unixCommand", ...request }),
        };
    }
}
