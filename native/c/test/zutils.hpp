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
#include <vector>
#include "../zjson.hpp"
const std::string zowex_command = "./../build-out/zowex";
const std::string zoweax_command = "./../build-out/zoweax";
int execute_command_with_input(const std::string &command, const std::string &input, bool suppress_output = true);
int execute_command_with_output(const std::string &command, std::string &output);
std::string get_random_string(const int length = 7, const bool allNumbers = true);
std::string get_random_uss(const std::string base_dir);
std::string get_random_ds(const int qualifier_count = 4, const std::string hlq = "");
std::string get_user();
std::string parse_etag_from_output(const std::string &output);
std::vector<std::string> parse_rfc_response(const std::string input, const char *delim = ",");
#endif