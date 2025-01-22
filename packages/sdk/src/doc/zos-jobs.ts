import type { IRpcRequest, IRpcResponse } from "./common";
import type { Job } from "./types";

export namespace ListJobs {
    export interface Request extends IRpcRequest {
        command: "listJobs";
        owner?: string;
        prefix?: string;
        status?: string;
    }

    export interface Response extends IRpcResponse {
        items: Job[];
    }
}
