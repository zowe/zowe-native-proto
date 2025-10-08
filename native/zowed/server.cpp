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
#include "logger.hpp"
#include <iostream>

void RpcServer::processRequest(const std::string &requestData)
{
  try
  {
    // Parse the JSON request
    auto parse_result = zjson::from_str<RpcRequest>(requestData);

    if (!parse_result.has_value())
    {
      ErrorDetails error{
          RpcErrorCode::PARSE_ERROR,
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
    CommandDispatcher &dispatcher = CommandDispatcher::get_instance();

    // Check if command is registered
    if (!dispatcher.has_command(request.method))
    {
      ErrorDetails error{
          RpcErrorCode::METHOD_NOT_FOUND,
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
      // Success - check if context has an object set, otherwise use output content
      zjson::Value resultJson;

      const ast::Node &astObject = context.get_object();
      if (astObject)
      {
        // Convert AST object to zjson::Value
        resultJson = convertAstToJson(astObject);
      }
      else
      {
        // Fallback to output content if no AST object is set
        std::string output = context.get_output_content();
        resultJson = convertOutputToJson(output);
      }

      resultJson.add_to_object("success", zjson::Value(context.get_error_content().empty()));
      response.result = zstd::optional<zjson::Value>(resultJson);
      response.error = zstd::optional<ErrorDetails>();
    }
    else
    {
      // Error occurred
      std::string errorOutput = context.get_error_content();
      ErrorDetails error{
          result, // Internal error
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
        RpcErrorCode::PARSE_ERROR,
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

std::string RpcServer::camelCaseToKebabCase(const std::string &input)
{
  std::string result;
  result.reserve(input.length() + 5); // Reserve some extra space for hyphens

  for (size_t i = 0; i < input.length(); ++i)
  {
    char c = input[i];

    // If uppercase letter, convert to lowercase and prepend hyphen (unless it's the first character)
    if (std::isupper(c))
    {
      if (i > 0)
      {
        result += '-';
      }
      result += std::tolower(c);
    }
    else
    {
      result += c;
    }
  }

  return result;
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
    // Convert camelCase keys to kebab-case
    const std::string kebabKey = camelCaseToKebabCase(pair.first);
    const zjson::Value &value = pair.second;

    if (value.is_bool())
    {
      args[kebabKey] = plugin::Argument(value.as_bool());
    }
    else if (value.is_integer())
    {
      args[kebabKey] = plugin::Argument(value.as_int64());
    }
    else if (value.is_double())
    {
      args[kebabKey] = plugin::Argument(value.as_double());
    }
    else if (value.is_string())
    {
      args[kebabKey] = plugin::Argument(value.as_string());
    }
    // For other types (null, array, object), convert to string representation
    else
    {
      auto str_result = zjson::to_string(value);
      args[kebabKey] = plugin::Argument(str_result.value_or(""));
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

zjson::Value RpcServer::convertAstToJson(const ast::Node &astNode)
{
  if (!astNode)
  {
    return zjson::Value(); // null
  }

  switch (astNode->kind())
  {
  case ast::Ast::Null:
    return zjson::Value(); // null

  case ast::Ast::Boolean:
    return zjson::Value(astNode->as_bool());

  case ast::Ast::Integer:
    return zjson::Value(static_cast<int>(astNode->as_integer()));

  case ast::Ast::Number:
    return zjson::Value(astNode->as_number());

  case ast::Ast::String:
    return zjson::Value(astNode->as_string());

  case ast::Ast::Array:
  {
    zjson::Value arrayValue = zjson::Value::create_array();
    const std::vector<ast::Node> &astArray = astNode->as_array();
    arrayValue.reserve_array(astArray.size());

    for (size_t i = 0; i < astArray.size(); ++i)
    {
      arrayValue.add_to_array(convertAstToJson(astArray[i]));
    }
    return arrayValue;
  }

  case ast::Ast::Object:
  {
    zjson::Value objectValue = zjson::Value::create_object();
    const ast::ObjMap &astObject = astNode->as_object();

    for (ast::ObjMap::const_iterator it = astObject.begin(); it != astObject.end(); ++it)
    {
      objectValue.add_to_object(it->first, convertAstToJson(it->second));
    }
    return objectValue;
  }

  default:
    return zjson::Value(); // null for unknown types
  }
}

void RpcServer::printResponse(const RpcResponse &response)
{
  std::lock_guard<std::mutex> lock(responseMutex);

  // Log errors to the log file
  if (response.error.has_value())
  {
    const ErrorDetails &error = response.error.value();
    LOG_ERROR("%s", error.message.c_str());
  }

  std::string jsonString = serializeJson(rpcResponseToJson(response));

  // Print errors to stderr, success responses to stdout
  if (response.error.has_value())
  {
    std::cerr << jsonString << std::endl;
  }
  else
  {
    std::cout << jsonString << std::endl;
  }
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

void RpcServer::sendNotification(const RpcNotification &notification)
{
  std::string jsonString = serializeJson(zjson::to_value(notification).value());
  std::cout << jsonString << std::endl;
}
