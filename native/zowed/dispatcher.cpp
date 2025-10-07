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
#include "server.hpp"
#include "logger.hpp"
#include <vector>
#include <algorithm>

CommandDispatcher &CommandDispatcher::getInstance()
{
  static CommandDispatcher instance;
  return instance;
}

bool CommandDispatcher::register_command(const std::string &command_name, const CommandBuilder &builder)
{
  if (command_name.empty() || builder.get_handler() == nullptr)
  {
    LOG_ERROR("Cannot register command: invalid name or handler");
    return false;
  }

  // Check if command already exists
  if (m_command_handlers.find(command_name) != m_command_handlers.end())
  {
    LOG_ERROR("Command already registered: %s", command_name.c_str());
    return false; // Command already registered
  }

  m_command_handlers[command_name] = builder.get_handler();
  m_builders.insert(std::make_pair(command_name, builder));

  LOG_DEBUG("Registered command: %s", command_name.c_str());
  return true;
}

int CommandDispatcher::dispatch(const std::string &command_name, MiddlewareContext &context)
{
  auto it = m_command_handlers.find(command_name);
  if (it == m_command_handlers.end())
  {
    std::string errMsg = "Command not found: " + command_name;
    context.errln(errMsg.c_str());
    LOG_ERROR("%s", errMsg.c_str());
    return RpcErrorCode::METHOD_NOT_FOUND;
  }

  CommandHandler handler = it->second;
  if (handler == nullptr)
  {
    context.errln("Invalid command handler");
    LOG_ERROR("Invalid command handler for: %s", command_name.c_str());
    return RpcErrorCode::INTERNAL_ERROR;
  }

  try
  {
    LOG_DEBUG("Dispatching command: %s", command_name.c_str());

    // Apply input transforms if builder exists
    auto builder_it = m_builders.find(command_name);
    bool has_builder = (builder_it != m_builders.end());

    if (has_builder)
    {
      builder_it->second.apply_input_transforms(context);
    }

    // Call the command handler with the context
    int result = handler(context);

    // Apply output transforms if builder exists
    if (has_builder)
    {
      builder_it->second.apply_output_transforms(context);
    }

    if (result != 0)
    {
      LOG_ERROR("Command failed with code %d: %s", result, command_name.c_str());
    }

    return result;
  }
  catch (const std::exception &e)
  {
    std::string errMsg = std::string("Command execution failed: ") + e.what();
    context.err("Command execution failed: ");
    context.errln(e.what());
    LOG_ERROR("%s (command: %s)", errMsg.c_str(), command_name.c_str());
    return RpcErrorCode::INTERNAL_ERROR;
  }
  catch (...)
  {
    context.errln("Command execution failed with unknown error");
    LOG_ERROR("Command execution failed with unknown error: %s", command_name.c_str());
    return RpcErrorCode::INTERNAL_ERROR;
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
    LOG_ERROR("Cannot unregister command (not found): %s", command_name.c_str());
    return false; // Command not found
  }

  m_command_handlers.erase(it);

  // Also remove builder if it exists
  auto builder_it = m_builders.find(command_name);
  if (builder_it != m_builders.end())
  {
    m_builders.erase(builder_it);
  }

  LOG_DEBUG("Unregistered command: %s", command_name.c_str());
  return true;
}

void CommandDispatcher::clear()
{
  LOG_DEBUG("Clearing all registered commands");
  m_command_handlers.clear();
  m_builders.clear();
}
