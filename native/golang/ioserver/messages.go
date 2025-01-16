package main

// Requests

type CommandRequest struct {
	Command string `json:"command"`
}

type ReadDatasetRequest struct {
	Encoding string `json:"encoding"`
	Dataset  string `json:"dataset"`
}

type ReadUssRequest struct {
	Encoding string `json:"encoding"`
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
