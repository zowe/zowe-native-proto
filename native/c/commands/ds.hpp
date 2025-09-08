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

namespace ds
{
int create_with_attributes(const parser::ParseResult &result);
int handle_data_set_create_fb(const parser::ParseResult &result);
int handle_data_set_create_vb(const parser::ParseResult &result);
int handle_data_set_create_adata(const parser::ParseResult &result);
int handle_data_set_create_loadlib(const parser::ParseResult &result);
int handle_data_set_view(const parser::ParseResult &result);
int handle_data_set_list(const parser::ParseResult &result);
int handle_data_set_list_members(const parser::ParseResult &result);
int handle_data_set_write(const parser::ParseResult &result);
int handle_data_set_delete(const parser::ParseResult &result);
int handle_data_set_restore(const parser::ParseResult &result);
int handle_data_set_compress(const parser::ParseResult &result);
int handle_data_set_create_member(const parser::ParseResult &result);
void register_commands(parser::Command &root_command);
} // namespace ds