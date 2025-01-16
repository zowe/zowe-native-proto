package main

import (
	"encoding/json"
	"log"
	"os"
)

func main() {
	input := make(chan []byte)

	go func() {
		buf := make([]byte, 1024)
		for {
			n, err := os.Stdin.Read(buf)
			if err != nil {
				if err.Error() == "EOF" {
					os.Exit(0)
				}
				log.Fatalln("Error reading from stdin:", err)
			}
			input <- buf[:n]
		}
	}()

	type CommandHandler func([]byte)
	commandHandlers := map[string]CommandHandler{
		"readDataset": HandleReadDatasetRequest,
		"listDataset": HandleListDatasetRequest,
		"listUss":     HandleListUssRequest,
	}

	for {
		select {
		case data := <-input:
			var request CommandRequest
			err := json.Unmarshal(data, &request)
			if err != nil {
				log.Println("Error parsing command request:", err)
				continue
			}

			if handler, ok := commandHandlers[request.Command]; ok {
				handler(data)
			}
		}
	}

}
