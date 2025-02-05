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

type ListJobsResponse struct {
	Items []common.Job `json:"items" tstype:"common.Job[]"`
}

type ListSpoolsResponse struct {
	Items []common.Spool `json:"items" tstype:"common.Spool[]"`
}

type GetJclResponse struct {
	JobId string `json:"jobId"`
	Data  string `json:"data"`
}

type ReadSpoolResponse struct {
	Encoding string `json:"encoding,omitempty"`
	DsnKey   int    `json:"spoolId"`
	JobId    string `json:"jobId"`
	Data     []byte `json:"data" tstype:"Buffer | string"`
}

type GetStatusResponse struct {
	Items []common.Job `json:"items" tstype:"common.Job[]"`
}
