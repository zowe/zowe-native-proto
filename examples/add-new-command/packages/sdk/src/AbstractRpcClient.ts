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

import type { CommandRequest, CommandResponse, cmds, ds, jobs, uss, sample } from "./doc/rpc";

export abstract class AbstractRpcClient {
    // ... existing methods (request, ds, jobs, uss, cmds) ...

    public get sample() {
        return {
            ping: (request: Omit<sample.PingRequest, "command">): Promise<sample.PingResponse> =>
                this.request({ command: "ping", ...request }),
        };
    }
}
