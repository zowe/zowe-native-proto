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
	"log"
	"os"

	t "zowe-native-proto/ioserver/types/common"
	utils "zowe-native-proto/ioserver/utils"
)

func main() {
	utils.SetAutoConvOnUntaggedStdio()
	input := make(chan []byte)

	cmd := utils.BuildCommand([]string{"./zowex", "--it"})
	cmd.Stderr = os.Stderr
	stdin, err := cmd.StdinPipe()
	if err != nil {
		panic(err)
	}
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		panic(err)
	}
	conn := utils.ReadWriteCloser{
		ReadCloser:  stdout,
		WriteCloser: stdin,
	}
	cmd.Start()
	if _, err = bufio.NewReader(stdout).ReadBytes('\n'); err != nil {
		panic(err)
	}
	defer conn.Close()

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

	type CommandHandler func(utils.ReadWriteCloser, []byte)
	commandHandlers := map[string]CommandHandler{
		"readDataset":    HandleReadDatasetRequest,
		"readFile":       HandleReadFileRequest,
		"readSpool":      HandleReadSpoolRequest,
		"getJcl":         HandleGetJclRequest,
		"getStatus":      HandleGetStatusRequest,
		"writeDataset":   HandleWriteDatasetRequest,
		"writeFile":      HandleWriteFileRequest,
		"listDatasets":   HandleListDatasetsRequest,
		"listDsMembers":  HandleListDsMembersRequest,
		"listFiles":      HandleListFilesRequest,
		"listJobs":       HandleListJobsRequest,
		"listSpools":     HandleListSpoolsRequest,
		"consoleCommand": HandleConsoleCommandRequest,
		"restoreDataset": HandleRestoreDatasetRequest,
		"deleteDataset":  HandleDeleteDatasetRequest,
	}

	for data := range input {
		var request t.CommandRequest
		err := json.Unmarshal(data, &request)
		if err != nil {
			log.Println("Error parsing command request:", err)
			continue
		}

		if handler, ok := commandHandlers[request.Command]; ok {
			handler(conn, data)
		}
	}
}
