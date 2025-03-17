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

	"zowe-native-proto/zowed/cmds"
	t "zowe-native-proto/zowed/types/common"
	"zowe-native-proto/zowed/utils"
)

// parseOptions parses command-line flags and returns the parsed options
func parseOptions() t.IoserverOptions {
	numWorkersFlag := flag.Int("num-workers", 10, "Number of worker threads for concurrent processing")

	flag.Parse()

	if *numWorkersFlag <= 0 {
		log.Fatalln("Number of workers must be greater than 0")
	}

	return t.IoserverOptions{
		NumWorkers: *numWorkersFlag,
	}
}

func main() {
	options := parseOptions()
	utils.InitLogger(false)
	utils.SetAutoConvOnUntaggedStdio()

	// Channel for receiving input from stdin
	input := make(chan []byte)

	// Buffered request queue for workers
	requestQueue := make(chan []byte, 100)

	// Initialize the command dispatcher and register all core commands
	dispatcher := cmds.NewDispatcher()
	cmds.InitializeCoreHandlers(dispatcher)

	wg, _ := CreateWorkerPool(options.NumWorkers, requestQueue, dispatcher)

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
		close(requestQueue)
	}()

	// Distribute incoming requests to the queue
	for data := range input {
		requestQueue <- data
	}

	// If stdin is closed (process likely terminated), close the request queue and wait for all workers to finish
	close(requestQueue)
	wg.Wait()
}
