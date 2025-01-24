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

export namespace IssueConsole {
    export interface Request extends IRpcRequest {
        command: "consoleCommand";
        commandText: string;
        consoleName: string;
    }

    export interface Response extends IRpcResponse {
        data: string;
    }
}

export namespace IssueTso {
    export interface Request extends IRpcRequest {
        command: "tsoCommand";
        commandText: string;
    }

    export interface Response extends IRpcResponse {
        responseText: string;
    }
}

export namespace IssueUnix {
    export interface Request extends IRpcRequest {
        command: "unixCommand";
        commandText: string;
    }

    export interface Response extends IRpcResponse {
        responseText: string;
    }
}
