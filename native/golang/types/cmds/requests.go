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

package cmds

type IssueConsoleRequest struct {
	Command     string `json:"command" tstype:"\"consoleCommand\""`
	CommandText string `json:"commandText"`
	ConsoleName string `json:"consoleName"`
}

type IssueTsoRequest struct {
	Command     string `json:"command" tstype:"\"tsoCommand\""`
	CommandText string `json:"commandText"`
}
type IssueUnixRequest struct {
	Command     string `json:"command" tstype:"\"unixCommand\""`
	CommandText string `json:"commandText"`
}
