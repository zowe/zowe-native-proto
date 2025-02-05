package uss

import common "zowe-native-proto/ioserver/types/common"

type ListFilesRequest struct {
	Command            string `json:"command" tstype:"\"listFiles\""`
	Path               string `json:"fspath"`
	common.ListOptions `tstype:",extends"`
}

type ReadFileRequest struct {
	Command  string `json:"command" tstype:"\"readFile\""`
	Encoding string `json:"encoding,omitempty"`
	Path     string `json:"path"`
}

type WriteFileRequest struct {
	Command  string `json:"command" tstype:"\"writeFile\""`
	Encoding string `json:"encoding,omitempty"`
	Path     string `json:"path"`
	Contents string `json:"contents"`
}
