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

import type * as zosjobs from "@zowe/zos-jobs-for-zowe-sdk";
import type { MainframeInteraction } from "@zowe/zowe-explorer-api";
import { ZSshUtils } from "zowe-native-proto-sdk";
import { SshCommonApi } from "./SshCommonApi";

export class SshJesApi extends SshCommonApi implements MainframeInteraction.IJes {
    public async getJobsByParameters(params: zosjobs.IGetJobsParms): Promise<zosjobs.IJob[]> {
        const response = await (await this.client).jobs.listJobs({
            owner: params.owner?.toUpperCase(),
        });
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
        const response = await (await this.client).jobs.listSpools({
            jobId: jobid.toUpperCase(),
        });
        return response.items as unknown as zosjobs.IJobFile[];
    }

    public async downloadSpoolContent(parms: zosjobs.IDownloadAllSpoolContentParms): Promise<void> {
        throw new Error("Not yet implemented");
    }

    public async getSpoolContentById(jobname: string, jobid: string, spoolId: number): Promise<string> {
        const response = await (await this.client).jobs.readSpool({
            spoolId,
            jobId: jobid.toUpperCase(),
        });
        return ZSshUtils.decodeByteArray(response.data).toString();
    }

    public async getJclForJob(job: zosjobs.IJob): Promise<string> {
        const response = await (await this.client).jobs.getJcl({
            jobId: job.jobid.toUpperCase(),
        });
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
