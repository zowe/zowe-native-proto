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

package main

import (
	cmds "zowe-native-proto/ioserver/types/cmds"
	utils "zowe-native-proto/ioserver/utils"
)

// HandleConsoleCommandRequest handles a ConsoleCommandRequest by invoking the `zowex console issue` command
func HandleConsoleCommandRequest(jsonData []byte) {
	request, err := utils.ParseCommandRequest[cmds.IssueConsoleRequest](jsonData)
	if err != nil {
		return
	}

	args := []string{"console", "issue", request.CommandText, "--cn", request.ConsoleName}
	out, err := utils.BuildCommandAuthorized(args).Output()
	if err != nil {
		utils.PrintErrorResponse("Failed to execute command: %v", err)
		return
	}

	utils.PrintCommandResponse(cmds.IssueConsoleResponse{
		Data: string(out),
	})
}
