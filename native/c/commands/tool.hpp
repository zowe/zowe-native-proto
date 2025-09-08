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

#include "../parser.hpp"

namespace tool
{
int handle_tool_convert_dsect(const parser::ParseResult &result);
int handle_tool_dynalloc(const parser::ParseResult &result);
int handle_tool_display_symbol(const parser::ParseResult &result);
int handle_tool_search(const parser::ParseResult &result);
int handle_tool_amblist(const parser::ParseResult &result);
int handle_tool_run(const parser::ParseResult &result);

void register_commands(parser::Command &root_command);
} // namespace tool