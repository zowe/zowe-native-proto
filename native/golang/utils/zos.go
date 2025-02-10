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

	"golang.org/x/term"
)

func SetAutoConvOnUntaggedStdio() {
	for _, file := range []*os.File{os.Stdin, os.Stdout, os.Stderr} {
		fd := int(file.Fd())
		if !term.IsTerminal(fd) && getfdccsid(fd) == 0 {
			runtime.SetZosAutoConvOnFd(fd, 1047)
		}
	}
}

func getfdccsid(fd int) int {
	// TODO Invoke fstat for fd and return the ccsid value
	// var st unix.Stat_t
	// unix.Fstat(fd, &st)
	// fmt.Printf("fstat(%d): %+v\n", fd, st)
	return 0
}
