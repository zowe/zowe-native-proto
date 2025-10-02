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

#include "commands.hpp"
#include "dispatcher.hpp"
#include "../c/commands/ds.hpp"
#include "../c/commands/job.hpp"
#include "../c/commands/uss.hpp"

void register_ds_commands(CommandDispatcher &dispatcher)
{
  // dispatcher.register_command("createDataset", ds::create_with_attributes);
  // dispatcher.register_command("createMember", ds::handle_data_set_create_member);
  // dispatcher.register_command("deleteDataset", ds::handle_data_set_delete);
  dispatcher.register_command("listDatasets", ds::handle_data_set_list,
                              {InputRename("pattern", "dsn"),
                               InputDefault("warn", false)});
  dispatcher.register_command("listDsMembers", ds::handle_data_set_list_members,
                              {InputRename("dsname", "dsn"),
                               InputDefault("warn", false)});
  dispatcher.register_command("readDataset", ds::handle_data_set_view,
                              {InputRename("dsname", "dsn"),
                               InputDefault("encoding", "IBM-1047"),
                               InputDefault("return-etag", true),
                               InputRename("volume", "volser"),
                               OutputStdout("data", true)});
  // dispatcher.register_command("restoreDataset", ds::handle_data_set_restore);
  dispatcher.register_command("writeDataset", ds::handle_data_set_write,
                              {InputRename("dsname", "dsn"),
                               InputStdin("data", true),
                               InputDefault("encoding", "IBM-1047"),
                               InputRename("volume", "volser")});
}

void register_job_commands(CommandDispatcher &dispatcher)
{
  // dispatcher.register_command("cancelJob", job::handle_job_cancel);
  // dispatcher.register_command("deleteJob", job::handle_job_delete);
  // dispatcher.register_command("getJcl", job::handle_job_view_jcl);
  // dispatcher.register_command("getJobStatus", job::handle_job_view_status);
  // dispatcher.register_command("holdJob", job::handle_job_hold);
  dispatcher.register_command("listJobs", job::handle_job_list,
                              {InputDefault("warn", false)});
  dispatcher.register_command("listSpools", job::handle_job_list_files,
                              {InputRename("job-id", "jobid")});
  dispatcher.register_command("readSpool", job::handle_job_view_file,
                              {InputRename("job-id", "jobid"),
                               InputRename("spool-id", "key"),
                               InputDefault("encoding", "IBM-1047"),
                               OutputStdout("data", true)});
  // dispatcher.register_command("releaseJob", job::handle_job_release);
  // dispatcher.register_command("submitJcl", job::handle_job_submit_jcl);
  // dispatcher.register_command("submitJob", job::handle_job_submit);
  // dispatcher.register_command("submitUss", job::handle_job_submit_uss);
}

void register_uss_commands(CommandDispatcher &dispatcher)
{
  // dispatcher.register_command("chmodFile", uss::handle_uss_chmod);
  // dispatcher.register_command("chownFile", uss::handle_uss_chown);
  // dispatcher.register_command("chtagFile", uss::handle_uss_chtag);
  // dispatcher.register_command("createFile", [](plugin::InvocationContext &context) -> int
  //                             {
  //   auto handler = context.get<bool>("isDir", false) ?
  //     uss::handle_uss_create_dir : uss::handle_uss_create_file;
  //   return handler(context); });
  // dispatcher.register_command("deleteFile", uss::handle_uss_delete);
  dispatcher.register_command("listFiles", uss::handle_uss_list,
                              {InputRename("fspath", "file-path"),
                               InputDefault("response-format-csv", true)});
  dispatcher.register_command("readFile", uss::handle_uss_view,
                              {InputRename("fspath", "file-path"),
                               InputDefault("encoding", "IBM-1047"),
                               OutputStdout("data", true)});
  dispatcher.register_command("writeFile", uss::handle_uss_write,
                              {InputRename("fspath", "file-path"),
                               InputStdin("data", true),
                               InputDefault("encoding", "IBM-1047")});
}

void register_cmd_commands(CommandDispatcher &dispatcher)
{
  // dispatcher.register_command("consoleCommand", console::handle_console_issue);
}
