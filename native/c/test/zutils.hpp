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

#ifndef ZUTILS_HPP
#define ZUTILS_HPP
#include <string>
int execute_command_with_input(const std::string &command, const std::string &input, bool suppress_output = true);
int execute_command_with_output(const std::string &command, std::string &output);
std::string get_random_string(const int length, const bool allNumbers);
std::string get_random_uss(const std::string base_dir);
std::string parse_etag_from_output(const std::string &output);
#endif