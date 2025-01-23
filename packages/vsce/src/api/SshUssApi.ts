import { readFileSync, writeFileSync } from "node:fs";
import type * as zosfiles from "@zowe/zos-files-for-zowe-sdk";
import { type MainframeInteraction, imperative } from "@zowe/zowe-explorer-api";
import { ZSshUtils } from "zowe-native-proto-sdk";
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
            path: ussFilePath,
            encoding: options.encoding,
        });
        if (options.file != null) {
            imperative.IO.createDirsSyncFromFilePath(options.file);
            writeFileSync(options.file, ZSshUtils.decodeByteArray(response.data));
        } else if (options.stream != null) {
            options.stream.write(ZSshUtils.decodeByteArray(response.data));
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
            path: filePath,
            encoding: options?.encoding,
            contents: ZSshUtils.encodeByteArray(buffer),
        });
        return this.buildZosFilesResponse({ etag: filePath });
    }

    public async putContent(
        inputFilePath: string,
        ussFilePath: string,
        options?: zosfiles.IUploadOptions,
    ): Promise<zosfiles.IZosFilesResponse> {
        const response = await (await this.client).uss.writeFile({
            path: ussFilePath,
            encoding: options?.encoding,
            contents: ZSshUtils.encodeByteArray(readFileSync(inputFilePath)),
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
        throw new Error("Not yet implemented");
    }

    public async delete(ussPath: string, recursive?: boolean | undefined): Promise<zosfiles.IZosFilesResponse> {
        throw new Error("Not yet implemented");
    }

    public async rename(currentUssPath: string, newUssPath: string): Promise<zosfiles.IZosFilesResponse> {
        throw new Error("Not yet implemented");
    }

    private buildZosFilesResponse(apiResponse: any, success = true): zosfiles.IZosFilesResponse {
        return { apiResponse, commandResponse: "", success };
    }
}
