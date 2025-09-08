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

namespace job
{
int handle_job_list(const parser::ParseResult &result);
int handle_job_list_files(const parser::ParseResult &result);
int handle_job_view_status(const parser::ParseResult &result);
int handle_job_view_file(const parser::ParseResult &result);
int handle_job_view_jcl(const parser::ParseResult &result);
int handle_job_submit(const parser::ParseResult &result);
int handle_job_submit_jcl(const parser::ParseResult &result);
int handle_job_submit_uss(const parser::ParseResult &result);
int handle_job_delete(const parser::ParseResult &result);
int handle_job_cancel(const parser::ParseResult &result);
int handle_job_hold(const parser::ParseResult &result);
int handle_job_release(const parser::ParseResult &result);
int job_submit_common(const parser::ParseResult &result, std::string jcl, std::string &jobid, std::string identifier);
void register_commands(parser::Command &root_command);
} // namespace job
