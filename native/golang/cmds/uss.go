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

package cmds

import (
	"encoding/base64"
	"os"
	"path/filepath"
	t "zowe-native-proto/ioserver/types/common"
	uss "zowe-native-proto/ioserver/types/uss"
	utils "zowe-native-proto/ioserver/utils"
)

// HandleListFilesRequest handles a ListFilesRequest by invoking built-in functions from Go's `os` module.
func HandleListFilesRequest(jsonData []byte) {
	listRequest, err := utils.ParseCommandRequest[uss.ListFilesRequest](jsonData)
	if err != nil {
		return
	}

	dirPath := listRequest.Path

	fileInfo, err := os.Stat(dirPath)
	if err != nil {
		utils.PrintErrorResponse("Failed to stat directory: %v", err)
		return
	}

	ussResponse := uss.ListFilesResponse{}

	// If the path is not a directory, return a single file item
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
			utils.PrintErrorResponse("Failed to read directory: %v", err)
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

	utils.PrintCommandResponse(ussResponse)
}

// HandleReadFileRequest handles a ReadFileRequest by invoking the `zowex uss view` command
func HandleReadFileRequest(jsonData []byte) {
	request, err := utils.ParseCommandRequest[uss.ReadFileRequest](jsonData)
	if err != nil || (request.Encoding == "" && request.Path == "") {
		return
	}

	args := []string{"uss", "view", request.Path}
	hasEncoding := len(request.Encoding) != 0
	if hasEncoding {
		args = append(args, "--encoding", request.Encoding, "--rfb", "true")
	}
	out, err := utils.BuildCommand(args).Output()
	if err != nil {
		utils.PrintErrorResponse("Error executing command: %v", err)
		return
	}

	output := string(out)
	var data []byte
	if len(output) > 0 {
		data = utils.CollectContentsAsBytes(output, hasEncoding)
	} else {
		data = []byte{}
	}

	response := uss.ReadFileResponse{
		Encoding: request.Encoding,
		Path:     request.Path,
		Data:     data,
	}
	utils.PrintCommandResponse(response)
}

// HandleWriteFileRequest handles a WriteFileRequest by invoking the `zowex uss write` command
func HandleWriteFileRequest(jsonData []byte) {
	request, err := utils.ParseCommandRequest[uss.WriteFileRequest](jsonData)
	if err != nil || (request.Encoding == "" && request.Path == "") {
		return
	}

	decodedBytes, err := base64.StdEncoding.DecodeString(request.Data)
	if err != nil {
		utils.PrintErrorResponse("[WriteFileRequest] Error decoding base64 contents: %v", err)
		return
	}
	args := []string{"uss", "write", request.Path}
	if len(request.Encoding) > 0 {
		args = append(args, "--encoding", request.Encoding)
	}
	cmd := utils.BuildCommandNoAutocvt(args)
	stdin, err := cmd.StdinPipe()
	if err != nil {
		utils.PrintErrorResponse("[WriteFileRequest] Error opening stdin pipe: %v", err)
		return
	}

	go func() {
		defer stdin.Close()
		_, err = stdin.Write(decodedBytes)
		if err != nil {
			utils.PrintErrorResponse("[WriteFileRequest] Error writing to stdin pipe: %v", err)
			return
		}
	}()

	_, err = cmd.Output()
	if err != nil {
		utils.PrintErrorResponse("[WriteFileRequest] Error piping stdin to command: %v", err)
		return
	}

	response := uss.WriteFileResponse{
		Success: err == nil,
		Path:    request.Path,
	}
	utils.PrintCommandResponse(response)
}

// HandleCreateFileRequest handles a CreateFileRequest by invoking the `zowex uss create-dir` or `create-file` command (depending on params)
func HandleCreateFileRequest(jsonData []byte) {
	request, err := utils.ParseCommandRequest[uss.CreateFileRequest](jsonData)
	if err != nil || len(request.Path) == 0 {
		return
	}

	createCmd := "create-file"
	if request.IsDir {
		createCmd = "create-dir"
	}
	args := []string{"uss", createCmd, request.Path}
	if len(request.Mode) > 0 {
		args = append(args, "--mode", request.Mode)
	}
	_, err = utils.BuildCommand(args).Output()
	response := uss.DeleteFileResponse{
		Success: err == nil,
		Path:    request.Path,
	}

	utils.PrintCommandResponse(response)
}

// HandleDeleteFileRequest handles a DeleteFileRequest by invoking the `zowex uss delete` command
func HandleDeleteFileRequest(jsonData []byte) {
	request, err := utils.ParseCommandRequest[uss.DeleteFileRequest](jsonData)
	if err != nil || len(request.Path) == 0 {
		return
	}

	args := []string{"uss", "delete", request.Path}
	if request.Recursive {
		args = append(args, "-r", "true")
	}
	_, err = utils.BuildCommand(args).Output()
	if err != nil {
		utils.PrintErrorResponse("Failed to delete USS item: %s", err.Error())
		return
	}

	response := uss.DeleteFileResponse{
		Success: true,
		Path:    request.Path,
	}

	utils.PrintCommandResponse(response)
}

// HandleChownFileRequest handles a ChownFileRequest by invoking the `zowex uss chown` command
func HandleChownFileRequest(jsonData []byte) {
	request, err := utils.ParseCommandRequest[uss.ChownFileRequest](jsonData)
	if err != nil || len(request.Path) == 0 {
		return
	}

	args := []string{"uss", "chown", request.Owner, request.Path}
	if request.Recursive {
		args = append(args, "-r", "true")
	}
	_, err = utils.BuildCommand(args).Output()
	response := uss.ChownFileResponse{
		Success: err == nil,
		Path:    request.Path,
	}

	utils.PrintCommandResponse(response)
}

// HandleChmodFileRequest handles a ChmodFileRequest by invoking the `zowex uss chmod` command
func HandleChmodFileRequest(jsonData []byte) {
	request, err := utils.ParseCommandRequest[uss.ChmodFileRequest](jsonData)
	if err != nil || len(request.Path) == 0 {
		return
	}

	args := []string{"uss", "chmod", request.Mode, request.Path}
	if request.Recursive {
		args = append(args, "-r", "true")
	}
	_, err = utils.BuildCommand(args).Output()
	response := uss.ChmodFileResponse{
		Success: err == nil,
		Path:    request.Path,
	}

	utils.PrintCommandResponse(response)
}

// HandleChtagFileRequest handles a ChtagFileRequest by invoking the `zowex uss chtag` command
func HandleChtagFileRequest(jsonData []byte) {
	request, err := utils.ParseCommandRequest[uss.ChtagFileRequest](jsonData)
	if err != nil || len(request.Path) == 0 {
		return
	}

	args := []string{"uss", "chtag", request.Tag, request.Path}
	if request.Recursive {
		args = append(args, "-r", "true")
	}
	_, err = utils.BuildCommand(args).Output()
	response := uss.ChtagFileResponse{
		Success: err == nil,
		Path:    request.Path,
	}

	utils.PrintCommandResponse(response)
}
