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
	"os/exec"
)

func HandleConsoleCommandRequest(jsonData []byte) {
	var request IssueConsoleRequest
	err := json.Unmarshal(jsonData, &request)
	if err != nil {
		return
	}
	args := []string{"./zowexx", "console", "issue", request.Command, "--cn", request.Console}
	out, err := exec.Command(args[0], args[1:]...).Output()
	if err != nil {
		log.Println("Error executing command:", err)
		log.Println(string(out))
		return
	}

	response := IssueConsoleResponse{
		Data: string(out),
	}
	v, err := json.Marshal(response)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		fmt.Println(string(v))
	}
}
