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

// Forward declaration from server.hpp
struct RpcNotification;

CommandBuilder::CommandBuilder(CommandHandler handler)
    : handler_(handler), transforms_()
{
}

CommandBuilder &CommandBuilder::rename_arg(const std::string &from, const std::string &to)
{
  transforms_.push_back(ArgTransform(ArgTransform::RenameArg, from, to));
  return *this;
}

CommandBuilder &CommandBuilder::set_default(const std::string &argName, const char *defaultValue)
{
  transforms_.push_back(ArgTransform(ArgTransform::SetDefault, argName, plugin::Argument(defaultValue)));
  return *this;
}

CommandBuilder &CommandBuilder::set_default(const std::string &argName, const std::string &defaultValue)
{
  transforms_.push_back(ArgTransform(ArgTransform::SetDefault, argName, plugin::Argument(defaultValue)));
  return *this;
}

CommandBuilder &CommandBuilder::set_default(const std::string &argName, bool defaultValue)
{
  transforms_.push_back(ArgTransform(ArgTransform::SetDefault, argName, plugin::Argument(defaultValue)));
  return *this;
}

CommandBuilder &CommandBuilder::set_default(const std::string &argName, int defaultValue)
{
  transforms_.push_back(ArgTransform(ArgTransform::SetDefault, argName, plugin::Argument(static_cast<long long>(defaultValue))));
  return *this;
}

CommandBuilder &CommandBuilder::set_default(const std::string &argName, long long defaultValue)
{
  transforms_.push_back(ArgTransform(ArgTransform::SetDefault, argName, plugin::Argument(defaultValue)));
  return *this;
}

CommandBuilder &CommandBuilder::set_default(const std::string &argName, double defaultValue)
{
  transforms_.push_back(ArgTransform(ArgTransform::SetDefault, argName, plugin::Argument(defaultValue)));
  return *this;
}

CommandBuilder &CommandBuilder::handle_fifo(const std::string &rpcId, const std::string &argName, FifoMode mode, bool defer)
{
  transforms_.push_back(ArgTransform(ArgTransform::HandleFifo, rpcId, argName, mode, defer));
  return *this;
}

CommandBuilder &CommandBuilder::read_stdout(const std::string &argName, bool b64Encode)
{
  transforms_.push_back(ArgTransform(ArgTransform::ReadStdout, argName, b64Encode));
  return *this;
}

CommandBuilder &CommandBuilder::write_stdin(const std::string &argName, bool b64Decode)
{
  transforms_.push_back(ArgTransform(ArgTransform::WriteStdin, argName, b64Decode));
  return *this;
}

CommandBuilder &CommandBuilder::flatten_obj(const std::string &argName)
{
  transforms_.push_back(ArgTransform(ArgTransform::FlattenObj, argName));
  return *this;
}

void CommandBuilder::apply_input_transforms(MiddlewareContext &context) const
{
  plugin::ArgumentMap &args = context.mutable_arguments();

  for (std::vector<ArgTransform>::const_iterator it = transforms_.begin(); it != transforms_.end(); ++it)
  {
    // Find the argument
    plugin::ArgumentMap::iterator arg_it = args.find(it->argName);

    switch (it->kind)
    {
    case ArgTransform::RenameArg:
    {
      // Rename: argument must exist
      if (arg_it == args.end())
      {
        continue; // Argument not found, skip rename
      }

      plugin::Argument value = arg_it->second;
      args.erase(arg_it);
      args[it->renamedTo] = value;
      break;
    }

    case ArgTransform::SetDefault:
    {
      // Set default value if argument doesn't exist
      if (arg_it == args.end())
      {
        args[it->argName] = it->defaultValue;
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
          std::string data = arg_it->second.get_string_value();

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
          std::string errMsg = std::string("Failed to process WriteStdin transform: ") + e.what();
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
            std::string errMsg = std::string("Failed to parse JSON for FlattenObj transform: ") + parse_result.error().what();
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
          std::string errMsg = std::string("Failed to process FlattenObj transform: ") + e.what();
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
      plugin::ArgumentMap::iterator rpc_it = args.find(it->rpcId);
      if (rpc_it != args.end())
      {
        try
        {
          // Get streamId from the argument
          const long long *streamIdPtr = rpc_it->second.get_int();
          if (streamIdPtr == nullptr)
          {
            context.errln("HandleFifo: RPC ID argument is not an integer");
            LOG_ERROR("HandleFifo: RPC ID argument is not an integer");
            break;
          }
          long long streamId = *streamIdPtr;

          // Create pipe path: /tmp/zowe-native-proto_{uid}_{pid}_{streamId}_fifo
          const char *tmpDir = std::getenv("TMPDIR");
          if (tmpDir == nullptr || tmpDir[0] == '\0')
          {
            tmpDir = "/tmp";
          }

          std::ostringstream pipePathStream;
          pipePathStream << tmpDir << "/zowe-native-proto_"
                         << geteuid() << "_"
                         << getpid() << "_"
                         << streamId << "_fifo";

          it->pipePath = pipePathStream.str();

          // Remove any existing pipe (ignore errors if it doesn't exist)
          if (unlink(it->pipePath.c_str()) != 0 && errno != ENOENT)
          {
            std::string errMsg = std::string("Failed to delete existing FIFO pipe: ") + it->pipePath;
            context.errln(errMsg.c_str());
            LOG_ERROR("%s", errMsg.c_str());
            break;
          }

          // Create the FIFO pipe
          if (mkfifo(it->pipePath.c_str(), 0600) != 0)
          {
            std::string errMsg = std::string("Failed to create FIFO pipe: ") + it->pipePath;
            context.errln(errMsg.c_str());
            LOG_ERROR("%s", errMsg.c_str());
            break;
          }

          LOG_DEBUG("Created FIFO pipe: %s", it->pipePath.c_str());

          // Set the pipe path as the output argument
          args[it->argName] = plugin::Argument(it->pipePath);

          // Create notification based on mode
          zjson::Value paramsObj = zjson::Value::create_object();
          paramsObj.add_to_object("id", zjson::Value(static_cast<int>(streamId)));
          paramsObj.add_to_object("pipePath", zjson::Value(it->pipePath));

          RpcNotification notification = RpcNotification{
              .jsonrpc = "2.0",
              .method = (it->fifoMode == FifoMode::GET) ? "receiveStream" : "sendStream",
              .params = zstd::optional<zjson::Value>(paramsObj),
          };

          // If defer is true, store the notification for later
          // Otherwise, send it immediately
          if (it->defer)
          {
            context.set_pending_notification(notification);
          }
          else
          {
            RpcServer::sendNotification(notification);
          }
        }
        catch (const std::exception &e)
        {
          std::string errMsg = std::string("Failed to process HandleFifo transform: ") + e.what();
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
    ast::Node fieldValue = obj->get(it->argName);

    switch (it->kind)
    {
    case ArgTransform::ReadStdout:
    {
      // ReadStdout: Read from stdout and write to output argument
      // If base64 is true, encode base64 before writing to output
      try
      {
        std::string data = context.get_output_content();

        // Encode base64 if requested
        if (it->base64)
        {
          data = zbase64::encode(data);
        }

        // Set the output field
        obj->set(it->argName, ast::Ast::string(data));
      }
      catch (const std::exception &e)
      {
        std::string errMsg = std::string("Failed to process ReadStdout transform: ") + e.what();
        context.errln(errMsg.c_str());
        LOG_ERROR("%s", errMsg.c_str());
      }
      break;
    }

    case ArgTransform::HandleFifo:
    {
      // HandleFifo cleanup: Remove the FIFO pipe after command execution
      if (!it->pipePath.empty())
      {
        // Remove the pipe (ignore errors if already removed)
        if (unlink(it->pipePath.c_str()) == 0)
        {
          LOG_DEBUG("Cleaned up FIFO pipe: %s", it->pipePath.c_str());
        }
        else
        {
          LOG_ERROR("Failed to delete FIFO pipe: %s", it->pipePath.c_str());
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
