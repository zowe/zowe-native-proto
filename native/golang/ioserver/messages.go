package main

/* Requests */

type CommandRequest struct {
	Command string `json:"command"`
}

// command: "listDatasets"
type ListDatasetsRequest struct {
	Pattern string `json:"pattern"`
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
