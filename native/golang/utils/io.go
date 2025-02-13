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
	"bufio"
	"io"
	"strings"
)

type ReadWriteCloser struct {
	io.ReadCloser
	io.WriteCloser
}

func (rwc ReadWriteCloser) ExecCmd(args []string) (out []byte, err error) {
	_, err = rwc.WriteCloser.Write([]byte(strings.Join(args, " ") + "\n"))
	if err != nil {
		return
	}
	reader := bufio.NewReader(rwc.ReadCloser)
	for {
		line, err := reader.ReadBytes('\n')
		out = append(out, line...)
		if err != nil || reader.Buffered() == 0 {
			break
		}
	}
	return
}

func (rwc ReadWriteCloser) Close() error {
	var err error
	if rwc.ReadCloser != nil {
		err = rwc.ReadCloser.Close()
	}
	if rwc.WriteCloser != nil {
		err = rwc.WriteCloser.Close()
	}
	return err
}
