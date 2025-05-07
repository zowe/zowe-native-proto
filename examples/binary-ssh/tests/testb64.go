package main

import (
	"bufio"
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
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		line := scanner.Text()
		data, err := base64.StdEncoding.DecodeString(line)
		if len(data) > 0 {
			if _, werr := file.Write(data); werr != nil {
				panic(werr)
			}
		}
		if err == io.EOF {
			break
		}
		if err != nil {
			panic(err)
		}
	}
}

func downloadB64(filepath string, chunksize int) {
	file, err := os.Open(filepath)
	if err != nil {
		panic(err)
	}
	defer file.Close()
	buf := make([]byte, chunksize)
	for {
		n, err := file.Read(buf)
		if n > 0 {
			data := base64.StdEncoding.EncodeToString(buf[:n])
			if _, werr := os.Stdout.WriteString(data + "\n"); werr != nil {
				panic(werr)
			}
		}
		if err == io.EOF {
			break
		}
		if err != nil {
			panic(err)
		}
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
