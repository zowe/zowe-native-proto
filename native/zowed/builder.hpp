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

#ifndef BUILDER_HPP
#define BUILDER_HPP

#include "../c/extend/plugin.hpp"
#include <string>
#include <vector>

// Forward declarations
class CommandDispatcher;
class MiddlewareContext;

// FIFO mode enumeration
enum FifoMode
{
  GET, // Download/read mode - server reads from command, client receives stream
  PUT  // Upload/write mode - client sends stream, server writes to command
};

// Argument transformation types
struct ArgTransform
{
  enum TransformKind
  {
    RenameArg,  // Rename an input argument
    SetDefault, // Set a default value for an input argument
    WriteStdin, // Write input argument to stdin
    ReadStdout, // Read stdout to output argument
    HandleFifo, // Handle FIFO pipe creation for streaming
    FlattenObj  // Flatten a JSON object argument into the argument map
  };

  TransformKind kind;
  std::string argName;
  std::string renamedTo;         // Used for RenameArg
  plugin::Argument defaultValue; // Used for SetDefault
  bool base64;                   // Used for WriteStdin and ReadStdout
  std::string rpcId;             // Used for HandleFifo (stream ID parameter name)
  FifoMode fifoMode;             // Used for HandleFifo
  mutable std::string pipePath;  // Used for HandleFifo (mutable for cleanup in output phase)
  bool defer;                    // Used for HandleFifo (defer notification until content length is known)

  // Constructor for RenameArg
  ArgTransform(TransformKind k, const std::string &from, const std::string &to)
      : kind(k), argName(from), renamedTo(to), defaultValue(), base64(false), rpcId(), fifoMode(FifoMode::GET), pipePath(), defer(false)
  {
  }

  // Constructor for SetDefault
  ArgTransform(TransformKind k, const std::string &arg, const plugin::Argument &defValue)
      : kind(k), argName(arg), renamedTo(""), defaultValue(defValue), base64(false), rpcId(), fifoMode(FifoMode::GET), pipePath(), defer(false)
  {
  }

  // Constructor for WriteStdin and ReadStdout
  ArgTransform(TransformKind k, const std::string &arg, bool b64)
      : kind(k), argName(arg), renamedTo(""), defaultValue(), base64(b64), rpcId(), fifoMode(FifoMode::GET), pipePath(), defer(false)
  {
  }

  // Constructor for HandleFifo
  ArgTransform(TransformKind k, const std::string &rpc_id, const std::string &arg, FifoMode mode, bool defer_arg = false)
      : kind(k), argName(arg), renamedTo(""), defaultValue(), base64(false), rpcId(rpc_id), fifoMode(mode), pipePath(), defer(defer_arg)
  {
  }

  // Constructor for FlattenObj
  ArgTransform(TransformKind k, const std::string &arg)
      : kind(k), argName(arg), renamedTo(""), defaultValue(), base64(false), rpcId(), fifoMode(FifoMode::GET), pipePath(), defer(false)
  {
  }
};

// CommandBuilder class for fluent command registration
class CommandBuilder
{
public:
  // CommandHandler type from plugin.hpp
  typedef plugin::CommandProviderImpl::CommandRegistrationContext::CommandHandler CommandHandler;

  // Constructor takes the command handler
  explicit CommandBuilder(CommandHandler handler);

  // Rename an input argument (e.g., from RPC name to internal name)
  CommandBuilder &rename_arg(const std::string &from, const std::string &to);

  // Set a default value for an argument if not provided
  CommandBuilder &set_default(const std::string &argName, const char *defaultValue);
  CommandBuilder &set_default(const std::string &argName, const std::string &defaultValue);
  CommandBuilder &set_default(const std::string &argName, bool defaultValue);
  CommandBuilder &set_default(const std::string &argName, int defaultValue);
  CommandBuilder &set_default(const std::string &argName, long long defaultValue);
  CommandBuilder &set_default(const std::string &argName, double defaultValue);

  // Handle FIFO pipe creation for streaming
  // mode: FifoMode::Get for download, FifoMode::Put for upload
  // defer: if true, defer notification until content length is known (via set_content_len)
  CommandBuilder &handle_fifo(const std::string &rpcId, const std::string &argName, FifoMode mode, bool defer = false);

  // Capture stdout and write to output argument (optionally base64 encoded)
  CommandBuilder &read_stdout(const std::string &argName, bool b64Encode = false);

  // Read input argument and write to stdin (optionally base64 decoded)
  CommandBuilder &write_stdin(const std::string &argName, bool b64Decode = false);

  // Flatten a JSON object argument into the argument map
  CommandBuilder &flatten_obj(const std::string &argName);

  // Get the command handler
  CommandHandler get_handler() const
  {
    return handler_;
  }

  // Get the list of transforms
  const std::vector<ArgTransform> &get_transforms() const
  {
    return transforms_;
  }

  // Apply input transforms to the context before command execution
  void apply_input_transforms(MiddlewareContext &context) const;

  // Apply output transforms to the context after command execution
  void apply_output_transforms(MiddlewareContext &context) const;

private:
  CommandHandler handler_;
  std::vector<ArgTransform> transforms_;
};

#endif
