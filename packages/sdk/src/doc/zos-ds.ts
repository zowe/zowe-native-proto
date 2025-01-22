import type { IRpcRequest, IRpcResponse } from "./common";
import type { Dataset, DsMember, Job, UssItem } from "./types";

export namespace ListDatasets {
    export interface Request extends IRpcRequest {
        command: "listDatasets";
        pattern: string;
    }

    export interface Response extends IRpcResponse {
        items: Dataset[];
        returnedRows: number;
    }
}

export namespace ListDsMembers {
    export interface Request extends IRpcRequest {
        command: "listDsMembers";
        dataset: string;
    }

    export interface Response extends IRpcResponse {
        items: DsMember[];
        returnedRows: number;
    }
}

export namespace ReadDataset {
    export interface Request extends IRpcRequest {
        command: "readDataset";
        dataset: string;
        encoding?: string;
    }

    export interface Response extends IRpcResponse {
        data: Buffer;
        dataset: string;
        encoding?: string;
    }
}
