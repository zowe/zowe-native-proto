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
#include "../c/zbase64.h"

void write_data_to_stdin(MiddlewareContext &context)
{
  // Check if "data" argument exists
  const plugin::Argument *dataArg = context.find("data");
  if (dataArg && dataArg->is_string())
  {
    try
    {
      // Get the base64 encoded data
      std::string base64Data = dataArg->get_string_value();

      // Base64 decode the data and write to input stream
      std::string decodedData = zbase64::decode(base64Data);
      context.set_input_content(decodedData);

      // Remove "data" from arguments since it's now in input stream
      plugin::ArgumentMap &args = const_cast<plugin::ArgumentMap &>(context.arguments());
      args.erase("data");
    }
    catch (const std::exception &e)
    {
      // If base64 decode fails, write error to error stream
      context.errln("Failed to decode base64 data");
    }
  }
}

void register_ds_commands(CommandDispatcher &dispatcher)
{
  // Register data set command handlers
  dispatcher.register_command("listDatasets", ds::handle_data_set_list);
  dispatcher.register_command("listDsMembers", ds::handle_data_set_list_members);
  dispatcher.register_command("readDataset", ds::handle_data_set_view);
  dispatcher.register_command("writeDataset", ds::handle_data_set_write, write_data_to_stdin);
  dispatcher.register_command("restoreDataset", ds::handle_data_set_restore);
  dispatcher.register_command("deleteDataset", ds::handle_data_set_delete);
  dispatcher.register_command("createDataset", ds::create_with_attributes);
  dispatcher.register_command("createMember", ds::handle_data_set_create_member);
}

void register_job_commands(CommandDispatcher &dispatcher)
{
  // Register jobs command handlers
  dispatcher.register_command("getJcl", job::handle_job_view_jcl);
  dispatcher.register_command("listJobs", job::handle_job_list);
  dispatcher.register_command("listSpools", job::handle_job_list_files);
  dispatcher.register_command("readSpool", job::handle_job_view_file);
  dispatcher.register_command("getStatus", job::handle_job_view_status);
  dispatcher.register_command("cancelJob", job::handle_job_cancel);
  dispatcher.register_command("deleteJob", job::handle_job_delete);
  dispatcher.register_command("submitJob", job::handle_job_submit);
  dispatcher.register_command("submitUss", job::handle_job_submit_uss);
  dispatcher.register_command("submitJcl", job::handle_job_submit_jcl);
  dispatcher.register_command("holdJob", job::handle_job_hold);
  dispatcher.register_command("releaseJob", job::handle_job_release);
}

void register_uss_commands(CommandDispatcher &dispatcher)
{
  // Register USS command handlers
  dispatcher.register_command("listFiles", uss::handle_uss_list);
  dispatcher.register_command("readFile", uss::handle_uss_view);
  dispatcher.register_command("writeFile", uss::handle_uss_write, write_data_to_stdin);
  dispatcher.register_command("deleteFile", uss::handle_uss_delete);
  dispatcher.register_command("createFile", uss::handle_uss_create_file);
  dispatcher.register_command("chmodFile", uss::handle_uss_chmod);
  dispatcher.register_command("chownFile", uss::handle_uss_chown);
  dispatcher.register_command("chtagFile", uss::handle_uss_chtag);
}
