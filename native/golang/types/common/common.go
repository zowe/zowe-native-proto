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

package types

import "encoding/json"

type RpcNotification struct {
	JsonRPC string         `json:"jsonrpc" tstype:"\"2.0\""`
	Method  string         `json:"method"`
	Params  map[string]any `json:"params,omitempty" tstype:"any"`
}

type RpcRequest struct {
	RpcNotification `tstype:",extends"`
	Params          json.RawMessage `json:"params,omitempty" tstype:"any"`
	Id              int             `json:"id"`
}

type RpcResponse struct {
	JsonRPC string        `json:"jsonrpc" tstype:"\"2.0\""`
	Result  any           `json:"result,omitempty"`
	Error   *ErrorDetails `json:"error,omitempty"`
	Id      *int          `json:"id,omitempty" tstype:",required"`
}

type CommandRequest struct {
	// Requested command to execute
	Command string `json:"command"`
}

type CommandResponse struct {
	// True if command succeeded
	Success *bool `json:"success,omitempty" tstype:",required"`
}

type ErrorDetails struct {
	Code    int    `json:"code"`
	Message string `json:"message"`
	Data    any    `json:"data,omitempty"`
}

type Dataset struct {
	// Dataset name
	Name string `json:"name"`
	// Dataset organization
	Dsorg string `json:"dsorg"`
	// Volume serial number
	Volser string `json:"volser"`
	// Dataset migrated
	Migr bool `json:"migr"`
}

type DsMember struct {
	// Dataset member name
	Name string `json:"name"`
}

type UssItem struct {
	// File name
	Name string `json:"name"`
	// Whether the item is a directory
	IsDir bool `json:"isDir"`
}

type Job struct {
	// Job ID
	Id string `json:"id"`
	// Job name
	Name string `json:"name"`
	// Job status
	Status string `json:"status"`
	// Job return code
	Retcode string `json:"retcode"`
}

type Spool struct {
	// Spool ID
	Id int `json:"id"`
	// DD name
	DdName string `json:"ddname"`
	// Step name in the job
	StepName string `json:"stepname"`
	// Dataset name
	DsName string `json:"dsname"`
	// Procedure name for the step
	ProcStep string `json:"procstep"`
}

// StatusMessage represents the status of `zowed` and serves as a health check
type StatusMessage struct {
	Status  string         `json:"status"`
	Message string         `json:"message"`
	Data    map[string]any `json:"data,omitempty"`
}
