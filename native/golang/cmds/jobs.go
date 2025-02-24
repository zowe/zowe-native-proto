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
	"strconv"
	"strings"
	t "zowe-native-proto/ioserver/types/common"
	"zowe-native-proto/ioserver/types/jobs"
	utils "zowe-native-proto/ioserver/utils"
)

// HandleListJobsRequest handles a ListJobsRequest by invoking the `zowex job list` command
func HandleListJobsRequest(conn utils.StdioConn, p any) (result any, e error) {
	params := p.(jobs.ListJobsRequest)
	args := []string{"job", "list", "--rfc", "true"}
	if len(params.Owner) != 0 {
		args = append(args, "--owner", params.Owner)
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
func HandleListSpoolsRequest(conn utils.StdioConn, p any) (result any, e error) {
	params := p.(jobs.ListSpoolsRequest)
	args := []string{"job", "list-files", params.JobId, "--rfc", "true"}

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
func HandleReadSpoolRequest(conn utils.StdioConn, p any) (result any, e error) {
	params := p.(jobs.ReadSpoolRequest)
	args := []string{"job", "view-file", params.JobId, strconv.Itoa(params.DsnKey)}
	hasEncoding := len(params.Encoding) != 0
	if hasEncoding {
		args = append(args, "--encoding", params.Encoding, "--rfb", "true")
	}
	out, err := conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Failed to read spool: %v", err)
		return
	}

	data, e := utils.CollectContentsAsBytes(string(out), hasEncoding)
	result = jobs.ReadSpoolResponse{
		Encoding: params.Encoding,
		DsnKey:   params.DsnKey,
		JobId:    params.JobId,
		Data:     data,
	}
	return
}

// HandleGetJclRequest handles a GetJclRequest by invoking the `zowex job view-jcl` command
func HandleGetJclRequest(conn utils.StdioConn, p any) (result any, e error) {
	params := p.(jobs.GetJclRequest)
	args := []string{"job", "view-jcl", params.JobId}
	out, err := conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Failed to get JCL: %v", err)
		return
	}

	result = jobs.GetJclResponse{
		JobId: params.JobId,
		Data:  string(out),
	}
	return
}

// HandleGetStatusRequest handles a GetStatusRequest by invoking the `zowex job view-status` command
func HandleGetStatusRequest(conn utils.StdioConn, p any) (result any, e error) {
	params := p.(jobs.GetJobStatusRequest)
	args := []string{"job", "view-status", params.JobId, "--rfc", "true"}
	out, err := conn.ExecCmd(args)
	if err != nil {
		e = fmt.Errorf("Failed to get status: %v", err)
		return
	}
	vals := strings.Split(string(out), ",")
	if len(vals) < 4 {
		e = fmt.Errorf("Missing job properties for %s", params.JobId)
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
func HandleCancelJobRequest(conn utils.StdioConn, p any) (result any, e error) {
	params := p.(jobs.CancelJobRequest)
	_, err := conn.ExecCmd([]string{"job", "cancel", params.JobId})

	if err != nil {
		e = fmt.Errorf("Failed to cancel job: %v", err)
		return
	}

	result = jobs.CancelJobResponse{
		Success: true,
		JobId:   params.JobId,
	}
	return
}

// HandleSubmitJobRequest handles a SubmitJobRequest by invoking the `zowex job submit` command
func HandleSubmitJobRequest(conn utils.StdioConn, p any) (result any, e error) {
	params := p.(jobs.SubmitJobRequest)
	out, err := conn.ExecCmd([]string{"job", "submit", params.Dsname, "--only-jobid", "true"})

	if err != nil {
		e = fmt.Errorf("Failed to submit job: %v", err)
		return
	}

	result = jobs.SubmitJobResponse{
		Success: true,
		Dsname:  params.Dsname,
		JobId:   strings.TrimSpace(string(out)),
	}
	return
}

// HandleSubmitJclRequest handles a SubmitJclRequest by invoking the `zowex job submit-jcl` command
func HandleSubmitJclRequest(conn utils.StdioConn, p any) (result any, e error) {
	params := p.(jobs.SubmitJclRequest)
	decodedBytes, err := base64.StdEncoding.DecodeString(params.Jcl)
	if err != nil {
		e = fmt.Errorf("Failed to decode JCL contents: %v", err)
		return
	}

	cmd := utils.BuildCommandNoAutocvt([]string{"job", "submit-jcl", "--only-jobid", "true"})
	stdin, err := cmd.StdinPipe()
	if err != nil {
		e = fmt.Errorf("Failed to open stdin pipe to zowex: %v", err)
		return
	}

	go func() {
		defer stdin.Close()
		_, err = stdin.Write(decodedBytes)
		if err != nil {
			e = fmt.Errorf("Failed to write to pipe: %v", err)
		}
	}()

	out, err := cmd.Output()
	if err != nil {
		e = fmt.Errorf("Failed to submit JCL: %v", err)
		return
	}

	result = jobs.SubmitJclResponse{
		Success: true,
		JobId:   strings.TrimSpace(string(out)),
	}
	return
}

// HandleDeleteJobRequest handles a DeleteJobRequest by invoking the `zowex job delete` command
func HandleDeleteJobRequest(conn utils.StdioConn, p any) (result any, e error) {
	params := p.(jobs.DeleteJobRequest)
	_, err := conn.ExecCmd([]string{"job", "delete", params.JobId})
	if err != nil {
		e = fmt.Errorf("Failed to delete job %s: %v", params.JobId, err)
		return
	}

	result = jobs.DeleteJobResponse{
		Success: true,
		JobId:   params.JobId,
	}
	return
}
