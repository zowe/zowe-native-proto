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
#include "validator.hpp"
#include <string>
#include <vector>
#include <memory>

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
  std::string arg_name;
  std::string renamed_to{};          // Used for RenameArg
  plugin::Argument default_value{};  // Used for SetDefault
  bool base64{false};                // Used for WriteStdin and ReadStdout
  std::string rpc_id_param{};        // Used for HandleFifo (stream ID parameter name)
  FifoMode fifo_mode{FifoMode::GET}; // Used for HandleFifo
  bool defer{false};                 // Used for HandleFifo (defer notification until content length is known)

  // Constructor for RenameArg
  ArgTransform(TransformKind k, const std::string &from, const std::string &to)
      : kind(k), arg_name(from), renamed_to(to)
  {
  }

  // Constructor for SetDefault
  ArgTransform(TransformKind k, const std::string &arg, const plugin::Argument &defValue)
      : kind(k), arg_name(arg), default_value(defValue)
  {
  }

  // Constructor for WriteStdin and ReadStdout
  ArgTransform(TransformKind k, const std::string &arg, bool b64)
      : kind(k), arg_name(arg), base64(b64)
  {
  }

  // Constructor for HandleFifo
  ArgTransform(TransformKind k, const std::string &rpc_id_param, const std::string &arg, FifoMode mode, bool defer_arg = false)
      : kind(k), arg_name(arg), rpc_id_param(rpc_id_param), fifo_mode(mode), defer(defer_arg)
  {
  }

  // Constructor for FlattenObj
  ArgTransform(TransformKind k, const std::string &arg)
      : kind(k), arg_name(arg)
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
  CommandBuilder &set_default(const std::string &arg_name, const char *default_value);
  CommandBuilder &set_default(const std::string &arg_name, const std::string &default_value);
  CommandBuilder &set_default(const std::string &arg_name, bool default_value);
  CommandBuilder &set_default(const std::string &arg_name, int default_value);
  CommandBuilder &set_default(const std::string &arg_name, long long default_value);
  CommandBuilder &set_default(const std::string &arg_name, double default_value);

  // Handle FIFO pipe creation for streaming
  // mode: FifoMode::Get for download, FifoMode::Put for upload
  // defer: if true, defer notification until content length is known (via set_content_len)
  CommandBuilder &handle_fifo(const std::string &rpc_id_param, const std::string &arg_name, FifoMode mode, bool defer = false);

  // Capture stdout and write to output argument (optionally base64 encoded)
  CommandBuilder &read_stdout(const std::string &arg_name, bool b64_encode = false);

  // Read input argument and write to stdin (optionally base64 decoded)
  CommandBuilder &write_stdin(const std::string &arg_name, bool b64_decode = false);

  // Flatten a JSON object argument into the argument map (recursion not supported)
  CommandBuilder &flatten_obj(const std::string &arg_name);

  // Validate both request and response using separate schemas
  template <typename RequestT, typename ResponseT>
  CommandBuilder &validate(bool allow_unknown_fields = false)
  {
    request_validator_ = [allow_unknown_fields](const zjson::Value &params) -> validator::ValidationResult
    {
      return validator::validate_schema(params, validator::SchemaRegistry<RequestT>::fields, allow_unknown_fields);
    };
    response_validator_ = [allow_unknown_fields](const zjson::Value &params) -> validator::ValidationResult
    {
      return validator::validate_schema(params, validator::SchemaRegistry<ResponseT>::fields, allow_unknown_fields);
    };
    return *this;
  }

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

  // Get the request validator (may be null)
  validator::ValidatorFn get_request_validator() const
  {
    return request_validator_;
  }

  // Get the response validator (may be null)
  validator::ValidatorFn get_response_validator() const
  {
    return response_validator_;
  }

  // Apply input transforms to the context before command execution
  void apply_input_transforms(MiddlewareContext &context) const;

  // Apply output transforms to the context after command execution
  void apply_output_transforms(MiddlewareContext &context) const;

private:
  CommandHandler handler_;
  std::vector<ArgTransform> transforms_;
  validator::ValidatorFn request_validator_;
  validator::ValidatorFn response_validator_;
};

#endif
