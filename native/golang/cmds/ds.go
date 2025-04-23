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
	"encoding/hex"
	"fmt"
	"strings"

	t "zowe-native-proto/zowed/types/common"
	"zowe-native-proto/zowed/types/ds"
	utils "zowe-native-proto/zowed/utils"
)

// HandleReadDatasetRequest handles a ReadDatasetRequest by invoking the `zowex data-set view` command
func HandleReadDatasetRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[ds.ReadDatasetRequest](params)
	if err != nil {
		return nil, err
	} else if request.Dsname == "" {
		return nil, fmt.Errorf("Missing required parameters: Dsname")
	}

	if len(request.Encoding) == 0 {
		request.Encoding = fmt.Sprintf("IBM-%d", utils.DefaultEncoding)
	}
	args := []string{"data-set", "view", request.Dsname, "--encoding", request.Encoding, "--rfb", "true"}
	out, err := conn.ExecCmd(args)
	if err != nil {
		return nil, fmt.Errorf("Error executing command: %v", err)
	}

	output := utils.YamlToMap(string(out))

	var data []byte
	if len(output) > 0 {
		data, _ = utils.CollectContentsAsBytes(output["data"], true)
	} else {
		data = []byte{}
	}

	result = ds.ReadDatasetResponse{
		Encoding: request.Encoding,
		Etag:     output["etag"],
		Dataset:  request.Dsname,
		Data:     data,
	}
	return
}

// HandleWriteDatasetRequest handles a WriteDatasetRequest by invoking the `zowex data-set write` command
func HandleWriteDatasetRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[ds.WriteDatasetRequest](params)
	if err != nil {
		return nil, err
	} else if request.Dsname == "" {
		return nil, fmt.Errorf("Missing required parameters: Dsname")
	}

	decodedBytes, err := base64.StdEncoding.DecodeString(request.Data)
	if err != nil {
		return nil, fmt.Errorf("Failed to decode dataset contents: %v", err)
	}

	byteString := hex.EncodeToString(decodedBytes)

	if len(request.Encoding) == 0 {
		request.Encoding = fmt.Sprintf("IBM-%d", utils.DefaultEncoding)
	}
	args := []string{"data-set", "write", request.Dsname, "--encoding", request.Encoding, "--etag-only", "true"}
	if len(request.Etag) > 0 {
		args = append(args, "--etag", request.Etag)
	}
	cmd := utils.BuildCommand(args)
	stdin, err := cmd.StdinPipe()
	if err != nil {
		return nil, fmt.Errorf("Failed to open stdin pipe: %v", err)
	}

	go func() {
		defer stdin.Close()
		_, err = stdin.Write([]byte(byteString))
		if err != nil {
			e = fmt.Errorf("Failed to write to stdin pipe: %v", err)
		}
	}()

	out, err := cmd.CombinedOutput()
	if err != nil {
		e = fmt.Errorf("Failed to pipe stdin to command: %v", string(out))
		conn.LastExitCode = cmd.ProcessState.ExitCode()
		return
	}

	result = ds.WriteDatasetResponse{
		Success: true,
		Dataset: request.Dsname,
		Etag:    strings.TrimRight(string(out), "\n"),
	}
	return
}

// HandleListDatasetsRequest handles a ListDatasetsRequest by invoking the `zowex data-set list` command
func HandleListDatasetsRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[ds.ListDatasetsRequest](params)
	if err != nil {
		return nil, err
	}

	args := []string{"data-set", "list", request.Pattern, "--no-warn", "--rfc"}
	// if len(listRequest.Start) != 0 {
	// 	args = append(args, "--start", listRequest.Start)
	// }

	out, err := conn.ExecCmd(args)
	if err != nil {
		return nil, fmt.Errorf("Error executing command: %v", err)
	}

	rawResponse := strings.TrimSpace(string(out))
	if len(rawResponse) == 0 {
		return ds.ListDatasetsResponse{
			Items:        []t.Dataset{},
			ReturnedRows: 0,
		}, nil
	}
	datasets := strings.Split(rawResponse, "\n")
	dsResponse := ds.ListDatasetsResponse{
		Items:        make([]t.Dataset, len(datasets)),
		ReturnedRows: len(datasets),
	}

	for i, ds := range datasets {
		vals := strings.Split(ds, ",")
		if len(vals) < 4 {
			continue
		}
		migr := false
		if vals[3] == "true" {
			migr = true
		}
		dsResponse.Items[i] = t.Dataset{
			Name:   strings.TrimSpace(vals[0]),
			Dsorg:  vals[1],
			Volser: vals[2],
			Migr:   migr,
		}
	}

	return dsResponse, nil
}

// HandleListDsMembersRequest handles a ListDsMembersRequest by invoking the `zowex data-set list-members` command
func HandleListDsMembersRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[ds.ListDsMembersRequest](params)
	if err != nil {
		return nil, err
	}

	args := []string{"data-set", "list-members", request.Dsname, "--no-warn"}
	// if len(listRequest.Start) != 0 {
	// 	args = append(args, "--start", listRequest.Start)
	// }

	out, err := conn.ExecCmd(args)
	if err != nil {
		return nil, fmt.Errorf("Error executing command: %v", err)
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

	return dsResponse, nil
}

// HandleRestoreDatasetRequest handles a RestoreDatasetRequest by invoking the `zowex data-set restore` command
func HandleRestoreDatasetRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[ds.RestoreDatasetRequest](params)
	if err != nil {
		return nil, err
	}

	args := []string{"data-set", "restore", request.Dsname}
	_, err = conn.ExecCmd(args)
	if err != nil {
		return nil, fmt.Errorf("Failed to restore data set: %v", err)
	}

	result = ds.RestoreDatasetResponse{
		Success: true,
	}
	return
}

// HandleDeleteDatasetRequest handles a DeleteDatasetRequest by invoking the `zowex data-set delete` command
func HandleDeleteDatasetRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[ds.DeleteDatasetRequest](params)
	if err != nil {
		return nil, err
	}

	args := []string{"data-set", "delete", request.Dsname}
	_, err = conn.ExecCmd(args)
	if err != nil {
		return nil, fmt.Errorf("Error executing command: %v", err)
	}

	result = ds.DeleteDatasetResponse{
		Success: true,
		Dsname:  request.Dsname,
	}
	return
}

// HandleCreateDatasetRequest handles a CreateDatasetRequest by invoking the `zowex data-set create` command
func HandleCreateDatasetRequest(conn *utils.StdioConn, jsonData []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[ds.CreateDatasetRequest](jsonData)
	if err != nil {
		return nil, err
	}

	args := []string{"data-set", "create", request.Dsname}
	_, err = conn.ExecCmd(args)
	if err != nil {
		return nil, fmt.Errorf("Failed to create data set: %v", err)
	}

	result = ds.CreateDatasetResponse{
		Success: true,
		Dsname:  request.Dsname,
	}
	return
}

// HandleCreateMemberRequest handles a CreateMemberRequest by invoking the `zowex data-set create-member` command
func HandleCreateMemberRequest(conn *utils.StdioConn, jsonData []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[ds.CreateMemberRequest](jsonData)
	if err != nil {
		return nil, err
	}

	args := []string{"data-set", "create-member", request.Dsname}
	_, err = conn.ExecCmd(args)
	if err != nil {
		return nil, fmt.Errorf("Failed to create data set member: %v", err)
	}

	result = ds.CreateMemberResponse{
		Success: true,
		Dsname:  request.Dsname,
	}
	return
}
