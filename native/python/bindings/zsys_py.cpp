/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 */

#include "zsys_py.hpp"
#include "zsys.hpp"
#include "zut.hpp"
#include "conversion.hpp"
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <iconv.h>

namespace
{
std::string to_utf8(const std::string &input)
{
  if (input.empty())
    return {};
  struct Converter
  {
    iconv_t value;
    ~Converter()
    {
      iconv_close(value);
    }
  } converter{iconv_open("UTF-8", "IBM-1047")};
  if (converter.value == (iconv_t)-1)
    throw std::runtime_error("Cannot open IBM-1047 to UTF-8 converter");
  std::string output;
  if (input.size() > (output.max_size() - 16) / 4)
    throw std::length_error("System text exceeds the supported string size");
  output.resize(input.size() * 4 + 16);
  char *source = const_cast<char *>(input.data());
  char *target = &output[0];
  size_t source_left = input.size();
  size_t target_left = output.size();
  if (iconv(converter.value, &source, &source_left, &target, &target_left) == (size_t)-1)
    throw std::runtime_error("Cannot convert system text from IBM-1047 to UTF-8");
  output.resize(output.size() - target_left);
  return output;
}

std::string identifier(const std::string &input)
{
  std::string result = to_utf8(input);
  const auto end = result.find_last_not_of(' ');
  result.erase(end == std::string::npos ? 0 : end + 1);
  return result;
}

void check_rc(const char *operation, int rc, const ZDIAG &diag)
{
  if (rc == RTNCD_SUCCESS)
    return;
  const size_t length = std::min(static_cast<size_t>(std::max(diag.e_msg_len, 0)), sizeof(diag.e_msg));
  std::string message = to_utf8(std::string(diag.e_msg, std::find(diag.e_msg, diag.e_msg + length, '\0')));
  if (message.empty())
    message = std::string(operation) + " failed (rc=" + std::to_string(rc) + ")";
  ZsysError error(message);
  error.operation = operation;
  error.rc = rc;
  error.service_name = identifier(std::string(diag.service_name,
      std::find(diag.service_name, diag.service_name + sizeof(diag.service_name), '\0')));
  error.detail_rc = diag.detail_rc;
  error.service_rc = diag.service_rc;
  error.service_rsn = diag.service_rsn;
  error.service_rsn_secondary = diag.service_rsn_secondary;
  throw error;
}

std::string input_name(const std::string &value, size_t maximum, bool filter)
{
  if (value.empty() || value.size() > maximum)
    throw std::invalid_argument(filter ? "filter must contain 1-4 ASCII characters" : "symbol must contain 1-126 ASCII characters");
  std::string result = value;
  for (char &ch : result)
  {
    if (ch >= 'a' && ch <= 'z')
      ch -= 'a' - 'A';
    const bool letter = ch >= 'A' && ch <= 'Z';
    const bool digit = ch >= '0' && ch <= '9';
    if (!letter && !digit && ch != '@' && ch != '#' && ch != '$' && ch != '_' && !(filter && ch == '*'))
      throw std::invalid_argument(filter ? "filter contains an unsupported character" : "symbol contains an unsupported character");
  }
  if (!filter)
    result.insert(0, "&");
  a2e_inplace(result);
  return result;
}

std::vector<std::string> names(const char *operation, int (*query)(ZDIAG &, std::vector<std::string> &))
{
  ZDIAG diag = {};
  std::vector<std::string> result;
  check_rc(operation, query(diag, result), diag);
  for (auto &value : result)
    value = identifier(value);
  return result;
}
} // namespace

std::string display_symbol(const std::string &symbol)
{
  const std::string native_symbol = input_name(symbol, 126, false);
  std::string result;
  ZDIAG diag = {};
  check_rc("display_symbol", zut_substitute_symbol(native_symbol, result), diag);
  return to_utf8(result);
}

std::vector<std::string> list_parmlib()
{
  return names("list_parmlib", zut_list_parmlib);
}

std::vector<std::string> list_proclib()
{
  return names("list_proclib", zsys_list_proclib);
}

std::vector<std::string> list_subsystems(const std::string &filter)
{
  std::string native_filter = input_name(filter, 4, true);
  ZDIAG diag = {};
  std::vector<std::string> result;
  check_rc("list_subsystems", zut_list_subsystems(diag, result, native_filter), diag);
  for (auto &value : result)
    value = identifier(value);
  return result;
}

std::vector<ApfEntry> list_apf()
{
  ZDIAG diag = {};
  std::vector<std::pair<std::string, std::string>> native;
  check_rc("list_apf", zut_list_apf(diag, native), diag);
  std::vector<ApfEntry> result;
  result.reserve(native.size());
  for (const auto &value : native)
    result.push_back({identifier(value.first), identifier(value.second)});
  return result;
}

std::vector<LinklistEntry> list_linklist()
{
  ZDIAG diag = {};
  std::vector<ZLinklstEntry> native;
  check_rc("list_linklist", zut_list_linklist(diag, native), diag);
  std::vector<LinklistEntry> result;
  result.reserve(native.size());
  for (const auto &value : native)
    result.push_back({identifier(value.dsname), identifier(value.volume), value.apf});
  return result;
}

SyslogResult view_syslog(const ZsysQuery &query)
{
  ZSyslogRequest request;
  request.use_system_timezone = true;
  request.date = query.date;
  request.time = query.time;
  a2e_inplace(request.date);
  a2e_inplace(request.time);
  request.seconds_ago = query.seconds_ago;
  request.max_lines = query.max_lines;
  request.has_max_lines = query.has_max_lines;
  ZSyslogOptions opts;
  std::string error;
  const int resolve_rc = zsys_resolve_syslog(request, opts, error);
  if (resolve_rc != RTNCD_SUCCESS)
    throw std::invalid_argument(error.empty() ? "Could not resolve SYSLOG options" : to_utf8(error));
  ZDIAG diag = {};
  ZSyslogResponse response;
  check_rc("view_syslog", zsys_read_syslog(diag, opts, response), diag);
  SyslogResult result;
  result.data = to_utf8(response.data);
  result.start_date = to_utf8(opts.date);
  result.start_time = to_utf8(opts.time);
  result.end_date = to_utf8(response.end_date);
  result.end_time = to_utf8(response.end_time);
  result.returned_lines = response.returned_lines;
  result.has_more = response.has_more;
  return result;
}
