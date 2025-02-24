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
	cmdTypes "zowe-native-proto/ioserver/types/cmds"
	utils "zowe-native-proto/ioserver/utils"
)

// HandleConsoleCommandRequest handles a ConsoleCommandRequest by invoking the `zowex console issue` command
func HandleConsoleCommandRequest(_conn utils.StdioConn, p any) (result any, e error) {
	params := p.(cmdTypes.IssueConsoleRequest)
	args := []string{"console", "issue", params.CommandText, "--cn", params.ConsoleName}
	out, err := utils.BuildCommandAuthorized(args).Output()
	if err != nil {
		e = fmt.Errorf("Failed to execute command: %v", err)
		return
	}

	result = cmdTypes.IssueConsoleResponse{
		Data: string(out),
	}
	return
}
