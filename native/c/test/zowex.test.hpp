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

#ifndef ZOWEX_TEST_HPP
#define ZOWEX_TEST_HPP
#include <string>
void zowex_tests();
int execute_command_with_input(const std::string &command, const std::string &input);
int execute_command_with_output(const std::string &command, std::string &output);
std::string get_random_string(const int length, const bool allNumbers);
std::string get_random_uss(const std::string base_dir);
#endif