/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0 & Apache-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

package main

type ListOptions struct {
	MaxItems        int `json:"maxItems,omitempty"`
	ResponseTimeout int `json:"responseTimeout,omitempty"`
}

type ListDatasetOptions struct {
	Start string `json:"start"`
}
