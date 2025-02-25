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
	"bufio"
	"encoding/json"
	"fmt"
	"log"
	"os"

	"zowe-native-proto/ioserver/cmds"
	t "zowe-native-proto/ioserver/types/common"
	utils "zowe-native-proto/ioserver/utils"
)

func main() {
	utils.InitLogger(false)
	utils.SetAutoConvOnUntaggedStdio()
	// Channel for receiving input from stdin
	input := make(chan []byte)

	cmd := utils.BuildCommand([]string{"--it"})
	stdin, err := cmd.StdinPipe()
	if err != nil {
		panic(err)
	}
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		panic(err)
	}
	stderr, err := cmd.StderrPipe()
	if err != nil {
		panic(err)
	}
	conn := utils.StdioConn{
		Stdin:  stdin,
		Stdout: stdout,
		Stderr: stderr,
	}
	cmd.Start()
	if _, err = bufio.NewReader(stdout).ReadBytes('\n'); err != nil {
		panic(err)
	}
	defer conn.Close()

	go func() {
		buf := make([]byte, 1024)
		for {
			// Read input from stdin
			n, err := os.Stdin.Read(buf)
			if err != nil {
				if err.Error() == "EOF" {
					os.Exit(0)
				}
				log.Fatalln("Error reading from stdin:", err)
			}
			input <- buf[:n]
		}
	}()

	// Initialize the command dispatcher and register all core commands
	dispatcher := cmds.NewDispatcher()
	cmds.InitializeCoreHandlers(dispatcher)

	for data := range input {
		// Parse the command request
		var request t.RpcRequest
		err := json.Unmarshal(data, &request)
		if err != nil {
			utils.PrintErrorResponse(t.ErrorDetails{
				Code:    -32700,
				Message: fmt.Sprintf("Failed to parse command request: %v", err),
			}, nil)
			continue
		}

		// Handle the command request if a supported command is provided
		if handler, ok := dispatcher.Get(request.Method); ok {
			result, err := handler(conn, request.Params)
			if err != nil {
				utils.PrintErrorResponse(t.ErrorDetails{
					Code:    1,
					Message: err.Error(),
				}, &request.Id)
			} else {
				utils.PrintCommandResponse(result, request.Id)
			}
		} else {
			utils.PrintErrorResponse(t.ErrorDetails{
				Code:    -32601,
				Message: fmt.Sprintf("Unrecognized command %s", request.Method),
			}, &request.Id)
		}
	}
}
