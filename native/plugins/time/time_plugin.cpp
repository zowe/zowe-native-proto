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

#include "time_plugin.h"
#include "ztso.hpp"
#include <iostream>
#include <string>

int time_command(plugin::InvocationContext &context)
{
  std::string response;
  // Execute the TSO time command using the helper from ztso.cpp
  int rc = ztso_issue("time", response);
  if (rc == 0)
  {
    context.output_stream() << "Time command executed successfully:" << std::endl;
    context.output_stream() << response.c_str() << std::endl;
    // context.println("Time command executed successfully:");
    // context.println(response.c_str());
  }
  else
  {
    std::string err = "Error executing time command. Return code: " + std::to_string(rc);
    context.error_stream() << err << std::endl;
    // context.println(err.c_str());
    if (!response.empty())
    {
      // context.println(response.c_str());
      context.output_stream() << response.c_str() << std::endl;
    }
  }
  return rc;
}

void BasicCommandRegistry::register_commands(CommandProviderImpl::CommandRegistrationContext &ctx)
{
  auto root = ctx.get_root_command();

  auto time_cmd = ctx.create_command("time", "Executes the TSO time command");
  // ctx.add_alias(time_cmd, "t");
  ctx.set_handler(time_cmd, time_command);

  ctx.add_subcommand(root, time_cmd);
}

void register_plugin(plugin::PluginManager &pm)
{
  pm.register_plugin_metadata("Time Plug-in", "1.0.0");
  pm.register_command_provider(std::make_unique<BasicCommandProvider>());
}
