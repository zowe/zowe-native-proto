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
	"fmt"
	"os"
	"path/filepath"
	t "zowe-native-proto/ioserver/types/common"
	uss "zowe-native-proto/ioserver/types/uss"
	utils "zowe-native-proto/ioserver/utils"
)

// HandleListFilesRequest handles a ListFilesRequest by invoking built-in functions from Go's `os` module.
func HandleListFilesRequest(_conn utils.StdioConn, p any) (result any, e error) {
	params := p.(uss.ListFilesRequest)
	dirPath := params.Path

	fileInfo, err := os.Stat(dirPath)
	if err != nil {
		e = fmt.Errorf("Failed to stat directory: %v", err)
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
			e = fmt.Errorf("Failed to read directory: %v", err)
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

	return ussResponse, nil
}

// HandleReadFileRequest handles a ReadFileRequest by invoking the `zowex uss view` command
func HandleReadFileRequest(conn utils.StdioConn, p any) (result any, e error) {
	params := p.(uss.ReadFileRequest)
	if params.Encoding == "" || params.Path == "" {
		e = fmt.Errorf("Missing required parameters: Encoding or Path")
		return
	}

	args := []string{"uss", "view", params.Path}
	hasEncoding := len(params.Encoding) != 0
	if hasEncoding {
		args = append(args, "--encoding", params.Encoding, "--rfb", "true")
	}
	out, err := conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Error executing command: %v", err)
		return
	}

	output := string(out)
	var data []byte
	if len(output) > 0 {
		data, e = utils.CollectContentsAsBytes(output, hasEncoding)
	} else {
		data = []byte{}
	}

	result = uss.ReadFileResponse{
		Encoding: params.Encoding,
		Path:     params.Path,
		Data:     data,
	}
	return
}

// HandleWriteFileRequest handles a WriteFileRequest by invoking the `zowex uss write` command
func HandleWriteFileRequest(_conn utils.StdioConn, p any) (result any, e error) {
	params := p.(uss.WriteFileRequest)
	if params.Encoding == "" || params.Path == "" {
		e = fmt.Errorf("Missing required parameters: Encoding or Path")
		return
	}

	decodedBytes, err := base64.StdEncoding.DecodeString(params.Data)
	if err != nil {
		e = fmt.Errorf("[WriteFileRequest] Error decoding base64 contents: %v", err)
		return
	}
	args := []string{"uss", "write", params.Path}
	if len(params.Encoding) > 0 {
		args = append(args, "--encoding", params.Encoding)
	}
	cmd := utils.BuildCommandNoAutocvt(args)
	stdin, err := cmd.StdinPipe()
	if err != nil {
		e = fmt.Errorf("[WriteFileRequest] Error opening stdin pipe: %v", err)
		return
	}

	go func() {
		defer stdin.Close()
		_, err = stdin.Write(decodedBytes)
		if err != nil {
			e = fmt.Errorf("[WriteFileRequest] Error writing to stdin pipe: %v", err)
			return
		}
	}()

	_, err = cmd.Output()
	if err != nil {
		e = fmt.Errorf("[WriteFileRequest] Error piping stdin to command: %v", err)
		return
	}

	result = uss.WriteFileResponse{
		Success: err == nil,
		Path:    params.Path,
	}
	return
}

// HandleCreateFileRequest handles a CreateFileRequest by invoking the `zowex uss create-dir` or `create-file` command (depending on params)
func HandleCreateFileRequest(conn utils.StdioConn, p any) (result any, e error) {
	params := p.(uss.CreateFileRequest)
	if params.Path == "" {
		e = fmt.Errorf("Missing required parameters: Path")
		return
	}

	createCmd := "create-file"
	if params.IsDir {
		createCmd = "create-dir"
	}
	args := []string{"uss", createCmd, params.Path}
	if len(params.Mode) > 0 {
		args = append(args, "--mode", params.Mode)
	}
	_, err := conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Error executing command: %v", err)
		return
	}

	result = uss.DeleteFileResponse{
		Success: err == nil,
		Path:    params.Path,
	}
	return
}

// HandleDeleteFileRequest handles a DeleteFileRequest by invoking the `zowex uss delete` command
func HandleDeleteFileRequest(conn utils.StdioConn, p any) (result any, e error) {
	params := p.(uss.DeleteFileRequest)
	if params.Path == "" {
		e = fmt.Errorf("Missing required parameters: Path")
		return
	}

	args := []string{"uss", "delete", params.Path}
	if params.Recursive {
		args = append(args, "-r", "true")
	}
	_, err := conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Failed to delete USS item: %s", err.Error())
		return
	}

	result = uss.DeleteFileResponse{
		Success: true,
		Path:    params.Path,
	}
	return
}

// HandleChownFileRequest handles a ChownFileRequest by invoking the `zowex uss chown` command
func HandleChownFileRequest(conn utils.StdioConn, p any) (result any, e error) {
	params := p.(uss.ChownFileRequest)
	if params.Path == "" {
		e = fmt.Errorf("Missing required parameters: Path")
		return
	}

	args := []string{"uss", "chown", params.Owner, params.Path}
	if params.Recursive {
		args = append(args, "-r", "true")
	}
	_, err := conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Error executing command: %v", err)
		return
	}

	result = uss.ChownFileResponse{
		Success: err == nil,
		Path:    params.Path,
	}
	return
}

// HandleChmodFileRequest handles a ChmodFileRequest by invoking the `zowex uss chmod` command
func HandleChmodFileRequest(conn utils.StdioConn, p any) (result any, e error) {
	params := p.(uss.ChmodFileRequest)
	if params.Path == "" {
		e = fmt.Errorf("Missing required parameters: Path")
		return
	}

	args := []string{"uss", "chmod", params.Mode, params.Path}
	if params.Recursive {
		args = append(args, "-r", "true")
	}
	_, err := conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Error executing command: %v", err)
		return
	}

	result = uss.ChmodFileResponse{
		Success: err == nil,
		Path:    params.Path,
	}
	return
}

// HandleChtagFileRequest handles a ChtagFileRequest by invoking the `zowex uss chtag` command
func HandleChtagFileRequest(conn utils.StdioConn, p any) (result any, e error) {
	params := p.(uss.ChtagFileRequest)
	if params.Path == "" {
		e = fmt.Errorf("Missing required parameters: Path")
		return
	}

	args := []string{"uss", "chtag", params.Tag, params.Path}
	if params.Recursive {
		args = append(args, "-r", "true")
	}
	_, err := conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Error executing command: %v", err)
		return
	}

	result = uss.ChtagFileResponse{
		Success: err == nil,
		Path:    params.Path,
	}
	return
}
