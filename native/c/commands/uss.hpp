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

namespace uss
{
int handle_uss_create_file(const parser::ParseResult &result);
int handle_uss_create_dir(const parser::ParseResult &result);
int handle_uss_list(const parser::ParseResult &result);
int handle_uss_view(const parser::ParseResult &result);
int handle_uss_write(const parser::ParseResult &result);
int handle_uss_delete(const parser::ParseResult &result);
int handle_uss_chmod(const parser::ParseResult &result);
int handle_uss_chown(const parser::ParseResult &result);
int handle_uss_chtag(const parser::ParseResult &result);
void register_commands(parser::Command &root_command);
} // namespace uss