import type * as common from "./common";

export interface PingRequest extends common.CommandRequest<"ping"> {
    message?: string;
}

export interface PingResponse extends common.CommandResponse {
    data: string;
    timestamp: string;
}
