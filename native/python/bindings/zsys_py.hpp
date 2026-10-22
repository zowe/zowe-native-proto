/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 */

#ifndef ZSYS_PY_HPP
#define ZSYS_PY_HPP

#include <stdexcept>
#include <string>
#include <vector>

struct ApfEntry
{
  std::string dsname;
  std::string volume;
};

struct LinklistEntry
{
  std::string dsname;
  std::string volume;
  bool apf = false;
};

struct SyslogResult
{
  std::string data;
  std::string start_date;
  std::string start_time;
  std::string end_date;
  std::string end_time;
  int returned_lines = 0;
  bool has_more = false;
};

struct ZsysQuery
{
  std::string date;
  std::string time;
  long long seconds_ago = -1;
  long long max_lines = 0;
  bool has_max_lines = false;
};

struct ZsysError : std::runtime_error
{
  std::string operation;
  int rc = 0;
  std::string service_name;
  int detail_rc = 0;
  int service_rc = 0;
  int service_rsn = 0;
  int service_rsn_secondary = 0;

  explicit ZsysError(const std::string &message) : std::runtime_error(message)
  {
  }
};

std::string display_symbol(const std::string &symbol);
std::vector<std::string> list_parmlib();
std::vector<std::string> list_proclib();
std::vector<std::string> list_subsystems(const std::string &filter = "*");
std::vector<ApfEntry> list_apf();
std::vector<LinklistEntry> list_linklist();
SyslogResult view_syslog(const ZsysQuery &query);

#endif
