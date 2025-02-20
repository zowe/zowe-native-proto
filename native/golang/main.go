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
	"log"
	"os"

	"zowe-native-proto/ioserver/cmds"
	t "zowe-native-proto/ioserver/types/common"
	utils "zowe-native-proto/ioserver/utils"
)

func main() {
	utils.InitLogger()
	utils.SetAutoConvOnUntaggedStdio()
	// Channel for receiving input from stdin
	input := make(chan []byte)

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

	dispatcher := cmds.NewDispatcher()
	cmds.InitializeCoreHandlers(dispatcher)

	for data := range input {
		// Parse the command request
		var request t.CommandRequest
		err := json.Unmarshal(data, &request)
		if err != nil {
			utils.PrintErrorResponse("Failed to parse command request: %v", err)
			continue
		}

		// Handle the command request if a supported command is provided
		if handler, ok := dispatcher.Get(request.Command); ok {
			handler(data)
		} else {
			utils.PrintErrorResponse("Unrecognized command %s", request.Command)
		}
	}
}
