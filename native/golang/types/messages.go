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

/* Requests */

type CommandRequest struct {
	Command string `json:"command"`
}

type ListDatasetsRequest struct {
	Command string `json:"command" tstype:"\"listDatasets\""`
	Pattern            string `json:"pattern"`
	Attributes         bool   `json:"attributes,omitempty"`
	ListOptions        `tstype:",extends"`
	ListDatasetOptions `tstype:",extends"`
}

type ListDsMembersRequest struct {
	Command string `json:"command" tstype:"\"listDsMembers\""`
	Dataset            string `json:"dataset"`
	ListOptions        `tstype:",extends"`
	ListDatasetOptions `tstype:",extends"`
}

type ListFilesRequest struct {
	Command string `json:"command" tstype:"\"listFiles\""`
	Path    string `json:"fspath"`
	ListOptions
}

type ListJobsRequest struct {
	Command string `json:"command" tstype:"\"listJobs\""`
	Owner       string `json:"owner,omitempty"`
	Prefix      string `json:"prefix,omitempty"`
	Status      string `json:"status,omitempty"`
	ListOptions `tstype:",extends"`
}

type ListSpoolsRequest struct {
	Command string `json:"command" tstype:"\"listSpools\""`
	JobId    string `json:"jobId"`
}

type ReadDatasetRequest struct {
	Command string `json:"command" tstype:"\"readDataset\""`
	Encoding string `json:"encoding,omitempty"`
	Dataset  string `json:"dataset"`
}

type ReadFileRequest struct {
	Command string `json:"command" tstype:"\"readFile\""`
	Encoding string `json:"encoding,omitempty"`
	Path     string `json:"path"`
}

type ReadSpoolRequest struct {
	Command string `json:"command" tstype:"\"readSpool\""`
	Encoding string `json:"encoding,omitempty"`
	DsnKey   int    `json:"dsnKey"`
	JobId    string `json:"jobId"`
}

type GetJclRequest struct {
	Command string `json:"command" tstype:"\"getJcl\""`
	JobId   string `json:"jobId"`
}

type WriteDatasetRequest struct {
	Command string `json:"command" tstype:"\"writeDataset\""`
	Encoding string `json:"encoding,omitempty"`
	Dataset  string `json:"dataset"`
	Contents string `json:"contents"`
}

type WriteFileRequest struct {
	Command string `json:"command" tstype:"\"writeFile\""`
	Encoding string `json:"encoding,omitempty"`
	Path     string `json:"path"`
	Contents string `json:"contents"`
}

/* Responses */

type ReadDatasetResponse struct {
	Encoding string `json:"encoding,omitempty"`
	Dataset  string `json:"dataset"`
	Data     []byte `json:"data" tstype:"Buffer | string"`
}

type ReadFileResponse struct {
	Encoding string `json:"encoding,omitempty"`
	Path     string `json:"path"`
	Data     []byte `json:"data" tstype:"Buffer | string"`
}

type ReadSpoolResponse struct {
	Encoding string `json:"encoding,omitempty"`
	DsnKey   int    `json:"dsnKey"`
	JobId    string `json:"jobId"`
	Data     []byte `json:"data" tstype:"Buffer | string"`
}

type WriteDatasetResponse struct {
	Success bool   `json:"success"`
	Dataset string `json:"dataset"`
}

type WriteFileResponse struct {
	Success bool   `json:"success"`
	Path    string `json:"path"`
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

type IssueConsoleRequest struct {
	Command string `json:"command" tstype:"\"consoleCommand\""`
	CommandText string `json:"commandText"`
	ConsoleName string `json:"consoleName"`
}

type IssueConsoleResponse struct {
	Data string `json:"data"`
}

type RestoreDatasetRequest struct {
	Command string `json:"command" tstype:"\"restoreDataset\""`
	Dataset string `json:"dataset"`
}

type RestoreDatasetResponse struct {
	Success bool `json:"success"`
}
