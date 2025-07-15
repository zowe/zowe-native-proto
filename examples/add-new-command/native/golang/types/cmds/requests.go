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

type IssueConsoleRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"consoleCommand\""`
	// Console command to execute
	CommandText string `json:"commandText"`
	// Name of the console
	ConsoleName string `json:"consoleName,omitempty"`
}

type IssueTsoRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"tsoCommand\""`
	// TSO command to execute
	CommandText string `json:"commandText"`
}

type IssueUnixRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"unixCommand\""`
	// UNIX command to execute
	CommandText string `json:"commandText"`
}

type PingRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"ping\""`
	// Optional message to include in ping
	Message string `json:"message,omitempty"`
}
