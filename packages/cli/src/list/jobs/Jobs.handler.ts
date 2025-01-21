/*
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

import type { IHandlerParameters } from "@zowe/imperative";
import type { SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import {
	type IListJobsRequest,
	type IListJobsResponse,
	ZSshClient,
} from "zowe-native-proto-sdk";
import { SshBaseHandler } from "../../SshBaseHandler";

export default class ListJobsHandler extends SshBaseHandler {
	public async processWithSession(
		params: IHandlerParameters,
		session: SshSession,
	): Promise<IListJobsResponse> {
		using client = await ZSshClient.create(session);
		const request: IListJobsRequest = {
			command: "listJobs",
			owner: params.arguments.owner,
			prefix: params.arguments.prefix,
			status: params.arguments.status,
		};
		const response = await client.request<IListJobsResponse>(request);
		params.response.data.setMessage(
			"Successfully listed %d matching jobs",
			response.items.length,
		);
		params.response.format.output({
			output: response.items,
			format: "table",
			fields: ["id", "name", "status", "retcode"],
		});
		return response;
	}
}
