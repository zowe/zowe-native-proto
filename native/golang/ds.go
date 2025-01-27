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
	"encoding/base64"
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

func HandleWriteDatasetRequest(jsonData []byte) {
	var dsRequest WriteDatasetRequest
	err := json.Unmarshal(jsonData, &dsRequest)
	if err != nil || (dsRequest.Encoding == "" && dsRequest.Dataset == "") {
		// log.Println("Error decoding ReadDatasetRequest:", err)
		return
	}
	// log.Println("ReadDatasetRequest received:", dsRequest.Dataset, dsRequest.Encoding)
	decodedBytes, err := base64.StdEncoding.DecodeString(dsRequest.Contents)
	if err != nil {
		log.Println("Error decoding base64 contents:", err)
		return
	}
	args := []string{"./zowex", "data-set", "write", dsRequest.Dataset}
	if len(dsRequest.Encoding) > 0 {
		args = append(args, "--encoding", dsRequest.Encoding)
	}
	cmd := exec.Command(args[0], args[1:]...)
	stdin, err := cmd.StdinPipe()
	if err != nil {
		log.Println("Error opening stdin pipe:", err)
		return
	}

	go func() {
		defer stdin.Close()
		_, err = stdin.Write(decodedBytes)
		if err != nil {
			log.Println("Error writing to stdin pipe:", err)
		}
	}()

	out, err := cmd.Output()
	if err != nil {
		log.Println("Error piping stdin to command:", err)
		return
	}
	// discard CLI output as its currently unused
	_ = out

	dsResponse := WriteDatasetResponse{
		Success: true,
		Dataset: dsRequest.Dataset,
	}
	v, err := json.Marshal(dsResponse)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		fmt.Println(string(v))
	}
}

func HandleListDatasetsRequest(jsonData []byte) {
	listRequest := ListDatasetsRequest{
		Attributes: false,
	}
	err := json.Unmarshal(jsonData, &listRequest)
	if err != nil {
		// log.Println("Error decoding ListDatasetsRequest:", err)
		return
	}

	args := []string{"./zowex", "data-set", "list", listRequest.Pattern, "--rfc", "true"}
	// if len(listRequest.Start) != 0 {
	// 	args = append(args, "--start", listRequest.Start)
	// }

	out, err := exec.Command(args[0], args[1:]...).Output()
	if err != nil {
		log.Println("Error executing command:", err)
		return
	}

	datasets := strings.Split(strings.TrimSpace(string(out)), "\n")

	dsResponse := ListDatasetsResponse{
		Items: make([]Dataset, len(datasets)),
	}

	for i, ds := range datasets {
		vals := strings.Split(ds, ",")
		dsResponse.Items[i] = Dataset{
			Name:   strings.TrimSpace(vals[0]),
			Dsorg:  vals[1],
			Volser: vals[2],
		}
	}
	dsResponse.ReturnedRows = len(datasets)

	v, err := json.Marshal(dsResponse)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		fmt.Println(string(v))
	}
}

func HandleListDsMembersRequest(jsonData []byte) {
	var listRequest ListDsMembersRequest
	err := json.Unmarshal(jsonData, &listRequest)
	if err != nil {
		// log.Println("Error decoding ListDsMembersRequest:", err)
		return
	}

	args := []string{"./zowex", "data-set", "list-members", listRequest.Dataset}
	// if len(listRequest.Start) != 0 {
	// 	args = append(args, "--start", listRequest.Start)
	// }

	out, err := exec.Command(args[0], args[1:]...).Output()
	if err != nil {
		log.Println("Error executing command:", err)
		return
	}

	members := strings.Split(strings.TrimSpace(string(out)), "\n")

	dsResponse := ListDsMembersResponse{
		Items: make([]DsMember, len(members)),
	}

	for i, member := range members {
		name := strings.TrimSpace(member)
		if len(name) == 0 {
			continue
		}
		dsResponse.Items[i] = DsMember{
			Name: name,
		}
		dsResponse.ReturnedRows++
	}

	v, err := json.Marshal(dsResponse)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		fmt.Println(string(v))
	}
}
