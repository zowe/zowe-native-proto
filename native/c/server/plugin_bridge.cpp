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

#include "plugin_bridge.hpp"
#include "dispatcher.hpp"
#include "builder.hpp"
#include "logger.hpp"
#include <string>
#include <cctype>
#include <algorithm>

namespace plugin_bridge
{

using namespace plugin;

// Forward declaration for recursion
static void traverse_and_register_impl(parser::Command *cmd, std::string &path_prefix, CommandDispatcher &dispatcher, int depth);

/**
 * @brief Recursively traverse command tree and register to middleware
 *
 * @param cmd The command to process
 * @param path_prefix The command path built so far (e.g., "sample.hello")
 * @param dispatcher The dispatcher to register commands to
 */
static void traverse_and_register(parser::Command *cmd, const std::string &path_prefix, CommandDispatcher &dispatcher)
{
  // Limit recursion depth to prevent stack issues on z/OS
  std::string mutable_path = path_prefix;
  traverse_and_register_impl(cmd, mutable_path, dispatcher, 0);
}

static void traverse_and_register_impl(parser::Command *cmd, std::string &path_prefix, CommandDispatcher &dispatcher, int depth)
{
  if (!cmd || depth > 10) // Prevent deep recursion on z/OS
    return;

  // Get handler for this command
  typedef plugin::CommandProviderImpl::CommandRegistrationContext::CommandHandler CommandHandler;
  CommandHandler handler = cmd->get_handler();

  // Check if this command has subcommands
  const std::map<std::string, parser::command_ptr> &subcommands = cmd->get_commands();

  if (handler != nullptr)
  {
    // This command has a handler, register it with dot notation
    std::string rpc_command_name = path_prefix;
    std::replace(rpc_command_name.begin(), rpc_command_name.end(), ' ', '.');

    try
    {
      LOG_DEBUG("Registering plugin command: %s (from path: %s)", rpc_command_name.c_str(), path_prefix.c_str());

      CommandBuilder builder(handler);

      // For plugin commands, arguments from JSON are passed in camelCase by the middleware.
      // Plugins typically register CLI-style argument names (kebab-case) using add_keyword_arg.
      // We dynamically infer renames from the command's declared arguments.
      for (const auto &arg : cmd->get_args())
      {
        // Help flags and automatically generated 'no-' flags are CLI-specific
        // and shouldn't be exposed as separate JSON-RPC parameters.
        if (arg.is_help_flag || arg.name.rfind("no-", 0) == 0)
          continue;

        // If the arg name has hyphens, map its camelCase equivalent
        // to the kebab-case name expected by the handler.
        if (arg.name.find('-') != std::string::npos)
        {
          std::string camel_name;
          bool capitalize_next = false;
          for (char c : arg.name)
          {
            if (c == '-')
            {
              capitalize_next = true;
            }
            else
            {
              camel_name += capitalize_next ? static_cast<char>(std::toupper(c)) : c;
              capitalize_next = false;
            }
          }
          builder.rename_arg(camel_name, arg.name);
        }

        // Also apply default values if provided
        if (!arg.default_value.is_none())
        {
          if (arg.default_value.is_bool())
            builder.set_default(arg.name, *arg.default_value.get_bool());
          else if (arg.default_value.is_int())
            builder.set_default(arg.name, *arg.default_value.get_int());
          else if (arg.default_value.is_double())
            builder.set_default(arg.name, *arg.default_value.get_double());
          else if (arg.default_value.is_string())
            builder.set_default(arg.name, *arg.default_value.get_string());
        }
      }

      bool success = dispatcher.register_command(rpc_command_name, builder);

      if (!success)
      {
        LOG_ERROR("Failed to register plugin command: %s", rpc_command_name.c_str());
      }
    }
    catch (const std::exception &e)
    {
      LOG_ERROR("Exception registering plugin command %s: %s", rpc_command_name.c_str(), e.what());
    }
    catch (...)
    {
      LOG_ERROR("Unknown exception registering plugin command: %s", rpc_command_name.c_str());
    }
  }

  // Recursively process subcommands
  // Build subcommand path and recurse - avoid creating many temporary strings
  size_t original_length = path_prefix.length();
  for (std::map<std::string, parser::command_ptr>::const_iterator it = subcommands.begin();
       it != subcommands.end(); ++it)
  {
    if (it->second) // Null check for safety
    {
      // Append to existing string instead of creating new ones
      path_prefix += ".";
      path_prefix += it->first;

      traverse_and_register_impl(it->second.get(), path_prefix, dispatcher, depth + 1);

      // Restore original path for next sibling
      path_prefix.resize(original_length);
    }
  }
}

void register_plugin_commands(PluginManager &pm, CommandDispatcher &dispatcher)
{
  parser::Command temp_root("_temp_root", "");

  // Build the command tree using the existing implementation from plugin.cpp
  // This reuses all the logic instead of duplicating it
  pm.register_commands(temp_root);

  // Get the commands map - the shared_ptrs will keep objects alive
  const std::map<std::string, parser::command_ptr> &top_level_cmds = temp_root.get_commands();

  // Traverse all top-level commands and register them to middleware
  for (std::map<std::string, parser::command_ptr>::const_iterator it = top_level_cmds.begin();
       it != top_level_cmds.end(); ++it)
  {
    LOG_DEBUG("Registering top-level plugin command: %s", it->first.c_str());
    if (it->second) // Null check for safety
    {
      traverse_and_register(it->second.get(), it->first, dispatcher);
    }
  }
}

} // namespace plugin_bridge