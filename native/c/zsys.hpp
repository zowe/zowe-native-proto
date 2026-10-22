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

#ifndef ZSYS_HPP
#define ZSYS_HPP

#include <ctime>
#include <string>
#include <vector>
#include "zspool.hpp"
#include "ztype.h"

struct ZSyslogOptions
{
  std::string date;
  std::string time;
  int max_lines = 0;
  ZEncode encoding_opts = {};
  ZSpoolOptions allocation;
};

struct ZSyslogResponse
{
  std::string data;
  bool has_more = false;
  std::string end_date;
  std::string end_time;
  int returned_lines = 0;
};

// Empty absolute values and negative seconds_ago preserve CLI omission semantics.
struct ZSyslogRequest
{
  std::string date;
  std::string time;
  long long seconds_ago = -1;
  long long max_lines = 0;
  bool has_max_lines = false;
  bool use_system_timezone = false; // Python defaults use the same offset as the SYSLOG reader.
  bool has_now = false;
  time_t now = 0; // Optional clock snapshot for deterministic native tests.
};

#ifdef SWIG
extern "C"
{
#endif
int zsys_list_proclib(ZDIAG &diag, std::vector<std::string> &proclib);
int zsys_read_syslog(ZDIAG &diag, const ZSyslogOptions &opts, ZSyslogResponse &response);
int zsys_resolve_syslog(const ZSyslogRequest &request, ZSyslogOptions &opts, std::string &error);
#ifdef SWIG
}
#endif

#endif
