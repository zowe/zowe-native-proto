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

func buildExecCommand(args []string) *exec.Cmd {
	cmd := exec.Command(args[0], args[1:]...)
	exePath, err := os.Executable()
	if err != nil {
		panic(err)
	}
	cmd.Dir = filepath.Dir(exePath)
	cmd.Env = append(os.Environ(), "_BPXK_AUTOCVT=ON")
	return cmd
}
