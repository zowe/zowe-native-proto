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
	"encoding/binary"
	"io"
	"log"
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

/*
Note(TAJ): This method is not used yet since I can't find a way to temporarily
disable auto-convert for an existing stdio pipe. If we figure this out in the
future, here is C++ code that may be useful:

	char dataLen[8];
	std::cin.read(dataLen, 8);
	__e2a_l(&dataLen[0], 8); // Only needed when autoconv is enabled
	byteSize = *reinterpret_cast<const uint64_t *>(dataLen);
	data.resize(byteSize);
	std::cin.read(&data[0], byteSize);
*/
func (rwc ReadWriteCloser) ExecCmdWithStdin(args []string, data []byte) (out []byte, err error) {
	_, err = rwc.WriteCloser.Write([]byte(strings.Join(args, " ") + "\n"))
	if err != nil {
		return
	}
	var dataLen [8]byte
	binary.BigEndian.PutUint64(dataLen[:], uint64(len(data)))
	// fd := int(rwc.WriteCloser.(*os.File).Fd())
	// DisableAutoConv(fd)
	// defer EnableAutoConv(fd)
	if _, err = rwc.WriteCloser.Write(dataLen[:]); err == nil {
		_, err = rwc.WriteCloser.Write(data)
	}
	if err != nil {
		log.Default().Println("exec cmd with stdin failed")
		log.Default().Println(err)
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
