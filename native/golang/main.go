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
	"flag"
	"log"
	"os"

	"zowe-native-proto/ioserver/cmds"
	t "zowe-native-proto/ioserver/types/common"
	"zowe-native-proto/ioserver/utils"
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

	// Start goroutine to read from stdin
	go func() {
		buf := make([]byte, 1024)
		for {
			// Read input from stdin
			n, err := os.Stdin.Read(buf)
			if err != nil {
				if err.Error() == "EOF" {
					close(requestQueue)
					os.Exit(0)
				}
				log.Fatalln("Error reading from stdin:", err)
			}
			// Copy the data to avoid race conditions with buffer reuse
			data := make([]byte, n)
			copy(data, buf[:n])
			input <- data
		}
	}()

	// Distribute incoming requests to the queue
	for data := range input {
		requestQueue <- data
	}

	// If stdin is closed (process likely terminated), close the request queue and wait for all workers to finish
	close(requestQueue)
	wg.Wait()
}
