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
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

var exePath string

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
