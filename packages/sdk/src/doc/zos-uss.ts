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
import type { UssItem } from "./types";

export namespace ListFiles {
    export interface Request extends IRpcRequest {
        command: "listFiles";
        fspath: string;
    }

    export interface Response extends IRpcResponse {
        items: UssItem[];
        returnedRows: number;
    }
}

export namespace ReadFile {
    export interface Request extends IRpcRequest {
        command: "readFile";
        path: string;
        encoding?: string;
    }

    export interface Response extends IRpcResponse {
        data: Buffer | string;
        file: string;
        encoding?: string;
    }
}

export namespace WriteFile {
    export interface Request extends IRpcRequest {
        command: "writeFile";
        path: string;
        contents: Buffer | string;
        encoding?: string;
    }

    export interface Response extends IRpcResponse {
        path: string;
        success: boolean;
    }
}
