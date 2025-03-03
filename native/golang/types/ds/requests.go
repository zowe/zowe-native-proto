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

import common "zowe-native-proto/ioserver/types/common"

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
}

type WriteDatasetRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"writeDataset\""`
	// Desired encoding for the dataset (optional)
	Encoding string `json:"encoding,omitempty"`
	// Dataset name
	Dsname string `json:"dsname"`
	// Dataset contents
	Data string `json:"data"`
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

// default: DSORG=PO, RECFM=FB, LRECL=80
// vb: DSORG=PO, RECFM=VB, LRECL=255
// adata: DSORG=PO, RECFM=VB, LRECL=32756
type CreateDatasetRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"createDataset\""`
	// Dataset name
	Dsname string `json:"dsname"`
	// Type of the dataset to make
	Type string `json:"dstype" tstype:"'default' | 'vb' | 'adata'"`
}

type CreateMemberRequest struct {
	common.CommandRequest `tstype:",extends"`
	Command               string `json:"command" tstype:"\"createMember\""`
	// Dataset name
	Dsname string `json:"dsname"`
}
