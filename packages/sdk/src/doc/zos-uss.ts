import type { IRpcRequest, IRpcResponse } from "./common";
import type { UssItem } from "./types";

export namespace ListFiles {
    export interface Request extends IRpcRequest {
        command: "listFiles";
        fspath: string;
    }

    export interface Response extends IRpcResponse {
        items: UssItem[];
        returnedRows: number;
    }
}
