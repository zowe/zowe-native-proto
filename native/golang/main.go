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
	pipeFlag := flag.String("pipe", "", "Path to the named pipe for reading Base64 data")
	verboseFlag := flag.Bool("verbose", false, "Enable verbose logging")

	flag.Parse()

	if *numWorkersFlag <= 0 {
		log.Fatalln("Number of workers must be greater than 0")
	}

	return t.IoserverOptions{
		NumWorkers: *numWorkersFlag,
		Pipe:       *pipeFlag,
		Verbose:    *verboseFlag,
	}
}

func main() {
	options := parseOptions()
	utils.InitLogger(false, options.Verbose)
	utils.SetAutoConvOnUntaggedStdio()

	if options.Pipe != "" {
		file, err := os.OpenFile(options.Pipe, os.O_RDONLY, 0600)
		if err != nil {
			panic(err)
		}
		defer file.Close()
		buf := make([]byte, 32768)
		if _, err := io.CopyBuffer(os.Stdout, file, buf); err != nil {
			panic(err)
		}
		return
	}

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
	data := make(map[string]any)
	data["checksums"] = utils.LoadChecksums()
	readyMsg := t.StatusMessage{
		Status:  "ready",
		Message: "zowed is ready to accept input",
		Data:    data,
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
