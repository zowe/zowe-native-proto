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

import (
	"fmt"
	cmdTypes "zowe-native-proto/zowed/types/cmds"
	utils "zowe-native-proto/zowed/utils"
	"time"
)

// HandleConsoleCommandRequest handles a ConsoleCommandRequest by invoking the `zowex console issue` command
func HandleConsoleCommandRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[cmdTypes.IssueConsoleRequest](params)
	if err != nil {
		return nil, err
	}

	consoleName := "ZOWE00CN"
	if request.ConsoleName != "" {
		consoleName = request.ConsoleName
	}
	args := []string{"console", "issue", request.CommandText, "--cn", consoleName}
	cmd := utils.BuildCommandAuthorized(args)
	out, err := cmd.Output()
	if err != nil {
		e = fmt.Errorf("Failed to execute command: %v", err)
		conn.LastExitCode = cmd.ProcessState.ExitCode()
		return
	}

	result = cmdTypes.IssueConsoleResponse{
		Data: string(out),
	}
	return
}

// HandlePingRequest handles a PingRequest by invoking the `zowex ping` command
func HandlePingRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[cmdTypes.PingRequest](params)
	if err != nil {
		return nil, err
	}

	args := []string{"ping"}
	if request.Message != "" {
		args = append(args, "--message", request.Message)
	}
	
	cmd := utils.BuildCommand(args)
	out, err := cmd.Output()
	if err != nil {
		e = fmt.Errorf("Failed to execute ping command: %v", err)
		conn.LastExitCode = cmd.ProcessState.ExitCode()
		return
	}

	result = cmdTypes.PingResponse{
		Data: string(out),
		Timestamp: fmt.Sprintf("%d", time.Now().Unix()),
	}
	return
}
