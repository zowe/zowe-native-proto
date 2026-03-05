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

#ifndef COMMANDS_SERVER_HPP
#define COMMANDS_SERVER_HPP

#include <string>
#include "../parser.hpp"

namespace server
{
void set_exec_dir(const std::string &dir);
void register_commands(parser::Command &root_command);
} // namespace server

#endif
