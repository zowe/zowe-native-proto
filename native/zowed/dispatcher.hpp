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
#include "builder.hpp"
#include "parser.hpp"
#include <string>
#include <vector>

// Forward declaration
struct RpcNotification;

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

  // Singleton access method
  static CommandDispatcher &getInstance();

  // Delete copy constructor and assignment operator to prevent copying
  CommandDispatcher(const CommandDispatcher &) = delete;
  CommandDispatcher &operator=(const CommandDispatcher &) = delete;

  // Register a new command using CommandBuilder
  bool register_command(const std::string &command_name, const CommandBuilder &builder);

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
  std::unordered_map<std::string, CommandBuilder> m_builders;
#else
  std::tr1::unordered_map<std::string, CommandHandler> m_command_handlers;
  std::tr1::unordered_map<std::string, CommandBuilder> m_builders;
#endif
};

#endif
