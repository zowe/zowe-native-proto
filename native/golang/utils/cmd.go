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
	t "zowe-native-proto/ioserver/types/common"
)

var exePath string

// BuildCommandShared builds a command with the shared logic for command builder functions
func BuildCommandShared(name string, args []string) *exec.Cmd {
	cmd := exec.Command(name, args...)
	if exePath == "" {
		path, err := os.Executable()
		if err != nil {
			panic(err)
		}
		exePath = path
	}
	cmd.Dir = filepath.Dir(exePath)
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

func ParseCommandRequest[T any](data []byte) (T, error) {
	var request T
	err := json.Unmarshal(data, &request)
	if err != nil {
		// Get the caller's function name for logging
		pc, _, _, ok := runtime.Caller(1)
		if ok {
			name := runtime.FuncForPC(pc).Name()
			PrintErrorResponse("[%s] Error unmarshalling JSON: %s", name, err)
		} else {
			PrintErrorResponse("Error unmarshalling JSON: %s", err)
		}
		return request, err
	}

	return request, nil
}

// PrintCommandResponse prints the response from a command handler. If the response cannot be marshaled, an error response is returned.
func PrintCommandResponse[T any](response T) {
	v, err := json.Marshal(response)
	if err != nil {
		details := fmt.Sprintf("Could not marshal response: %s\n", err.Error())
		errResponse, err2 := json.Marshal(t.ErrorResponse{
			Msg: details,
		})
		if err2 != nil {
			fmt.Fprintf(os.Stderr, "[PrintCommandResponse] Could not marshal response: %s\n", err.Error())
		} else {
			fmt.Println(string(errResponse))
		}
	} else {
		fmt.Println(string(v))
	}
}
