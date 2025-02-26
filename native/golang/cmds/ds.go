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
	"fmt"
	"strings"

	t "zowe-native-proto/ioserver/types/common"
	"zowe-native-proto/ioserver/types/ds"
	utils "zowe-native-proto/ioserver/utils"
)

// HandleReadDatasetRequest handles a ReadDatasetRequest by invoking the `zowex data-set view` command
func HandleReadDatasetRequest(conn utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[ds.ReadDatasetRequest](params)
	if err != nil {
		return nil, err
	} else if request.Dsname == "" {
		e = fmt.Errorf("Missing required parameters: Dsname")
		return
	}

	if len(request.Encoding) == 0 {
		request.Encoding = fmt.Sprintf("IBM-%d", utils.DefaultEncoding)
	}
	args := []string{"data-set", "view", request.Dsname, "--encoding", request.Encoding, "--rfb", "true"}
	out, err := conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Error executing command:", err)
		return
	}

	data, e := utils.CollectContentsAsBytes(string(out), true)
	result = ds.ReadDatasetResponse{
		Encoding: request.Encoding,
		Dataset:  request.Dsname,
		Data:     data,
	}
	return
}

// HandleWriteDatasetRequest handles a WriteDatasetRequest by invoking the `zowex data-set write` command
func HandleWriteDatasetRequest(conn utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[ds.WriteDatasetRequest](params)
	if err != nil {
		return nil, err
	} else if request.Dsname == "" {
		e = fmt.Errorf("Missing required parameters: Dsname")
		return
	}

	decodedBytes, err := base64.StdEncoding.DecodeString(request.Data)
	if err != nil {
		e = fmt.Errorf("Failed to decode dataset contents: %v", err)
		return
	}
	if len(request.Encoding) == 0 {
		request.Encoding = fmt.Sprintf("IBM-%d", utils.DefaultEncoding)
	}
	args := []string{"data-set", "write", request.Dsname, "--encoding", request.Encoding}
	cmd := utils.BuildCommandNoAutocvt(args)
	stdin, err := cmd.StdinPipe()
	if err != nil {
		e = fmt.Errorf("Failed to open stdin pipe: %v", err)
		return
	}

	go func() {
		defer stdin.Close()
		_, err = stdin.Write(decodedBytes)
		if err != nil {
			e = fmt.Errorf("Failed to write to stdin pipe: %v", err)
		}
	}()

	_, err = cmd.Output()
	if err != nil {
		e = fmt.Errorf("Failed to pipe stdin to command: %v", err)
		*conn.LastExitCode = cmd.ProcessState.ExitCode()
		return
	}

	result = ds.WriteDatasetResponse{
		Success: true,
		Dataset: request.Dsname,
	}
	return
}

// HandleListDatasetsRequest handles a ListDatasetsRequest by invoking the `zowex data-set list` command
func HandleListDatasetsRequest(conn utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[ds.ListDatasetsRequest](params)
	if err != nil {
		return nil, err
	}

	args := []string{"data-set", "list", request.Pattern, "--warn", "false", "--rfc", "true"}
	// if len(listRequest.Start) != 0 {
	// 	args = append(args, "--start", listRequest.Start)
	// }

	out, err := conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Error executing command: %v", err)
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

	return dsResponse, nil
}

// HandleListDsMembersRequest handles a ListDsMembersRequest by invoking the `zowex data-set list-members` command
func HandleListDsMembersRequest(conn utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[ds.ListDsMembersRequest](params)
	if err != nil {
		return nil, err
	}

	args := []string{"data-set", "list-members", request.Dsname}
	// if len(listRequest.Start) != 0 {
	// 	args = append(args, "--start", listRequest.Start)
	// }

	out, err := conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Error executing command: %v", err)
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

	return dsResponse, nil
}

// HandleRestoreDatasetRequest handles a RestoreDatasetRequest by invoking the `zowex data-set restore` command
func HandleRestoreDatasetRequest(conn utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[ds.RestoreDatasetRequest](params)
	if err != nil {
		return nil, err
	}

	args := []string{"data-set", "restore", request.Dsname}
	_, err = conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Failed to restore data set: %v", err)
		return
	}

	result = ds.RestoreDatasetResponse{
		Success: true,
	}
	return
}

// HandleDeleteDatasetRequest handles a DeleteDatasetRequest by invoking the `zowex data-set delete` command
func HandleDeleteDatasetRequest(conn utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[ds.DeleteDatasetRequest](params)
	if err != nil {
		return nil, err
	}

	args := []string{"data-set", "delete", request.Dsname}
	_, err = conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Error executing command: %v", err)
		return
	}

	result = ds.DeleteDatasetResponse{
		Success: true,
		Dsname:  request.Dsname,
	}
	return
}
