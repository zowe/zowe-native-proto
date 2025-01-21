export interface IRpcRequest {
    command: string;
}

export interface IListDatasetsRequest extends IRpcRequest {
    command: "listDatasets";
    pattern: string;
}

export interface IListDsMembersRequest extends IRpcRequest {
    command: "listDsMembers";
    dataset: string;
}

export interface IListFilesRequest extends IRpcRequest {
    command: "listFiles";
    fspath: string;
}

export interface IListJobsRequest extends IRpcRequest {
    command: "listJobs";
    owner?: string;
    prefix?: string;
    status?: string;
}

export interface IReadDatasetRequest extends IRpcRequest {
    command: "readDataset";
    dataset: string;
    encoding?: string;
}
