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

package main

/* Requests */

type CommandRequest struct {
	Command string `json:"command"`
}

// command: "listDatasets"
type ListDatasetsRequest struct {
	Pattern    string `json:"pattern"`
	Attributes bool   `json:"attributes,omitempty"`
	ListOptions
	ListDatasetOptions
}

// command: "listDsMembers"
type ListDsMembersRequest struct {
	Dataset string `json:"dataset"`
	ListOptions
	ListDatasetOptions
}

// command: "listFiles"
type ListFilesRequest struct {
	Path string `json:"fspath"`
	ListOptions
}

// command: "listJobs"
type ListJobsRequest struct {
	Owner  string `json:"owner,omitempty"`
	Prefix string `json:"prefix,omitempty"`
	Status string `json:"status,omitempty"`
	ListOptions
}

// command: "listSpools"
type ListSpoolsRequest struct {
	JobId string `json:"jobId"`
}

// command: "readDataset"
type ReadDatasetRequest struct {
	Encoding string `json:"encoding,omitempty"`
	Dataset  string `json:"dataset"`
}

// command: "readFile"
type ReadFileRequest struct {
	Encoding string `json:"encoding,omitempty"`
	Path     string `json:"path"`
}

// command: "readSpool"
type ReadSpoolRequest struct {
	Encoding string `json:"encoding,omitempty"`
	DsnKey   int    `json:"dsnKey"`
	JobId    string `json:"jobId"`
}

// command: "getJcl"
type GetJclRequest struct {
	JobId string `json:"jobId"`
}

// command: "writeDataset"
type WriteDatasetRequest struct {
	Encoding string `json:"encoding,omitempty"`
	Dataset  string `json:"dataset"`
	Contents string `json:"contents"`
}

/* Responses */

type ReadDatasetResponse struct {
	Encoding string `json:"encoding,omitempty"`
	Dataset  string `json:"dataset"`
	Data     []byte `json:"data"`
}

type ReadFileResponse struct {
	Encoding string `json:"encoding,omitempty"`
	File     string `json:"file"`
	Data     []byte `json:"data"`
}

type ReadSpoolResponse struct {
	Encoding string `json:"encoding,omitempty"`
	DsnKey   int    `json:"dsnKey"`
	JobId    string `json:"jobId"`
	Data     []byte `json:"data"`
}

type WriteDatasetResponse struct {
	Success bool   `json:"success"`
	Dataset string `json:"dataset"`
}

type ListDatasetsResponse struct {
	Items        []Dataset `json:"items"`
	ReturnedRows int       `json:"returnedRows"`
}

type ListDsMembersResponse struct {
	Items        []DsMember `json:"items"`
	ReturnedRows int        `json:"returnedRows"`
}

type ListFilesResponse struct {
	Items        []UssItem `json:"items"`
	ReturnedRows int       `json:"returnedRows"`
}

type ListJobsResponse struct {
	Items []Job `json:"items"`
}

type ListSpoolsResponse struct {
	Items []Spool `json:"items"`
}

type GetJclResponse struct {
	JobId string `json:"jobId"`
	Data  string `json:"data"`
}

// command: "consoleCommand"
type IssueConsoleRequest struct {
	Command string `json:"commandText"`
	Console string `json:"consoleName"`
}

type IssueConsoleResponse struct {
	Data string `json:"data"`
}
