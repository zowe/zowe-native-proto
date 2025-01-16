package main

import (
	"encoding/json"
	"fmt"
	"log"
	"os"
)

func HandleListUssRequest(jsonData []byte) {
	var listRequest ListUssRequest
	err := json.Unmarshal(jsonData, &listRequest)
	if err != nil {
		// log.Println("Error decoding ListDatasetsRequest:", err)
		return
	}

	dirPath := listRequest.Path

	files, err := os.ReadDir(dirPath)
	if err != nil {
		log.Println("Error reading directory:", err)
		return
	}

	ussResponse := ListUssResponse{
		Items: []UssItem{},
	}

	for _, file := range files {
		ussResponse.Items = append(ussResponse.Items, UssItem{
			Name: file.Name(),
		})
	}

	ussResponse.ReturnedRows = len(ussResponse.Items)

	v, err := json.Marshal(ussResponse)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		fmt.Println(string(v))
	}
}
