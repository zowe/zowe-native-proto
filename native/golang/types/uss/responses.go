package uss

import common "zowe-native-proto/ioserver/types/common"

type ReadFileResponse struct {
	Encoding string `json:"encoding,omitempty"`
	Path     string `json:"path"`
	Data     []byte `json:"data" tstype:"Buffer | string"`
}

type WriteFileResponse struct {
	Success bool   `json:"success"`
	Path    string `json:"path"`
}
type ListFilesResponse struct {
	Items        []common.UssItem `json:"items"`
	ReturnedRows int              `json:"returnedRows"`
}
