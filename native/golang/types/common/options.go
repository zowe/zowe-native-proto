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

package types

type ListOptions struct {
	// Maximum number of items to return
	MaxItems int `json:"maxItems,omitempty"`
	// Response timeout in seconds
	ResponseTimeout int `json:"responseTimeout,omitempty"`
}

type ListDatasetOptions struct {
	// Skip data sets that come before this data set name
	Start string `json:"start,omitempty"`
}
