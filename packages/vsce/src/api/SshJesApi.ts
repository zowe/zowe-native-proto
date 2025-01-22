import type * as zosjobs from "@zowe/zos-jobs-for-zowe-sdk";
import type { MainframeInteraction } from "@zowe/zowe-explorer-api";
import { type GetJcl, type ListJobs, type ListSpools, type ReadSpool, ZSshUtils } from "zowe-native-proto-sdk";
import { SshClientCache } from "../SshClientCache";
import { SshCommonApi } from "./SshCommonApi";

export class SshJesApi extends SshCommonApi implements MainframeInteraction.IJes {
    public async getJobsByParameters(params: zosjobs.IGetJobsParms): Promise<zosjobs.IJob[]> {
        const client = await SshClientCache.inst.connect(this.getSshSession());
        const request: ListJobs.Request = {
            command: "listJobs",
            owner: params.owner?.toUpperCase(),
        };
        const response = await client.request<ListJobs.Response>(request);
        return response.items.map(
            (item): Partial<zosjobs.IJob> => ({
                jobid: item.id,
                jobname: item.name,
                status: item.status,
                retcode: item.retcode,
            }),
        ) as zosjobs.IJob[];
    }

    public async getJob(jobid: string): Promise<zosjobs.IJob> {
        throw new Error("Not yet implemented");
    }

    public async getSpoolFiles(jobname: string, jobid: string): Promise<zosjobs.IJobFile[]> {
        const client = await SshClientCache.inst.connect(this.getSshSession());
        const request: ListSpools.Request = {
            command: "listSpools",
            jobId: jobid.toUpperCase(),
        };
        const response = await client.request<ListSpools.Response>(request);
        return response.items as zosjobs.IJobFile[];
    }

    public async downloadSpoolContent(parms: zosjobs.IDownloadAllSpoolContentParms): Promise<void> {
        throw new Error("Not yet implemented");
    }

    public async getSpoolContentById(jobname: string, jobid: string, spoolId: number): Promise<string> {
        const client = await SshClientCache.inst.connect(this.getSshSession());
        const request: ReadSpool.Request = {
            command: "readSpool",
            dsnKey: spoolId,
            jobId: jobid.toUpperCase(),
        };
        const response = await client.request<ReadSpool.Response>(request);
        return ZSshUtils.decodeByteArray(response.data).toString();
    }

    public async getJclForJob(job: zosjobs.IJob): Promise<string> {
        const client = await SshClientCache.inst.connect(this.getSshSession());
        const request: GetJcl.Request = {
            command: "getJcl",
            jobId: job.jobid.toUpperCase(),
        };
        const response = await client.request<GetJcl.Response>(request);
        return response.data;
    }

    public async submitJcl(
        jcl: string,
        internalReaderRecfm?: string,
        internalReaderLrecl?: string,
    ): Promise<zosjobs.IJob> {
        throw new Error("Not yet implemented");
    }

    public async submitJob(jobDataSet: string): Promise<zosjobs.IJob> {
        throw new Error("Not yet implemented");
    }

    public async deleteJob(jobname: string, jobid: string): Promise<void> {
        throw new Error("Not yet implemented");
    }
}
