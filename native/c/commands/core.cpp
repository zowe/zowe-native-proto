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

#include "core.hpp"
#include "../zshmem.hpp"
#include "../ztype.h"
#include "../extend/plugin.hpp"
#include <dirent.h>
#include <algorithm>
#include <set>
#include <vector>

using namespace parser;
using namespace std;

// Version information
#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "unknown"
#endif

#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

namespace core
{
std::tr1::shared_ptr<ArgumentParser> g_arg_parser;
namespace
{
plugin::PluginManager *g_plugin_manager = nullptr;
}

void set_plugin_manager(plugin::PluginManager *manager)
{
  g_plugin_manager = manager;
}

plugin::PluginManager *get_plugin_manager()
{
  return g_plugin_manager;
}

bool should_quit(const std::string &input)
{
  return (input == "quit" || input == "exit" ||
          input == "QUIT" || input == "EXIT");
}

int interactive_mode(plugin::InvocationContext &context)
{
  // Initialize shared memory
  int shm_id;
  ZSharedRegion *shm_ptr = nullptr;

  const auto shm_file_path = context.get<std::string>("shm-file", "");

  if (!shm_file_path.empty())
  {
    // Create new shared memory for this process (each process gets its own)
    shm_id = init_shared_memory(&shm_ptr, shm_file_path.c_str());
    if (shm_id == -1)
    {
      context.error_stream() << "Failed to initialize shared memory" << endl;
      return RTNCD_FAILURE;
    }
  }

  context.output_stream() << "Started, enter command or 'quit' to quit..." << std::endl;

  std::string command;
  int rc = 0;
  int is_tty = isatty(fileno(stdout));

  do
  {
    if (is_tty)
      context.output_stream() << "\r> " << std::flush;

    std::getline(context.input_stream(), command);

    if (should_quit(command))
      break;

    // Parse and execute the command
    ParseResult result = g_arg_parser->parse(command);
    rc = result.exit_code;

    if (!is_tty)
    {
      context.output_stream() << "[" << rc << "]" << std::endl;
      // EBCDIC \x37 = ASCII \x04 = End of Transmission (Ctrl+D)
      context.output_stream() << '\x37' << std::flush;
      context.error_stream() << '\x37' << std::flush;
    }

  } while (!should_quit(command));

  context.output_stream() << "...terminated" << std::endl;

  // Clean up this process's shared memory
  if (!shm_file_path.empty())
  {
    cleanup_shared_memory(shm_id, shm_ptr, shm_file_path.c_str());
  }

  return rc;
}

int handle_version(plugin::InvocationContext &context)
{
  context.output_stream() << "Zowe Native Protocol CLI (zowex)" << endl;
  context.output_stream() << "Version: " << PACKAGE_VERSION << endl;
  context.output_stream() << "Build Date: " << BUILD_DATE << " " << BUILD_TIME << endl;
  context.output_stream() << "Copyright Contributors to the Zowe Project." << endl;
  return 0;
}

int handle_plugins_list(plugin::InvocationContext &context)
{
  std::ostream &out = context.output_stream();

  std::vector<std::string> plugin_files;
  DIR *plugins_dir = opendir("./plugins");
  if (plugins_dir != nullptr)
  {
    struct dirent *entry;
    while ((entry = readdir(plugins_dir)) != nullptr)
    {
      std::string name(entry->d_name ? entry->d_name : "");
      if (name == "." || name == ".." || name.empty())
        continue;
      plugin_files.push_back(name);
    }
    closedir(plugins_dir);
  }

  std::sort(plugin_files.begin(), plugin_files.end());

  std::set<std::string> registered_files;
  auto *manager = get_plugin_manager();
  if (manager != nullptr)
  {
    const auto &loaded_plugins = manager->get_loaded_plugins();
    for (auto it = loaded_plugins.begin(); it != loaded_plugins.end(); ++it)
    {
      auto &plugin = *it;
      registered_files.insert(plugin.metadata.filename);

      const auto &metadata = plugin.metadata;
      out << metadata.display_name;
      if (!metadata.filename.empty())
      {
        out << " (" << metadata.filename << ")";
      }
      out << endl;
      out << "  Version: " << (metadata.version.empty() ? "n/a" : metadata.version) << endl;
      out << endl;
    }
  }

  bool has_unregistered = false;
  for (auto it = plugin_files.begin(); it != plugin_files.end(); ++it)
  {
    if (registered_files.find(*it) != registered_files.end())
    {
      continue;
    }
    if (!has_unregistered)
    {
      out << "The following unregistered plug-ins were found in the plugins/ dir:" << endl;
      has_unregistered = true;
    }
    out << *it << endl;
    out << endl;
  }

  return 0;
}

int handle_command(plugin::InvocationContext &context)
{
  const auto is_interactive = context.get<bool>("interactive", false);
  if (context.get<bool>("version", false))
  {
    const auto version_rc = handle_version(context);
    if (!is_interactive)
    {
      return version_rc;
    }
  }

  if (is_interactive)
  {
    return interactive_mode(context);
  }

  // If no interactive mode and no subcommands were invoked, show help
  g_arg_parser->get_root_command().generate_help(context.output_stream());
  return 0;
}

int execute_command(int argc, char *argv[])
{
  auto result = g_arg_parser->parse(argc, argv);
  return result.exit_code;
}

Command &setup_root_command(int argc, char *argv[])
{
  g_arg_parser = std::tr1::shared_ptr<ArgumentParser>(new ArgumentParser(argv[0], "Zowe Native Protocol CLI"));
  auto &root_command = g_arg_parser->get_root_command();

  root_command.add_keyword_arg("interactive",
                               make_aliases("--interactive", "--it"),
                               "interactive (REPL) mode", ArgType_Flag, false,
                               ArgValue(false));
  root_command.add_keyword_arg("version",
                               make_aliases("--version", "-v"),
                               "display version information", ArgType_Flag, false,
                               ArgValue(false));
  root_command.add_keyword_arg("shm-file",
                               make_aliases("--shm-file"),
                               "shared memory file path", ArgType_Single, false);
  root_command.set_handler(handle_command);

  // Core commands
  {
    auto version_cmd = command_ptr(new Command("version", "display version information"));
    version_cmd->add_alias("--version");
    version_cmd->add_alias("-v");
    version_cmd->set_handler(handle_version);
    root_command.add_command(version_cmd);

    auto plugins_cmd = command_ptr(new Command("plugins", "plug-in management commands"));
    auto list_cmd = command_ptr(new Command("list", "list available plug-ins"));
    list_cmd->set_handler(handle_plugins_list);
    plugins_cmd->add_command(list_cmd);
    root_command.add_command(plugins_cmd);
  }

  return root_command;
}
} // namespace core
