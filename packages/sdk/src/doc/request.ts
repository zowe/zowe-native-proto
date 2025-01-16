export interface IRpcRequest {
    command: string;
}

export interface IListUssRequest extends IRpcRequest {
    command: "listUss";
    fspath: string;
}
