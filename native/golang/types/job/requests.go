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

package job

import common "zowe-native-proto/ioserver/types/common"

type ListJobsRequest struct {
	Command            string `json:"command" tstype:"\"listJobs\""`
	Owner              string `json:"owner,omitempty"`
	Prefix             string `json:"prefix,omitempty"`
	Status             string `json:"status,omitempty"`
	common.ListOptions `tstype:",extends"`
}

type ListSpoolsRequest struct {
	Command string `json:"command" tstype:"\"listSpools\""`
	JobId   string `json:"jobId"`
}

type ReadSpoolRequest struct {
	Command  string `json:"command" tstype:"\"readSpool\""`
	Encoding string `json:"encoding,omitempty"`
	DsnKey   int    `json:"spoolId"`
	JobId    string `json:"jobId"`
}

type GetJclRequest struct {
	Command string `json:"command" tstype:"\"getJcl\""`
	JobId   string `json:"jobId"`
}

type GetStatusRequest struct {
	Command string `json:"command" tstype:"\"getStatus\""`
	JobId   string `json:"jobId"`
}
