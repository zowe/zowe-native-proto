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

type Dataset struct {
	Name   string `json:"name"`
	Dsorg  string `json:"dsorg"`
	Volser string `json:"volser"`
}

type DsMember struct {
	Name string `json:"name"`
}

type UssItem struct {
	Name  string `json:"name"`
	IsDir bool   `json:"isDir"`
}

type Job struct {
	Id      string `json:"id"`
	Name    string `json:"name"`
	Status  string `json:"status"`
	Retcode string `json:"retcode"`
}

type Spool struct {
	Id       int    `json:"id"`
	DdName   string `json:"ddname"`
	StepName string `json:"stepname"`
	DsName   string `json:"dsname"`
	ProcStep string `json:"procstep"`
}
