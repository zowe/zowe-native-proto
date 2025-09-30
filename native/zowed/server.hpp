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

#pragma once

#include <string>
#include <mutex>
#include "../c/zjson.hpp"
#include "../c/types/common.h"
#include "../c/extend/plugin.hpp"
#include "parser.hpp"

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
  plugin::ArgumentMap convertJsonParamsToArgumentMap(const zjson::Value &params);
  zjson::Value convertOutputToJson(const std::string &output);
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
};
