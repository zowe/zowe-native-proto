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
	"os"
	"strings"

	t "zowe-native-proto/ioserver/types/common"
	"zowe-native-proto/ioserver/types/ds"
	utils "zowe-native-proto/ioserver/utils"
)

// HandleReadDatasetRequest handles a ReadDatasetRequest by invoking the `zowex data-set view` command
func HandleReadDatasetRequest(jsonData []byte) {
	var dsRequest ds.ReadDatasetRequest
	err := json.Unmarshal(jsonData, &dsRequest)
	if err != nil || (dsRequest.Encoding == "" && dsRequest.Dsname == "") {
		// log.Println("Error decoding ReadDatasetRequest:", err)
		return
	}
	// log.Println("ReadDatasetRequest received:", dsRequest.Dataset, dsRequest.Encoding)
	args := []string{"./zowex", "data-set", "view", dsRequest.Dsname}
	hasEncoding := len(dsRequest.Encoding) != 0
	if hasEncoding {
		args = append(args, "--encoding", dsRequest.Encoding, "--rfb", "true")
	}
	out, err := utils.BuildCommand(args).Output()
	if err != nil {
		log.Println("Error executing command:", err)
		return
	}

	data := utils.CollectContentsAsBytes(string(out), hasEncoding)

	dsResponse := ds.ReadDatasetResponse{
		Encoding: dsRequest.Encoding,
		Dataset:  dsRequest.Dsname,
		Data:     data,
	}
	utils.PrintCommandResponse(dsResponse)
}

// HandleWriteDatasetRequest handles a WriteDatasetRequest by invoking the `zowex data-set write` command
func HandleWriteDatasetRequest(jsonData []byte) {
	var dsRequest ds.WriteDatasetRequest
	err := json.Unmarshal(jsonData, &dsRequest)
	if err != nil || (dsRequest.Encoding == "" && dsRequest.Dsname == "") {
		// log.Println("Error decoding WriteDatasetRequest:", err)
		return
	}

	// log.Println("WriteDatasetRequest received:", dsRequest.Dataset, dsRequest.Encoding)
	decodedBytes, err := base64.StdEncoding.DecodeString(dsRequest.Data)
	if err != nil {
		log.Println("Error decoding base64 contents:", err)
		return
	}
	args := []string{"./zowex", "data-set", "write", dsRequest.Dsname}
	if len(dsRequest.Encoding) > 0 {
		args = append(args, "--encoding", dsRequest.Encoding)
	}
	cmd := utils.BuildCommandNoAutocvt(args)
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

	dsResponse := ds.WriteDatasetResponse{
		Success: true,
		Dataset: dsRequest.Dsname,
	}
	utils.PrintCommandResponse(dsResponse)
}

// HandleListDatasetsRequest handles a ListDatasetsRequest by invoking the `zowex data-set list` command
func HandleListDatasetsRequest(jsonData []byte) {
	listRequest := ds.ListDatasetsRequest{
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

	out, err := utils.BuildCommand(args).Output()
	if err != nil {
		log.Println("Error executing command:", err)
		return
	}

	datasets := strings.Split(strings.TrimSpace(string(out)), "\n")

	dsResponse := ds.ListDatasetsResponse{
		Items: make([]t.Dataset, len(datasets)),
	}

	for i, ds := range datasets {
		vals := strings.Split(ds, ",")
		dsResponse.Items[i] = t.Dataset{
			Name:   strings.TrimSpace(vals[0]),
			Dsorg:  vals[1],
			Volser: vals[2],
		}
	}
	dsResponse.ReturnedRows = len(datasets)

	utils.PrintCommandResponse(dsResponse)
}

// HandleListDsMembersRequest handles a ListDsMembersRequest by invoking the `zowex data-set list-members` command
func HandleListDsMembersRequest(jsonData []byte) {
	var listRequest ds.ListDsMembersRequest
	err := json.Unmarshal(jsonData, &listRequest)
	if err != nil {
		// log.Println("Error decoding ListDsMembersRequest:", err)
		return
	}

	args := []string{"./zowex", "data-set", "list-members", listRequest.Dsname}
	// if len(listRequest.Start) != 0 {
	// 	args = append(args, "--start", listRequest.Start)
	// }

	out, err := utils.BuildCommand(args).Output()
	if err != nil {
		log.Println("Error executing command:", err)
		return
	}

	members := strings.Split(strings.TrimSpace(string(out)), "\n")

	dsResponse := ds.ListDsMembersResponse{
		Items: make([]t.DsMember, len(members)),
	}

	for i, member := range members {
		name := strings.TrimSpace(member)
		if len(name) == 0 {
			continue
		}
		dsResponse.Items[i] = t.DsMember{
			Name: name,
		}
		dsResponse.ReturnedRows++
	}

	utils.PrintCommandResponse(dsResponse)
}

// HandleRestoreDatasetRequest handles a RestoreDatasetRequest by invoking the `zowex data-set restore` command
func HandleRestoreDatasetRequest(jsonData []byte) {
	var dsRequest ds.RestoreDatasetRequest
	err := json.Unmarshal(jsonData, &dsRequest)
	if err != nil {
		return
	}

	args := []string{"./zowex", "data-set", "restore", dsRequest.Dsname}

	out, err := utils.BuildCommand(args).Output()
	if err != nil {
		log.Println("Error executing command:", err)
		log.Println(string(out))
		return
	}

	dsResponse := ds.RestoreDatasetResponse{
		Success: true,
	}
	v, err := json.Marshal(dsResponse)
	if err != nil {
		fmt.Fprintln(os.Stderr, err.Error())
	} else {
		fmt.Println(string(v))
	}
}

// HandleDeleteDatasetRequest handles a DeleteDatasetRequest by invoking the `zowex data-set delete` command
func HandleDeleteDatasetRequest(jsonData []byte) {
	var dsRequest ds.DeleteDatasetRequest
	err := json.Unmarshal(jsonData, &dsRequest)
	if err != nil {
		return
	}

	args := []string{"./zowex", "data-set", "delete", dsRequest.Dsname}

	out, err := utils.BuildCommand(args).Output()
	_ = out
	dsResponse := ds.DeleteDatasetResponse{
		Success: true,
		Dsname:  dsRequest.Dsname,
	}

	if err != nil {
		dsResponse.Success = false
	}

	utils.PrintCommandResponse(dsResponse)
}
