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

package cmds

import (
	"encoding/base64"
	"log"
	"strings"

	t "zowe-native-proto/ioserver/types/common"
	"zowe-native-proto/ioserver/types/ds"
	utils "zowe-native-proto/ioserver/utils"
)

// HandleReadDatasetRequest handles a ReadDatasetRequest by invoking the `zowex data-set view` command
func HandleReadDatasetRequest(jsonData []byte) {
	dsRequest, err := utils.ParseCommandRequest[ds.ReadDatasetRequest](jsonData)
	if err != nil || (dsRequest.Encoding == "" && dsRequest.Dsname == "") {
		return
	}

	args := []string{"data-set", "view", dsRequest.Dsname}
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
	utils.PrintCommandResponse(ds.ReadDatasetResponse{
		Encoding: dsRequest.Encoding,
		Dataset:  dsRequest.Dsname,
		Data:     data,
	})
}

// HandleWriteDatasetRequest handles a WriteDatasetRequest by invoking the `zowex data-set write` command
func HandleWriteDatasetRequest(jsonData []byte) {
	dsRequest, err := utils.ParseCommandRequest[ds.WriteDatasetRequest](jsonData)
	if err != nil || (dsRequest.Encoding == "" && dsRequest.Dsname == "") {
		return
	}

	decodedBytes, err := base64.StdEncoding.DecodeString(dsRequest.Data)
	if err != nil {
		utils.PrintErrorResponse("Failed to decode dataset contents: %v", err)
		return
	}
	args := []string{"data-set", "write", dsRequest.Dsname}
	if len(dsRequest.Encoding) > 0 {
		args = append(args, "--encoding", dsRequest.Encoding)
	}
	cmd := utils.BuildCommandNoAutocvt(args)
	stdin, err := cmd.StdinPipe()
	if err != nil {
		utils.PrintErrorResponse("Failed to open stdin pipe: %v", err)
		return
	}

	go func() {
		defer stdin.Close()
		_, err = stdin.Write(decodedBytes)
		if err != nil {
			utils.PrintErrorResponse("Failed to write to stdin pipe: %v", err)
		}
	}()

	_, err = cmd.Output()
	if err != nil {
		utils.PrintErrorResponse("Failed to pipe stdin to command: %v", err)
		return
	}

	utils.PrintCommandResponse(ds.WriteDatasetResponse{
		Success: true,
		Dataset: dsRequest.Dsname,
	})
}

// HandleListDatasetsRequest handles a ListDatasetsRequest by invoking the `zowex data-set list` command
func HandleListDatasetsRequest(jsonData []byte) {
	listRequest, err := utils.ParseCommandRequest[ds.ListDatasetsRequest](jsonData)
	if err != nil {
		return
	}

	args := []string{"data-set", "list", listRequest.Pattern, "--warn", "false", "--rfc", "true"}
	// if len(listRequest.Start) != 0 {
	// 	args = append(args, "--start", listRequest.Start)
	// }

	out, err := utils.BuildCommand(args).CombinedOutput()
	if err != nil {
		utils.PrintErrorResponse("Error executing command: %v", err)
		return
	}

	datasets := strings.Split(strings.TrimSpace(string(out)), "\n")
	dsResponse := ds.ListDatasetsResponse{
		Items:        make([]t.Dataset, len(datasets)),
		ReturnedRows: len(datasets),
	}

	for i, ds := range datasets {
		vals := strings.Split(ds, ",")
		dsResponse.Items[i] = t.Dataset{
			Name:   strings.TrimSpace(vals[0]),
			Dsorg:  vals[1],
			Volser: vals[2],
		}
	}
	utils.PrintCommandResponse(dsResponse)
}

// HandleListDsMembersRequest handles a ListDsMembersRequest by invoking the `zowex data-set list-members` command
func HandleListDsMembersRequest(jsonData []byte) {
	listRequest, err := utils.ParseCommandRequest[ds.ListDsMembersRequest](jsonData)
	if err != nil {
		return
	}

	args := []string{"data-set", "list-members", listRequest.Dsname}
	// if len(listRequest.Start) != 0 {
	// 	args = append(args, "--start", listRequest.Start)
	// }

	out, err := utils.BuildCommand(args).Output()
	if err != nil {
		utils.PrintErrorResponse("Error executing command: %v", err)
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
	dsRequest, err := utils.ParseCommandRequest[ds.RestoreDatasetRequest](jsonData)
	if err != nil {
		return
	}

	args := []string{"data-set", "restore", dsRequest.Dsname}
	_, err = utils.BuildCommand(args).Output()
	if err != nil {
		utils.PrintErrorResponse("Failed to restore data set: %v", err)
		return
	}

	utils.PrintCommandResponse(ds.RestoreDatasetResponse{
		Success: true,
	})
}

// HandleDeleteDatasetRequest handles a DeleteDatasetRequest by invoking the `zowex data-set delete` command
func HandleDeleteDatasetRequest(jsonData []byte) {
	dsRequest, err := utils.ParseCommandRequest[ds.DeleteDatasetRequest](jsonData)
	if err != nil {
		return
	}

	args := []string{"data-set", "delete", dsRequest.Dsname}
	_, err = utils.BuildCommand(args).Output()

	utils.PrintCommandResponse(ds.DeleteDatasetResponse{
		Success: true,
		Dsname:  dsRequest.Dsname,
	})
}

// HandleCreateDatasetRequest handles a CreateDatasetRequest by invoking the `zowex data-set create` command
func HandleCreateDatasetRequest(jsonData []byte) {
	dsRequest, err := utils.ParseCommandRequest[ds.CreateDatasetRequest](jsonData)
	if err != nil {
		return
	}

	args := []string{"./zowex", "data-set", "create", dsRequest.Dsname}
	_, err = utils.BuildCommand(args).Output()
	if err != nil {
		utils.PrintErrorResponse("Failed to create data set: %v", err)
		return
	}

	utils.PrintCommandResponse(ds.CreateDatasetResponse{
		Success: true,
	})
}

// HandleCreateMemberRequest handles a CreateMemberRequest by invoking the `zowex data-set create-member` command
func HandleCreateMemberRequest(jsonData []byte) {
	dsRequest, err := utils.ParseCommandRequest[ds.CreateMemberRequest](jsonData)
	if err != nil {
		return
	}

	args := []string{"./zowex", "data-set", "create-member", dsRequest.Dsname}
	_, err = utils.BuildCommand(args).Output()
	if err != nil {
		utils.PrintErrorResponse("Failed to create data-set member: %v", err)
		return
	}

	utils.PrintCommandResponse(ds.CreateMemberResponse{
		Success: true,
	})
}
