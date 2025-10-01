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

    // For simple renames, the argument must exist
    if (it->isRename)
    {
      if (arg_it == args.end())
      {
        continue; // Argument not found, skip rename
      }

      plugin::Argument value = arg_it->second;
      // Simple rename: remove old key and add with new name
      args.erase(arg_it);
      args[it->newName] = value;
    }
    // For callbacks, they can create new values even if argName doesn't exist
    else if (it->isNoArgsCallback && it->callbackNoArgs)
    {
      // No-argument callback transformation - can create new values
      std::string result = it->callbackNoArgs();

      // Remove the old argument if it existed
      if (arg_it != args.end())
      {
        args.erase(arg_it);
      }

      // If callback returned a non-empty result, add it as new argument
      if (!result.empty())
      {
        args[it->argName] = plugin::Argument(result);
      }
    }
    else if (it->callback)
    {
      // Callback transformation (context first, value second)
      // If argument exists, pass it; otherwise pass empty argument
      plugin::Argument value = (arg_it != args.end()) ? arg_it->second : plugin::Argument();

      std::string result = it->callback(context, value);

      // Remove the old argument if it existed
      if (arg_it != args.end())
      {
        args.erase(arg_it);
      }

      // If callback returned a non-empty result, add it as new argument
      if (!result.empty())
      {
        args[it->argName] = plugin::Argument(result);
      }
    }
  }
}

void CommandDispatcher::apply_output_transforms(const std::vector<ArgTransform> &transforms, MiddlewareContext &context)
{
  ast::Node obj = context.get_object();

  // If no object is set, create one
  if (!obj)
  {
    obj = ast::Ast::object();
    context.set_object(obj);
  }

  // Only process objects (not arrays or primitives)
  if (!obj->is_object())
  {
    return;
  }

  for (std::vector<ArgTransform>::const_iterator it = transforms.begin(); it != transforms.end(); ++it)
  {
    if (it->type != ArgTransform::Output)
    {
      continue;
    }

    // Get the field value from the object if it exists
    ast::Node fieldValue = obj->get(it->argName);

    // For simple renames
    if (it->isRename)
    {
      if (!fieldValue)
      {
        continue; // Field not found, skip rename
      }

      // Remove old field and add with new name (AST doesn't support erase, so we just add the new one)
      obj->set(it->newName, fieldValue);
    }
    // For callbacks, they can create new values even if argName doesn't exist
    else if (it->isNoArgsCallback && it->callbackNoArgs)
    {
      // No-argument callback transformation - can create new values
      std::string result = it->callbackNoArgs();

      // If callback returned a non-empty result, set it on the object
      if (!result.empty())
      {
        obj->set(it->argName, ast::Ast::string(result));
      }
    }
    else if (it->callback)
    {
      // Callback transformation (context first, value second)
      // Create empty argument if field doesn't exist
      plugin::Argument value;
      if (fieldValue)
      {
        // Convert AST node to string for the argument
        if (fieldValue->is_string())
        {
          value = plugin::Argument(fieldValue->as_string());
        }
        else if (fieldValue->is_integer())
        {
          value = plugin::Argument(fieldValue->as_integer());
        }
        else if (fieldValue->is_bool())
        {
          value = plugin::Argument(fieldValue->as_bool());
        }
        // For complex types, use JSON representation
        else
        {
          value = plugin::Argument(fieldValue->as_json());
        }
      }

      std::string result = it->callback(context, value);

      // If callback returned a non-empty result, set it on the object
      if (!result.empty())
      {
        // std::cout << "argName: " << it->argName << ", result: " << result << std::endl;
        obj->set(it->argName, ast::Ast::string(result));
      }
    }
  }
}
