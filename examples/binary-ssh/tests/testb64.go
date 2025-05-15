package main

import (
	"encoding/base64"
	"io"
	"os"
	"runtime"
	"strconv"
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
	encoder := base64.NewEncoder(base64.StdEncoding, os.Stdout)
	defer encoder.Close()
	buf := make([]byte, chunksize)
	if _, err := io.CopyBuffer(encoder, file, buf); err != nil {
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
	}
}
