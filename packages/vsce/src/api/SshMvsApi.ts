import { writeFileSync } from "node:fs";
import type * as zosfiles from "@zowe/zos-files-for-zowe-sdk";
import { type MainframeInteraction, imperative } from "@zowe/zowe-explorer-api";
import { type ListDatasets, type ListDsMembers, type ReadDataset, ZSshUtils } from "zowe-native-proto-sdk";
import { SshClientCache } from "../SshClientCache";
import { SshCommonApi } from "./SshCommonApi";

export class SshMvsApi extends SshCommonApi implements MainframeInteraction.IMvs {
    public async dataSet(filter: string, options?: zosfiles.IListOptions): Promise<zosfiles.IZosFilesResponse> {
        const client = await SshClientCache.inst.connect(this.getSshSession());
        const request: ListDatasets.Request = {
            command: "listDatasets",
            pattern: filter,
        };
        const response = await client.request<ListDatasets.Response>(request);
        return this.buildZosFilesResponse({
            items: response.items.map((item) => ({ dsname: item.name, dsorg: item.dsorg, vol: item.volser })),
            returnedRows: response.returnedRows,
        });
    }

    public async allMembers(dataSetName: string, options?: zosfiles.IListOptions): Promise<zosfiles.IZosFilesResponse> {
        const client = await SshClientCache.inst.connect(this.getSshSession());
        const request: ListDsMembers.Request = {
            command: "listDsMembers",
            dataset: dataSetName,
        };
        const response = await client.request<ListDsMembers.Response>(request);
        return this.buildZosFilesResponse({
            items: response.items.map((item) => ({ member: item.name })),
            returnedRows: response.returnedRows,
        });
    }

    public async getContents(
        dataSetName: string,
        options: zosfiles.IDownloadSingleOptions,
    ): Promise<zosfiles.IZosFilesResponse> {
        const client = await SshClientCache.inst.connect(this.getSshSession());
        const request: ReadDataset.Request = {
            command: "readDataset",
            dataset: dataSetName,
            encoding: options.encoding,
        };
        const response = await client.request<ReadDataset.Response>(request);
        if (options.file != null) {
            imperative.IO.createDirsSyncFromFilePath(options.file);
            writeFileSync(options.file, ZSshUtils.decodeByteArray(response.data));
        } else if (options.stream != null) {
            options.stream.write(ZSshUtils.decodeByteArray(response.data));
            options.stream.end();
        }
        return this.buildZosFilesResponse({ etag: dataSetName });
    }

    public uploadFromBuffer(
        buffer: Buffer,
        dataSetName: string,
        _options?: zosfiles.IUploadOptions,
    ): Promise<zosfiles.IZosFilesResponse> {
        throw new Error("Not yet implemented");
    }

    public async putContents(
        inputFilePath: string,
        dataSetName: string,
        _options?: zosfiles.IUploadOptions,
    ): Promise<zosfiles.IZosFilesResponse> {
        throw new Error("Not yet implemented");
    }

    public async createDataSet(
        dataSetType: zosfiles.CreateDataSetTypeEnum,
        dataSetName: string,
        options?: Partial<zosfiles.ICreateDataSetOptions>,
    ): Promise<zosfiles.IZosFilesResponse> {
        throw new Error("Not yet implemented");
    }

    public async createDataSetMember(
        dataSetName: string,
        options?: zosfiles.IUploadOptions,
    ): Promise<zosfiles.IZosFilesResponse> {
        throw new Error("Not yet implemented");
    }

    public async allocateLikeDataSet(
        dataSetName: string,
        likeDataSetName: string,
    ): Promise<zosfiles.IZosFilesResponse> {
        throw new Error("Not yet implemented");
    }

    public async copyDataSetMember(
        { dsn: fromDataSetName, member: fromMemberName }: zosfiles.IDataSet,
        { dsn: toDataSetName, member: toMemberName }: zosfiles.IDataSet,
        options?: { replace?: boolean },
    ): Promise<zosfiles.IZosFilesResponse> {
        throw new Error("Not yet implemented");
    }

    public async renameDataSet(
        currentDataSetName: string,
        newDataSetName: string,
    ): Promise<zosfiles.IZosFilesResponse> {
        throw new Error("Not yet implemented");
    }

    public async renameDataSetMember(
        dataSetName: string,
        currentMemberName: string,
        newMemberName: string,
    ): Promise<zosfiles.IZosFilesResponse> {
        throw new Error("Not yet implemented");
    }

    public async hMigrateDataSet(dataSetName: string): Promise<zosfiles.IZosFilesResponse> {
        throw new Error("Not yet implemented");
    }

    public async hRecallDataSet(dataSetName: string): Promise<zosfiles.IZosFilesResponse> {
        throw new Error("Not yet implemented");
    }

    public async deleteDataSet(
        dataSetName: string,
        options?: zosfiles.IDeleteDatasetOptions,
    ): Promise<zosfiles.IZosFilesResponse> {
        throw new Error("Not yet implemented");
    }

    private buildZosFilesResponse(apiResponse: any, success = true): zosfiles.IZosFilesResponse {
        return { apiResponse, commandResponse: "", success };
    }
}
