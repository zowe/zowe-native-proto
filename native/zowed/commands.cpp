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
  // dispatcher.register_command("createDataset", CommandBuilder(ds::create_with_attributes));
  // dispatcher.register_command("createMember", CommandBuilder(ds::handle_data_set_create_member));
  // dispatcher.register_command("deleteDataset", CommandBuilder(ds::handle_data_set_delete));
  dispatcher.register_command("listDatasets",
                              CommandBuilder(ds::handle_data_set_list)
                                  .rename_arg("pattern", "dsn")
                                  .set_default("warn", false));
  dispatcher.register_command("listDsMembers",
                              CommandBuilder(ds::handle_data_set_list_members)
                                  .rename_arg("dsname", "dsn")
                                  .set_default("warn", false));
  dispatcher.register_command("readDataset",
                              CommandBuilder(ds::handle_data_set_view)
                                  .rename_arg("dsname", "dsn")
                                  .rename_arg("volume", "volser")
                                  .set_default("encoding", "IBM-1047")
                                  .set_default("return-etag", true)
                                  .read_stdout("data", true)
                                  .handle_fifo("stream", "pipe-path", FifoMode::GET));
  // dispatcher.register_command("restoreDataset", CommandBuilder(ds::handle_data_set_restore));
  dispatcher.register_command("writeDataset",
                              CommandBuilder(ds::handle_data_set_write)
                                  .rename_arg("dsname", "dsn")
                                  .rename_arg("volume", "volser")
                                  .set_default("encoding", "IBM-1047")
                                  .write_stdin("data", true)
                                  .handle_fifo("stream", "pipe-path", FifoMode::PUT));
}

void register_job_commands(CommandDispatcher &dispatcher)
{
  // dispatcher.register_command("cancelJob", CommandBuilder(job::handle_job_cancel));
  // dispatcher.register_command("deleteJob", CommandBuilder(job::handle_job_delete));
  // dispatcher.register_command("getJcl", CommandBuilder(job::handle_job_view_jcl));
  // dispatcher.register_command("getJobStatus", CommandBuilder(job::handle_job_view_status));
  // dispatcher.register_command("holdJob", CommandBuilder(job::handle_job_hold));
  dispatcher.register_command("listJobs",
                              CommandBuilder(job::handle_job_list)
                                  .set_default("warn", false));
  dispatcher.register_command("listSpools",
                              CommandBuilder(job::handle_job_list_files)
                                  .rename_arg("job-id", "jobid"));
  dispatcher.register_command("readSpool",
                              CommandBuilder(job::handle_job_view_file)
                                  .rename_arg("job-id", "jobid")
                                  .rename_arg("spool-id", "key")
                                  .set_default("encoding", "IBM-1047")
                                  .read_stdout("data", true));
  // dispatcher.register_command("releaseJob", CommandBuilder(job::handle_job_release));
  // dispatcher.register_command("submitJcl", CommandBuilder(job::handle_job_submit_jcl));
  // dispatcher.register_command("submitJob", CommandBuilder(job::handle_job_submit));
  // dispatcher.register_command("submitUss", CommandBuilder(job::handle_job_submit_uss));
}

void register_uss_commands(CommandDispatcher &dispatcher)
{
  // dispatcher.register_command("chmodFile", CommandBuilder(uss::handle_uss_chmod));
  // dispatcher.register_command("chownFile", CommandBuilder(uss::handle_uss_chown));
  // dispatcher.register_command("chtagFile", CommandBuilder(uss::handle_uss_chtag));
  // dispatcher.register_command("createFile", CommandBuilder([](plugin::InvocationContext &context) -> int
  //                             {
  //   auto handler = context.get<bool>("isDir", false) ?
  //     uss::handle_uss_create_dir : uss::handle_uss_create_file;
  //   return handler(context); }));
  // dispatcher.register_command("deleteFile", CommandBuilder(uss::handle_uss_delete));
  dispatcher.register_command("listFiles",
                              CommandBuilder(uss::handle_uss_list)
                                  .rename_arg("fspath", "file-path")
                                  .set_default("response-format-csv", true));
  dispatcher.register_command("readFile",
                              CommandBuilder(uss::handle_uss_view)
                                  .rename_arg("fspath", "file-path")
                                  .set_default("encoding", "IBM-1047")
                                  .read_stdout("data", true)
                                  .handle_fifo("stream", "pipe-path", FifoMode::GET, true));
  dispatcher.register_command("writeFile",
                              CommandBuilder(uss::handle_uss_write)
                                  .rename_arg("fspath", "file-path")
                                  .set_default("encoding", "IBM-1047")
                                  .write_stdin("data", true)
                                  .handle_fifo("stream", "pipe-path", FifoMode::PUT));
}

void register_cmd_commands(CommandDispatcher &dispatcher)
{
  // dispatcher.register_command("consoleCommand", CommandBuilder(console::handle_console_issue));
}
