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

#ifndef ZTSO_HPP
#define ZTSO_HPP

#include <iostream>
#include <vector>
#include <string>

int ztso_issue(std::string, std::string &);
int issue_command_combined_stdoutstderr(std::string, std::string &);
#endif
