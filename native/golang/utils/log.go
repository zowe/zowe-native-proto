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
	"path/filepath"
	t "zowe-native-proto/zowed/types/common"
)

var logFile *os.File
var verboseLogging bool

// IsVerboseLogging returns whether verbose logging is enabled
func IsVerboseLogging() bool {
	return verboseLogging
}

// LogDebug logs a debug message to the log file if verbose logging is enabled
func LogDebug(format string, args ...any) {
	if !verboseLogging {
		return
	}
	_, err := logFile.WriteString(fmt.Sprintf("[DEBUG] "+format, args...) + "\n")
	if err != nil {
		log.Fatalln("Failed to write to log file:", err)
	}
}

// LogError logs an error to the log file
func LogError(format string, args ...any) {
	_, err := logFile.WriteString(fmt.Sprintf(format, args...) + "\n")
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
		InitLogger(true, verboseLogging)
		_, _ = logFile.WriteString("Log file truncated due to size limit\n")
	}
}

// LogFatal logs a fatal error to the log file and aborts
func LogFatal(format string, args ...any) {
	LogError(format, args...)
	log.Fatalf(format, args...)
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
func InitLogger(truncate bool, verbose bool) {
	verboseLogging = verbose
	logsDir := filepath.Dir(os.Args[0]) + "/logs"
	err := os.Mkdir(logsDir, 0700)
	if err != nil {
		log.Fatalln("Failed to create logs directory:", err)
		return
	}

	logFilePath := logsDir + "/" + filepath.Base(os.Args[0]) + ".log"
	access := os.O_APPEND
	if truncate {
		access = os.O_TRUNC
	}
	file, err := os.OpenFile(logFilePath, access|os.O_CREATE|os.O_WRONLY, 0644)
	if err != nil {
		log.Fatalln("Failed to initialize logger:", err)
		return
	}

	logFile = file
	if verbose {
		LogDebug("Verbose logging enabled")
	}
}
