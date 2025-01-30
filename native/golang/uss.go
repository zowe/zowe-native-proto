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
)

func HandleListFilesRequest(jsonData []byte) {
	var listRequest ListFilesRequest
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

	ussResponse := ListFilesResponse{}

	if !fileInfo.IsDir() {
		ussResponse.Items = make([]UssItem, 1)
		ussResponse.Items[0] = UssItem{
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
		ussResponse.Items = make([]UssItem, len(entries))

		for i, entry := range entries {
			ussResponse.Items[i] = UssItem{
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
	out, err := buildExecCommand(args).Output()
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
		fmt.Fprintln(os.Stderr, err.Error())
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

	// Temporarily disable _BPXK_AUTOCVT for this process and all children. Otherwise, this will cause issues when
	// piping data between layers.
	autocvt := os.Getenv("_BPXK_AUTOCVT")

	err = os.Setenv("_BPXK_AUTOCVT", "")
	if err != nil {
		log.Println("Error disabling _BPXK_AUTOCVT during write:", err)
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
	cmd := buildExecCommand(args)
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

	// Restore value for _BPXK_AUTOCVT
	err = os.Setenv("_BPXK_AUTOCVT", autocvt)
	if err != nil {
		log.Println("Error restoring _BPXK_AUTOCVT after write:", err)
		return
	}

	response := WriteFileResponse{
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
