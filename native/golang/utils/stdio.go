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
	"errors"
	"io"
	"log"
	"strings"
)

type StdioConn struct {
	Stdin  io.WriteCloser
	Stdout io.ReadCloser
	Stderr io.ReadCloser
}

func (conn StdioConn) ExecCmd(args []string) (stdout []byte, stderr error) {
	_, err := conn.Stdin.Write([]byte("./zowex " + BuildArgString(args) + "\n"))
	if err != nil {
		return
	}
	done := make(chan struct{})

	go func() {
		defer close(done)
		reader := bufio.NewReader(conn.Stdout)
		for {
			line, err := reader.ReadBytes('\n')
			stdout = append(stdout, line...)
			if err != nil || reader.Buffered() == 0 {
				break
			}
		}
	}()

	go func() {
		defer close(done)
		reader := bufio.NewReader(conn.Stderr)
		var tempErr []byte
		for {
			line, err := reader.ReadBytes('\n')
			tempErr = append(tempErr, line...)
			if err != nil || reader.Buffered() == 0 {
				break
			}
		}
		if len(tempErr) > 0 {
			stderr = errors.New(string(tempErr))
		}
	}()

	<-done
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
func (conn StdioConn) ExecCmdWithStdin(args []string, data []byte) (out []byte, err error) {
	_, err = conn.Stdin.Write([]byte(strings.Join(args, " ") + "\n"))
	if err != nil {
		return
	}
	var dataLen [8]byte
	binary.BigEndian.PutUint64(dataLen[:], uint64(len(data)))
	// fd := int(conn.Stdin.(*os.File).Fd())
	// DisableAutoConv(fd)
	// defer EnableAutoConv(fd)
	if _, err = conn.Stdin.Write(dataLen[:]); err == nil {
		_, err = conn.Stdin.Write(data)
	}
	if err != nil {
		log.Default().Println("exec cmd with stdin failed")
		log.Default().Println(err)
		return
	}
	reader := bufio.NewReader(conn.Stdout)
	for {
		line, err := reader.ReadBytes('\n')
		out = append(out, line...)
		if err != nil || reader.Buffered() == 0 {
			break
		}
	}
	return
}

func (conn StdioConn) Close() error {
	var err error
	if conn.Stdin != nil {
		err = conn.Stdin.Close()
	}
	if conn.Stdout != nil {
		err = conn.Stdout.Close()
	}
	if conn.Stderr != nil {
		err = conn.Stderr.Close()
	}
	return err
}
