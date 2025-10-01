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
#include <vector>

#if defined(__IBMTR1_CPP__) && !defined(__clang__)
#include <tr1/unordered_map>
#else
#include <unordered_map>
#endif

// Argument transformation types
struct ArgTransform
{
  enum TransformType
  {
    Input, // Transform input arguments before command execution
    Output // Transform output arguments after command execution
  };

  // Transformation function that processes an argument value
  // Returns new name if transformation succeeded, or empty string if transformation failed/should be removed
  typedef std::function<std::string(MiddlewareContext &context, const plugin::Argument &value)> TransformCallback;
  typedef std::function<std::string()> TransformCallbackNoArgs;

  TransformType type;
  std::string argName;

  // Either a simple rename (string) or a callback function
  bool isRename;
  bool isNoArgsCallback;
  std::string newName;                    // Used when isRename is true
  TransformCallback callback;             // Used when isRename is false
  TransformCallbackNoArgs callbackNoArgs; // Used for 0-argument callbacks

  // Constructor for simple rename
  ArgTransform(TransformType t, const std::string &arg, const std::string &newArgName)
      : type(t), argName(arg), isRename(true), isNoArgsCallback(false), newName(newArgName), callback(nullptr), callbackNoArgs(nullptr)
  {
  }

  // Constructor for callback-based transformation (with context and value)
  ArgTransform(TransformType t, const std::string &arg, TransformCallback cb)
      : type(t), argName(arg), isRename(false), isNoArgsCallback(false), newName(""), callback(cb), callbackNoArgs(nullptr)
  {
  }

  // Constructor for callback-based transformation (no arguments)
  ArgTransform(TransformType t, const std::string &arg, TransformCallbackNoArgs cb)
      : type(t), argName(arg), isRename(false), isNoArgsCallback(true), newName(""), callback(nullptr), callbackNoArgs(cb)
  {
  }
};

// Helper functions to create transforms with cleaner syntax
inline ArgTransform InputTransform(const std::string &argName, ArgTransform::TransformCallback callback)
{
  return ArgTransform(ArgTransform::Input, argName, callback);
}

inline ArgTransform InputTransform(const std::string &argName, ArgTransform::TransformCallbackNoArgs callback)
{
  return ArgTransform(ArgTransform::Input, argName, callback);
}

inline ArgTransform InputTransform(const std::string &rpcName, const std::string &argName)
{
  return ArgTransform(ArgTransform::Input, rpcName, argName);
}

inline ArgTransform OutputTransform(const std::string &argName, ArgTransform::TransformCallback callback)
{
  return ArgTransform(ArgTransform::Output, argName, callback);
}

inline ArgTransform OutputTransform(const std::string &argName, ArgTransform::TransformCallbackNoArgs callback)
{
  return ArgTransform(ArgTransform::Output, argName, callback);
}

inline ArgTransform OutputTransform(const std::string &argName, const std::string &rpcName)
{
  return ArgTransform(ArgTransform::Output, argName, rpcName);
}

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

  // Register a new command with its handler and optional argument transforms
  bool register_command(const std::string &command_name, CommandHandler handler, const std::vector<ArgTransform> &transforms = std::vector<ArgTransform>());

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

  // Apply input transforms to the context before command execution
  void apply_input_transforms(const std::vector<ArgTransform> &transforms, MiddlewareContext &context);

  // Apply output transforms to the context after command execution
  void apply_output_transforms(const std::vector<ArgTransform> &transforms, MiddlewareContext &context);

#if defined(__clang__)
  std::unordered_map<std::string, CommandHandler> m_command_handlers;
  std::unordered_map<std::string, std::vector<ArgTransform>> m_transforms;
#else
  std::tr1::unordered_map<std::string, CommandHandler> m_command_handlers;
  std::tr1::unordered_map<std::string, std::vector<ArgTransform>> m_transforms;
#endif
};

#endif
