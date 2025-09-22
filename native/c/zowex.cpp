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

#pragma runopts("TRAP(ON,NOSPIE)")

#define _UNIX03_SOURCE
#include <dirent.h>
#include <dlfcn.h>
#include "commands/console.hpp"
#include "commands/core.hpp"
#include "commands/ds.hpp"
#include "commands/job.hpp"
#include "commands/tool.hpp"
#include "commands/tso.hpp"
#include "commands/uss.hpp"
#include "extend/plugin.hpp"

using namespace parser;
using namespace std;

int main(int argc, char *argv[])
{
  auto &root_cmd = core::setup_root_command(argc, argv);

  plugin::PluginManager pm;
  pm.load_plugins();

  console::register_commands(root_cmd);
  ds::register_commands(root_cmd);
  job::register_commands(root_cmd);
  tool::register_commands(root_cmd);
  tso::register_commands(root_cmd);
  uss::register_commands(root_cmd);

  pm.register_commands(root_cmd);

  return core::execute_command(argc, argv);
}