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

import common "zowe-native-proto/ioserver/types/common"

type GenericFileResponse struct {
	// Whether the operation was successful
	Success bool `json:"success"`
	// Remote file path
	Path string `json:"fspath"`
}

type ReadFileResponse struct {
	// Returned encoding for the file
	Encoding string `json:"encoding,omitempty"`
	// Remote file path
	Path string `json:"fspath"`
	// File contents
	Data []byte `json:"data" tstype:"Buffer | string"`
}

type WriteFileResponse = GenericFileResponse

type ListFilesResponse struct {
	Items        []common.UssItem `tstype:"common.UssItem[]" json:"items"`
	ReturnedRows int              `json:"returnedRows"`
}

type DeleteFileResponse = GenericFileResponse
type ChmodFileResponse = GenericFileResponse
type ChownFileResponse = GenericFileResponse
type ChtagFileResponse = GenericFileResponse
