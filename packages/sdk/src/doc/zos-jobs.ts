/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

import type { IRpcRequest, IRpcResponse } from "./common";
import type { Job, Spool } from "./types";

export namespace GetJcl {
    export interface Request extends IRpcRequest {
        command: "getJcl";
        jobId: string;
        // TODO(zFernand0): Add encoding support?
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

export namespace GetStatus {
    export interface Request extends IRpcRequest {
        command: "getStatus";
        jobId: string;
    }

    export interface Response extends IRpcResponse {
        items: Job[];
    }
}
