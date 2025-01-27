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
)

func main() {
	// First, disable _BPXK_AUTOCVT for this process and all children. Otherwise, this will cause issues when
	// piping data between layers.
	err := os.Setenv("_BPXK_AUTOCVT", "")
	if err != nil {
		log.Fatalln("Error disabling _BPXK_AUTOCVT during initialization:", err)
		return
	}

	input := make(chan []byte)

	go func() {
		buf := make([]byte, 1024)
		for {
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
	commandHandlers := map[string]CommandHandler{
		"readDataset":    HandleReadDatasetRequest,
		"readFile":       HandleReadFileRequest,
		"readSpool":      HandleReadSpoolRequest,
		"getJcl":         HandleGetJclRequest,
		"writeDataset":   HandleWriteDatasetRequest,
		"writeFile":      HandleWriteFileRequest,
		"listDatasets":   HandleListDatasetsRequest,
		"listDsMembers":  HandleListDsMembersRequest,
		"listFiles":      HandleListFilesRequest,
		"listJobs":       HandleListJobsRequest,
		"listSpools":     HandleListSpoolsRequest,
		"consoleCommand": HandleConsoleCommandRequest,
	}

	for data := range input {
		var request CommandRequest
		err := json.Unmarshal(data, &request)
		if err != nil {
			log.Println("Error parsing command request:", err)
			continue
		}

		if handler, ok := commandHandlers[request.Command]; ok {
			handler(data)
		}
	}
}
