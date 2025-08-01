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

import { createReadStream, createWriteStream } from "node:fs";
import type * as zosfiles from "@zowe/zos-files-for-zowe-sdk";
import { Gui, type MainframeInteraction, imperative } from "@zowe/zowe-explorer-api";
import { B64String, type ds } from "zowe-native-proto-sdk";
import type * as common from "../../../sdk/lib/doc/gen/common.ts";
import { SshCommonApi } from "./SshCommonApi";

export class SshMvsApi extends SshCommonApi implements MainframeInteraction.IMvs {
    public async dataSet(filter: string, options?: zosfiles.IListOptions): Promise<zosfiles.IZosFilesResponse> {
        try {
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
        } catch (err) {
            if (err instanceof imperative.ImperativeError) {
                Gui.errorMessage(`Failed to list data sets: ${err.additionalDetails.replace("Error: ", "")}`);
            }
            return this.buildZosFilesResponse(
                {
                    items: [],
                    returnedRows: 0,
                },
                false,
            );
        }
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
        let writeStream = options.stream;
        if (options.file != null) {
            imperative.IO.createDirsSyncFromFilePath(options.file);
            writeStream = createWriteStream(options.file);
        }
        if (writeStream == null) {
            throw new Error("Failed to get contents: No stream or file path provided");
        }
        const response = await (await this.client).ds.readDataset({
            dsname: dataSetName,
            encoding: options.binary ? "binary" : options.encoding,
            stream: writeStream,
        });
        return this.buildZosFilesResponse({ etag: response.etag });
    }

    public async uploadFromBuffer(
        buffer: Buffer,
        dataSetName: string,
        options?: zosfiles.IUploadOptions,
    ): Promise<zosfiles.IZosFilesResponse> {
        let response: ds.WriteDatasetResponse;
        try {
            response = await (await this.client).ds.writeDataset({
                dsname: dataSetName,
                encoding: options?.binary ? "binary" : options?.encoding,
                data: B64String.encode(buffer),
                etag: options?.etag,
            });
        } catch (err) {
            if (err instanceof imperative.ImperativeError && err.additionalDetails.includes("Etag mismatch")) {
                throw new Error("Rest API failure with HTTP(S) status 412");
            }
            throw err;
        }
        return this.buildZosFilesResponse({ etag: response.etag });
    }

    public async putContents(
        inputFilePath: string,
        dataSetName: string,
        options?: zosfiles.IUploadOptions,
    ): Promise<zosfiles.IZosFilesResponse> {
        const response = await (await this.client).ds.writeDataset({
            dsname: dataSetName,
            encoding: options?.encoding,
            stream: createReadStream(inputFilePath),
            etag: options?.etag,
        });
        return this.buildZosFilesResponse({ etag: response.etag });
    }

    public async createDataSet(
        dataSetType: zosfiles.CreateDataSetTypeEnum,
        dataSetName: string,
        options?: Partial<zosfiles.ICreateDataSetOptions>,
    ): Promise<zosfiles.IZosFilesResponse> {
        const datasetAttributes: common.DatasetAttributes = {
            dsname: dataSetName,
            primary: 1,
            lrecl: 80,
            ...(options || {}),
        };

        const response = await (await this.client).ds.createDataset({
            dsname: dataSetName,
            attributes: datasetAttributes,
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
