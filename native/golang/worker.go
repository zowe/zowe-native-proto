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
	"strconv"
	"strings"
	"sync"
	"syscall"
	"unsafe"

	"zowe-native-proto/zowed/cmds"
	t "zowe-native-proto/zowed/types/common"
	"zowe-native-proto/zowed/utils"
)

// Worker represents a worker that processes command requests
type Worker struct {
	ID           int
	RequestQueue chan []byte
	Dispatcher   *cmds.CmdDispatcher
	Conn         utils.StdioConn
	ResponseMu   *sync.Mutex // Mutex to synchronize response printing
	Ready        bool        // Indicates if the worker is ready to process requests
	ShmID        int         // Shared memory ID
	ShmAddr      uintptr     // Shared memory address
}

// WorkerPool manages a pool of workers
type WorkerPool struct {
	Workers    []*Worker
	ReadyCount int32 // Number of workers that are ready
	ResponseMu *sync.Mutex
	ReadyMu    *sync.Mutex // Mutex to synchronize access to ReadyCount and Workers
	ReadyCond  *sync.Cond  // Condition variable to signal when workers become ready
	Queue      chan []byte // Request queue for all workers
}

// GetReadyWorker returns a worker that's ready to process requests
// Blocks if no workers are ready
func (wp *WorkerPool) GetReadyWorker() *Worker {
	wp.ReadyMu.Lock()
	defer wp.ReadyMu.Unlock()

	// Wait until at least one worker is ready
	for wp.ReadyCount == 0 {
		wp.ReadyCond.Wait()
	}

	// Find a ready worker
	for _, worker := range wp.Workers {
		if worker.Ready {
			return worker
		}
	}

	// Should never reach here if ReadyCount > 0
	return nil
}

// SetWorkerReady marks a worker as ready and updates the ready count
func (wp *WorkerPool) SetWorkerReady(workerID int) {
	wp.ReadyMu.Lock()
	defer wp.ReadyMu.Unlock()

	for _, worker := range wp.Workers {
		if worker.ID == workerID && !worker.Ready {
			worker.Ready = true
			wp.ReadyCount++
			wp.ReadyCond.Signal()
			break
		}
	}
}

// DistributeRequest sends a request to the shared worker queue
func (wp *WorkerPool) DistributeRequest(data []byte) {
	// Non-blocking send to the shared queue
	// If no workers are ready yet, this will still queue the request
	// for when workers become available
	go func() {
		wp.Queue <- data
	}()
}

// Start starts the worker
func (w *Worker) Start() {
	for data := range w.RequestQueue {
		// Only process requests if worker is ready
		if w.Ready {
			w.processRequest(data)
		} else {
			// If this worker isn't ready yet, put the request back in the shared queue
			// This should rarely happen as workers are initialized asynchronously
			go func(reqData []byte) {
				w.RequestQueue <- reqData
			}(data)
		}
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
// Now returns a WorkerPool that manages worker lifecycle and state
func CreateWorkerPool(numWorkers int, requestQueue chan []byte, dispatcher *cmds.CmdDispatcher) *WorkerPool {
	responseMu := &sync.Mutex{}
	readyMu := &sync.Mutex{}
	readyCond := sync.NewCond(readyMu)

	pool := &WorkerPool{
		Workers:    make([]*Worker, numWorkers),
		ReadyCount: 0,
		ResponseMu: responseMu,
		ReadyMu:    readyMu,
		ReadyCond:  readyCond,
		Queue:      requestQueue,
	}

	// Create each worker
	for i := 0; i < numWorkers; i++ {
		worker := &Worker{
			ID:           i,
			RequestQueue: requestQueue,
			Dispatcher:   dispatcher,
			ResponseMu:   responseMu,
			Ready:        false, // Worker starts as not ready
		}
		pool.Workers[i] = worker

		// Initialize worker asynchronously
		go initializeWorker(worker, pool)
	}

	return pool
}

// initializeWorker initializes a worker for forwarding `zowex` requests/responses
func initializeWorker(worker *Worker, pool *WorkerPool) {
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
	err = workerCmd.Start()
	if err != nil {
		panic(err)
	}

	// Wait for the instance of `zowex` to be ready and capture shared memory info
	reader := bufio.NewReader(workerStdout)

	// Read the startup message
	if _, err = reader.ReadBytes('\n'); err != nil {
		panic(err)
	}

	// Read shared memory ID line
	shmIDLine, err := reader.ReadBytes('\n')
	if err != nil {
		panic(err)
	}

	// Parse shared memory ID from "Shared memory initialized (ID: 12345, Key: 0x12348000)"
	shmIDStr := string(shmIDLine)
	if idx := strings.Index(shmIDStr, "ID: "); idx != -1 {
		start := idx + 4
		end := strings.Index(shmIDStr[start:], ",")
		if end != -1 {
			if shmID, err := strconv.Atoi(shmIDStr[start : start+end]); err == nil {
				worker.ShmID = shmID
			}
		}
	}
	fmt.Printf("Shared memory ID: %d\n", worker.ShmID)

	// Read shared memory address line
	shmAddrLine, err := reader.ReadBytes('\n')
	if err != nil {
		panic(err)
	}

	// Parse shared memory address from "Shared memory address: 0x12345678"
	shmAddrStr := string(shmAddrLine)
	if idx := strings.Index(shmAddrStr, "0x"); idx != -1 {
		addrHex := strings.TrimSpace(shmAddrStr[idx+2:])
		if addr, err := strconv.ParseUint(addrHex, 16, 64); err == nil {
			worker.ShmAddr = uintptr(addr)
		}
	}
	fmt.Printf("Shared memory address: %d\n", worker.ShmAddr)

	// Read the remaining status lines (animal count and raw data)
	reader.ReadBytes('\n') // Animal count line
	reader.ReadBytes('\n') // Raw data line

	// Set up the connection for the worker
	worker.Conn = workerConn

	// Log shared memory information
	worker.LogSharedMemoryInfo()

	// Start the worker
	go worker.Start()

	// Mark the worker as ready
	pool.SetWorkerReady(worker.ID)
}

// GetAvailableWorkersCount returns the number of ready workers
func (wp *WorkerPool) GetAvailableWorkersCount() int32 {
	wp.ReadyMu.Lock()
	defer wp.ReadyMu.Unlock()
	return wp.ReadyCount
}

// SharedMemoryData represents the structure of shared memory from zowex
type SharedMemoryData struct {
	Mutex       [40]byte   // pthread_mutex_t (size varies by platform, using generous size)
	AnimalCount int32      // int animal_count
	RawData     [4096]byte // char raw_data[4096]
}

// GetAnimalCountFromSharedMemory reads the animal count from shared memory using syscalls
func (w *Worker) GetAnimalCountFromSharedMemory() (int32, error) {
	if w.ShmID == 0 {
		return 0, fmt.Errorf("shared memory not initialized")
	}

	// Attach to shared memory using syscall
	addr, _, errno := syscall.Syscall(syscall.SYS_SHMAT, uintptr(w.ShmID), 0, 0)
	if errno != 0 {
		return 0, fmt.Errorf("failed to attach to shared memory: %v", errno)
	}
	defer func() {
		// Detach from shared memory
		syscall.Syscall(syscall.SYS_SHMDT, addr, 0, 0)
	}()

	// Cast to our shared memory structure
	shmData := (*SharedMemoryData)(unsafe.Pointer(addr))

	// Read the animal count (offset after mutex)
	return shmData.AnimalCount, nil
}

// LogSharedMemoryInfo logs information about the worker's shared memory
func (w *Worker) LogSharedMemoryInfo() {
	if count, err := w.GetAnimalCountFromSharedMemory(); err == nil {
		fmt.Printf("Worker %d: Shared memory ID=%d, Address=0x%x, Animal count=%d\n",
			w.ID, w.ShmID, w.ShmAddr, count)
	} else {
		fmt.Printf("Worker %d: Failed to read shared memory: %v\n", w.ID, err)
	}
}

// LogAllWorkersSharedMemory logs shared memory information for all ready workers
func (wp *WorkerPool) LogAllWorkersSharedMemory() {
	wp.ReadyMu.Lock()
	defer wp.ReadyMu.Unlock()

	fmt.Println("=== Shared Memory Status for All Workers ===")
	for _, worker := range wp.Workers {
		if worker.Ready {
			worker.LogSharedMemoryInfo()
		}
	}
	fmt.Println("============================================")
}

// TestSharedMemoryAccess demonstrates reading from shared memory
func (wp *WorkerPool) TestSharedMemoryAccess() {
	wp.ReadyMu.Lock()
	defer wp.ReadyMu.Unlock()

	fmt.Println("=== Testing Shared Memory Access ===")
	for _, worker := range wp.Workers {
		if worker.Ready && worker.ShmID != 0 {
			if count, err := worker.GetAnimalCountFromSharedMemory(); err == nil {
				fmt.Printf("Worker %d successfully read animal count: %d\n", worker.ID, count)
			} else {
				fmt.Printf("Worker %d failed to read shared memory: %v\n", worker.ID, err)
			}
		}
	}
	fmt.Println("====================================")
}
