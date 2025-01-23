/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0 & Apache-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

package main

import (
	"encoding/json"
	"fmt"
	"log"
	"os"
)

func HandleListFilesRequest(jsonData []byte) {
	var listRequest ListFilesRequest
	err := json.Unmarshal(jsonData, &listRequest)
	if err != nil {
		// log.Println("Error decoding ListFilesRequest:", err)
		return
	}

	dirPath := listRequest.Path

	entries, err := os.ReadDir(dirPath)
	if err != nil {
		log.Println("Error reading directory:", err)
		return
	}

	ussResponse := ListFilesResponse{
		Items: make([]UssItem, len(entries)),
	}

	for i, entry := range entries {
		ussResponse.Items[i] = UssItem{
			Name:  entry.Name(),
			IsDir: entry.IsDir(),
		}
	}

	ussResponse.ReturnedRows = len(ussResponse.Items)

	v, err := json.Marshal(ussResponse)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		fmt.Println(string(v))
	}
}
