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
	"os/exec"
)

func HandleListFilesRequest(jsonData []byte) {
	var listRequest ListFilesRequest
	err := json.Unmarshal(jsonData, &listRequest)
	if err != nil {
		// log.Println("Error decoding ListFilesRequest:", err)
		return
	}

	dirPath := listRequest.Path

	entries, err := os.ReadDir(dirPath)
	if err != nil {
		log.Println("Error reading directory:", err)
		return
	}

	ussResponse := ListFilesResponse{
		Items: make([]UssItem, len(entries)),
	}

	for i, entry := range entries {
		ussResponse.Items[i] = UssItem{
			Name:  entry.Name(),
			IsDir: entry.IsDir(),
		}
	}

	ussResponse.ReturnedRows = len(ussResponse.Items)

	v, err := json.Marshal(ussResponse)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		fmt.Println(string(v))
	}
}

func HandleReadFileRequest(jsonData []byte) {
	var request ReadFileRequest
	err := json.Unmarshal(jsonData, &request)
	if err != nil || (request.Encoding == "" && request.Path == "") {
		// log.Println("Error decoding ReadFileRequest:", err)
		return
	}
	// log.Println("ReadFileRequest received:", ...)
	args := []string{"./zowex", "uss", "view", request.Path}
	hasEncoding := len(request.Encoding) != 0
	if hasEncoding {
		args = append(args, "--encoding", request.Encoding)
	}
	out, err := exec.Command(args[0], args[1:]...).Output()
	if err != nil {
		log.Println("Error executing command:", err)
		return
	}

	data := collectContentsAsBytes(string(out), hasEncoding)

	response := ReadFileResponse{
		Encoding: request.Encoding,
		Path:     request.Path,
		Data:     data,
	}
	v, err := json.Marshal(response)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		fmt.Println(string(v))
	}
}

func HandleWriteFileRequest(jsonData []byte) {
	var request WriteFileRequest
	err := json.Unmarshal(jsonData, &request)
	if err != nil || (request.Encoding == "" && request.Path == "") {
		// log.Println("Error decoding WriteFileRequest:", err)
		return
	}
	// log.Println("WriteFileRequest received:", ...)
	decodedBytes, err := base64.StdEncoding.DecodeString(request.Contents)
	if err != nil {
		log.Println("Error decoding base64 contents:", err)
		return
	}
	args := []string{"./zowex", "uss", "write", request.Path}
	if len(request.Encoding) > 0 {
		args = append(args, "--encoding", request.Encoding)
	}
	cmd := exec.Command(args[0], args[1:]...)
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

	response := WriteFileResponse{
		Success: true,
		Path:    request.Path,
	}
	v, err := json.Marshal(response)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		fmt.Println(string(v))
	}
}
