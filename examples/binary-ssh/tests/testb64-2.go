package main

import (
	"encoding/base64"
	"io"
	"os"
	"runtime"
	"strconv"
	"syscall"
)

func uploadB64(filepath string, chunksize int) {
	file, err := os.Create(filepath)
	if err != nil {
		panic(err)
	}
	defer file.Close()
	decoder := base64.NewDecoder(base64.StdEncoding, os.Stdin)
	buf := make([]byte, chunksize)
	if _, err := io.CopyBuffer(file, decoder, buf); err != nil {
		panic(err)
	}
}

func downloadB64(filepath string, chunksize int) {
	file, err := os.Open(filepath)
	if err != nil {
		panic(err)
	}
	defer file.Close()
	fifoPath := "/tmp/zowe-test-fifo"
	// os.Remove(fifoPath)
	// if err := syscall.Mkfifo(fifoPath, 0600); err != nil {
	// 	panic(err)
	// }
	pipe, err := os.OpenFile(fifoPath, os.O_WRONLY, os.ModeNamedPipe)
	if err != nil {
		panic(err)
	}
	defer pipe.Close()
	encoder := base64.NewEncoder(base64.StdEncoding, pipe)
	// encoder := base64.NewEncoder(base64.StdEncoding, os.Stdout)
	defer encoder.Close()
	buf := make([]byte, chunksize)
	if _, err := io.CopyBuffer(encoder, file, buf); err != nil {
		panic(err)
	}
}

func passthruFifo(_filepath string, chunksize int) {
	fifoPath := "/tmp/zowe-test-fifo"
	os.Remove(fifoPath)
	if err := syscall.Mkfifo(fifoPath, 0600); err != nil {
		panic(err)
	}
	pipe, err := os.OpenFile(fifoPath, os.O_RDONLY, os.ModeNamedPipe)
	if err != nil {
		panic(err)
	}
	defer pipe.Close()
	runtime.SetZosAutoConvOnFd(int(pipe.Fd()), 1047)
	buf := make([]byte, chunksize)
	if _, err := io.CopyBuffer(os.Stdout, pipe, buf); err != nil {
		panic(err)
	}
}

func main() {
	// Set auto conv on untagged stdio
	for _, file := range []*os.File{os.Stdin, os.Stdout, os.Stderr} {
		runtime.SetZosAutoConvOnFd(int(file.Fd()), 1047)
	}

	command := os.Args[1]
	filepath := os.Args[2]
	chunksize, _ := strconv.Atoi(os.Args[3])
	switch command {
	case "upload":
		uploadB64(filepath, chunksize)
		break
	case "download":
		downloadB64(filepath, chunksize)
		break
	case "passthru":
		passthruFifo(filepath, chunksize)
		break
	}
}
