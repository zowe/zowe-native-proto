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

package ds

import common "zowe-native-proto/zowed/types/common"

type ListDatasetsRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"listDatasets\""`
	// Pattern to match against dataset names
	Pattern string `json:"pattern"`
	// Whether to include attributes in the response
	Attributes                bool `json:"attributes,omitempty"`
	common.ListOptions        `tstype:",extends"`
	common.ListDatasetOptions `tstype:",extends"`
}

type ListDsMembersRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"listDsMembers\""`
	// Dataset name
	Dsname string `json:"dsname"`
	// Whether to include attributes in the response
	Attributes                bool `json:"attributes,omitempty"`
	common.ListOptions        `tstype:",extends"`
	common.ListDatasetOptions `tstype:",extends"`
}

type ReadDatasetRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"readDataset\""`
	// Desired encoding for the dataset (optional)
	Encoding string `json:"encoding,omitempty"`
	// Dataset name
	Dsname string `json:"dsname"`
	// Stream to write contents to
	StreamId int `json:"stream,omitempty" tstype:"Writable"`
}

type WriteDatasetRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"writeDataset\""`
	// Desired encoding for the dataset (optional)
	Encoding string `json:"encoding,omitempty"`
	// Last e-tag for the data set (optional, omit to overwrite)
	Etag string `json:"etag,omitempty"`
	// Dataset name
	Dsname string `json:"dsname"`
	// Dataset contents
	Data string `json:"data" tstype:"B64String"`
	// Stream to read contents from
	StreamId int `json:"stream,omitempty" tstype:"Readable"`
}

type DeleteDatasetRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"deleteDataset\""`
	// Dataset name
	Dsname string `json:"dsname"`
}

type RestoreDatasetRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"restoreDataset\""`
	// Dataset name
	Dsname string `json:"dsname"`
}

type CreateDatasetRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"createDataset\""`
	// Dataset name
	Dsname string `json:"dsname"`

	// Dataset attributes
	Attributes common.DatasetAttributes `json:"attributes" tstype:"common.DatasetAttributes"`
}

type CreateMemberRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"createMember\""`
	// Dataset name
	Dsname string `json:"dsname"`
}
