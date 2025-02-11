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
	"encoding/json"
	"log"
	"strconv"
	"strings"
	t "zowe-native-proto/ioserver/types/common"
	"zowe-native-proto/ioserver/types/jobs"
	utils "zowe-native-proto/ioserver/utils"
)

// HandleListJobsRequest handles a ListJobsRequest by invoking the `zowex job list` command
func HandleListJobsRequest(jsonData []byte) {
	var listRequest jobs.ListJobsRequest
	err := json.Unmarshal(jsonData, &listRequest)
	if err != nil {
		// log.Println("Error decoding ListJobsRequest:", err)
		return
	}

	args := []string{"./zowex", "job", "list", "--rfc", "true"}
	if len(listRequest.Owner) != 0 {
		args = append(args, "--owner", listRequest.Owner)
	}

	out, err := utils.BuildCommand(args).Output()
	if err != nil {
		log.Println("Error executing command:", err)
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
	var listRequest jobs.ListSpoolsRequest
	err := json.Unmarshal(jsonData, &listRequest)
	if err != nil {
		// log.Println("Error decoding ListSpoolsRequest:", err)
		return
	}

	args := []string{"./zowex", "job", "list-files", listRequest.JobId, "--rfc", "true"}

	out, err := utils.BuildCommand(args).Output()
	if err != nil {
		log.Println("Error executing command:", err)
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
	var request jobs.ReadSpoolRequest
	err := json.Unmarshal(jsonData, &request)
	if err != nil {
		// log.Println("Error decoding ReadSpoolRequest:", err)
		return
	}
	// log.Println("ReadSpoolRequest received:", ...)
	args := []string{"./zowex", "job", "view-file", request.JobId, strconv.Itoa(request.DsnKey)}
	hasEncoding := len(request.Encoding) != 0
	if hasEncoding {
		args = append(args, "--encoding", request.Encoding, "--rfb", "true")
	}
	out, err := utils.BuildCommand(args).Output()
	if err != nil {
		log.Println("Error executing command:", err)
		return
	}

	data := utils.CollectContentsAsBytes(string(out), hasEncoding)

	response := jobs.ReadSpoolResponse{
		Encoding: request.Encoding,
		DsnKey:   request.DsnKey,
		JobId:    request.JobId,
		Data:     data,
	}
	utils.PrintCommandResponse(response)
}

// HandleGetJclRequest handles a GetJclRequest by invoking the `zowex job view-jcl` command
func HandleGetJclRequest(jsonData []byte) {
	var request jobs.GetJclRequest
	err := json.Unmarshal(jsonData, &request)
	if err != nil {
		// log.Println("Error decoding GetJclRequest:", err)
		return
	}
	// log.Println("GetJclRequest received:", ...)
	args := []string{"./zowex", "job", "view-jcl", request.JobId}
	out, err := utils.BuildCommand(args).Output()
	if err != nil {
		log.Println("Error executing command:", err)
		return
	}

	response := jobs.GetJclResponse{
		JobId: request.JobId,
		Data:  string(out),
	}
	utils.PrintCommandResponse(response)
}

// HandleGetStatusRequest handles a GetStatusRequest by invoking the `zowex job view-status` command
func HandleGetStatusRequest(jsonData []byte) {
	var request jobs.GetStatusRequest
	err := json.Unmarshal(jsonData, &request)
	if err != nil {
		return
	}
	args := []string{"./zowex", "job", "view-status", request.JobId, "--rfc", "true"}
	out, err := utils.BuildCommand(args).Output()
	if err != nil {
		log.Println("Error executing command:", err)
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
