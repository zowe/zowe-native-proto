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

#include "dispatcher.hpp"
#include <vector>
#include <algorithm>

CommandDispatcher &CommandDispatcher::getInstance()
{
  static CommandDispatcher instance;
  return instance;
}

CommandDispatcher::CommandDispatcher()
{
}

CommandDispatcher::~CommandDispatcher()
{
}

bool CommandDispatcher::register_command(const std::string &command_name, const CommandBuilder &builder)
{
  if (command_name.empty() || builder.get_handler() == nullptr)
  {
    return false;
  }

  // Check if command already exists
  if (m_command_handlers.find(command_name) != m_command_handlers.end())
  {
    return false; // Command already registered
  }

  m_command_handlers[command_name] = builder.get_handler();
  m_builders.insert(std::make_pair(command_name, builder));

  return true;
}

int CommandDispatcher::dispatch(const std::string &command_name, MiddlewareContext &context)
{
  auto it = m_command_handlers.find(command_name);
  if (it == m_command_handlers.end())
  {
    // Command not found - return error code
    context.errln("Command not found");
    return -1;
  }

  CommandHandler handler = it->second;
  if (handler == nullptr)
  {
    context.errln("Invalid command handler");
    return -2;
  }

  try
  {
    // Apply input transforms if builder exists
    auto builder_it = m_builders.find(command_name);
    if (builder_it != m_builders.end())
    {
      builder_it->second.apply_input_transforms(context);
    }

    // Call the command handler with the context
    int result = handler(context);

    // Apply output transforms if builder exists
    if (builder_it != m_builders.end())
    {
      builder_it->second.apply_output_transforms(context);
    }

    return result;
  }
  catch (const std::exception &e)
  {
    context.err("Command execution failed: ");
    context.errln(e.what());
    return -3;
  }
  catch (...)
  {
    context.errln("Command execution failed with unknown error");
    return -4;
  }
}

bool CommandDispatcher::has_command(const std::string &command_name) const
{
  return m_command_handlers.find(command_name) != m_command_handlers.end();
}

std::vector<std::string> CommandDispatcher::get_registered_commands() const
{
  std::vector<std::string> commands;
  commands.reserve(m_command_handlers.size());

  for (auto it = m_command_handlers.begin(); it != m_command_handlers.end(); ++it)
  {
    commands.push_back(it->first);
  }

  // Sort for consistent ordering
  std::sort(commands.begin(), commands.end());
  return commands;
}

bool CommandDispatcher::unregister_command(const std::string &command_name)
{
  auto it = m_command_handlers.find(command_name);
  if (it == m_command_handlers.end())
  {
    return false; // Command not found
  }

  m_command_handlers.erase(it);

  // Also remove builder if it exists
  auto builder_it = m_builders.find(command_name);
  if (builder_it != m_builders.end())
  {
    m_builders.erase(builder_it);
  }

  return true;
}

void CommandDispatcher::clear()
{
  m_command_handlers.clear();
  m_builders.clear();
}
