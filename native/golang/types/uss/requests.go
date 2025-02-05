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

type ListFilesRequest struct {
	Command            string `json:"command" tstype:"\"listFiles\""`
	Path               string `json:"fspath"`
	common.ListOptions `tstype:",extends"`
}

type ReadFileRequest struct {
	Command  string `json:"command" tstype:"\"readFile\""`
	Encoding string `json:"encoding,omitempty"`
	Path     string `json:"path"`
}

type WriteFileRequest struct {
	Command  string `json:"command" tstype:"\"writeFile\""`
	Encoding string `json:"encoding,omitempty"`
	Path     string `json:"path"`
	Contents string `json:"contents"`
}
