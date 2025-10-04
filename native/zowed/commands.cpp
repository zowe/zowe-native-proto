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

// Helper functions to create builders with common argument mappings
static CommandBuilder create_ds_builder(CommandBuilder::CommandHandler handler)
{
  return CommandBuilder(handler).rename_arg("dsname", "dsn");
}

static CommandBuilder create_job_builder(CommandBuilder::CommandHandler handler)
{
  return CommandBuilder(handler).rename_arg("job-id", "jobid");
}

static CommandBuilder create_uss_builder(CommandBuilder::CommandHandler handler)
{
  return CommandBuilder(handler).rename_arg("fspath", "file-path");
}

void register_ds_commands(CommandDispatcher &dispatcher)
{
  dispatcher.register_command("createDataset",
                              create_ds_builder(ds::create_with_attributes)
                                  .flatten_obj("attributes"));
  dispatcher.register_command("createMember",
                              create_ds_builder(ds::handle_data_set_create_member));
  dispatcher.register_command("deleteDataset",
                              create_ds_builder(ds::handle_data_set_delete));
  dispatcher.register_command("listDatasets",
                              CommandBuilder(ds::handle_data_set_list)
                                  .rename_arg("pattern", "dsn")
                                  .set_default("warn", false));
  dispatcher.register_command("listDsMembers",
                              create_ds_builder(ds::handle_data_set_list_members)
                                  .set_default("warn", false));
  dispatcher.register_command("readDataset",
                              create_ds_builder(ds::handle_data_set_view)
                                  .rename_arg("volume", "volser")
                                  .set_default("encoding", "IBM-1047")
                                  .set_default("return-etag", true)
                                  .read_stdout("data", true)
                                  .handle_fifo("stream", "pipe-path", FifoMode::GET));
  dispatcher.register_command("restoreDataset",
                              create_ds_builder(ds::handle_data_set_restore));
  dispatcher.register_command("writeDataset",
                              create_ds_builder(ds::handle_data_set_write)
                                  .rename_arg("volume", "volser")
                                  .set_default("encoding", "IBM-1047")
                                  .write_stdin("data", true)
                                  .handle_fifo("stream", "pipe-path", FifoMode::PUT));
}

void register_job_commands(CommandDispatcher &dispatcher)
{
  dispatcher.register_command("cancelJob",
                              create_job_builder(job::handle_job_cancel));
  dispatcher.register_command("deleteJob",
                              create_job_builder(job::handle_job_delete));
  dispatcher.register_command("getJcl",
                              create_job_builder(job::handle_job_view_jcl)
                                  .read_stdout("data", false));
  dispatcher.register_command("getJobStatus",
                              create_job_builder(job::handle_job_view_status));
  dispatcher.register_command("holdJob",
                              create_job_builder(job::handle_job_hold));
  dispatcher.register_command("listJobs",
                              CommandBuilder(job::handle_job_list)
                                  .set_default("warn", false));
  dispatcher.register_command("listSpools",
                              create_job_builder(job::handle_job_list_files));
  dispatcher.register_command("readSpool",
                              create_job_builder(job::handle_job_view_file)
                                  .rename_arg("spool-id", "key")
                                  .set_default("encoding", "IBM-1047")
                                  .read_stdout("data", true));
  dispatcher.register_command("releaseJob",
                              create_job_builder(job::handle_job_release));
  dispatcher.register_command("submitJcl",
                              CommandBuilder(job::handle_job_submit_jcl)
                                  .write_stdin("jcl", true));
  dispatcher.register_command("submitJob",
                              create_ds_builder(job::handle_job_submit));
  dispatcher.register_command("submitUss",
                              create_uss_builder(job::handle_job_submit_uss));
}

void register_uss_commands(CommandDispatcher &dispatcher)
{
  dispatcher.register_command("chmodFile",
                              create_uss_builder(uss::handle_uss_chmod));
  dispatcher.register_command("chownFile",
                              create_uss_builder(uss::handle_uss_chown));
  dispatcher.register_command("chtagFile",
                              create_uss_builder(uss::handle_uss_chtag));
  const auto handle_uss_create = [](plugin::InvocationContext &context) -> int
  {
    auto handler = context.get<bool>("is-dir", false) ?
      uss::handle_uss_create_dir : uss::handle_uss_create_file;
    return handler(context); };
  dispatcher.register_command("createFile",
                              create_uss_builder(handle_uss_create)
                                  .rename_arg("permissions", "mode"));
  dispatcher.register_command("deleteFile",
                              create_uss_builder(uss::handle_uss_delete));
  dispatcher.register_command("listFiles",
                              create_uss_builder(uss::handle_uss_list)
                                  .set_default("response-format-csv", true));
  dispatcher.register_command("readFile",
                              create_uss_builder(uss::handle_uss_view)
                                  .set_default("encoding", "IBM-1047")
                                  .read_stdout("data", true)
                                  .handle_fifo("stream", "pipe-path", FifoMode::GET, true));
  dispatcher.register_command("writeFile",
                              create_uss_builder(uss::handle_uss_write)
                                  .set_default("encoding", "IBM-1047")
                                  .write_stdin("data", true)
                                  .handle_fifo("stream", "pipe-path", FifoMode::PUT));
}

void register_cmd_commands(CommandDispatcher &dispatcher)
{
  // TODO Support APF authorized commands with zoweax
  // dispatcher.register_command("consoleCommand", CommandBuilder(console::handle_console_issue));
}
