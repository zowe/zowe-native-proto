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

#ifndef ZCLI_HPP
#define ZCLI_HPP

#include "parser.hpp"

using namespace parser;
using namespace std;

// Data set command handlers
int handle_data_set_create(const ParseResult &result);
int handle_data_set_create_fb(const ParseResult &result);
int handle_data_set_create_vb(const ParseResult &result);
int handle_data_set_create_adata(const ParseResult &result);
int handle_data_set_create_loadlib(const ParseResult &result);
int handle_data_set_create_member(const ParseResult &result);
int handle_data_set_view(const ParseResult &result);
int handle_data_set_list(const ParseResult &result);
int handle_data_set_list_members(const ParseResult &result);
int handle_data_set_write(const ParseResult &result);
int handle_data_set_delete(const ParseResult &result);
int handle_data_set_restore(const ParseResult &result);
int handle_data_set_compress(const ParseResult &result);

// USS command handlers
int handle_uss_create_file(const ParseResult &result);
int handle_uss_create_dir(const ParseResult &result);
int handle_uss_list(const ParseResult &result);
int handle_uss_view(const ParseResult &result);
int handle_uss_write(const ParseResult &result);
int handle_uss_delete(const ParseResult &result);
int handle_uss_chmod(const ParseResult &result);
int handle_uss_chown(const ParseResult &result);
int handle_uss_chtag(const ParseResult &result);

// Job command handlers
int handle_job_list(const ParseResult &result);
int handle_job_list_files(const ParseResult &result);
int handle_job_view_status(const ParseResult &result);
int handle_job_view_file(const ParseResult &result);
int handle_job_view_jcl(const ParseResult &result);
int handle_job_submit(const ParseResult &result);
int handle_job_submit_jcl(const ParseResult &result);
int handle_job_submit_uss(const ParseResult &result);
int handle_job_delete(const ParseResult &result);
int handle_job_cancel(const ParseResult &result);
int handle_job_hold(const ParseResult &result);
int handle_job_release(const ParseResult &result);

// Tool command handlers
int handle_tool_convert_dsect(const ParseResult &result);
int handle_tool_dynalloc(const ParseResult &result);
int handle_tool_display_symbol(const ParseResult &result);
int handle_tool_search(const ParseResult &result);
int handle_tool_amblist(const ParseResult &result);
int handle_tool_run(const ParseResult &result);

// Misc command handlers
int handle_console_issue(const ParseResult &result);
int handle_tso_issue(const ParseResult &result);

// Registration methods
void register_console(command_ptr root_cmd);
void register_ds(command_ptr root_cmd);
void register_job(command_ptr root_cmd);
void register_tso(command_ptr root_cmd);
void register_tool(command_ptr root_cmd);
void register_uss(command_ptr root_cmd);

// Common utility methods
int loop_dynalloc(vector<string> &list);
int free_dynalloc_dds(vector<string> &list);

#endif // ZCLI_HPP
