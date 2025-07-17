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

package cmds

import common "zowe-native-proto/zowed/types/common"

type IssueConsoleResponse struct {
	common.CommandResponse `tstype:",extends"`
	// Data returned from the console command
	Data string `json:"data"`
}

type IssueTsoResponse struct {
	common.CommandResponse `tstype:",extends"`
	// Data returned from the TSO command
	Data string `json:"data"`
}

type IssueUnixResponse struct {
	common.CommandResponse `tstype:",extends"`
	// Data returned from the UNIX command
	Data string `json:"data"`
}

type PingResponse struct {
	common.CommandResponse `tstype:",extends"`
	// Data returned from the ping command
	Data string `json:"data"`
	// Timestamp when the ping was processed
	Timestamp string `json:"timestamp"`
}
