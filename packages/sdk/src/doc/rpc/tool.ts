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

import type * as common from "./common";
export interface ToolSearchRequest extends common.CommandRequest<"toolSearch"> {
    /**
     * Data set name to search
     */
    dsname: string;

    /**
     * String to search for
     */
    string: string;

    /**
     * Parms to pass to the ISRSUPC
     */
    parms?: string;
}

export interface ToolSearchResponse extends common.CommandResponse {
    /**
     * Data returned from the TSO command
     */
    data: string;
}
