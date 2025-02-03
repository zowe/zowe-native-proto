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
	"os"
	"os/exec"
	"path/filepath"
)

var exePath string

/**
 * Shared logic for command builder functions.
 */
func buildCommandShared(args []string) *exec.Cmd {
	cmd := exec.Command(args[0], args[1:]...)
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

/**
 * Default builder function for invoking zowex commands.
 */
func buildCommand(args []string) *exec.Cmd {
	cmd := buildCommandShared(args)
	cmd.Env = append(os.Environ(), "_BPXK_AUTOCVT=ON")
	return cmd
}

/**
 * Builds a command with _BPXK_AUTOCVT=OFF.
 */
func buildCommandNoAutocvt(args []string) *exec.Cmd {
	cmd := buildCommandShared(args)
	cmd.Env = append(os.Environ(), "_BPXK_AUTOCVT=OFF")
	return cmd
}
