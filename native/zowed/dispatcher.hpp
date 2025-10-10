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
#include "../c/singleton.hpp"
#include "builder.hpp"
#include "rpcio.hpp"
#include <string>
#include <vector>

// Forward declaration
struct RpcNotification;

#if defined(__IBMTR1_CPP__) && !defined(__clang__)
#include <tr1/unordered_map>
#else
#include <unordered_map>
#endif

class CommandDispatcher : public Singleton<CommandDispatcher>
{
  friend class Singleton<CommandDispatcher>;

public:
  // CommandHandler type from plugin.hpp
  typedef plugin::CommandProviderImpl::CommandRegistrationContext::CommandHandler CommandHandler;

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

  // Get the builders map (for accessing validators)
  const std::unordered_map<std::string, CommandBuilder> &get_builders() const
  {
    return m_commands;
  }

private:
  // Private constructor for singleton
  CommandDispatcher() = default;

  std::unordered_map<std::string, CommandBuilder> m_commands;
};

#endif
