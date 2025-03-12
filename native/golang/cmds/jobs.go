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
	"strconv"
	"strings"
	t "zowe-native-proto/zowed/types/common"
	"zowe-native-proto/zowed/types/jobs"
	utils "zowe-native-proto/zowed/utils"
)

// HandleListJobsRequest handles a ListJobsRequest by invoking the `zowex job list` command
func HandleListJobsRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[jobs.ListJobsRequest](params)
	if err != nil {
		return nil, err
	}

	args := []string{"job", "list", "--warn", "false", "--rfc", "true"}
	if len(request.Owner) != 0 {
		args = append(args, "--owner", request.Owner)
	}
	if len(request.Prefix) != 0 {
		args = append(args, "--prefix", request.Prefix)
	}

	out, err := conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Failed to list jobs: %v", err)
		return
	}

	returnedJobs := strings.Split(strings.TrimSpace(string(out)), "\n")

	jobsResponse := jobs.ListJobsResponse{
		Items: make([]t.Job, len(returnedJobs)),
	}

	for i, job := range returnedJobs {
		vals := strings.Split(job, ",")
		if len(vals) < 4 {
			continue
		}
		jobsResponse.Items[i] = t.Job{
			Id:      vals[0],
			Retcode: vals[1],
			Name:    strings.TrimSpace(vals[2]),
			Status:  vals[3],
		}
	}

	return jobsResponse, nil
}

// HandleListSpoolsRequest handles a ListSpoolsRequest by invoking the `zowex job list-files` command
func HandleListSpoolsRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[jobs.ListSpoolsRequest](params)
	if err != nil {
		return nil, err
	}

	args := []string{"job", "list-files", request.JobId, "--rfc", "true"}

	out, err := conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Failed to list spools: %v", err)
		return
	}

	spools := strings.Split(strings.TrimSpace(string(out)), "\n")

	response := jobs.ListSpoolsResponse{
		Items: make([]t.Spool, len(spools)),
	}

	for i, spool := range spools {
		vals := strings.Split(spool, ",")
		if len(vals) < 4 {
			continue
		}
		id, err := strconv.Atoi(strings.TrimSpace(vals[2]))
		if err != nil {
			continue
		}
		response.Items[i] = t.Spool{
			Id:       id,
			DdName:   vals[0],
			StepName: vals[3],
			DsName:   vals[1],
			ProcStep: vals[4],
		}
	}

	return response, nil
}

// HandleReadSpoolRequest handles a ReadSpoolRequest by invoking the `zowex job view-file` command
func HandleReadSpoolRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[jobs.ReadSpoolRequest](params)
	if err != nil {
		return nil, err
	}

	if len(request.Encoding) == 0 {
		request.Encoding = fmt.Sprintf("IBM-%d", utils.DefaultEncoding)
	}
	args := []string{"job", "view-file", request.JobId, strconv.Itoa(request.DsnKey), "--encoding", request.Encoding, "--rfb", "true"}
	out, err := conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Failed to read spool: %v", err)
		return
	}

	data, e := utils.CollectContentsAsBytes(string(out), true)
	result = jobs.ReadSpoolResponse{
		Encoding: request.Encoding,
		DsnKey:   request.DsnKey,
		JobId:    request.JobId,
		Data:     data,
	}
	return
}

// HandleGetJclRequest handles a GetJclRequest by invoking the `zowex job view-jcl` command
func HandleGetJclRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[jobs.GetJclRequest](params)
	if err != nil {
		return nil, err
	}

	args := []string{"job", "view-jcl", request.JobId}
	out, err := conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Failed to get JCL: %v", err)
		return
	}

	result = jobs.GetJclResponse{
		JobId: request.JobId,
		Data:  string(out),
	}
	return
}

// HandleGetStatusRequest handles a GetStatusRequest by invoking the `zowex job view-status` command
func HandleGetStatusRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[jobs.GetJobStatusRequest](params)
	if err != nil {
		return nil, err
	}

	args := []string{"job", "view-status", request.JobId, "--rfc", "true"}
	out, err := conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Failed to get status: %v", err)
		return
	}
	vals := strings.Split(string(out), ",")
	if len(vals) < 4 {
		e = fmt.Errorf("Missing job properties for %s", request.JobId)
	} else {
		result = t.Job{
			Id:      vals[0],
			Retcode: vals[1],
			Name:    strings.TrimSpace(vals[2]),
			Status:  vals[3],
		}
	}
	return
}

// HandleSubmitJobRequest handles a SubmitJobRequest by invoking the `zowex job submit` command
func HandleCancelJobRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[jobs.CancelJobRequest](params)
	if err != nil {
		return nil, err
	}

	_, err = conn.ExecCmd([]string{"job", "cancel", request.JobId})

	if err != nil {
		e = fmt.Errorf("Failed to cancel job: %v", err)
		return
	}

	result = jobs.CancelJobResponse{
		Success: true,
		JobId:   request.JobId,
	}
	return
}

// HandleHoldJobRequest holds a job by invoking the `zowex job hold` command
func HandleHoldJobRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[jobs.HoldJobRequest](params)
	if err != nil {
		return nil, err
	}

	_, err = conn.ExecCmd([]string{"job", "hold", request.JobId})

	if err != nil {
		e = fmt.Errorf("Failed to hold job: %v", err)
		return
	}

	result = jobs.HoldJobResponse{
		Success: true,
		JobId:   request.JobId,
	}
	return
}

// HandleReleaseJobRequest releases a job by invoking the `zowex job release` command
func HandleReleaseJobRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[jobs.ReleaseJobRequest](params)
	if err != nil {
		return nil, err
	}

	_, err = conn.ExecCmd([]string{"job", "release", request.JobId})

	if err != nil {
		e = fmt.Errorf("Failed to release job: %v", err)
		return
	}

	result = jobs.ReleaseJobResponse{
		Success: true,
		JobId:   request.JobId,
	}
	return
}

// HandleSubmitJobRequest handles a SubmitJobRequest by invoking the `zowex job submit` command
func HandleSubmitJobRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[jobs.SubmitJobRequest](params)
	if err != nil {
		return nil, err
	}

	out, err := conn.ExecCmd([]string{"job", "submit", request.Dsname, "--only-jobid", "true"})

	if err != nil {
		e = fmt.Errorf("Failed to submit job: %v", err)
		return
	}

	result = jobs.SubmitJobResponse{
		Success: true,
		Dsname:  request.Dsname,
		JobId:   string(out),
	}
	return
}

// HandleSubmitUssRequest handles a SubmitUssRequest by invoking the `zowex job submit-uss` command
func HandleSubmitUssRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[jobs.SubmitUssRequest](params)
	if err != nil {
		return nil, err
	}

	out, err := conn.ExecCmd([]string{"job", "submit-uss", request.Path, "--only-jobid", "true"})

	if err != nil {
		e = fmt.Errorf("Failed to submit job: %v", err)
		return
	}

	result = jobs.SubmitUssResponse{
		Success: true,
		Path:    request.Path,
		JobId:   string(out),
	}
	return
}

// HandleSubmitJclRequest handles a SubmitJclRequest by invoking the `zowex job submit-jcl` command
func HandleSubmitJclRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[jobs.SubmitJclRequest](params)
	if err != nil {
		return nil, err
	}

	decodedBytes, err := base64.StdEncoding.DecodeString(request.Jcl)
	if err != nil {
		e = fmt.Errorf("failed to decode JCL contents: %v", err)
		return
	}

	byteString := hex.EncodeToString(decodedBytes)
	if len(request.Encoding) == 0 {
		request.Encoding = fmt.Sprintf("IBM-%d", utils.DefaultEncoding)
	}

	cmd := utils.BuildCommand([]string{"job", "submit-jcl", "--only-jobid", "true", "--encoding", request.Encoding})
	stdin, err := cmd.StdinPipe()
	if err != nil {
		e = fmt.Errorf("failed to open stdin pipe to zowex: %v", err)
		return
	}

	go func() {
		defer stdin.Close()
		_, err = stdin.Write([]byte(byteString))
		if err != nil {
			e = fmt.Errorf("failed to write to pipe: %v", err)
		}
	}()

	out, err := cmd.CombinedOutput()
	if err != nil {
		e = fmt.Errorf("failed to submit JCL: %v", err)
		conn.LastExitCode = cmd.ProcessState.ExitCode()
		return
	}

	result = jobs.SubmitJclResponse{
		Success: true,
		JobId:   string(out),
	}
	return
}

// HandleDeleteJobRequest handles a DeleteJobRequest by invoking the `zowex job delete` command
func HandleDeleteJobRequest(conn *utils.StdioConn, params []byte) (result any, e error) {
	request, err := utils.ParseCommandRequest[jobs.DeleteJobRequest](params)
	if err != nil {
		return nil, err
	}

	_, err = conn.ExecCmd([]string{"job", "delete", request.JobId})
	if err != nil {
		e = fmt.Errorf("Failed to delete job %s: %v", request.JobId, err)
		return
	}

	result = jobs.DeleteJobResponse{
		Success: true,
		JobId:   request.JobId,
	}
	return
}
