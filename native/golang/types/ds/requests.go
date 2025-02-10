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
	Command string `json:"command" tstype:"\"listDatasets\""`
	Pattern string `json:"pattern"`

	Attributes                bool `json:"attributes,omitempty"`
	common.ListOptions        `tstype:",extends"`
	common.ListDatasetOptions `tstype:",extends"`
}

type ListDsMembersRequest struct {
	Command                   string `json:"command" tstype:"\"listDsMembers\""`
	Dsname                    string `json:"dsname"`
	common.ListOptions        `tstype:",extends"`
	common.ListDatasetOptions `tstype:",extends"`
}

type ReadDatasetRequest struct {
	Command  string `json:"command" tstype:"\"readDataset\""`
	Encoding string `json:"encoding,omitempty"`
	Dsname   string `json:"dsname"`
}

type WriteDatasetRequest struct {
	Command  string `json:"command" tstype:"\"writeDataset\""`
	Encoding string `json:"encoding,omitempty"`
	Dsname   string `json:"dsname"`
	Data     string `json:"data"`
}

type DeleteDatasetRequest struct {
	Dsname string `json:"dsname"`
}

type RestoreDatasetRequest struct {
	Command string `json:"command" tstype:"\"restoreDataset\""`
	Dsname  string `json:"dsname"`
}
