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

package utils

import "strings"

func YamlToMap(yaml string) map[string]string {
	res := make(map[string]string)
	lines := strings.Split(yaml, "\n")

	for _, line := range lines {
		tokens := strings.Split(strings.TrimSpace(line), ":")
		if len(tokens) < 2 {
			continue
		}
		res[strings.TrimSpace(tokens[0])] = strings.TrimSpace(tokens[1])
	}

	return res
}
