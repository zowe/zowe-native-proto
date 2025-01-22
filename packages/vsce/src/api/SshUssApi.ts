import { writeFileSync } from "node:fs";
import type * as zosfiles from "@zowe/zos-files-for-zowe-sdk";
import { type MainframeInteraction, imperative } from "@zowe/zowe-explorer-api";
import { type ListFiles, type ReadFile, ZSshUtils } from "zowe-native-proto-sdk";
import { SshClientCache } from "../SshClientCache";
import { SshCommonApi } from "./SshCommonApi";

export class SshUssApi extends SshCommonApi implements MainframeInteraction.IUss {
    public async fileList(ussFilePath: string): Promise<zosfiles.IZosFilesResponse> {
        const client = await SshClientCache.inst.connect(this.getSshSession());
        const request: ListFiles.Request = {
            command: "listFiles",
            fspath: ussFilePath,
        };
        const response = await client.request<ListFiles.Response>(request);
        return this.buildZosFilesResponse({
            items: response.items.map((item) => ({ name: item.name, mode: item.isDir ? "d" : "f" })),
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
        const client = await SshClientCache.inst.connect(this.getSshSession());
        const request: ReadFile.Request = {
            command: "readFile",
            path: ussFilePath,
            encoding: options.encoding,
        };
        const response = await client.request<ReadFile.Response>(request);
        if (options.file != null) {
            imperative.IO.createDirsSyncFromFilePath(options.file);
            writeFileSync(options.file, ZSshUtils.decodeByteArray(response.data));
        } else if (options.stream != null) {
            options.stream.write(ZSshUtils.decodeByteArray(response.data));
            options.stream.end();
        }
        return this.buildZosFilesResponse({ etag: ussFilePath });
    }

    public uploadFromBuffer(
        buffer: Buffer,
        filePath: string,
        _options?: zosfiles.IUploadOptions,
    ): Promise<zosfiles.IZosFilesResponse> {
        throw new Error("Not yet implemented");
    }

    public async putContent(
        inputFilePath: string,
        ussFilePath: string,
        _options?: zosfiles.IUploadOptions,
    ): Promise<zosfiles.IZosFilesResponse> {
        throw new Error("Not yet implemented");
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
