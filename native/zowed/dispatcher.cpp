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

bool CommandDispatcher::register_command(const std::string &command_name, CommandHandler handler, const std::vector<ArgTransform> &transforms)
{
  if (command_name.empty() || handler == nullptr)
  {
    return false;
  }

  // Check if command already exists
  if (m_command_handlers.find(command_name) != m_command_handlers.end())
  {
    return false; // Command already registered
  }

  m_command_handlers[command_name] = handler;

  // Store transforms if provided
  if (!transforms.empty())
  {
    m_transforms[command_name] = transforms;
  }

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
    // Apply input transforms if they exist
    auto transforms_it = m_transforms.find(command_name);
    if (transforms_it != m_transforms.end())
    {
      apply_input_transforms(transforms_it->second, context);
    }

    // Call the command handler with the context
    int result = handler(context);

    // Apply output transforms if they exist
    if (transforms_it != m_transforms.end())
    {
      apply_output_transforms(transforms_it->second, context);
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

  // Also remove transforms if they exist
  auto transforms_it = m_transforms.find(command_name);
  if (transforms_it != m_transforms.end())
  {
    m_transforms.erase(transforms_it);
  }

  return true;
}

void CommandDispatcher::clear()
{
  m_command_handlers.clear();
  m_transforms.clear();
}

void CommandDispatcher::apply_input_transforms(const std::vector<ArgTransform> &transforms, MiddlewareContext &context)
{
  plugin::ArgumentMap &args = context.mutable_arguments();

  for (std::vector<ArgTransform>::const_iterator it = transforms.begin(); it != transforms.end(); ++it)
  {
    if (it->type != ArgTransform::Input)
    {
      continue;
    }

    // Find the argument
    plugin::ArgumentMap::iterator arg_it = args.find(it->argName);
    if (arg_it == args.end())
    {
      continue; // Argument not found, skip
    }

    plugin::Argument value = arg_it->second;

    if (it->isRename)
    {
      // Simple rename: remove old key and add with new name
      args.erase(arg_it);
      args[it->newName] = value;
    }
    else if (it->callback)
    {
      // Callback transformation
      std::string newArgName = it->callback(value, context);

      // Remove the old argument
      args.erase(arg_it);

      // If callback returned a non-empty name, add with new name
      if (!newArgName.empty())
      {
        args[newArgName] = value;
      }
    }
  }
}

void CommandDispatcher::apply_output_transforms(const std::vector<ArgTransform> &transforms, MiddlewareContext &context)
{
  plugin::ArgumentMap &output = context.mutable_output();

  for (std::vector<ArgTransform>::const_iterator it = transforms.begin(); it != transforms.end(); ++it)
  {
    if (it->type != ArgTransform::Output)
    {
      continue;
    }

    // Find the output argument
    plugin::ArgumentMap::iterator out_it = output.find(it->argName);
    if (out_it == output.end())
    {
      continue; // Output not found, skip
    }

    plugin::Argument value = out_it->second;

    if (it->isRename)
    {
      // Simple rename: remove old key and add with new name
      output.erase(out_it);
      output[it->newName] = value;
    }
    else if (it->callback)
    {
      // Callback transformation
      std::string newArgName = it->callback(value, context);

      // Remove the old output
      output.erase(out_it);

      // If callback returned a non-empty name, add with new name
      if (!newArgName.empty())
      {
        output[newArgName] = value;
      }
    }
  }
}
