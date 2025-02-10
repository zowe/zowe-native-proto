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
	"runtime"
	"strconv"

	"golang.org/x/term"
)

func SetAutoConvOnUntaggedStdio() {
	env_vars := []string{"__STDIN_CCSID", "__STDOUT_CCSID", "__STDERR_CCSID"}
	for i, file := range []*os.File{os.Stdin, os.Stdout, os.Stderr} {
		fd := int(file.Fd())
		if env := os.Getenv(env_vars[i]); env != "" {
			ccsid, err := strconv.Atoi(env)
			if err != nil {
				panic(err)
			}
			runtime.SetZosAutoConvOnFd(fd, uint16(ccsid))
		} else if !term.IsTerminal(fd) {
			runtime.SetZosAutoConvOnFd(fd, 1047)
		}
	}
}
