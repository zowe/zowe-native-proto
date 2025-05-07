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

import type { CommandRequest, CommandResponse, cmds, ds, jobs, uss } from "./doc";

export abstract class AbstractRpcClient {
    public abstract request<Req extends CommandRequest, Resp extends CommandResponse>(request: Req): Promise<Resp>;

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
            deleteDataset: (request: Omit<ds.DeleteDatasetRequest, "command">): Promise<ds.DeleteDatasetResponse> =>
                this.request({ command: "deleteDataset", ...request }),
            createDataset: (request: Omit<ds.CreateDatasetRequest, "command">): Promise<ds.CreateDatasetResponse> =>
                this.request({ command: "createDataset", ...request }),
            createDatasetAttr: (request: Omit<ds.CreateDatasetRequestAttr, "command">): Promise<ds.CreateDatasetResponse> =>
                this.request({ command: "createDatasetAttributes", ...request }),
            createMember: (request: Omit<ds.CreateMemberRequest, "command">): Promise<ds.CreateMemberResponse> =>
                this.request({ command: "createMember", ...request }),
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
            getStatus: (request: Omit<jobs.GetJobStatusRequest, "command">): Promise<jobs.GetJobStatusResponse> =>
                this.request({ command: "getJobStatus", ...request }),
            cancelJob: (request: Omit<jobs.CancelJobRequest, "command">): Promise<jobs.CancelJobResponse> =>
                this.request({ command: "cancelJob", ...request }),
            deleteJob: (request: Omit<jobs.DeleteJobRequest, "command">): Promise<jobs.DeleteJobResponse> =>
                this.request({ command: "deleteJob", ...request }),
            submitJob: (request: Omit<jobs.SubmitJobRequest, "command">): Promise<jobs.SubmitJobResponse> =>
                this.request({ command: "submitJob", ...request }),
            submitUss: (request: Omit<jobs.SubmitUssRequest, "command">): Promise<jobs.SubmitUssResponse> =>
                this.request({ command: "submitUss", ...request }),
            submitJcl: (request: Omit<jobs.SubmitJclRequest, "command">): Promise<jobs.SubmitJclResponse> =>
                this.request({ command: "submitJcl", ...request }),
            holdJob: (request: Omit<jobs.HoldJobRequest, "command">): Promise<jobs.HoldJobResponse> =>
                this.request({ command: "holdJob", ...request }),
            releaseJob: (request: Omit<jobs.ReleaseJobRequest, "command">): Promise<jobs.ReleaseJobResponse> =>
                this.request({ command: "releaseJob", ...request }),
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
            deleteFile: (request: Omit<uss.DeleteFileRequest, "command">): Promise<uss.DeleteFileResponse> =>
                this.request({ command: "deleteFile", ...request }),
            createFile: (request: Omit<uss.CreateFileRequest, "command">): Promise<uss.CreateFileResponse> =>
                this.request({ command: "createFile", ...request }),
            chmodFile: (request: Omit<uss.ChmodFileRequest, "command">): Promise<uss.ChmodFileResponse> =>
                this.request({ command: "chmodFile", ...request }),
            chownFile: (request: Omit<uss.ChownFileRequest, "command">): Promise<uss.ChownFileResponse> =>
                this.request({ command: "chownFile", ...request }),
            chtagFile: (request: Omit<uss.ChtagFileRequest, "command">): Promise<uss.ChtagFileResponse> =>
                this.request({ command: "chtagFile", ...request }),
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
