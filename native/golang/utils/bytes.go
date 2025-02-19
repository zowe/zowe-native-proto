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
	"strconv"
	"strings"
)

// CollectContentsAsBytes converts a string of bytes into a byte slice.
// If isByteString is true, the string is assumed to be a space-separated list of hexadecimal values.
// Otherwise, the string is assumed to be a raw byte string.
func CollectContentsAsBytes(input string, isByteString bool) []byte {
	var data = []byte{}
	if len(input) == 0 {
		return data
	}

	if isByteString {
		data_split := strings.Split(string(input), " ")
		for _, b := range data_split {
			byteNum, err := strconv.ParseUint(b, 16, 8)
			if err != nil {
				PrintErrorResponse("Failed to parse byte %s in input: %s", b, input, err)
				break
			}
			data = append(data, byte(byteNum))
		}
	} else {
		data = []byte(input)
	}

	return data
}
