package ds

import common "zowe-native-proto/ioserver/types/common"

type ListDatasetsRequest struct {
	Command string `json:"command" tstype:"\"listDatasets\""`
	Pattern string `json:"pattern"`

	Attributes                bool `json:"attributes,omitempty"`
	common.ListOptions        `tstype:",extends"`
	common.ListDatasetOptions `tstype:",extends"`
}

type ListDsMembersRequest struct {
	Command                   string `json:"command" tstype:"\"listDsMembers\""`
	Dataset                   string `json:"dataset"`
	common.ListOptions        `tstype:",extends"`
	common.ListDatasetOptions `tstype:",extends"`
}

type ReadDatasetRequest struct {
	Command  string `json:"command" tstype:"\"readDataset\""`
	Encoding string `json:"encoding,omitempty"`
	Dataset  string `json:"dataset"`
}

type WriteDatasetRequest struct {
	Command  string `json:"command" tstype:"\"writeDataset\""`
	Encoding string `json:"encoding,omitempty"`
	Dataset  string `json:"dataset"`
	Contents string `json:"contents"`
}

type RestoreDatasetRequest struct {
	Command string `json:"command" tstype:"\"restoreDataset\""`
	Dataset string `json:"dataset"`
}
