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

using std::string;

void RpcServer::process_request(const string &request_data)
{
  try
  {
    // Parse the JSON request
    auto parse_result = zjson::from_str<RpcRequest>(request_data);

    if (!parse_result.has_value())
    {
      ErrorDetails error{
          RpcErrorCode::PARSE_ERROR,
          string("Failed to parse command request: ") + parse_result.error().what(),
          zstd::optional<zjson::Value>()};

      RpcResponse response;
      response.jsonrpc = "2.0";
      response.result = zstd::optional<zjson::Value>();
      response.error = zstd::optional<ErrorDetails>(error);
      response.id = 0; // No request ID available

      print_response(response);
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

      print_response(response);
      return;
    }

    // Convert JSON params to ArgumentMap
    plugin::ArgumentMap args;
    if (request.params.has_value())
    {
      args = convert_json_params_to_argument_map(request.params.value());
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
      zjson::Value result_json;

      const ast::Node &ast_object = context.get_object();
      if (ast_object)
      {
        // Convert AST object to zjson::Value
        result_json = convert_ast_to_json(ast_object);
      }
      else
      {
        // Fallback to output content if no AST object is set
        string output = context.get_output_content();
        result_json = convert_output_to_json(output);
      }

      result_json.add_to_object("success", zjson::Value(context.get_error_content().empty()));
      response.result = zstd::optional<zjson::Value>(result_json);
      response.error = zstd::optional<ErrorDetails>();
    }
    else
    {
      // Error occurred
      string error_output = context.get_error_content();
      ErrorDetails error{
          result, // Internal error
          "Command execution failed",
          error_output.empty() ? zstd::optional<zjson::Value>() : zstd::optional<zjson::Value>(zjson::Value(error_output))};

      response.result = zstd::optional<zjson::Value>();
      response.error = zstd::optional<ErrorDetails>(error);
    }

    print_response(response);
  }
  catch (const std::exception &e)
  {
    ErrorDetails error{
        RpcErrorCode::PARSE_ERROR,
        "Failed to parse command request: " + string(e.what()),
        zstd::optional<zjson::Value>()};

    RpcResponse response;
    response.jsonrpc = "2.0";
    response.result = zstd::optional<zjson::Value>();
    response.error = zstd::optional<ErrorDetails>(error);
    response.id = 0; // No request ID available

    print_response(response);
  }
}

RpcRequest RpcServer::parse_rpc_request(const zjson::Value &json)
{
  if (!json.is_object())
  {
    throw std::runtime_error("JSON-RPC request must be an object");
  }

  auto result = zjson::from_value<RpcRequest>(json);
  if (!result.has_value())
  {
    throw std::runtime_error(string("Failed to parse RPC request: ") + result.error().what());
  }
  return result.value();
}

string RpcServer::camel_case_to_kebab_case(const string &input)
{
  string result;
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

plugin::ArgumentMap RpcServer::convert_json_params_to_argument_map(const zjson::Value &params)
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
    const string kebab_key = camel_case_to_kebab_case(pair.first);
    const zjson::Value &value = pair.second;

    if (value.is_bool())
    {
      args[kebab_key] = plugin::Argument(value.as_bool());
    }
    else if (value.is_integer())
    {
      args[kebab_key] = plugin::Argument(value.as_int64());
    }
    else if (value.is_double())
    {
      args[kebab_key] = plugin::Argument(value.as_double());
    }
    else if (value.is_string())
    {
      args[kebab_key] = plugin::Argument(value.as_string());
    }
    // For other types (null, array, object), convert to string representation
    else
    {
      auto str_result = zjson::to_string(value);
      args[kebab_key] = plugin::Argument(str_result.value_or(""));
    }
  }

  return args;
}

zjson::Value RpcServer::convert_output_to_json(const string &output)
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

zjson::Value RpcServer::convert_ast_to_json(const ast::Node &ast_node)
{
  if (!ast_node)
  {
    return zjson::Value(); // null
  }

  switch (ast_node->kind())
  {
  case ast::Ast::Null:
    return zjson::Value(); // null

  case ast::Ast::Boolean:
    return zjson::Value(ast_node->as_bool());

  case ast::Ast::Integer:
    return zjson::Value(static_cast<int>(ast_node->as_integer()));

  case ast::Ast::Number:
    return zjson::Value(ast_node->as_number());

  case ast::Ast::String:
    return zjson::Value(ast_node->as_string());

  case ast::Ast::Array:
  {
    zjson::Value array_value = zjson::Value::create_array();
    const std::vector<ast::Node> &ast_array = ast_node->as_array();
    array_value.reserve_array(ast_array.size());

    for (size_t i = 0; i < ast_array.size(); ++i)
    {
      array_value.add_to_array(convert_ast_to_json(ast_array[i]));
    }
    return array_value;
  }

  case ast::Ast::Object:
  {
    zjson::Value object_value = zjson::Value::create_object();
    const ast::ObjMap &ast_object = ast_node->as_object();

    for (ast::ObjMap::const_iterator it = ast_object.begin(); it != ast_object.end(); ++it)
    {
      object_value.add_to_object(it->first, convert_ast_to_json(it->second));
    }
    return object_value;
  }

  default:
    return zjson::Value(); // null for unknown types
  }
}

void RpcServer::print_response(const RpcResponse &response)
{
  std::lock_guard<std::mutex> lock(response_mutex);

  // Log errors to the log file
  if (response.error.has_value())
  {
    const ErrorDetails &error = response.error.value();
    LOG_ERROR("%s", error.message.c_str());
  }

  string json_string = serialize_json(rpc_response_to_json(response));

  // Print errors to stderr, success responses to stdout
  if (response.error.has_value())
  {
    std::cerr << json_string << std::endl;
  }
  else
  {
    std::cout << json_string << std::endl;
  }
}

// Static utility methods
string RpcServer::serialize_json(const zjson::Value &val, bool prettify)
{
  auto result = prettify ? zjson::to_string_pretty(val) : zjson::to_string(val);
  if (!result.has_value())
  {
    throw std::runtime_error(string("Failed to serialize JSON: ") + result.error().what());
  }
  return result.value();
}

RpcRequest RpcServer::parse_rpc_request_from_json(const zjson::Value &json)
{
  if (!json.is_object())
  {
    throw std::runtime_error("JSON-RPC request must be an object");
  }

  auto result = zjson::from_value<RpcRequest>(json);
  if (!result.has_value())
  {
    throw std::runtime_error(string("Failed to parse RPC request: ") + result.error().what());
  }
  return result.value();
}

zjson::Value RpcServer::rpc_response_to_json(const RpcResponse &response)
{
  auto result = zjson::to_value(response);
  return result.value_or(zjson::Value::create_object());
}

zjson::Value RpcServer::error_details_to_json(const ErrorDetails &error)
{
  auto result = zjson::to_value(error);
  return result.value_or(zjson::Value::create_object());
}

void RpcServer::send_notification(const RpcNotification &notification)
{
  string json_string = serialize_json(zjson::to_value(notification).value());
  std::cout << json_string << std::endl;
}
