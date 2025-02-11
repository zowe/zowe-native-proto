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

type CommandRequest struct {
	// Requested command to execute
	Command string `json:"command"`
}

type Dataset struct {
	// Dataset name
	Name string `json:"name"`
	// Dataset organization
	Dsorg string `json:"dsorg"`
	// Volume serial number
	Volser string `json:"volser"`
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
