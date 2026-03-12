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
#include <iostream>
#include <string>
#include "commands/console.hpp"
#include "commands/core.hpp"
#include "commands/ds.hpp"
#include "commands/job.hpp"
#include "commands/server.hpp"
#include "commands/system.hpp"
#include "commands/tool.hpp"
#include "commands/tso.hpp"
#include "commands/uss.hpp"
#include "extend/plugin.hpp"

static std::string get_executable_dir(const char *argv0)
{
  std::string full_path(argv0);
  size_t last_slash = full_path.find_last_of('/');
  if (last_slash != std::string::npos)
    return full_path.substr(0, last_slash);
  return ".";
}

int main(int argc, char *argv[])
{
  server::set_exec_dir(get_executable_dir(argv[0]));

  try
  {
    auto &root_cmd = core::setup_root_command(argv);

    plugin::PluginManager pm;
    core::set_plugin_manager(&pm);
    pm.load_plugins();

    console::register_commands(root_cmd);
    ds::register_commands(root_cmd);
    job::register_commands(root_cmd);
    server::register_commands(root_cmd);
    sys::register_commands(root_cmd);
    tool::register_commands(root_cmd);
    tso::register_commands(root_cmd);
    uss::register_commands(root_cmd);

    pm.register_commands(root_cmd);

    return core::execute_command(argc, argv);
  }
  catch (const std::exception &e)
  {
    std::cerr << "Fatal error encountered in zowex: " << e.what() << std::endl;
    return 1;
  }
}
