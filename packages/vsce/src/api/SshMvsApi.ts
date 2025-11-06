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
import {
    type AttributeEntryInfo,
    type AttributeInfo,
    type DataSetAttributesProvider,
    type DsInfo,
    Gui,
    type IAttributesProvider,
    imperative,
    type MainframeInteraction,
} from "@zowe/zowe-explorer-api";
import { B64String, type Dataset, type DatasetAttributes, type ds } from "zowe-native-proto-sdk";
import { SshCommonApi } from "./SshCommonApi";

class SshAttributesProvider implements IAttributesProvider {
    private readonly extensionName = require("../../package.json").displayName;

    public constructor(public cachedAttrs?: Dataset) {}

    public fetchAttributes(_context: DsInfo): AttributeInfo {
        const keys = new Map<string, AttributeEntryInfo>();
        const addAttribute = <K extends keyof Dataset>(prop: K, label: string, description?: string): void => {
            const value = this.cachedAttrs?.[prop];
            if (value != null) {
                keys.set(label, {
                    value: typeof value === "boolean" ? (value ? "YES" : "NO") : value.toLocaleString(),
                    description,
                });
            }
        };

        const spacu = this.cachedAttrs?.spacu?.toLocaleLowerCase() ?? "bytes";
        addAttribute("alloc", "Allocated Units", `Allocated units (${spacu})`);
        addAttribute("allocx", "Allocated Extents");
        addAttribute("dataclass", "Data Class");
        addAttribute("encrypted", "Encryption");
        addAttribute("mgmtclass", "Management Class");
        addAttribute("primary", "Primary Space", `Primary space (${spacu})`);
        addAttribute("secondary", "Secondary Space", `Secondary space (${spacu})`);
        addAttribute("storclass", "Storage Class");
        addAttribute("usedx", "Used Extents");

        return [{ title: this.extensionName, keys }];
    }
}

export class SshMvsApi extends SshCommonApi implements MainframeInteraction.IMvs {
    private attrProvider = new SshAttributesProvider();

    public constructor(
        dsAttrProvider?: DataSetAttributesProvider,
        public profile?: imperative.IProfileLoaded,
    ) {
        super(profile);
        dsAttrProvider?.register(this.attrProvider);
    }

    public async dataSet(filter: string, options?: zosfiles.IListOptions): Promise<zosfiles.IZosFilesResponse> {
        try {
            const response = await (await this.client).ds.listDatasets({
                pattern: filter,
                attributes: options?.attributes,
            });
            this.attrProvider.cachedAttrs = response.items[0];
            return this.buildZosFilesResponse({
                items: response.items.map((item) => {
                    const entry: Record<string, unknown> = { dsname: item.name };
                    if (options?.attributes) {
                        entry.blksz = item.blksize;
                        entry.cdate = item.cdate;
                        entry.dev = item.devtype;
                        entry.dsntp = item.dsntype;
                        entry.dsorg = item.dsorg;
                        entry.edate = item.edate;
                        entry.lrecl = item.lrecl;
                        entry.migr = item.migrated ? "YES" : "NO";
                        entry.rdate = item.rdate;
                        entry.recfm = item.recfm;
                        entry.spacu = item.spacu;
                        entry.used = `${item.usedp}%`;
                        entry.vols = item.volser;
                    }
                    return entry;
                }),
                returnedRows: response.returnedRows,
            });
        } catch (_err) {
            return this.buildZosFilesResponse(
                {
                    items: [],
                    returnedRows: 0,
                },
                false,
            );
        }
    }

    public async allMembers(
        dataSetName: string,
        _options?: zosfiles.IListOptions,
    ): Promise<zosfiles.IZosFilesResponse> {
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
            // Pass stream if file is provided, otherwise use buffer to read into memory
            stream: options.file ? writeStream : undefined,
        });
        if (options.stream != null) {
            options.stream.write(B64String.decode(response.data));
            options.stream.end();
        }
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
        _dataSetType: zosfiles.CreateDataSetTypeEnum,
        dataSetName: string,
        options?: Partial<zosfiles.ICreateDataSetOptions>,
    ): Promise<zosfiles.IZosFilesResponse> {
        const datasetAttributes: DatasetAttributes = {
            dsname: dataSetName,
            primary: 1,
            lrecl: 80,
            ...(options || {}),
        };
        let response: ds.CreateDatasetResponse = { success: false };
        try {
            response = await (await this.client).ds.createDataset({
                dsname: dataSetName,
                attributes: datasetAttributes,
            });
        } catch (error) {
            if (error instanceof imperative.ImperativeError) {
                Gui.errorMessage(error.additionalDetails);
            }
        }
        return this.buildZosFilesResponse(response, response.success);
    }

    public async createDataSetMember(
        dataSetName: string,
        _options?: zosfiles.IUploadOptions,
    ): Promise<zosfiles.IZosFilesResponse> {
        let response: ds.CreateMemberResponse = { success: false };
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
        _dataSetName: string,
        _likeDataSetName: string,
    ): Promise<zosfiles.IZosFilesResponse> {
        throw new Error("Not yet implemented");
    }

    public async copyDataSetMember(
        { dsn: _fromDataSetName, member: _fromMemberName }: zosfiles.IDataSet,
        { dsn: _toDataSetName, member: _toMemberName }: zosfiles.IDataSet,
        _options?: { replace?: boolean },
    ): Promise<zosfiles.IZosFilesResponse> {
        throw new Error("Not yet implemented");
    }

    public async renameDataSet(
        _currentDataSetName: string,
        _newDataSetName: string,
    ): Promise<zosfiles.IZosFilesResponse> {
        throw new Error("Not yet implemented");
    }

    public async renameDataSetMember(
        _dataSetName: string,
        _currentMemberName: string,
        _newMemberName: string,
    ): Promise<zosfiles.IZosFilesResponse> {
        throw new Error("Not yet implemented");
    }

    public async hMigrateDataSet(_dataSetName: string): Promise<zosfiles.IZosFilesResponse> {
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
        _options?: zosfiles.IDeleteDatasetOptions,
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
