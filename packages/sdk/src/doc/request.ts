export interface IRpcRequest {
    command: string;
}

export interface IListDatasetRequest extends IRpcRequest {
    command: "listDataset";
    pattern: string;
}

export interface IListUssRequest extends IRpcRequest {
    command: "listUss";
    fspath: string;
}

export interface IReadDatasetRequest extends IRpcRequest {
    command: "readDataset";
    dataset: string;
    encoding?: string;
}
