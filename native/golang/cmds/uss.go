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
	"encoding/hex"
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"syscall"
	t "zowe-native-proto/zowed/types/common"
	uss "zowe-native-proto/zowed/types/uss"
	utils "zowe-native-proto/zowed/utils"
)

// HandleListFilesRequest handles a ListFilesRequest by invoking built-in functions from Go's `os` module.
func HandleListFilesRequest(_conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[uss.ListFilesRequest](params)
	if err != nil {
		return nil, err
	}

	dirPath := request.Path

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
			Mode:  fileInfo.Mode().String(),
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
				Mode:  entry.Type().String(),
			}
		}

		ussResponse.ReturnedRows = len(ussResponse.Items)
	}

	return ussResponse, nil
}

// HandleReadFileRequest handles a ReadFileRequest by invoking the `zowex uss view` command
func HandleReadFileRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[uss.ReadFileRequest](params)
	if err != nil {
		return nil, err
	} else if request.Path == "" {
		e = fmt.Errorf("Missing required parameters: Path")
		return
	}

	if len(request.Encoding) == 0 {
		request.Encoding = fmt.Sprintf("IBM-%d", utils.DefaultEncoding)
	}
	args := []string{"uss", "view", request.Path, "--encoding", request.Encoding, "--return-etag", "true"}

	var etag string
	var data []byte
	if request.StreamId == 0 {
		args = append(args, "--rfb", "true")
		out, err := conn.ExecCmd(args)
		if err != nil {
			e = fmt.Errorf("Error executing command: %v", err)
			return
		}

		output := utils.YamlToMap(string(out))
		etag = output["etag"]

		if len(output) > 0 {
			data, e = utils.CollectContentsAsBytes(output["data"], true)
		} else {
			data = []byte{}
		}
	} else {
		pipePath := fmt.Sprintf("%s/zowe-native-proto_%d-%d-%d_fifo", os.TempDir(), os.Geteuid(), os.Getpid(), request.StreamId)
		os.Remove(pipePath)

		err := syscall.Mkfifo(pipePath, 0600)
		if err != nil {
			e = fmt.Errorf("[ReadFileRequest] Error creating named pipe: %v", err)
			return
		}

		notify, err := json.Marshal(t.RpcNotification{
			JsonRPC: "2.0",
			Method:  "receiveStream",
			Params: map[string]interface{}{
				"id":       request.StreamId,
				"pipePath": pipePath,
			},
		})
		if err != nil {
			e = fmt.Errorf("[ReadFileRequest] Error marshalling notification: %v", err)
			return
		}
		fmt.Println(string(notify))

		args = append(args, "--pipe-path", pipePath)
		out, err := conn.ExecCmd(args)
		if err != nil {
			return nil, fmt.Errorf("Error executing command: %v", err)
		}

		err = os.Remove(pipePath)
		if err != nil {
			e = fmt.Errorf("[ReadFileRequest] Error deleting named pipe: %v", err)
			return
		}

		etag = strings.TrimRight(string(out), "\n")
	}

	result = uss.ReadFileResponse{
		Encoding: request.Encoding,
		Etag:     etag,
		Path:     request.Path,
		Data:     &data,
	}
	return
}

// HandleWriteFileRequest handles a WriteFileRequest by invoking the `zowex uss write` command
func HandleWriteFileRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[uss.WriteFileRequest](params)
	if err != nil {
		return nil, err
	} else if request.Path == "" {
		e = fmt.Errorf("Missing required parameters: Path")
		return
	}

	if len(request.Encoding) == 0 {
		request.Encoding = fmt.Sprintf("IBM-%d", utils.DefaultEncoding)
	}
	args := []string{"uss", "write", request.Path, "--encoding", request.Encoding, "--etag-only", "true"}
	if len(request.Etag) > 0 {
		args = append(args, "--etag", request.Etag)
	}

	var out []byte
	if request.StreamId == 0 {
		decodedBytes, err := base64.StdEncoding.DecodeString(request.Data)
		if err != nil {
			e = fmt.Errorf("[WriteFileRequest] Error decoding base64 contents: %v", err)
			return
		}

		byteString := hex.EncodeToString(decodedBytes)

		cmd := utils.BuildCommand(args)
		stdin, err := cmd.StdinPipe()
		if err != nil {
			e = fmt.Errorf("[WriteFileRequest] Error opening stdin pipe: %v", err)
			return
		}

		go func() {
			defer stdin.Close()
			_, err = stdin.Write([]byte(byteString))
			if err != nil {
				e = fmt.Errorf("[WriteFileRequest] Error writing to stdin pipe: %v", err)
				return
			}
		}()

		out, err := cmd.CombinedOutput()
		if err != nil {
			e = fmt.Errorf("[WriteFileRequest] Error piping stdin to command: %s", string(out))
			conn.LastExitCode = cmd.ProcessState.ExitCode()
			return
		}
	} else {
		pipePath := fmt.Sprintf("%s/zowe-native-proto_%d-%d-%d_fifo", os.TempDir(), os.Geteuid(), os.Getpid(), request.StreamId)
		os.Remove(pipePath)

		err := syscall.Mkfifo(pipePath, 0600)
		if err != nil {
			e = fmt.Errorf("[WriteFileRequest] Error creating named pipe: %v", err)
			return
		}

		notify, err := json.Marshal(t.RpcNotification{
			JsonRPC: "2.0",
			Method:  "sendStream",
			Params: map[string]interface{}{
				"id":       request.StreamId,
				"pipePath": pipePath,
			},
		})
		if err != nil {
			e = fmt.Errorf("[WriteFileRequest] Error marshalling notification: %v", err)
			return
		}
		fmt.Println(string(notify))

		args = append(args, "--pipe-path", pipePath)
		out, err = conn.ExecCmd(args)
		if err != nil {
			return nil, fmt.Errorf("Error executing command: %v", err)
		}

		err = os.Remove(pipePath)
		if err != nil {
			e = fmt.Errorf("[WriteFileRequest] Error deleting named pipe: %v", err)
			return
		}
	}

	result = uss.WriteFileResponse{
		GenericFileResponse: uss.GenericFileResponse{
			Success: true,
			Path:    request.Path,
		},
		Etag: strings.TrimRight(string(out), "\n"),
	}
	return
}

// HandleCreateFileRequest handles a CreateFileRequest by invoking the `zowex uss create-dir` or `create-file` command (depending on params)
func HandleCreateFileRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[uss.CreateFileRequest](params)
	if err != nil {
		return nil, err
	} else if request.Path == "" {
		e = fmt.Errorf("Missing required parameters: Path")
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
	_, err = conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Error executing command: %v", err)
		return
	}

	result = uss.DeleteFileResponse{
		Success: err == nil,
		Path:    request.Path,
	}
	return
}

// HandleDeleteFileRequest handles a DeleteFileRequest by invoking the `zowex uss delete` command
func HandleDeleteFileRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[uss.DeleteFileRequest](params)
	if err != nil {
		return nil, err
	} else if request.Path == "" {
		e = fmt.Errorf("Missing required parameters: Path")
		return
	}

	args := []string{"uss", "delete", request.Path}
	if request.Recursive {
		args = append(args, "-r", "true")
	}
	_, err = conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Failed to delete USS item: %s", err.Error())
		return
	}

	result = uss.DeleteFileResponse{
		Success: true,
		Path:    request.Path,
	}
	return
}

// HandleChownFileRequest handles a ChownFileRequest by invoking the `zowex uss chown` command
func HandleChownFileRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[uss.ChownFileRequest](params)
	if err != nil {
		return nil, err
	} else if request.Path == "" {
		e = fmt.Errorf("Missing required parameters: Path")
		return
	}

	args := []string{"uss", "chown", request.Owner, request.Path}
	if request.Recursive {
		args = append(args, "-r", "true")
	}
	_, err = conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Error executing command: %v", err)
		return
	}

	result = uss.ChownFileResponse{
		Success: err == nil,
		Path:    request.Path,
	}
	return
}

// HandleChmodFileRequest handles a ChmodFileRequest by invoking the `zowex uss chmod` command
func HandleChmodFileRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[uss.ChmodFileRequest](params)
	if err != nil {
		return nil, err
	} else if request.Path == "" {
		e = fmt.Errorf("Missing required parameters: Path")
		return
	}

	args := []string{"uss", "chmod", request.Mode, request.Path}
	if request.Recursive {
		args = append(args, "-r", "true")
	}
	_, err = conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Error executing command: %v", err)
		return
	}

	result = uss.ChmodFileResponse{
		Success: err == nil,
		Path:    request.Path,
	}
	return
}

// HandleChtagFileRequest handles a ChtagFileRequest by invoking the `zowex uss chtag` command
func HandleChtagFileRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[uss.ChtagFileRequest](params)
	if err != nil {
		return nil, err
	} else if request.Path == "" {
		e = fmt.Errorf("Missing required parameters: Path")
		return
	}

	args := []string{"uss", "chtag", request.Tag, request.Path}
	if request.Recursive {
		args = append(args, "-r", "true")
	}
	_, err = conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Error executing command: %v", err)
		return
	}

	result = uss.ChtagFileResponse{
		Success: err == nil,
		Path:    request.Path,
	}
	return
}
