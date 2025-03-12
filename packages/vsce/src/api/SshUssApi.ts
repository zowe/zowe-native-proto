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
import type * as zosfiles from "@zowe/zos-files-for-zowe-sdk";
import { type MainframeInteraction, type Types, imperative } from "@zowe/zowe-explorer-api";
import { B64String } from "zowe-native-proto-sdk";
import { SshCommonApi } from "./SshCommonApi";

export class SshUssApi extends SshCommonApi implements MainframeInteraction.IUss {
    public async fileList(ussFilePath: string): Promise<zosfiles.IZosFilesResponse> {
        const response = await (await this.client).uss.listFiles({
            fspath: ussFilePath,
        });
        return this.buildZosFilesResponse({
            items: response.items.map((item) => ({ name: item.name, mode: item.isDir ? "d" : "-" })),
            returnedRows: response.returnedRows,
        });
    }

    public isFileTagBinOrAscii(ussFilePath: string): Promise<boolean> {
        return Promise.resolve(false);
    }

    public async getContents(
        ussFilePath: string,
        options: zosfiles.IDownloadSingleOptions,
    ): Promise<zosfiles.IZosFilesResponse> {
        const response = await (await this.client).uss.readFile({
            fspath: ussFilePath,
            encoding: options.binary ? "binary" : options.encoding,
        });
        if (options.file != null) {
            imperative.IO.createDirsSyncFromFilePath(options.file);
            writeFileSync(options.file, B64String.decodeBytes(response.data));
        } else if (options.stream != null) {
            options.stream.write(B64String.decodeBytes(response.data));
            options.stream.end();
        }
        return this.buildZosFilesResponse({ etag: ussFilePath });
    }

    public async uploadFromBuffer(
        buffer: Buffer,
        filePath: string,
        options?: zosfiles.IUploadOptions,
    ): Promise<zosfiles.IZosFilesResponse> {
        const response = await (await this.client).uss.writeFile({
            fspath: filePath,
            encoding: options?.binary ? "binary" : options?.encoding,
            data: B64String.encode(buffer),
        });
        return this.buildZosFilesResponse({ etag: filePath });
    }

    public async putContent(
        inputFilePath: string,
        ussFilePath: string,
        options?: zosfiles.IUploadOptions,
    ): Promise<zosfiles.IZosFilesResponse> {
        const response = await (await this.client).uss.writeFile({
            fspath: ussFilePath,
            encoding: options?.encoding,
            data: B64String.encode(readFileSync(inputFilePath)),
        });
        return this.buildZosFilesResponse({ etag: ussFilePath });
    }

    public async uploadDirectory(
        inputDirectoryPath: string,
        ussDirectoryPath: string,
        options: zosfiles.IUploadOptions,
    ): Promise<zosfiles.IZosFilesResponse> {
        throw new Error("Not yet implemented");
    }

    public async create(ussPath: string, type: string, mode?: string | undefined): Promise<zosfiles.IZosFilesResponse> {
        const response = await (await this.client).uss.createFile({
            fspath: ussPath,
            isDir: type === "directory",
            permissions: mode,
        });
        return this.buildZosFilesResponse(response, response.success);
    }

    public async delete(ussPath: string, recursive?: boolean | undefined): Promise<zosfiles.IZosFilesResponse> {
        const response = await (await this.client).uss.deleteFile({
            fspath: ussPath,
            recursive: recursive ?? false,
        });
        return this.buildZosFilesResponse(response, response.success);
    }

    public async rename(currentUssPath: string, newUssPath: string): Promise<zosfiles.IZosFilesResponse> {
        throw new Error("Not yet implemented");
    }

    public async updateAttributes(
        ussPath: string,
        attributes: Partial<Types.FileAttributes>,
    ): Promise<zosfiles.IZosFilesResponse> {
        const ussItem = await this.fileList(ussPath);
        if (!ussItem.success || ussItem.apiResponse?.items.length !== 1) {
            throw new Error("File no longer exists");
        }
        const isDir = ussItem.apiResponse.items[0].mode === "d";
        let success = false;
        if (attributes.tag) {
            const response = await (await this.client).uss.chtagFile({
                fspath: ussPath,
                tag: attributes.tag,
                recursive: isDir,
            });
            success &&= response.success;
        }

        if (attributes.uid || attributes.owner) {
            const group = (attributes.gid ?? attributes.group)?.toString();
            const response = await (await this.client).uss.chownFile({
                fspath: ussPath,
                owner: `${attributes.uid?.toString() ?? attributes.owner!}${group ? `:${group}` : ""}`,
                recursive: isDir,
            });
            success &&= response.success;
        }

        if (attributes.perms) {
            const response = await (await this.client).uss.chmodFile({
                fspath: ussPath,
                mode: attributes.perms,
                recursive: isDir,
            });
            success &&= response.success;
        }

        return this.buildZosFilesResponse(undefined, success);
    }

    // biome-ignore lint/suspicious/noExplicitAny: The apiResponse has no strong type
    private buildZosFilesResponse(apiResponse: any, success = true): zosfiles.IZosFilesResponse {
        return { apiResponse, commandResponse: "", success };
    }
}
