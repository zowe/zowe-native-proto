package main

import (
	"encoding/json"
	"fmt"
	"log"
	"os/exec"
	"strings"
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

func HandleListDatasetRequest(jsonData []byte) {
	var listRequest ListDatasetRequest
	err := json.Unmarshal(jsonData, &listRequest)
	if err != nil {
		// log.Println("Error decoding ListDatasetsRequest:", err)
		return
	}

	args := []string{"./zowex", "data-set", "list", listRequest.Pattern}
	// if len(listRequest.Start) != 0 {
	// 	args = append(args, "--start", listRequest.Start)
	// }

	out, err := exec.Command(args[0], args[1:]...).Output()
	if err != nil {
		log.Println("Error executing command:", err)
		return
	}

	items := strings.Split(string(out), "\n")

	dsResponse := ListDatasetResponse{
		Items: []Dataset{},
	}

	for _, item := range items {
		vals := strings.Split(item, "\t\t\t")
		dsResponse.Items = append(dsResponse.Items, Dataset{
			Name:  vals[0],
			Dsorg: vals[1],
		})
	}
	dsResponse.ReturnedRows = len(items)

	v, err := json.Marshal(dsResponse)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		fmt.Println(string(v))
	}
}
