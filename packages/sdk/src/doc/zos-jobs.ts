import type { IRpcRequest, IRpcResponse } from "./common";
import type { Job, Spool } from "./types";

export namespace GetJcl {
    export interface Request extends IRpcRequest {
        command: "getJcl";
        jobId: string;
    }

    export interface Response extends IRpcResponse {
        data: string;
        jobId: string;
    }
}

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

export namespace ListSpools {
    export interface Request extends IRpcRequest {
        command: "listSpools";
        jobId: string;
    }

    export interface Response extends IRpcResponse {
        items: Spool[];
    }
}

export namespace ReadSpool {
    export interface Request extends IRpcRequest {
        command: "readSpool";
        dsnKey: number;
        jobId: string;
        encoding?: string;
    }

    export interface Response extends IRpcResponse {
        data: Buffer | string;
        dsnKey: number;
        jobId: string;
        encoding?: string;
    }
}
