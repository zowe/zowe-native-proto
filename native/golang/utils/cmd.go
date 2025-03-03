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

package utils

import (
	"encoding/json"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	t "zowe-native-proto/ioserver/types/common"
)

var execDir string

func BuildArgString(args []string) string {
	var sb strings.Builder

	for i, arg := range args {
		if i > 0 {
			sb.WriteString(" ")
		}

		if strings.Contains(arg, " ") || strings.Contains(arg, "\"") {
			sb.WriteString("\"")
			for _, char := range arg {
				if char == '"' {
					sb.WriteString("\\\"")
				} else {
					sb.WriteRune(char)
				}
			}
			sb.WriteString("\"")
		} else {
			sb.WriteString(arg)
		}
	}

	return sb.String()
}

// BuildCommandShared builds a command with the shared logic for command builder functions
func BuildCommandShared(name string, args []string) *exec.Cmd {
	cmd := exec.Command(name, args...)
	cmd.Dir = GetExecDir()
	return cmd
}

// BuildCommand builds a command with _BPXK_AUTOCVT=ON
func BuildCommand(args []string) *exec.Cmd {
	cmd := BuildCommandShared("./zowex", args)
	cmd.Env = append(os.Environ(), "_BPXK_AUTOCVT=ON")
	return cmd
}

// BuildCommand builds a `zowexx` command to perform operations that require authorization
func BuildCommandAuthorized(args []string) *exec.Cmd {
	cmd := BuildCommandShared("./zowexx", args)
	cmd.Env = append(os.Environ(), "_BPXK_AUTOCVT=ON")
	return cmd
}

// BuildCommandNoAutocvt builds a command with _BPXK_AUTOCVT=OFF
func BuildCommandNoAutocvt(args []string) *exec.Cmd {
	cmd := BuildCommandShared("./zowex", args)
	cmd.Env = append(os.Environ(), "_BPXK_AUTOCVT=OFF")
	return cmd
}

// GetExecDir retrieves directory of the current executable
func GetExecDir() string {
	if execDir == "" {
		path, err := os.Executable()
		if err != nil {
			panic(err)
		}
		execDir = filepath.Dir(path)
	}
	return execDir
}

// ParseCommandRequest parses a command request and returns the parsed request as the given type.
func ParseCommandRequest[T any](data []byte) (T, error) {
	var request T
	err := json.Unmarshal(data, &request)
	if err != nil {
		// Get the caller's function name for logging
		pc, _, _, ok := runtime.Caller(1)
		if ok {
			name := runtime.FuncForPC(pc).Name()
			err = fmt.Errorf("[%s] Error unmarshalling JSON: %s", name, err)
		} else {
			err = fmt.Errorf("Error unmarshalling JSON: %v", err)
		}
		return request, err
	}

	return request, nil
}

// PrintCommandResponse prints the response from a command handler. If the response cannot be marshaled, an error response is returned.
func PrintCommandResponse[T any](result T, reqId int) {
	response, err := json.Marshal(t.RpcResponse{
		JsonRPC: "2.0",
		Result:  result,
		Error:   nil,
		Id:      &reqId,
	})
	if err != nil {
		PrintErrorResponse(t.ErrorDetails{
			Code:    -32603,
			Message: fmt.Sprintf("Could not marshal response: %s\n", err.Error()),
		}, &reqId)
	} else {
		fmt.Println(string(response))
	}
}
