package job

import common "zowe-native-proto/ioserver/types/common"

type ListJobsResponse struct {
	Items []common.Job `json:"items"`
}

type ListSpoolsResponse struct {
	Items []common.Spool `json:"items"`
}

type GetJclResponse struct {
	JobId string `json:"jobId"`
	Data  string `json:"data"`
}

type ReadSpoolResponse struct {
	Encoding string `json:"encoding,omitempty"`
	DsnKey   int    `json:"dsnKey"`
	JobId    string `json:"jobId"`
	Data     []byte `json:"data" tstype:"Buffer | string"`
}
