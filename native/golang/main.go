/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0 & Apache-2.0
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
		"readDataset":   HandleReadDatasetRequest,
		"readSpool":     HandleReadSpoolRequest,
		"getJcl":        HandleGetJclRequest,
		"writeDataset":  HandleWriteDatasetRequest,
		"listDatasets":  HandleListDatasetsRequest,
		"listDsMembers": HandleListDsMembersRequest,
		"listFiles":     HandleListFilesRequest,
		"listJobs":      HandleListJobsRequest,
		"listSpools":    HandleListSpoolsRequest,
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
