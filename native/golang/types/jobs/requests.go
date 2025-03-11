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

package jobs

import common "zowe-native-proto/ioserver/types/common"

type ListJobsRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"listJobs\""`
	// Filter jobs by matching job owner (optional)
	Owner string `json:"owner,omitempty"`
	// Filter jobs by prefix (optional)
	Prefix string `json:"prefix,omitempty"`
	// Filter jobs by status (optional)
	Status             string `json:"status,omitempty"`
	common.ListOptions `tstype:",extends"`
}

type ListSpoolsRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"listSpools\""`
	// Job ID to list spools for
	JobId string `json:"jobId"`
}

type ReadSpoolRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"readSpool\""`
	// Desired encoding for the spool file (optional)
	Encoding string `json:"encoding,omitempty"`
	// Spool ID to read under the given job ID
	DsnKey int `json:"spoolId"`
	// Job ID with spools to read from
	JobId string `json:"jobId"`
}

type GetJclRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"getJcl\""`
	// Job ID to get JCL for
	JobId string `json:"jobId"`
}

type GetJobStatusRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"getJobStatus\""`
	// Job ID to get status for
	JobId string `json:"jobId"`
}

type SubmitJobRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"submitJob\""`
	// Dataset name w/ contents to submit as JCL
	Dsname string `json:"dsname"`
}

type SubmitJclRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"submitJcl\""`
	// Desired encoding for the spool file (optional)
	Encoding string `json:"encoding,omitempty"`
	// JCL contents to submit as a job
	Jcl string `json:"jcl"`
}

type CancelJobRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"cancelJob\""`
	// Job ID to cancel
	JobId string `json:"jobId"`
}

type DeleteJobRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"deleteJob\""`
	// Job ID to cancel
	JobId string `json:"jobId"`
}

type HoldJobRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"holdJob\""`
	// Job ID to hold
	JobId string `json:"jobId"`
}

type ReleaseJobRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"releaseJob\""`
	// Job ID to release
	JobId string `json:"jobId"`
}
