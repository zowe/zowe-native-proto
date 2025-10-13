import type { CommandRequest, CommandResponse, cmds, ds, jobs, uss } from "./doc/rpc";

type RequestHandler = (
    request: CommandRequest,
    progressCallback?: (percent: number) => void,
) => Promise<CommandResponse>;
type ProgressCallback = (percent: number) => void;

function withProgress<ReqT, RespT>(
    handler: (request: ReqT, progressCallback?: ProgressCallback) => Promise<RespT>,
): (request: ReqT, progressCallback?: ProgressCallback) => Promise<RespT> {
    return (request: ReqT, progressCallback?: ProgressCallback) => handler(request, progressCallback);
}

// biome-ignore lint/complexity/noStaticOnlyClass: Builder class has static methods
export class RpcApiBuilder {
    public static build(requestFn: RequestHandler) {
        return {
            cmds: RpcApiBuilder.buildCmdsApi(requestFn),
            ds: RpcApiBuilder.buildDsApi(requestFn),
            jobs: RpcApiBuilder.buildJobsApi(requestFn),
            uss: RpcApiBuilder.buildUssApi(requestFn),
        };
    }

    private static buildCmdsApi(requestFn: RequestHandler) {
        const rpc = RpcApiBuilder.requestHandler(requestFn);
        return {
            issueConsole: rpc<cmds.IssueConsoleRequest, cmds.IssueConsoleResponse>("consoleCommand"),
            issueTso: rpc<cmds.IssueTsoRequest, cmds.IssueTsoResponse>("tsoCommand"),
            issueUnix: rpc<cmds.IssueUnixRequest, cmds.IssueUnixResponse>("unixCommand"),
        };
    }

    private static buildDsApi(requestFn: RequestHandler) {
        const rpc = RpcApiBuilder.requestHandler(requestFn);
        return {
            createDataset: rpc<ds.CreateDatasetRequest, ds.CreateDatasetResponse>("createDataset"),
            createMember: rpc<ds.CreateMemberRequest, ds.CreateMemberResponse>("createMember"),
            deleteDataset: rpc<ds.DeleteDatasetRequest, ds.DeleteDatasetResponse>("deleteDataset"),
            listDatasets: rpc<ds.ListDatasetsRequest, ds.ListDatasetsResponse>("listDatasets"),
            listDsMembers: rpc<ds.ListDsMembersRequest, ds.ListDsMembersResponse>("listDsMembers"),
            readDataset: rpc<ds.ReadDatasetRequest, ds.ReadDatasetResponse>("readDataset"),
            restoreDataset: rpc<ds.RestoreDatasetRequest, ds.RestoreDatasetResponse>("restoreDataset"),
            writeDataset: rpc<ds.WriteDatasetRequest, ds.WriteDatasetResponse>("writeDataset"),
        };
    }

    private static buildJobsApi(requestFn: RequestHandler) {
        const rpc = RpcApiBuilder.requestHandler(requestFn);
        return {
            cancelJob: rpc<jobs.CancelJobRequest, jobs.CancelJobResponse>("cancelJob"),
            deleteJob: rpc<jobs.DeleteJobRequest, jobs.DeleteJobResponse>("deleteJob"),
            getJcl: rpc<jobs.GetJclRequest, jobs.GetJclResponse>("getJcl"),
            getStatus: rpc<jobs.GetJobStatusRequest, jobs.GetJobStatusResponse>("getJobStatus"),
            holdJob: rpc<jobs.HoldJobRequest, jobs.HoldJobResponse>("holdJob"),
            listJobs: rpc<jobs.ListJobsRequest, jobs.ListJobsResponse>("listJobs"),
            listSpools: rpc<jobs.ListSpoolsRequest, jobs.ListSpoolsResponse>("listSpools"),
            readSpool: rpc<jobs.ReadSpoolRequest, jobs.ReadSpoolResponse>("readSpool"),
            releaseJob: rpc<jobs.ReleaseJobRequest, jobs.ReleaseJobResponse>("releaseJob"),
            submitJcl: rpc<jobs.SubmitJclRequest, jobs.SubmitJclResponse>("submitJcl"),
            submitJob: rpc<jobs.SubmitJobRequest, jobs.SubmitJobResponse>("submitJob"),
            submitUss: rpc<jobs.SubmitUssRequest, jobs.SubmitUssResponse>("submitUss"),
        };
    }

    private static buildUssApi(requestFn: RequestHandler) {
        const rpc = RpcApiBuilder.requestHandler(requestFn);
        return {
            chmodFile: rpc<uss.ChmodFileRequest, uss.ChmodFileResponse>("chmodFile"),
            chownFile: rpc<uss.ChownFileRequest, uss.ChownFileResponse>("chownFile"),
            chtagFile: rpc<uss.ChtagFileRequest, uss.ChtagFileResponse>("chtagFile"),
            createFile: rpc<uss.CreateFileRequest, uss.CreateFileResponse>("createFile"),
            deleteFile: rpc<uss.DeleteFileRequest, uss.DeleteFileResponse>("deleteFile"),
            listFiles: rpc<uss.ListFilesRequest, uss.ListFilesResponse>("listFiles"),
            readFile: withProgress(rpc<uss.ReadFileRequest, uss.ReadFileResponse>("readFile")),
            writeFile: withProgress(rpc<uss.WriteFileRequest, uss.WriteFileResponse>("writeFile")),
        };
    }

    private static requestHandler(requestFn: RequestHandler) {
        return <ReqT extends CommandRequest, RespT extends CommandResponse>(command: string) => {
            return (request: Omit<ReqT, "command">): Promise<RespT> =>
                requestFn({ command, ...request }) as Promise<RespT>;
        };
    }
}

export type ZSshRpcApi = ReturnType<typeof RpcApiBuilder.build>;
