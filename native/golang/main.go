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
	"io"
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

	// Start a goroutine to periodically log shared memory information
	go func() {
		time.Sleep(2 * time.Second) // Wait for workers to initialize
		
		// Run initial test
		workerPool.TestSharedMemoryAccess()
		
		// Then log periodically
		for {
			workerPool.LogAllWorkersSharedMemory()
			time.Sleep(10 * time.Second) // Log every 10 seconds
		}
	}()

	// Print ready message to stdout as JSON
	data := make(map[string]any)
	data["checksums"] = utils.LoadChecksums()
	readyMsg := t.StatusMessage{
		Status:  "ready",
		Message: "zowed is ready to accept input",
		Data:    data,
	}
	readyJson, err := json.Marshal(readyMsg)
	if err != nil {
		utils.LogFatal("Failed to marshal ready message: %s", err)
	}
	fmt.Println(string(readyJson))

	// Start goroutine to read from stdin
	go func() {
		reader := bufio.NewReader(os.Stdin)
		for {
			line, err := reader.ReadBytes('\n')
			if len(line) > 0 {
				// Process each line (it should be a complete JSON request)
				input <- line
			}
			if err != nil {
				if err == io.EOF {
					break
				}
				utils.LogFatal("Error reading from stdin: %s", err)
			}
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
