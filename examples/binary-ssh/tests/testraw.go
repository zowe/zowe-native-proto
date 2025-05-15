package main

import (
	"fmt"
	"io"
	"os"
	"strconv"

	"golang.org/x/term"
)

func uploadRaw(filepath string, chunksize int, totalsize int) {
	file, err := os.Create(filepath)
	if err != nil {
		panic(err)
	}
	defer file.Close()
	buf := make([]byte, chunksize)
	received := 0
	for {
		n, err := os.Stdin.Read(buf)
		if n > 0 {
			if _, werr := file.Write(buf[:n]); werr != nil {
				panic(werr)
			}
			received += n
		}
		if err == io.EOF || received >= totalsize {
			break
		}
		if err != nil {
			panic(err)
		}
	}
}

func downloadRaw(filepath string, chunksize int) {
	file, err := os.Open(filepath)
	if err != nil {
		panic(err)
	}
	defer file.Close()
	buf := make([]byte, chunksize)
	if _, err := io.CopyBuffer(os.Stdout, file, buf); err != nil {
		panic(err)
	}
}

func main() {
	// Set stdin to raw mode
	oldState, err := term.MakeRaw(int(os.Stdin.Fd()))
	if err != nil {
		fmt.Println("Error setting raw mode:", err)
		return
	}
	defer term.Restore(int(os.Stdin.Fd()), oldState)

	// Set stdout to raw mode
	oldState2, err := term.MakeRaw(int(os.Stdout.Fd()))
	if err != nil {
		fmt.Println("Error setting raw mode:", err)
		return
	}
	defer term.Restore(int(os.Stdout.Fd()), oldState2)

	command := os.Args[1]
	filepath := os.Args[2]
	chunksize, _ := strconv.Atoi(os.Args[3])
	switch command {
	case "upload":
		totalsize, _ := strconv.Atoi(os.Args[4])
		uploadRaw(filepath, chunksize, totalsize)
		break
	case "download":
		downloadRaw(filepath, chunksize)
		break
	}
}
