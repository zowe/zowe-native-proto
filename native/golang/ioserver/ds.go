package main

import (
	"encoding/json"
	"fmt"
	"log"
	"os/exec"
)

func HandleReadDatasetRequest(jsonData []byte) {
	var dsRequest ReadDatasetRequest
	err := json.Unmarshal(jsonData, &dsRequest)
	if err != nil || (dsRequest.Encoding == "" && dsRequest.Dataset == "") {
		// log.Println("Error decoding ReadDatasetRequest:", err)
		return
	}
	// log.Println("ReadDatasetRequest received:", dsRequest.Dataset, dsRequest.Encoding)
	args := []string{"./zowex", "data-set", "view", dsRequest.Dataset}
	hasEncoding := len(dsRequest.Encoding) != 0
	if hasEncoding {
		args = append(args, "--encoding", dsRequest.Encoding)
	}
	out, err := exec.Command(args[0], args[1:]...).Output()
	if err != nil {
		log.Println("Error executing command:", err)
		return
	}

	data := collectContentsAsBytes(string(out), hasEncoding)

	dsResponse := ReadDatasetResponse{
		Encoding: dsRequest.Encoding,
		Dataset:  dsRequest.Dataset,
		Data:     data,
	}
	v, err := json.Marshal(dsResponse)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		fmt.Println(string(v))
	}
}
