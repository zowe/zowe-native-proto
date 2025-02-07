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
	"encoding/base64"
	"encoding/json"
	"fmt"
	"log"
	"os"
	"path/filepath"
	t "zowe-native-proto/ioserver/types/common"
	uss "zowe-native-proto/ioserver/types/uss"
	utils "zowe-native-proto/ioserver/utils"
)

func HandleListFilesRequest(jsonData []byte) {
	var listRequest uss.ListFilesRequest
	err := json.Unmarshal(jsonData, &listRequest)
	if err != nil {
		// log.Println("Error decoding ListFilesRequest:", err)
		return
	}

	dirPath := listRequest.Path

	fileInfo, err := os.Stat(dirPath)
	if err != nil {
		return
	}

	ussResponse := uss.ListFilesResponse{}

	if !fileInfo.IsDir() {
		ussResponse.Items = make([]t.UssItem, 1)
		ussResponse.Items[0] = t.UssItem{
			Name:  filepath.Base(dirPath),
			IsDir: false,
		}
		ussResponse.ReturnedRows = 1
	} else {
		entries, err := os.ReadDir(dirPath)
		if err != nil {
			log.Println("Error reading directory:", err)
			return
		}
		ussResponse.Items = make([]t.UssItem, len(entries))

		for i, entry := range entries {
			ussResponse.Items[i] = t.UssItem{
				Name:  entry.Name(),
				IsDir: entry.IsDir(),
			}
		}

		ussResponse.ReturnedRows = len(ussResponse.Items)
	}

	v, err := json.Marshal(ussResponse)
	if err != nil {
		fmt.Fprintln(os.Stderr, err.Error())
	} else {
		fmt.Println(string(v))
	}
}

func HandleReadFileRequest(jsonData []byte) {
	var request uss.ReadFileRequest
	err := json.Unmarshal(jsonData, &request)
	if err != nil || (request.Encoding == "" && request.Path == "") {
		// log.Println("Error decoding ReadFileRequest:", err)
		return
	}
	// log.Println("ReadFileRequest received:", ...)
	args := []string{"./zowex", "uss", "view", request.Path}
	hasEncoding := len(request.Encoding) != 0
	if hasEncoding {
		args = append(args, "--encoding", request.Encoding, "--rfb", "true")
	}
	out, err := utils.BuildCommand(args).Output()
	if err != nil {
		log.Println("Error executing command:", err)
		return
	}

	data := utils.CollectContentsAsBytes(string(out), hasEncoding)

	response := uss.ReadFileResponse{
		Encoding: request.Encoding,
		Path:     request.Path,
		Data:     data,
	}
	v, err := json.Marshal(response)
	if err != nil {
		fmt.Fprintln(os.Stderr, err.Error())
	} else {
		fmt.Println(string(v))
	}
}

func HandleWriteFileRequest(jsonData []byte) {
	var request uss.WriteFileRequest
	err := json.Unmarshal(jsonData, &request)
	if err != nil || (request.Encoding == "" && request.Path == "") {
		// log.Println("Error decoding WriteFileRequest:", err)
		return
	}

	// log.Println("WriteFileRequest received:", ...)
	decodedBytes, err := base64.StdEncoding.DecodeString(request.Data)
	if err != nil {
		log.Println("Error decoding base64 contents:", err)
		return
	}
	args := []string{"./zowex", "uss", "write", request.Path}
	if len(request.Encoding) > 0 {
		args = append(args, "--encoding", request.Encoding)
	}
	cmd := utils.BuildCommandNoAutocvt(args)
	stdin, err := cmd.StdinPipe()
	if err != nil {
		log.Println("Error opening stdin pipe:", err)
		return
	}

	go func() {
		defer stdin.Close()
		_, err = stdin.Write(decodedBytes)
		if err != nil {
			log.Println("Error writing to stdin pipe:", err)
		}
	}()

	out, err := cmd.Output()
	if err != nil {
		log.Println("Error piping stdin to command:", err)
		return
	}
	// discard CLI output as its currently unused
	_ = out

	response := uss.WriteFileResponse{
		Success: true,
		Path:    request.Path,
	}
	v, err := json.Marshal(response)
	if err != nil {
		fmt.Fprintln(os.Stderr, err.Error())
	} else {
		fmt.Println(string(v))
	}
}
