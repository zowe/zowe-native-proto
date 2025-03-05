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

import { readFileSync, writeFileSync } from "node:fs";
import * as zosfiles from "@zowe/zos-files-for-zowe-sdk";
import { Gui, type MainframeInteraction, imperative } from "@zowe/zowe-explorer-api";
import { ZSshUtils, type ds } from "zowe-native-proto-sdk";
import { SshCommonApi } from "./SshCommonApi";

export class SshMvsApi extends SshCommonApi implements MainframeInteraction.IMvs {
    public async dataSet(filter: string, options?: zosfiles.IListOptions): Promise<zosfiles.IZosFilesResponse> {
        const response = await (await this.client).ds.listDatasets({
            pattern: filter,
        });
        return this.buildZosFilesResponse({
            items: response.items.map((item) => ({
                dsname: item.name,
                dsorg: item.dsorg,
                vol: item.volser,
                migr: item.migr ? "YES" : "NO",
            })),
            returnedRows: response.returnedRows,
        });
    }

    public async allMembers(dataSetName: string, options?: zosfiles.IListOptions): Promise<zosfiles.IZosFilesResponse> {
        const response = await (await this.client).ds.listDsMembers({
            dsname: dataSetName,
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
            dsname: dataSetName,
            encoding: options.binary ? "binary" : options.encoding,
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

    public async uploadFromBuffer(
        buffer: Buffer,
        dataSetName: string,
        options?: zosfiles.IUploadOptions,
    ): Promise<zosfiles.IZosFilesResponse> {
        const response = await (await this.client).ds.writeDataset({
            dsname: dataSetName,
            encoding: options?.binary ? "binary" : options?.encoding,
            data: ZSshUtils.encodeByteArray(buffer),
        });
        return this.buildZosFilesResponse({ etag: dataSetName });
    }

    public async putContents(
        inputFilePath: string,
        dataSetName: string,
        options?: zosfiles.IUploadOptions,
    ): Promise<zosfiles.IZosFilesResponse> {
        const response = await (await this.client).ds.writeDataset({
            dsname: dataSetName,
            encoding: options?.encoding,
            data: ZSshUtils.encodeByteArray(readFileSync(inputFilePath)),
        });
        return this.buildZosFilesResponse({ etag: dataSetName });
    }

    public async createDataSet(
        dataSetType: zosfiles.CreateDataSetTypeEnum,
        dataSetName: string,
        options?: Partial<zosfiles.ICreateDataSetOptions>,
    ): Promise<zosfiles.IZosFilesResponse> {
        let datasetTyp: ds.CreateDatasetRequest["dstype"];
        switch (dataSetType) {
            case zosfiles.CreateDataSetTypeEnum.DATA_SET_C:
                datasetTyp = "default";
                break;
            case zosfiles.CreateDataSetTypeEnum.DATA_SET_CLASSIC:
                datasetTyp = "adata";
                break;
            default:
                throw new Error("Not yet implemented");
        }

        const response = await (await this.client).ds.createDataset({
            dsname: dataSetName,
            dstype: datasetTyp,
        });
        return this.buildZosFilesResponse(response, response.success);
    }

    public async createDataSetMember(
        dataSetName: string,
        options?: zosfiles.IUploadOptions,
    ): Promise<zosfiles.IZosFilesResponse> {
        let response: ds.CreateMemberResponse = { success: false, dsname: dataSetName };
        try {
            response = await (await this.client).ds.createMember({
                dsname: dataSetName,
            });
            if (!response.success) {
                Gui.errorMessage(`Failed to create data set member: ${dataSetName}`);
            }
        } catch (error) {
            Gui.errorMessage(`Failed to create data set member: ${dataSetName}`);
            Gui.errorMessage(`Error: ${error}`);
        }
        return this.buildZosFilesResponse(response, response.success);
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
        let response: ds.RestoreDatasetResponse = { success: false };
        try {
            response = await (await this.client).ds.restoreDataset({
                dsname: dataSetName,
            });
            if (!response.success) {
                Gui.errorMessage(`Failed to restore dataset ${dataSetName}`);
            }
        } catch (error) {
            Gui.errorMessage(`Failed to restore dataset ${dataSetName}`);
            Gui.errorMessage(`Error: ${error}`);
        }
        return this.buildZosFilesResponse(response);
    }

    public async deleteDataSet(
        dataSetName: string,
        options?: zosfiles.IDeleteDatasetOptions,
    ): Promise<zosfiles.IZosFilesResponse> {
        const response = await (await this.client).ds.deleteDataset({
            dsname: dataSetName,
        });
        return this.buildZosFilesResponse({
            success: response.success,
        });
    }

    // biome-ignore lint/suspicious/noExplicitAny: apiResponse has no strong type
    private buildZosFilesResponse(apiResponse: any, success = true, errorText?: string): zosfiles.IZosFilesResponse {
        return { apiResponse, commandResponse: "", success, errorMessage: errorText };
    }
}
