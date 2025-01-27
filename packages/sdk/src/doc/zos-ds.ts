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
import type { Dataset, DsMember } from "./types";

export namespace ListDatasets {
    export interface Request extends IRpcRequest {
        command: "listDatasets";
        pattern: string;
        attributes?: boolean;
    }

    export interface Response extends IRpcResponse {
        items: Dataset[];
        returnedRows: number;
    }
}

export namespace ListDsMembers {
    export interface Request extends IRpcRequest {
        command: "listDsMembers";
        dataset: string;
    }

    export interface Response extends IRpcResponse {
        items: DsMember[];
        returnedRows: number;
    }
}

export namespace ReadDataset {
    export interface Request extends IRpcRequest {
        command: "readDataset";
        dataset: string;
        encoding?: string;
    }

    export interface Response extends IRpcResponse {
        data: Buffer | string;
        dataset: string;
        encoding?: string;
    }
}

export namespace WriteDataset {
    export interface Request extends IRpcRequest {
        command: "writeDataset";
        dataset: string;
        contents: Buffer | string;
        encoding?: string;
    }

    export interface Response extends IRpcResponse {
        dataset: string;
        success: boolean;
    }
}
