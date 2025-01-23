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

import { writeFileSync } from "node:fs";
import type * as zosfiles from "@zowe/zos-files-for-zowe-sdk";
import { type MainframeInteraction, imperative } from "@zowe/zowe-explorer-api";
import { ZSshUtils } from "zowe-native-proto-sdk";
import { SshCommonApi } from "./SshCommonApi";

export class SshMvsApi extends SshCommonApi implements MainframeInteraction.IMvs {
    public async dataSet(filter: string, options?: zosfiles.IListOptions): Promise<zosfiles.IZosFilesResponse> {
        const response = await (await this.client).ds.listDatasets({
            pattern: filter,
        });
        return this.buildZosFilesResponse({
            items: response.items.map((item) => ({ dsname: item.name, dsorg: item.dsorg, vol: item.volser })),
            returnedRows: response.returnedRows,
        });
    }

    public async allMembers(dataSetName: string, options?: zosfiles.IListOptions): Promise<zosfiles.IZosFilesResponse> {
        const response = await (await this.client).ds.listDsMembers({
            dataset: dataSetName,
        });
        return this.buildZosFilesResponse({
            items: response.items.map((item) => ({ member: item.name })),
            returnedRows: response.returnedRows,
        });
    }

    public async getContents(
        dataSetName: string,
        options: zosfiles.IDownloadSingleOptions,
    ): Promise<zosfiles.IZosFilesResponse> {
        const response = await (await this.client).ds.readDataset({
            dataset: dataSetName,
            encoding: options.encoding,
        });
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
