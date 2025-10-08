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

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <mutex>
#include "../c/zjson.hpp"
#include "../c/extend/plugin.hpp"
#include "../c/singleton.hpp"
#include "rpcio.hpp"

// JSON-RPC 2.0 Standard Error Codes
namespace RpcErrorCode
{
enum
{
  PARSE_ERROR = -32700,      // Invalid JSON was received
  INVALID_REQUEST = -32600,  // The JSON sent is not a valid Request object
  METHOD_NOT_FOUND = -32601, // The method does not exist / is not available
  INVALID_PARAMS = -32602,   // Invalid method parameter(s)
  INTERNAL_ERROR = -32603    // Internal JSON-RPC error
  // -32000 to -32099 are reserved for implementation-defined server-errors
};
}

struct RpcNotification
{
  std::string jsonrpc;
  std::string method;
  zstd::optional<zjson::Value> params;
};
ZJSON_DERIVE(RpcNotification, jsonrpc, method, params);

struct RpcRequest : RpcNotification
{
  int id;
};
ZJSON_DERIVE(RpcRequest, jsonrpc, method, params, id);

struct ErrorDetails
{
  int code;
  std::string message;
  zstd::optional<zjson::Value> data;
};
ZJSON_DERIVE(ErrorDetails, code, message, data);

struct RpcResponse
{
  std::string jsonrpc;
  zstd::optional<zjson::Value> result;
  zstd::optional<ErrorDetails> error;
  int id;
};
ZJSON_SERIALIZABLE(RpcResponse,
                   ZJSON_FIELD(RpcResponse, jsonrpc),
                   ZJSON_FIELD(RpcResponse, result).skip_serializing_if_none(),
                   ZJSON_FIELD(RpcResponse, error).skip_serializing_if_none(),
                   ZJSON_FIELD(RpcResponse, id));

/**
 * Thread-safe singleton RPC server that handles JSON-RPC request parsing,
 * command execution, and response serialization.
 */
class RpcServer : public Singleton<RpcServer>
{
  friend class Singleton<RpcServer>;

private:
  std::mutex response_mutex;

  // Private constructor for singleton
  RpcServer() = default;

  // Helper methods for JSON processing
  RpcRequest parse_rpc_request(const zjson::Value &json);
  std::string camel_case_to_kebab_case(const std::string &input);
  plugin::ArgumentMap convert_json_params_to_argument_map(const zjson::Value &params);
  zjson::Value convert_output_to_json(const std::string &output);
  zjson::Value convert_ast_to_json(const ast::Node &ast_node);
  void print_response(const RpcResponse &response);

public:
  /**
   * Process a JSON-RPC request string and return the response
   * This method is thread-safe and handles all JSON parsing, command execution,
   * and response serialization
   * @param request_data The raw JSON-RPC request string
   */
  void process_request(const std::string &request_data);

  /**
   * Utility function to serialize JSON with error handling
   * @param val The JSON value to serialize
   * @param prettify Whether to format the output with indentation
   * @return Serialized JSON string
   */
  static std::string serialize_json(const zjson::Value &val, bool prettify = false);

  /**
   * Helper function for parsing RPC requests from JSON
   * @param json The JSON value containing the RPC request
   * @return Parsed RpcRequest structure
   */
  static RpcRequest parse_rpc_request_from_json(const zjson::Value &json);

  /**
   * Helper function to convert RPC response to JSON
   * @param response The RpcResponse structure
   * @return JSON representation of the response
   */
  static zjson::Value rpc_response_to_json(const RpcResponse &response);

  /**
   * Helper function to convert error details to JSON
   * @param error The ErrorDetails structure
   * @return JSON representation of the error
   */
  static zjson::Value error_details_to_json(const ErrorDetails &error);

  /**
   * Helper function to send an RPC notification
   * Serializes the notification and outputs it to stdout
   * @param notification The RpcNotification to send
   */
  static void send_notification(const RpcNotification &notification);
};

#endif
