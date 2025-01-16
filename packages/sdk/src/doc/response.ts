export interface IRpcResponse {}

export interface IListUssResponse extends IRpcResponse {
    items: { name: string }[];
    returnedRows: number;
}
