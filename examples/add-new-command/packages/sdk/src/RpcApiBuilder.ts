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

type ProgressCallback = (percent: number) => void;
type RequestHandler = (request: CommandRequest, progressCallback?: ProgressCallback) => Promise<CommandResponse>;

// biome-ignore lint/complexity/noStaticOnlyClass: Builder class has static methods
export abstract class RpcApiBuilder {
    public static build(requestFn: RequestHandler) {
        return {
            cmds: RpcApiBuilder.buildCmdsApi(requestFn),
            ds: RpcApiBuilder.buildDsApi(requestFn),
            jobs: RpcApiBuilder.buildJobsApi(requestFn),
            uss: RpcApiBuilder.buildUssApi(requestFn),
            sample: RpcApiBuilder.buildSampleApi(requestFn), // Add this line
        };
    }

    // ... existing api builders (cmds, ds, jobs, uss) ...

    public static buildSampleApi(requestFn: RequestHandler) {
        const rpc = RpcApiBuilder.requestHandler(requestFn);
        return {
            ping: rpc<sample.PingRequest, sample.PingResponse>("ping"),
        };
    }
}
