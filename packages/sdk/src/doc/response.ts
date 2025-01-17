import { Dataset, DsMember, Job, UssItem } from "./types";

export interface IRpcResponse {}

export interface IListDatasetsResponse extends IRpcResponse {
    items: Dataset[];
    returnedRows: number;
}

export interface IListDsMembersResponse extends IRpcResponse {
    items: DsMember[];
    returnedRows: number;
}

export interface IListFilesResponse extends IRpcResponse {
    items: UssItem[];
    returnedRows: number;
}

export interface IListJobsResponse extends IRpcResponse {
    items: Job[];
}

export interface IReadDatasetResponse extends IRpcResponse {
    data: Buffer;
    dataset: string;
    encoding?: string;
}
