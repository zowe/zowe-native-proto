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

#include "builder.hpp"
#include "rpcio.hpp"
#include "server.hpp"
#include "logger.hpp"
#include "../c/zbase64.h"
#include "../c/zjson.hpp"
#include <sstream>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>

using std::string;

// Forward declaration from server.hpp
struct RpcNotification;

CommandBuilder::CommandBuilder(CommandHandler handler)
    : handler_(handler), transforms_()
{
}

CommandBuilder &CommandBuilder::rename_arg(const string &from, const string &to)
{
  transforms_.push_back(ArgTransform(ArgTransform::RenameArg, from, to));
  return *this;
}

CommandBuilder &CommandBuilder::set_default(const string &arg_name, const char *default_value)
{
  transforms_.push_back(ArgTransform(ArgTransform::SetDefault, arg_name, plugin::Argument(default_value)));
  return *this;
}

CommandBuilder &CommandBuilder::set_default(const string &arg_name, const string &default_value)
{
  transforms_.push_back(ArgTransform(ArgTransform::SetDefault, arg_name, plugin::Argument(default_value)));
  return *this;
}

CommandBuilder &CommandBuilder::set_default(const string &arg_name, bool default_value)
{
  transforms_.push_back(ArgTransform(ArgTransform::SetDefault, arg_name, plugin::Argument(default_value)));
  return *this;
}

CommandBuilder &CommandBuilder::set_default(const string &arg_name, int default_value)
{
  transforms_.push_back(ArgTransform(ArgTransform::SetDefault, arg_name, plugin::Argument(static_cast<long long>(default_value))));
  return *this;
}

CommandBuilder &CommandBuilder::set_default(const string &arg_name, long long default_value)
{
  transforms_.push_back(ArgTransform(ArgTransform::SetDefault, arg_name, plugin::Argument(default_value)));
  return *this;
}

CommandBuilder &CommandBuilder::set_default(const string &arg_name, double default_value)
{
  transforms_.push_back(ArgTransform(ArgTransform::SetDefault, arg_name, plugin::Argument(default_value)));
  return *this;
}

CommandBuilder &CommandBuilder::handle_fifo(const string &rpc_id, const string &arg_name, FifoMode mode, bool defer)
{
  transforms_.push_back(ArgTransform(ArgTransform::HandleFifo, rpc_id, arg_name, mode, defer));
  return *this;
}

CommandBuilder &CommandBuilder::read_stdout(const string &arg_name, bool b64_encode)
{
  transforms_.push_back(ArgTransform(ArgTransform::ReadStdout, arg_name, b64_encode));
  return *this;
}

CommandBuilder &CommandBuilder::write_stdin(const string &arg_name, bool b64_decode)
{
  transforms_.push_back(ArgTransform(ArgTransform::WriteStdin, arg_name, b64_decode));
  return *this;
}

CommandBuilder &CommandBuilder::flatten_obj(const string &arg_name)
{
  transforms_.push_back(ArgTransform(ArgTransform::FlattenObj, arg_name));
  return *this;
}

void CommandBuilder::apply_input_transforms(MiddlewareContext &context) const
{
  plugin::ArgumentMap &args = context.mutable_arguments();

  for (std::vector<ArgTransform>::const_iterator it = transforms_.begin(); it != transforms_.end(); ++it)
  {
    // Find the argument
    plugin::ArgumentMap::iterator arg_it = args.find(it->arg_name);

    switch (it->kind)
    {
    case ArgTransform::RenameArg:
    {
      // Rename: argument must exist
      if (arg_it == args.end())
      {
        LOG_WARN("Argument '%s' not found for rename transform, skipping", it->arg_name.c_str());
        continue;
      }

      plugin::Argument value = arg_it->second;
      args.erase(arg_it);
      args[it->renamed_to] = value;
      break;
    }

    case ArgTransform::SetDefault:
    {
      // Set default value if argument doesn't exist
      if (arg_it == args.end())
      {
        args[it->arg_name] = it->default_value;
      }
      break;
    }

    case ArgTransform::WriteStdin:
    {
      // WriteStdin: Read argument value and write to stdin
      // If base64 is true, decode base64 before writing to stdin
      if (arg_it != args.end())
      {
        try
        {
          string data = arg_it->second.get_string_value();

          // Decode base64 if requested
          if (it->base64)
          {
            data = zbase64::decode(data);
          }

          // Write to stdin
          context.set_input_content(data);

          // Remove the argument from args
          args.erase(arg_it);
        }
        catch (const std::exception &e)
        {
          string errMsg = string("Failed to process WriteStdin transform: ") + e.what();
          context.errln(errMsg.c_str());
          LOG_ERROR("%s", errMsg.c_str());
        }
      }
      break;
    }

    case ArgTransform::FlattenObj:
    {
      // FlattenObj: Flatten a JSON object argument into the argument map
      if (arg_it != args.end())
      {
        try
        {
          // Parse the JSON string into a zjson::Value
          const auto parse_result = zjson::from_str<zjson::Value>(arg_it->second.get_string_value());
          if (!parse_result.has_value())
          {
            string errMsg = string("Failed to parse JSON for FlattenObj transform: ") + parse_result.error().what();
            context.errln(errMsg.c_str());
            LOG_ERROR("%s", errMsg.c_str());
            break;
          }

          const auto &jsonValue = parse_result.value();

          // Verify it's an object
          if (!jsonValue.is_object())
          {
            context.errln("FlattenObj transform requires a JSON object");
            LOG_ERROR("FlattenObj transform requires a JSON object");
            break;
          }

          // Add each property from the object to the argument map
          for (const auto &property : jsonValue.as_object())
          {
            const auto &key = property.first;
            const auto &value = property.second;

            // Convert zjson::Value to plugin::Argument
            if (value.is_bool())
            {
              args[key] = plugin::Argument(value.as_bool());
            }
            else if (value.is_integer())
            {
              args[key] = plugin::Argument(value.as_int64());
            }
            else if (value.is_double())
            {
              args[key] = plugin::Argument(value.as_double());
            }
            else if (value.is_string())
            {
              args[key] = plugin::Argument(value.as_string());
            }
          }

          // Remove the original object argument
          args.erase(arg_it);
        }
        catch (const std::exception &e)
        {
          string errMsg = string("Failed to process FlattenObj transform: ") + e.what();
          context.errln(errMsg.c_str());
          LOG_ERROR("%s", errMsg.c_str());
        }
      }
      break;
    }

    case ArgTransform::HandleFifo:
    {
      // HandleFifo: Create FIFO pipe and send appropriate notification
      // Find the RPC ID argument
      plugin::ArgumentMap::iterator rpc_it = args.find(it->rpc_id);
      if (rpc_it != args.end())
      {
        try
        {
          // Get stream_id from the argument
          const long long *stream_id_ptr = rpc_it->second.get_int();
          if (stream_id_ptr == nullptr)
          {
            context.errln("HandleFifo: RPC ID argument is not an integer");
            LOG_ERROR("HandleFifo: RPC ID argument is not an integer");
            break;
          }
          long long stream_id = *stream_id_ptr;

          // Create pipe path: /tmp/zowe-native-proto_{uid}_{pid}_{stream_id}_fifo
          const char *tmp_dir = std::getenv("TMPDIR");
          if (tmp_dir == nullptr || tmp_dir[0] == '\0')
          {
            tmp_dir = "/tmp";
          }

          std::ostringstream pipe_path_stream;
          pipe_path_stream << tmp_dir << "/zowe-native-proto_"
                           << geteuid() << "_"
                           << getpid() << "_"
                           << stream_id << "_fifo";

          it->pipe_path = pipe_path_stream.str();

          // Remove any existing pipe (ignore errors if it doesn't exist)
          if (unlink(it->pipe_path.c_str()) != 0 && errno != ENOENT)
          {
            string errMsg = string("Failed to delete existing FIFO pipe: ") + it->pipe_path + ", errno: " + std::to_string(errno);
            context.errln(errMsg.c_str());
            LOG_ERROR("%s", errMsg.c_str());
            break;
          }

          // Create the FIFO pipe
          if (mkfifo(it->pipe_path.c_str(), 0600) != 0)
          {
            string errMsg = string("Failed to create FIFO pipe: ") + it->pipe_path + ", errno: " + std::to_string(errno);
            context.errln(errMsg.c_str());
            LOG_ERROR("%s", errMsg.c_str());
            break;
          }

          LOG_DEBUG("Created FIFO pipe: %s", it->pipe_path.c_str());

          // Set the pipe path as the output argument
          args[it->arg_name] = plugin::Argument(it->pipe_path);

          // Create notification based on mode
          zjson::Value params_obj = zjson::Value::create_object();
          params_obj.add_to_object("id", zjson::Value(static_cast<int>(stream_id)));
          params_obj.add_to_object("pipePath", zjson::Value(it->pipe_path));

          RpcNotification notification = RpcNotification{
              .jsonrpc = "2.0",
              .method = (it->fifo_mode == FifoMode::GET) ? "receiveStream" : "sendStream",
              .params = zstd::optional<zjson::Value>(params_obj),
          };

          // If defer is true, store the notification for later
          // Otherwise, send it immediately
          if (it->defer)
          {
            context.set_pending_notification(notification);
          }
          else
          {
            RpcServer::send_notification(notification);
          }
        }
        catch (const std::exception &e)
        {
          string errMsg = string("Failed to process HandleFifo transform: ") + e.what();
          context.errln(errMsg.c_str());
          LOG_ERROR("%s", errMsg.c_str());
        }
      }
      break;
    }

    default:
      // Not an input transform, skip
      break;
    }
  }
}

void CommandBuilder::apply_output_transforms(MiddlewareContext &context) const
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

  for (std::vector<ArgTransform>::const_iterator it = transforms_.begin(); it != transforms_.end(); ++it)
  {
    // Get the field value from the object if it exists
    ast::Node field_value = obj->get(it->arg_name);

    switch (it->kind)
    {
    case ArgTransform::ReadStdout:
    {
      // ReadStdout: Read from stdout and write to output argument
      // If base64 is true, encode base64 before writing to output
      try
      {
        string data = context.get_output_content();

        // Encode base64 if requested
        if (it->base64)
        {
          data = zbase64::encode(data);
        }

        // Set the output field
        obj->set(it->arg_name, ast::Ast::string(data));
      }
      catch (const std::exception &e)
      {
        string errMsg = string("Failed to process ReadStdout transform: ") + e.what();
        context.errln(errMsg.c_str());
        LOG_ERROR("%s", errMsg.c_str());
      }
      break;
    }

    case ArgTransform::HandleFifo:
    {
      // HandleFifo cleanup: Remove the FIFO pipe after command execution
      if (!it->pipe_path.empty())
      {
        // Remove the pipe (ignore errors if already removed)
        if (unlink(it->pipe_path.c_str()) == 0)
        {
          LOG_DEBUG("Cleaned up FIFO pipe: %s", it->pipe_path.c_str());
        }
        else if (errno != ENOENT)
        {
          LOG_ERROR("Failed to delete FIFO pipe: %s, errno: %d", it->pipe_path.c_str(), errno);
        }
      }
      break;
    }

    default:
      // Not an output transform, skip
      break;
    }
  }
}
