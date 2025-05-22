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
	"encoding/ascii85"
	"io"
	"os"
	"runtime"
	"strconv"
)

func uploadB85(filepath string, chunksize int) {
	file, err := os.Create(filepath)
	if err != nil {
		panic(err)
	}
	defer file.Close()
	decoder := ascii85.NewDecoder(os.Stdin)
	buf := make([]byte, chunksize)
	if _, err := io.CopyBuffer(file, decoder, buf); err != nil {
		panic(err)
	}
}

func downloadB85(filepath string, chunksize int) {
	file, err := os.Open(filepath)
	if err != nil {
		panic(err)
	}
	defer file.Close()
	encoder := ascii85.NewEncoder(os.Stdout)
	defer encoder.Close()
	buf := make([]byte, chunksize)
	if _, err := io.CopyBuffer(encoder, file, buf); err != nil {
		panic(err)
	}
}

func main() {
	// Set auto conv on untagged stdio
	for _, file := range []*os.File{os.Stdin, os.Stdout, os.Stderr} {
		runtime.SetZosAutoConvOnFd(int(file.Fd()), 1047)
	}

	command := os.Args[1]
	filepath := os.Args[2]
	chunksize, _ := strconv.Atoi(os.Args[3])
	switch command {
	case "upload":
		uploadB85(filepath, chunksize)
		break
	case "download":
		downloadB85(filepath, chunksize)
		break
	}
}
