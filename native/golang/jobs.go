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
	"strconv"
	"strings"
	t "zowe-native-proto/ioserver/types/common"
	"zowe-native-proto/ioserver/types/jobs"
	utils "zowe-native-proto/ioserver/utils"
)

// HandleListJobsRequest handles a ListJobsRequest by invoking the `zowex job list` command
func HandleListJobsRequest(jsonData []byte) {
	listRequest, err := utils.ParseCommandRequest[jobs.ListJobsRequest](jsonData)
	if err != nil {
		return
	}

	args := []string{"job", "list", "--rfc", "true"}
	if len(listRequest.Owner) != 0 {
		args = append(args, "--owner", listRequest.Owner)
	}

	out, err := utils.BuildCommand(args).Output()
	if err != nil {
		utils.PrintErrorResponse("Failed to list jobs: %s", err)
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

	utils.PrintCommandResponse(jobsResponse)
}

// HandleListSpoolsRequest handles a ListSpoolsRequest by invoking the `zowex job list-files` command
func HandleListSpoolsRequest(jsonData []byte) {
	listRequest, err := utils.ParseCommandRequest[jobs.ListSpoolsRequest](jsonData)
	if err != nil {
		return
	}

	args := []string{"job", "list-files", listRequest.JobId, "--rfc", "true"}

	out, err := utils.BuildCommand(args).Output()
	if err != nil {
		utils.PrintErrorResponse("Failed to list spools: %s", err)
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

	utils.PrintCommandResponse(response)
}

// HandleReadSpoolRequest handles a ReadSpoolRequest by invoking the `zowex job view-file` command
func HandleReadSpoolRequest(jsonData []byte) {
	request, err := utils.ParseCommandRequest[jobs.ReadSpoolRequest](jsonData)
	if err != nil {
		return
	}

	args := []string{"job", "view-file", request.JobId, strconv.Itoa(request.DsnKey)}
	hasEncoding := len(request.Encoding) != 0
	if hasEncoding {
		args = append(args, "--encoding", request.Encoding, "--rfb", "true")
	}
	out, err := utils.BuildCommand(args).Output()
	if err != nil {
		utils.PrintErrorResponse("Failed to read spool: %s", err)
		return
	}

	data := utils.CollectContentsAsBytes(string(out), hasEncoding)
	utils.PrintCommandResponse(jobs.ReadSpoolResponse{
		Encoding: request.Encoding,
		DsnKey:   request.DsnKey,
		JobId:    request.JobId,
		Data:     data,
	})
}

// HandleGetJclRequest handles a GetJclRequest by invoking the `zowex job view-jcl` command
func HandleGetJclRequest(jsonData []byte) {
	request, err := utils.ParseCommandRequest[jobs.GetJclRequest](jsonData)
	if err != nil {
		return
	}

	args := []string{"job", "view-jcl", request.JobId}
	out, err := utils.BuildCommand(args).Output()
	if err != nil {
		utils.PrintErrorResponse("Failed to get JCL: %s", err)
		return
	}

	utils.PrintCommandResponse(jobs.GetJclResponse{
		JobId: request.JobId,
		Data:  string(out),
	})
}

// HandleGetStatusRequest handles a GetStatusRequest by invoking the `zowex job view-status` command
func HandleGetStatusRequest(jsonData []byte) {
	request, err := utils.ParseCommandRequest[jobs.GetStatusRequest](jsonData)
	if err != nil {
		return
	}

	args := []string{"job", "view-status", request.JobId, "--rfc", "true"}
	out, err := utils.BuildCommand(args).Output()
	if err != nil {
		utils.PrintErrorResponse("Failed to get status: %s", err)
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

	utils.PrintCommandResponse(jobsResponse)
}

// HandleSubmitJobRequest handles a SubmitJobRequest by invoking the `zowex job submit` command
func HandleCancelJobRequest(jsonData []byte) {
	request, err := utils.ParseCommandRequest[jobs.CancelJobRequest](jsonData)
	if err != nil {
		return
	}

	_, err = utils.BuildCommand([]string{"job", "cancel", request.JobId}).Output()

	if err != nil {
		utils.PrintErrorResponse("Failed to cancel job: %s", err)
		return
	}

	utils.PrintCommandResponse(jobs.CancelJobResponse{
		Success: true,
		JobId:   request.JobId,
	})
}

// HandleSubmitJobRequest handles a SubmitJobRequest by invoking the `zowex job submit` command
func HandleSubmitJobRequest(jsonData []byte) {
	request, err := utils.ParseCommandRequest[jobs.SubmitJobRequest](jsonData)
	if err != nil {
		return
	}

	out, err := utils.BuildCommand([]string{"job", "submit", request.Dsname, "--only-jobid", "true"}).Output()

	if err != nil {
		utils.PrintErrorResponse("Failed to submit job: %s", err)
		return
	}

	utils.PrintCommandResponse(jobs.SubmitJobResponse{
		Success: true,
		Dsname:  request.Dsname,
		JobId:   strings.TrimSpace(string(out)),
	})
}

// HandleSubmitJclRequest handles a SubmitJclRequest by invoking the `zowex job submit-jcl` command
func HandleSubmitJclRequest(jsonData []byte) {
	request, err := utils.ParseCommandRequest[jobs.SubmitJclRequest](jsonData)
	if err != nil {
		return
	}

	decodedBytes, err := base64.StdEncoding.DecodeString(request.Jcl)
	if err != nil {
		utils.PrintErrorResponse("Failed to decode JCL contents: %s", err)
		return
	}

	cmd := utils.BuildCommandNoAutocvt([]string{"job", "submit-jcl", "--only-jobid", "true"})
	stdin, err := cmd.StdinPipe()
	if err != nil {
		utils.PrintErrorResponse("Failed to open stdin pipe to zowex: %s", err)
		return
	}

	go func() {
		defer stdin.Close()
		_, err = stdin.Write(decodedBytes)
		if err != nil {
			utils.PrintErrorResponse("Failed to write to pipe: %s", err)
		}
	}()

	out, err := cmd.Output()
	if err != nil {
		utils.PrintErrorResponse("Failed to submit JCL: %s", err)
		return
	}

	utils.PrintCommandResponse(jobs.SubmitJclResponse{
		Success: true,
		JobId:   strings.TrimSpace(string(out)),
	})
}

// HandleDeleteJobRequest handles a DeleteJobRequest by invoking the `zowex job delete` command
func HandleDeleteJobRequest(jsonData []byte) {
	request, err := utils.ParseCommandRequest[jobs.DeleteJobRequest](jsonData)
	if err != nil {
		return
	}

	_, err = utils.BuildCommand([]string{"job", "delete", request.JobId}).Output()
	if err != nil {
		utils.PrintErrorResponse("Failed to delete job %s: %s", request.JobId, err)
		return
	}

	utils.PrintCommandResponse(jobs.DeleteJobResponse{
		Success: true,
		JobId:   request.JobId,
	})
}
