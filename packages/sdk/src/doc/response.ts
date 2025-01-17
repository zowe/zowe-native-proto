export interface IRpcResponse {}

export interface IListDatasetResponse extends IRpcResponse {
    items: { name: string; dsorg: string; }[];
    returnedRows: number;
}

export interface IListUssResponse extends IRpcResponse {
    items: { name: string }[];
    returnedRows: number;
}

export interface IReadDatasetResponse extends IRpcResponse {
    data: Buffer;
    dataset: string;
    encoding?: string;
}
