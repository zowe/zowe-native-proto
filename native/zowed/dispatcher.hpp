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

#if defined(__IBMTR1_CPP__) && !defined(__clang__)
#include <tr1/unordered_map>
#else
#include <unordered_map>
#endif

class RpcDispatcher
{
public:
  // CommandHandler type from plugin.hpp
  typedef plugin::CommandProviderImpl::CommandRegistrationContext::CommandHandler CommandHandler;

  // Singleton access method
  static RpcDispatcher &getInstance();

  // Delete copy constructor and assignment operator to prevent copying
  RpcDispatcher(const RpcDispatcher &) = delete;
  RpcDispatcher &operator=(const RpcDispatcher &) = delete;

  // Register a new command with its handler
  bool register_command(const std::string &command_name, CommandHandler handler);

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
  RpcDispatcher();

  // Private destructor
  ~RpcDispatcher();

#if defined(__clang__)
  std::unordered_map<std::string, CommandHandler> m_command_handlers;
#else
  std::tr1::unordered_map<std::string, CommandHandler> m_command_handlers;
#endif
};

#endif
