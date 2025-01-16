package main

import (
	"encoding/json"
	"fmt"
	"log"
	"os"
	"os/exec"
)

type CommandRequest struct {
	Command string `json:"command"`
}

type ReadDatasetRequest struct {
	Encoding string `json:"encoding"`
	Dataset  string `json:"dataset"`
}

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

	for {
		select {
		case data := <-input:
			var request CommandRequest
			err := json.Unmarshal(data, &request)
			if err != nil {
				log.Println("Error decoding input:", err)
				continue
			}

			switch request.Command {
			case "readDataset":
				var dsRequest ReadDatasetRequest
				err := json.Unmarshal(data, &dsRequest)
				if err != nil || (dsRequest.Encoding == "" && dsRequest.Dataset == "") {
					// log.Println("Error decoding ReadDatasetRequest:", err)
					continue
				}
				// log.Println("ReadDatasetRequest received:", dsRequest.Dataset, dsRequest.Encoding)
				args := []string{"./../c/zowex", "data-set", "view", dsRequest.Dataset}
				if len(dsRequest.Encoding) != 0 {
					args = append(args, "--encoding", dsRequest.Encoding)
				}
				out, err := exec.Command(args[0], args[1:]...).Output()
				fmt.Println(string(out))
			}
		}
	}

}
