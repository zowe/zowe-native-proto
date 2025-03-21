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
	"bufio"
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"os"
	"time"

	"zowe-native-proto/zowed/cmds"
	t "zowe-native-proto/zowed/types/common"
	"zowe-native-proto/zowed/utils"
)

// parseOptions parses command-line flags and returns the parsed options
func parseOptions() t.IoserverOptions {
	numWorkersFlag := flag.Int("num-workers", 10, "Number of worker threads for concurrent processing")
	verboseFlag := flag.Bool("verbose", false, "Enable verbose logging")

	flag.Parse()

	if *numWorkersFlag <= 0 {
		log.Fatalln("Number of workers must be greater than 0")
	}

	return t.IoserverOptions{
		NumWorkers: *numWorkersFlag,
		Verbose:    *verboseFlag,
	}
}

func main() {
	options := parseOptions()
	utils.InitLogger(false, options.Verbose)
	utils.SetAutoConvOnUntaggedStdio()

	// Channel for receiving input from stdin
	input := make(chan []byte)

	// Buffered request queue for workers
	requestQueue := make(chan []byte, 100)

	// Initialize the command dispatcher and register all core commands
	dispatcher := cmds.NewDispatcher()
	cmds.InitializeCoreHandlers(dispatcher)

	// Initialize workers in background
	workerPool := CreateWorkerPool(options.NumWorkers, requestQueue, dispatcher)

	// Log available worker count at initialization when verbose is enabled
	if utils.IsVerboseLogging() {
		go func() {
			for {
				count := workerPool.GetAvailableWorkersCount()
				utils.LogDebug("Available workers: %d/%d", count, options.NumWorkers)
				time.Sleep(500 * time.Millisecond)
				if count == int32(options.NumWorkers) {
					break
				}
			}
		}()
	}

	// Print ready message to stdout as JSON
	readyMsg := t.StatusMessage{
		Status:  "ready",
		Message: "zowed is ready to accept input",
	}
	readyJson, err := json.Marshal(readyMsg)
	if err != nil {
		log.Fatalln("Failed to marshal ready message:", err)
	}
	fmt.Println(string(readyJson))

	// Start goroutine to read from stdin
	go func() {
		scanner := bufio.NewScanner(os.Stdin)
		for scanner.Scan() {
			line := scanner.Text()
			// Process each line (it should be a complete JSON request)
			input <- []byte(line)
		}
		// Close the input channel once stdin is closed
		close(input)
	}()

	// Distribute incoming requests to available workers
	for data := range input {
		workerPool.DistributeRequest(data)
	}

	// `zowed` exits after stdin is closed and all pending requests are processed
}
