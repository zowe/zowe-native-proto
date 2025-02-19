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

package ds

import common "zowe-native-proto/ioserver/types/common"

type WriteDatasetResponse struct {
	Success bool   `json:"success"`
	Dataset string `json:"dataset"`
}

type RestoreDatasetResponse struct {
	Success bool `json:"success"`
}

type ReadDatasetResponse struct {
	Encoding string `json:"encoding,omitempty"`
	Dataset  string `json:"dataset"`
	Data     []byte `json:"data" tstype:"Buffer | string"`
}

type ListDatasetsResponse struct {
	Items        []common.Dataset `tstype:"common.Dataset[]" json:"items"`
	ReturnedRows int              `json:"returnedRows"`
}

type ListDsMembersResponse struct {
	Items        []common.DsMember `tstype:"common.DsMember[]" json:"items"`
	ReturnedRows int               `json:"returnedRows"`
}

type DeleteDatasetResponse struct {
	Success bool `json:"success"`
}

type CreateDatasetResponse struct {
	Success bool `json:"success"`
}
