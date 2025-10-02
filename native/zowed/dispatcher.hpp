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
#include <vector>

#if defined(__IBMTR1_CPP__) && !defined(__clang__)
#include <tr1/unordered_map>
#else
#include <unordered_map>
#endif

// Argument transformation types
struct ArgTransform
{
  enum TransformKind
  {
    InputRename,  // Rename an input argument
    InputDefault, // Set a default value for an input argument
    InputStdin,   // Transform input from argument to stdin
    OutputStdout  // Transform output from stdout to argument
  };

  TransformKind kind;
  std::string argName;
  std::string newName;         // Used for InputRename
  plugin::Argument defaultArg; // Used for InputDefault
  bool b64Encode;              // Used for InputStdin and OutputStdout

  // Constructor for InputRename
  ArgTransform(TransformKind k, const std::string &arg, const std::string &newArgName)
      : kind(k), argName(arg), newName(newArgName), defaultArg(), b64Encode(false)
  {
  }

  // Constructor for InputDefault (internal use)
  ArgTransform(TransformKind k, const std::string &arg, const plugin::Argument &defValue)
      : kind(k), argName(arg), newName(""), defaultArg(defValue), b64Encode(false)
  {
  }

  // Constructor for InputStdin and OutputStdout
  ArgTransform(TransformKind k, const std::string &arg, bool encode)
      : kind(k), argName(arg), newName(""), defaultArg(), b64Encode(encode)
  {
  }
};

// Helper functions to create transforms with cleaner syntax

// InputRename: Rename an argument from rpcName to argName
inline ArgTransform InputRename(const std::string &rpcName, const std::string &argName)
{
  return ArgTransform(ArgTransform::InputRename, rpcName, argName);
}

// InputDefault: Set a default value for an argument if not provided
inline ArgTransform InputDefault(const std::string &argName, const char *defaultValue)
{
  return ArgTransform(ArgTransform::InputDefault, argName, plugin::Argument(defaultValue));
}

inline ArgTransform InputDefault(const std::string &argName, const std::string &defaultValue)
{
  return ArgTransform(ArgTransform::InputDefault, argName, plugin::Argument(defaultValue));
}

inline ArgTransform InputDefault(const std::string &argName, bool defaultValue)
{
  return ArgTransform(ArgTransform::InputDefault, argName, plugin::Argument(defaultValue));
}

inline ArgTransform InputDefault(const std::string &argName, int defaultValue)
{
  return ArgTransform(ArgTransform::InputDefault, argName, plugin::Argument(static_cast<long long>(defaultValue)));
}

inline ArgTransform InputDefault(const std::string &argName, long long defaultValue)
{
  return ArgTransform(ArgTransform::InputDefault, argName, plugin::Argument(defaultValue));
}

inline ArgTransform InputDefault(const std::string &argName, double defaultValue)
{
  return ArgTransform(ArgTransform::InputDefault, argName, plugin::Argument(defaultValue));
}

// InputStdin: Read input argument and write to stdin (optionally base64 decoding)
inline ArgTransform InputStdin(const std::string &argName, bool b64Encode)
{
  return ArgTransform(ArgTransform::InputStdin, argName, b64Encode);
}

// OutputStdout: Read from stdout and write to output argument (optionally base64 encoding)
inline ArgTransform OutputStdout(const std::string &argName, bool b64Encode)
{
  return ArgTransform(ArgTransform::OutputStdout, argName, b64Encode);
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
