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
	JobId string `json:"jobid"`
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
	DataType string `json:"dataType,omitempty"`
	Dataset  string `json:"dataset"`
	Contents string `json:"contents"`
}

// command: "writeFile"
type WriteFileRequest struct {
	Encoding string `json:"encoding,omitempty"`
	DataType string `json:"dataType,omitempty"`
	Path     string `json:"path"`
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
	Path     string `json:"path"`
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
