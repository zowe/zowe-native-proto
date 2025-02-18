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

type ListJobsResponse struct {
	// List of returned jobs
	Items []common.Job `json:"items" tstype:"common.Job[]"`
}

type ListSpoolsResponse struct {
	// List of returned spools
	Items []common.Spool `json:"items" tstype:"common.Spool[]"`
}

type GetJclResponse struct {
	// Job ID for the returned JCL
	JobId string `json:"jobId"`
	// JCL contents
	Data string `json:"data"`
}

type ReadSpoolResponse struct {
	// Desired encoding for the spool file (optional)
	Encoding string `json:"encoding,omitempty"`
	// Spool ID matching the returned spool contents
	DsnKey int `json:"spoolId"`
	// Job ID associated with the returned spool
	JobId string `json:"jobId"`
	// Spool contents
	Data []byte `json:"data" tstype:"Buffer | string"`
}

type GetStatusResponse struct {
	// List of returned job statuses
	Items []common.Job `json:"items" tstype:"common.Job[]"`
}

type SubmitJobResponse struct {
	// Whether the job was successfully submitted
	Success bool `json:"success"`
	// The data set name where the JCL was read from
	Dsname string `json:"dsname"`
}

type DeleteJobResponse struct {
	// Whether the job was successfully deleted
	Success bool `json:"success"`
	// The ID for the job that was deletede
	JobId string `json:"jobId"`
}

type CancelJobResponse struct {
	// Whether the job was successfully cancelled
	Success bool `json:"success"`
	// The ID for the job that was cancelled
	JobId string `json:"jobId"`
}
