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
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

const DefaultEncoding = 1047

// CollectContentsAsBytes converts a string of bytes into a byte slice.
// If isByteString is true, the string is assumed to be a space-separated list of hexadecimal values.
// Otherwise, the string is assumed to be a raw byte string.
func CollectContentsAsBytes(input string, isByteString bool) (data []byte, err error) {
	data = []byte{}
	if len(input) == 0 {
		return data, nil
	}

	if isByteString {
		data_split := strings.Split(string(input), " ")
		for _, b := range data_split {
			byteNum, err := strconv.ParseUint(b, 16, 8)
			if err != nil {
				err = fmt.Errorf("Failed to parse byte %s in input: %v", b, err)
				return data, err
			}
			data = append(data, byte(byteNum))
		}
	} else {
		data = []byte(input)
	}

	return
}

func LoadChecksums() map[string]string {
	checksumsFile := filepath.Join(GetExecDir(), "checksums.asc")
	file, err := os.Open(checksumsFile)
	if os.IsNotExist(err) {
		// Checksums file does not exist for dev builds
		return nil
	} else if err != nil {
		LogError("Failed to open checksums file: %v", err)
		panic(err)
	}
	defer file.Close()
	checksums := make(map[string]string)
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		parts := strings.Fields(scanner.Text())
		if len(parts) == 2 {
			checksum, file := parts[0], parts[1]
			checksums[file] = checksum
		}
	}
	return checksums
}
