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

#include "parser.hpp"
#include "../c/extend/plugin.hpp"
#include <sstream>

MiddlewareContext::MiddlewareContext(const std::string &command_path, const plugin::ArgumentMap &args)
    : plugin::InvocationContext(command_path, args, &m_input_stream, &m_output_stream, &m_error_stream)
{
}

std::stringstream &MiddlewareContext::get_input_stream()
{
  return m_input_stream;
}

std::stringstream &MiddlewareContext::get_output_stream()
{
  return m_output_stream;
}

std::stringstream &MiddlewareContext::get_error_stream()
{
  return m_error_stream;
}

void MiddlewareContext::set_input_content(const std::string &content)
{
  m_input_stream.str(content);
  m_input_stream.clear(); // Clear any error flags
}

std::string MiddlewareContext::get_output_content() const
{
  return m_output_stream.str();
}

std::string MiddlewareContext::get_error_content() const
{
  return m_error_stream.str();
}

void MiddlewareContext::clear_streams()
{
  m_input_stream.str("");
  m_input_stream.clear();
  m_output_stream.str("");
  m_output_stream.clear();
  m_error_stream.str("");
  m_error_stream.clear();
}
