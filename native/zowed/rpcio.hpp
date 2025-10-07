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

#ifndef RPCIO_HPP
#define RPCIO_HPP

#include "../c/extend/plugin.hpp"
#include "../c/zstd.hpp"
#include <sstream>

// Forward declaration
struct RpcNotification;

class MiddlewareContext : public plugin::InvocationContext
{
public:
  MiddlewareContext(const std::string &command_path, const plugin::ArgumentMap &args);

  // Get access to the string streams for reading/writing content
  std::stringstream &get_input_stream();
  std::stringstream &get_output_stream();
  std::stringstream &get_error_stream();

  // Helper methods to set input content and get output/error content
  void set_input_content(const std::string &content);
  std::string get_output_content() const;
  std::string get_error_content() const;
  void clear_streams();

  // Provide mutable access to arguments for transforms
  plugin::ArgumentMap &mutable_arguments()
  {
    return m_args;
  }

  // Set content length and send pending notification if present
  void set_content_len(size_t content_length);

  // Store pending notification for delayed sending
  void set_pending_notification(const RpcNotification &notification);
  bool has_pending_notification() const;

private:
  std::stringstream m_input_stream;
  std::stringstream m_output_stream;
  std::stringstream m_error_stream;
  zstd::unique_ptr<RpcNotification> m_pending_notification;
};

#endif
