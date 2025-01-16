package main

// Requests

type CommandRequest struct {
	Command string `json:"command"`
}

// command: "listDataset"
type ListDatasetRequest struct {
	Pattern string `json:"pattern"`
	ListOptions
	ListDatasetOptions
}

// command: "listUss"
type ListUssRequest struct {
	Path string `json:"fspath"`
	ListOptions
}

// command: "listJobs"
type ListJobsRequest struct {
	Owner  string `json:"owner"`
	Prefix string `json:"prefix"`
	Status string `json:"status"`
	ListOptions
}

// command: "readDataset"
type ReadDatasetRequest struct {
	Encoding string `json:"encoding,omitempty"`
	Dataset  string `json:"dataset"`
}

// command: "readUss"
type ReadUssRequest struct {
	Encoding string `json:"encoding,omitempty"`
	Path     string `json:"path"`
}

// Responses

type ReadDatasetResponse struct {
	Encoding string `json:"encoding,omitempty"`
	Dataset  string `json:"dataset"`
	Data     []byte `json:"data"`
}

type ReadUssResponse struct {
	Encoding string `json:"encoding,omitempty"`
	File     string `json:"file"`
	Data     []byte `json:"data"`
}

type Dataset struct {
	Name  string `json:"name"`
	Dsorg string `json:"dsorg"`
}

type ListDatasetResponse struct {
	Items        []Dataset `json:"items"`
	ReturnedRows int       `json:"returnedRows"`
}

type UssItem struct {
	Name string `json:"name"`
}

type ListUssResponse struct {
	Items        []UssItem `json:"items"`
	ReturnedRows int       `json:"returnedRows"`
}
