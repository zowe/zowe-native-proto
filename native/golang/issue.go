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
	"encoding/json"
	"fmt"
	"log"
	"os"
	cmds "zowe-native-proto/ioserver/types/cmds"
	utils "zowe-native-proto/ioserver/utils"
)

func HandleConsoleCommandRequest(conn utils.ReadWriteCloser, jsonData []byte) {
	var request cmds.IssueConsoleRequest
	err := json.Unmarshal(jsonData, &request)
	if err != nil {
		return
	}
	args := []string{"./zowexx", "console", "issue", request.CommandText, "--cn", request.ConsoleName}
	out, err := conn.ExecCmd(args)
	if err != nil {
		log.Println("Error executing command:", err)
		log.Println(string(out))
		return
	}

	response := cmds.IssueConsoleResponse{
		Data: string(out),
	}
	v, err := json.Marshal(response)
	if err != nil {
		fmt.Fprintln(os.Stderr, err.Error())
	} else {
		fmt.Println(string(v))
	}
}
