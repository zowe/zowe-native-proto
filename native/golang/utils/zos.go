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
	"syscall"
)

// Enables auto conversion on untagged stdio. This is necessary because the z/OS
// Go runtime tags stdio with 1047 (EBCDIC) if TTY and 819 (ASCII) if not TTY.
// We always want 1047 for compatibility with the z/OS OpenSSH implementation,
// unless the user explicitly sets the CCSID through environment variables.
//
// See https://www.ibm.com/docs/en/zos/3.1.0?topic=systems-openssh-globalization
// and https://github.com/ibmruntimes/zoslib/blob/zopen/src/zos.cc#L2742
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
		} else if !isatty(fd) {
			runtime.SetZosAutoConvOnFd(fd, 1047)
		}
	}
}

// Implements the C `isatty` method without external dependencies
func isatty(fd int) bool {
	var st syscall.Stat_t
	if err := syscall.Fstat(fd, &st); err != nil {
		return false
	}
	return st.Mode&syscall.S_IFMT == syscall.S_IFCHR
}
