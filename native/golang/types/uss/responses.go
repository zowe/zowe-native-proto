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

package uss

import common "zowe-native-proto/zowed/types/common"

type GenericFileResponse struct {
	common.CommandResponse `tstype:",extends"`
	// Whether the operation was successful
	Success bool `json:"success"`
	// Remote file path
	Path string `json:"fspath"`
}

type ReadFileResponse struct {
	common.CommandResponse `tstype:",extends"`
	// Returned encoding for the file
	Encoding string `json:"encoding,omitempty"`
	// Returned e-tag for the file
	Etag string `json:"etag"`
	// Remote file path
	Path string `json:"fspath"`
	// File contents (omitted if streaming)
	Data *[]byte `json:"data,omitempty" tstype:"B64String"`
}

type WriteFileResponse struct {
	GenericFileResponse `tstype:",extends"`
	Etag                string `json:"etag"`
	Created             string `json:"created"`
}

type ListFilesResponse struct {
	common.CommandResponse `tstype:",extends"`
	Items                  []common.UssItem `tstype:"common.UssItem[]" json:"items"`
	ReturnedRows           int              `json:"returnedRows"`
}

type CreateFileResponse = GenericFileResponse
type DeleteFileResponse = GenericFileResponse
type ChmodFileResponse = GenericFileResponse
type ChownFileResponse = GenericFileResponse
type ChtagFileResponse = GenericFileResponse
