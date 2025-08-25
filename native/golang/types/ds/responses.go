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

import common "zowe-native-proto/zowed/types/common"

type WriteDatasetResponse struct {
	common.CommandResponse `tstype:",extends"`
	// Whether the new data was stored successfully
	Success bool `json:"success"`
	// Dataset name
	Dataset string `json:"dataset"`
	// Returned e-tag for the data set
	Etag string `json:"etag"`
	// Length of dataset contents in bytes (only used for streaming)
	ContentLen *int `json:"contentLen,omitempty"`
}

type RestoreDatasetResponse struct {
	common.CommandResponse `tstype:",extends"`
	// Whether the dataset was restored successfully
	Success bool `json:"success"`
}

type ReadDatasetResponse struct {
	common.CommandResponse `tstype:",extends"`
	// Desired encoding for the dataset (optional)
	Encoding string `json:"encoding,omitempty"`
	// Returned e-tag for the data set
	Etag string `json:"etag"`
	// Dataset name
	Dataset string `json:"dataset"`
	// Dataset contents (omitted if streaming)
	Data *[]byte `json:"data" tstype:"B64String,required"`
	// Length of dataset contents in bytes (only used for streaming)
	ContentLen *int `json:"contentLen,omitempty"`
}

type ListDatasetsResponse struct {
	common.CommandResponse `tstype:",extends"`
	// List of returned datasets
	Items []common.Dataset `tstype:"common.Dataset[]" json:"items"`
	// Number of rows returned
	ReturnedRows int `json:"returnedRows"`
}

type ListDsMembersResponse struct {
	common.CommandResponse `tstype:",extends"`
	// List of returned dataset members
	Items []common.DsMember `tstype:"common.DsMember[]" json:"items"`
	// Number of rows returned
	ReturnedRows int `json:"returnedRows"`
}

type GenericDatasetResponse struct {
	common.CommandResponse `tstype:",extends"`
	// Whether the dataset operation was successful
	Success bool `json:"success"`
	// Dataset name
	Dsname string `json:"dsname"`
}

type CreateDatasetResponse = GenericDatasetResponse
type CreateMemberResponse = GenericDatasetResponse
type DeleteDatasetResponse = GenericDatasetResponse
