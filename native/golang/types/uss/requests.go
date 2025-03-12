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

type ListFilesRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"listFiles\""`
	// Directory to list files for
	Path               string `json:"fspath"`
	common.ListOptions `tstype:",extends"`
}

type ReadFileRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"readFile\""`
	// Desired encoding for the file (optional)
	Encoding string `json:"encoding,omitempty"`
	// Remote file path to read contents from
	Path string `json:"fspath"`
}

type WriteFileRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"writeFile\""`
	// Desired encoding for the file (optional)
	Encoding string `json:"encoding,omitempty"`
	// Remote file path to write contents to
	Path string `json:"fspath"`
	// New contents for the file
	Data string `json:"data"`
}

type CreateFileRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"createFile\""`
	// Permissions for the new path
	Mode string `json:"permissions,omitempty"`
	// Remote file path to create
	Path string `json:"fspath"`
	// Whether to create a directory (true) or a file (false)
	IsDir bool `json:"isDir,omitempty"`
}

type DeleteFileRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"deleteFile\""`
	// Remote file path to delete
	Path string `json:"fspath"`
	// Whether to delete the file recursively
	Recursive bool `json:"recursive"`
}

type ChmodFileRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"chmodFile\""`
	// Desired permissions for the file (represented as an octal value, e.g. "755")
	Mode string `json:"mode"`
	// Remote file path to change permissions for
	Path string `json:"fspath"`
	// Whether to change permissions recursively
	Recursive bool `json:"recursive"`
}

type ChownFileRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"chownFile\""`
	// New owner for the file
	Owner string `json:"owner"`
	// Remote file path to change ownership for
	Path string `json:"fspath"`
	// Whether to apply ownership to inner files and directories
	Recursive bool `json:"recursive"`
}

type ChtagFileRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"chtagFile\""`
	// Remote file path to change tags for
	Path string `json:"fspath"`
	// New tag for the file
	Tag string `json:"tag"`
	// Whether to apply the tag to inner files and directories
	Recursive bool `json:"recursive"`
}
