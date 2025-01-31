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
	"log"
	"strconv"
	"strings"
)

func collectContentsAsBytes(input string, isByteString bool) []byte {
	var data []byte

	if isByteString {
		data_split := strings.Split(string(input), " ")
		for _, b := range data_split {
			byteNum, err := strconv.ParseUint(b, 16, 8)
			if err != nil {
				log.Println("Error parsing byte:", err)
				continue
			}
			data = append(data, byte(byteNum))
		}
	} else {
		data = []byte(input)
	}

	return data
}
