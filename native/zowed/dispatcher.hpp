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

#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include "../c/extend/plugin.hpp"
#include "parser.hpp"
#include <string>
#include <functional>

#if defined(__IBMTR1_CPP__) && !defined(__clang__)
#include <tr1/unordered_map>
#else
#include <unordered_map>
#endif

class CommandDispatcher
{
public:
  // CommandHandler type from plugin.hpp
  typedef plugin::CommandProviderImpl::CommandRegistrationContext::CommandHandler CommandHandler;

  // Input handler type for preprocessing
  typedef std::function<void(MiddlewareContext &)> InputHandler;

  // Singleton access method
  static CommandDispatcher &getInstance();

  // Delete copy constructor and assignment operator to prevent copying
  CommandDispatcher(const CommandDispatcher &) = delete;
  CommandDispatcher &operator=(const CommandDispatcher &) = delete;

  // Register a new command with its handler and optional input handler
  bool register_command(const std::string &command_name, CommandHandler handler, InputHandler input_handler = nullptr);

  // Dispatch a command by name using the provided context
  int dispatch(const std::string &command_name, MiddlewareContext &context);

  // Check if a command is registered
  bool has_command(const std::string &command_name) const;

  // Get list of registered command names
  std::vector<std::string> get_registered_commands() const;

  // Remove a command registration
  bool unregister_command(const std::string &command_name);

  // Clear all registered commands
  void clear();

private:
  // Private constructor for singleton
  CommandDispatcher();

  // Private destructor
  ~CommandDispatcher();

#if defined(__clang__)
  std::unordered_map<std::string, CommandHandler> m_command_handlers;
  std::unordered_map<std::string, InputHandler> m_input_handlers;
#else
  std::tr1::unordered_map<std::string, CommandHandler> m_command_handlers;
  std::tr1::unordered_map<std::string, InputHandler> m_input_handlers;
#endif
};

#endif
