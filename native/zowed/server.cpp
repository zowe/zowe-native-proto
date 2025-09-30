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

#include "server.hpp"
#include "dispatcher.hpp"
#include <iostream>

// Static member definitions
RpcServer *RpcServer::instance = nullptr;
std::mutex RpcServer::instanceMutex;

RpcServer &RpcServer::getInstance()
{
  std::lock_guard<std::mutex> lock(instanceMutex);
  if (instance == nullptr)
  {
    instance = new RpcServer();
  }
  return *instance;
}

void RpcServer::processRequest(const std::string &requestData)
{
  try
  {
    // Parse the JSON request
    auto parse_result = zjson::from_str<RpcRequest>(requestData);

    if (!parse_result.has_value())
    {
      ErrorDetails error{
          -32700,
          std::string("Failed to parse command request: ") + parse_result.error().what(),
          zstd::optional<zjson::Value>()};

      RpcResponse response;
      response.jsonrpc = "2.0";
      response.result = zstd::optional<zjson::Value>();
      response.error = zstd::optional<ErrorDetails>(error);
      response.id = 0; // No request ID available

      printResponse(response);
      return;
    }

    RpcRequest request = parse_result.value();

    // Use CommandDispatcher singleton to handle the command
    CommandDispatcher &dispatcher = CommandDispatcher::getInstance();

    // Check if command is registered
    if (!dispatcher.has_command(request.method))
    {
      ErrorDetails error{
          -32601,
          "Unrecognized command " + request.method,
          zstd::optional<zjson::Value>()};

      RpcResponse response;
      response.jsonrpc = "2.0";
      response.result = zstd::optional<zjson::Value>();
      response.error = zstd::optional<ErrorDetails>(error);
      response.id = request.id;

      printResponse(response);
      return;
    }

    // Convert JSON params to ArgumentMap
    plugin::ArgumentMap args;
    if (request.params.has_value())
    {
      args = convertJsonParamsToArgumentMap(request.params.value());
    }

    // Create MiddlewareContext for the command
    MiddlewareContext context(request.method, args);

    // Dispatch the command
    int result = dispatcher.dispatch(request.method, context);

    RpcResponse response;
    response.jsonrpc = "2.0";
    response.id = request.id;

    if (result == 0)
    {
      // Success - get output and convert to JSON
      std::string output = context.get_output_content();
      zjson::Value resultJson = convertOutputToJson(output);

      response.result = zstd::optional<zjson::Value>(resultJson);
      response.error = zstd::optional<ErrorDetails>();
    }
    else
    {
      // Error occurred
      std::string errorOutput = context.get_error_content();
      ErrorDetails error{
          -32603, // Internal error
          "Command execution failed",
          errorOutput.empty() ? zstd::optional<zjson::Value>() : zstd::optional<zjson::Value>(zjson::Value(errorOutput))};

      response.result = zstd::optional<zjson::Value>();
      response.error = zstd::optional<ErrorDetails>(error);
    }

    printResponse(response);
  }
  catch (const std::exception &e)
  {
    ErrorDetails error{
        -32700,
        "Failed to parse command request: " + std::string(e.what()),
        zstd::optional<zjson::Value>()};

    RpcResponse response;
    response.jsonrpc = "2.0";
    response.result = zstd::optional<zjson::Value>();
    response.error = zstd::optional<ErrorDetails>(error);
    response.id = 0; // No request ID available

    printResponse(response);
  }
}

RpcRequest RpcServer::parseRpcRequest(const zjson::Value &json)
{
  if (!json.is_object())
  {
    throw std::runtime_error("JSON-RPC request must be an object");
  }

  auto result = zjson::from_value<RpcRequest>(json);
  if (!result.has_value())
  {
    throw std::runtime_error(std::string("Failed to parse RPC request: ") + result.error().what());
  }
  return result.value();
}

plugin::ArgumentMap RpcServer::convertJsonParamsToArgumentMap(const zjson::Value &params)
{
  plugin::ArgumentMap args;

  if (!params.is_object())
  {
    throw std::runtime_error("Invalid parameters - must be an object");
  }

  // Convert JSON object to ArgumentMap
  for (const auto &pair : params.as_object())
  {
    const std::string &key = pair.first;
    const zjson::Value &value = pair.second;

    if (value.is_bool())
    {
      args[key] = plugin::Argument(value.as_bool());
    }
    else if (value.is_number())
    {
      args[key] = plugin::Argument(static_cast<long long>(value.as_number()));
    }
    else if (value.is_string())
    {
      args[key] = plugin::Argument(value.as_string());
    }
    else if (value.is_array())
    {
      // Convert array to vector<string>
      std::vector<std::string> stringArray;
      for (const auto &arrayItem : value.as_array())
      {
        if (arrayItem.is_string())
        {
          stringArray.push_back(arrayItem.as_string());
        }
        else
        {
          // TODO Handle non-string values in arrays
        }
      }
      args[key] = plugin::Argument(stringArray);
    }
    // For other types (null, object), convert to string representation
    else
    {
      auto str_result = zjson::to_string(value);
      args[key] = plugin::Argument(str_result.value_or(""));
    }
  }

  return args;
}

zjson::Value RpcServer::convertOutputToJson(const std::string &output)
{
  if (!output.empty())
  {
    // Try to parse output as JSON, fallback to string
    auto parse_result = zjson::from_str<zjson::Value>(output);
    if (parse_result.has_value())
    {
      return parse_result.value();
    }
    else
    {
      return zjson::Value(output);
    }
  }
  else
  {
    return zjson::Value::create_object();
  }
}

void RpcServer::printResponse(const RpcResponse &response)
{
  std::lock_guard<std::mutex> lock(responseMutex);
  std::string jsonString = serializeJson(rpcResponseToJson(response));
  std::cout << jsonString << std::endl;
}

// Static utility methods
std::string RpcServer::serializeJson(const zjson::Value &val, bool prettify)
{
  auto result = prettify ? zjson::to_string_pretty(val) : zjson::to_string(val);
  if (!result.has_value())
  {
    throw std::runtime_error(std::string("Failed to serialize JSON: ") + result.error().what());
  }
  return result.value();
}

RpcRequest RpcServer::parseRpcRequestFromJson(const zjson::Value &json)
{
  if (!json.is_object())
  {
    throw std::runtime_error("JSON-RPC request must be an object");
  }

  auto result = zjson::from_value<RpcRequest>(json);
  if (!result.has_value())
  {
    throw std::runtime_error(std::string("Failed to parse RPC request: ") + result.error().what());
  }
  return result.value();
}

zjson::Value RpcServer::rpcResponseToJson(const RpcResponse &response)
{
  auto result = zjson::to_value(response);
  return result.value_or(zjson::Value::create_object());
}

zjson::Value RpcServer::errorDetailsToJson(const ErrorDetails &error)
{
  auto result = zjson::to_value(error);
  return result.value_or(zjson::Value::create_object());
}
