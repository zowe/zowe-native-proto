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

type ReadFileResponse struct {
	Encoding string `json:"encoding,omitempty"`
	Path     string `json:"fspath"`
	Data     []byte `json:"data" tstype:"Buffer | string"`
}

type WriteFileResponse struct {
	Success bool   `json:"success"`
	Path    string `json:"fspath"`
}
type ListFilesResponse struct {
	Items        []common.UssItem `tstype:"common.UssItem[]" json:"items"`
	ReturnedRows int              `json:"returnedRows"`
}
