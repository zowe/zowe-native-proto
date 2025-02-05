package ds

import common "zowe-native-proto/ioserver/types/common"

type WriteDatasetResponse struct {
	Success bool   `json:"success"`
	Dataset string `json:"dataset"`
}

type RestoreDatasetResponse struct {
	Success bool `json:"success"`
}

type ReadDatasetResponse struct {
	Encoding string `json:"encoding,omitempty"`
	Dataset  string `json:"dataset"`
	Data     []byte `json:"data" tstype:"Buffer | string"`
}

type ListDatasetsResponse struct {
	Items        []common.Dataset `tstype:"common.Dataset[]" json:"items"`
	ReturnedRows int              `json:"returnedRows"`
}

type ListDsMembersResponse struct {
	Items        []common.DsMember `tstype:"common.DsMember[]" json:"items"`
	ReturnedRows int               `json:"returnedRows"`
}
