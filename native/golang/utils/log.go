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
	"encoding/json"
	"fmt"
	"log"
	"os"
	t "zowe-native-proto/ioserver/types/common"
)

var logFile *os.File

// LogError logs an error to the log file
func LogError(format string, args ...any) {
	_, err := logFile.WriteString(fmt.Sprintf(format, args))
	_, err = logFile.WriteString("\n")
	if err != nil {
		log.Fatalln("Failed to write to log file:", err)
	}

	info, err := logFile.Stat()
	if err != nil {
		log.Fatalln("Failed to get log file info:", err)
		return
	}

	// If log file size exceeds 10MB, truncate and reopen the file
	if info.Size() > 10*1024*1024 {
		logFile.Close()
		InitLogger(true)
		logFile.WriteString("Log file truncated due to size limit\n")
	}
}

// PrintErrorResponse prints a JSON-serialized error response to stderr and logs the error to the log file
func PrintErrorResponse(details t.ErrorDetails, rpcId *int) {
	LogError(details.Message)
	errResponse := t.RpcResponse{
		JsonRPC: "2.0",
		Result:  nil,
		Error:   &details,
		Id:      rpcId,
	}
	out, _ := json.Marshal(errResponse)
	fmt.Fprintln(os.Stderr, string(out))
}

// InitLogger initializes the logger
func InitLogger(truncate bool) {
	access := os.O_APPEND
	if truncate {
		access = os.O_TRUNC
	}
	file, err := os.OpenFile("./ioserver.log", access|os.O_CREATE|os.O_WRONLY, 0644)
	if err != nil {
		log.Fatalln("Failed to initialize logger:", err)
		return
	}

	logFile = file
}
