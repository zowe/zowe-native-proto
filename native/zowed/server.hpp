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
#include "rpcio.hpp"

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
class RpcServer
{
private:
  static RpcServer *instance;
  static std::mutex instanceMutex;
  std::mutex responseMutex;

  // Private constructor for singleton
  RpcServer() = default;

  // Helper methods for JSON processing
  RpcRequest parseRpcRequest(const zjson::Value &json);
  std::string camelCaseToKebabCase(const std::string &input);
  plugin::ArgumentMap convertJsonParamsToArgumentMap(const zjson::Value &params);
  zjson::Value convertOutputToJson(const std::string &output);
  zjson::Value convertAstToJson(const ast::Node &astNode);
  void printResponse(const RpcResponse &response);

public:
  // Singleton access
  static RpcServer &getInstance();

  // Delete copy constructor and assignment operator
  RpcServer(const RpcServer &) = delete;
  RpcServer &operator=(const RpcServer &) = delete;

  // Destructor
  ~RpcServer() = default;

  /**
   * Process a JSON-RPC request string and return the response
   * This method is thread-safe and handles all JSON parsing, command execution,
   * and response serialization
   * @param requestData The raw JSON-RPC request string
   */
  void processRequest(const std::string &requestData);

  /**
   * Utility function to serialize JSON with error handling
   * @param val The JSON value to serialize
   * @param prettify Whether to format the output with indentation
   * @return Serialized JSON string
   */
  static std::string serializeJson(const zjson::Value &val, bool prettify = false);

  /**
   * Helper function for parsing RPC requests from JSON
   * @param json The JSON value containing the RPC request
   * @return Parsed RpcRequest structure
   */
  static RpcRequest parseRpcRequestFromJson(const zjson::Value &json);

  /**
   * Helper function to convert RPC response to JSON
   * @param response The RpcResponse structure
   * @return JSON representation of the response
   */
  static zjson::Value rpcResponseToJson(const RpcResponse &response);

  /**
   * Helper function to convert error details to JSON
   * @param error The ErrorDetails structure
   * @return JSON representation of the error
   */
  static zjson::Value errorDetailsToJson(const ErrorDetails &error);

  /**
   * Helper function to send an RPC notification
   * Serializes the notification and outputs it to stdout
   * @param notification The RpcNotification to send
   */
  static void sendNotification(const RpcNotification &notification);
};

#endif
