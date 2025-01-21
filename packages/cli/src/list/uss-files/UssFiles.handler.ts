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
	type IListFilesRequest,
	type IListFilesResponse,
	ZSshClient,
} from "zowe-native-proto-sdk";
import { SshBaseHandler } from "../../SshBaseHandler";

export default class ListUssFilesHandler extends SshBaseHandler {
	public async processWithSession(
		params: IHandlerParameters,
		session: SshSession,
	): Promise<IListFilesResponse> {
		// const directory = UssUtils.normalizeUnixPath(params.arguments.directory);
		const directory = params.arguments.directory;
		using client = await ZSshClient.create(session);
		const request: IListFilesRequest = {
			command: "listFiles",
			fspath: directory,
		};
		const response = await client.request<IListFilesResponse>(request);
		params.response.data.setMessage(
			"Listed files in uss directory %s",
			directory,
		);
		params.response.format.output({
			output: response.items,
			format: "table",
			// fields: ["name", "size", "owner", "group", "permissions"]
			fields: ["name"],
		});
		return response;
	}
}
