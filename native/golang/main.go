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

	type CommandHandler func([]byte)
	// Supported ioserver commands
	commandHandlers := map[string]CommandHandler{
		"chownFile":      HandleChownFileRequest,
		"chmodFile":      HandleChmodFileRequest,
		"chtagFile":      HandleChtagFileRequest,
		"consoleCommand": HandleConsoleCommandRequest,
		"deleteDataset":  HandleDeleteDatasetRequest,
		"deleteFile":     HandleDeleteFileRequest,
		"getJcl":         HandleGetJclRequest,
		"getStatus":      HandleGetStatusRequest,
		"listDatasets":   HandleListDatasetsRequest,
		"listDsMembers":  HandleListDsMembersRequest,
		"listFiles":      HandleListFilesRequest,
		"listJobs":       HandleListJobsRequest,
		"listSpools":     HandleListSpoolsRequest,
		"readDataset":    HandleReadDatasetRequest,
		"readFile":       HandleReadFileRequest,
		"readSpool":      HandleReadSpoolRequest,
		"restoreDataset": HandleRestoreDatasetRequest,
		"submitJob":      HandleSubmitJobRequest,
		"submitJcl":      HandleSubmitJclRequest,
		"writeDataset":   HandleWriteDatasetRequest,
		"writeFile":      HandleWriteFileRequest,
	}

	for data := range input {
		// Parse the command request
		var request t.CommandRequest
		err := json.Unmarshal(data, &request)
		if err != nil {
			utils.PrintErrorResponse("Failed to parse command request: %v", err)
			continue
		}

		// Handle the command request if a supported command is provided
		if handler, ok := commandHandlers[request.Command]; ok {
			handler(data)
		}
	}
}
