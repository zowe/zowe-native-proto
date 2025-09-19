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

#include "sample_plugin.h"

int hello_command(plugin::InvocationContext &context)
{
  std::string str = "hello from sample command";
  if (context.has("name"))
  {
    str += " to " + context.get<std::string>("name");
  }
  context.println(str.c_str());
  return 0;
}

void BasicCommandRegistry::registerCommands(CommandProviderImpl::CommandRegistrationContext &ctx)
{
  auto sample_group = ctx.createCommand("sample", "Sample command group for plug-in operations");
  auto hello = ctx.createCommand("hello", "says hi to the caller");
  ctx.addAlias(hello, "hi");
  ctx.addPositionalArg(hello, "name", "the name of the person calling the command", CommandProviderImpl::CommandRegistrationContext::ArgumentType_Positional, 0, nullptr);
  ctx.setHandler(hello, hello_command);
  ctx.addSubcommand(sample_group, hello);
}

void registerPlugin(plugin::PluginManager &pm)
{
  auto provider = BasicCommandProvider();
  pm.registerCommandProvider(&provider);
}
