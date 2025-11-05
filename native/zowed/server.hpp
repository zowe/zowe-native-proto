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
#include "../c/extend/plugin.hpp"
#include "../c/singleton.hpp"
#include "../c/zstd.hpp"

// Forward declarations
namespace zjson
{
class Value;
}
struct RpcRequest;
struct RpcResponse;
struct RpcNotification;
struct ErrorDetails;

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
  void print_error(int request_id, int code, const std::string &message, const std::string *data = nullptr);
  zstd::optional<std::string> validate_json_with_schema(const std::string &method, const zjson::Value &params, bool is_request);

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
