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
	// Whether the new data was stored successfully
	Success bool `json:"success"`
	// Dataset name
	Dataset string `json:"dataset"`
}

type RestoreDatasetResponse struct {
	// Whether the dataset was restored successfully
	Success bool `json:"success"`
}

type ReadDatasetResponse struct {
	// Desired encoding for the dataset (optional)
	Encoding string `json:"encoding,omitempty"`
	// Dataset name
	Dataset string `json:"dataset"`
	// Dataset contents
	Data []byte `json:"data" tstype:"Buffer | string"`
}

type ListDatasetsResponse struct {
	// List of returned datasets
	Items []common.Dataset `tstype:"common.Dataset[]" json:"items"`
	// Number of rows returned
	ReturnedRows int `json:"returnedRows"`
}

type ListDsMembersResponse struct {
	// List of returned dataset members
	Items []common.DsMember `tstype:"common.DsMember[]" json:"items"`
	// Number of rows returned
	ReturnedRows int `json:"returnedRows"`
}

type DeleteDatasetResponse struct {
	// Whether the dataset was deleted successfully
	Success bool `json:"success"`
	// Dataset name
	Dsname string `json:"dsname"`
}
