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
	"fmt"
	"strings"
	"sync"

	"zowe-native-proto/ioserver/cmds"
	t "zowe-native-proto/ioserver/types/common"
	"zowe-native-proto/ioserver/utils"
)

// Worker represents a worker that processes command requests
type Worker struct {
	ID           int
	RequestQueue chan []byte
	Dispatcher   *cmds.CmdDispatcher
	Conn         utils.StdioConn
	WG           *sync.WaitGroup
	ResponseMu   *sync.Mutex // Mutex to synchronize response printing
}

// Start starts the worker
func (w *Worker) Start() {
	defer w.WG.Done()

	for data := range w.RequestQueue {
		w.processRequest(data)
	}
}

// processRequest handles a single command request
func (w *Worker) processRequest(data []byte) {
	// Parse the command request
	var request t.RpcRequest
	err := json.Unmarshal(data, &request)
	if err != nil {
		w.ResponseMu.Lock()
		utils.PrintErrorResponse(t.ErrorDetails{
			Code:    -32700,
			Message: fmt.Sprintf("Failed to parse command request: %v", err),
		}, nil)
		w.ResponseMu.Unlock()
		return
	}

	// Handle the command request if a supported command is provided
	if handler, ok := w.Dispatcher.Get(request.Method); ok {
		result, err := handler(&w.Conn, request.Params)
		w.ResponseMu.Lock()
		if err != nil {
			errMsg := err.Error()
			var errData string
			if strings.Index(errMsg, "Error: ") == 0 {
				errMsg = errMsg[7:]
			}
			if parts := strings.SplitN(errMsg, ": ", 2); len(parts) > 1 {
				errMsg, errData = parts[0], parts[1]
			}
			utils.PrintErrorResponse(t.ErrorDetails{
				Code:    w.Conn.LastExitCode,
				Message: errMsg,
				Data:    errData,
			}, &request.Id)
		} else {
			utils.PrintCommandResponse(result, request.Id)
		}
		w.ResponseMu.Unlock()
	} else {
		w.ResponseMu.Lock()
		utils.PrintErrorResponse(t.ErrorDetails{
			Code:    -32601,
			Message: fmt.Sprintf("Unrecognized command %s", request.Method),
		}, &request.Id)
		w.ResponseMu.Unlock()
	}
}

// CreateWorkerPool creates a pool of workers to start interpreting command requests
func CreateWorkerPool(numWorkers int, requestQueue chan []byte, dispatcher *cmds.CmdDispatcher) (*sync.WaitGroup, *sync.Mutex) {
	var wg sync.WaitGroup
	responseMu := &sync.Mutex{}

	for i := 0; i < numWorkers; i++ {
		// Create a separate `zowex` process in interactive mode for each worker
		workerCmd := utils.BuildCommand([]string{"--it"})
		workerStdin, err := workerCmd.StdinPipe()
		if err != nil {
			panic(err)
		}
		workerStdout, err := workerCmd.StdoutPipe()
		if err != nil {
			panic(err)
		}
		workerStderr, err := workerCmd.StderrPipe()
		if err != nil {
			panic(err)
		}
		workerConn := utils.StdioConn{
			Stdin:  workerStdin,
			Stdout: workerStdout,
			Stderr: workerStderr,
		}
		workerCmd.Start()

		// Wait for the instance of `zowex` to be ready
		if _, err = bufio.NewReader(workerStdout).ReadBytes('\n'); err != nil {
			panic(err)
		}

		// Create and start the worker with the stdio pipe connections for the `zowex` instance
		worker := &Worker{
			ID:           i,
			RequestQueue: requestQueue,
			Dispatcher:   dispatcher,
			Conn:         workerConn,
			WG:           &wg,
			ResponseMu:   responseMu,
		}
		wg.Add(1)
		go worker.Start()
	}

	return &wg, responseMu
}
