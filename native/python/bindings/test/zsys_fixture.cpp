/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 */

// EBCDIC service substitutes linked only into the isolated adapter test extension.
#include "zsys.hpp"
#include "zut.hpp"
#include <cstring>

int zut_substitute_symbol(const std::string &name, std::string &value)
{
  if (name == "&FAIL")
    return 42;
  if (name == "&NUL")
  {
    const char record[] = {'A', 0, 'B', '\n'};
    value.assign(record, sizeof(record));
  }
  else
    value.assign("\xC3\x81\x86\x51", 4); // IBM-1047 Cafe with e acute.
  return 0;
}

int zut_list_parmlib(ZDIAG &, std::vector<std::string> &values)
{
  values = {"SYS1.PARMLIB   ", "SYS1.PARMLIB   "};
  return 0;
}

int zsys_list_proclib(ZDIAG &diag, std::vector<std::string> &)
{
  memset(diag.service_name, 'S', sizeof(diag.service_name));
  memset(diag.e_msg, 'X', sizeof(diag.e_msg));
  diag.e_msg_len = 1024; // Deliberately larger than the native buffer.
  diag.detail_rc = 11;
  diag.service_rc = 12;
  diag.service_rsn = 13;
  diag.service_rsn_secondary = 14;
  return -1;
}

int zut_list_subsystems(ZDIAG &, std::vector<std::string> &values, std::string filter)
{
  values = {filter};
  return 0;
}

int zut_list_apf(ZDIAG &, std::vector<std::pair<std::string, std::string>> &values)
{
  values = {{"SYS1.LINKLIB  ", "VOL001"}, {"SYS1.OTHER", "      "}};
  return 0;
}

int zut_list_linklist(ZDIAG &, std::vector<ZLinklstEntry> &values)
{
  values = {{"SYS1.LINKLIB  ", "VOL001", true}, {"SYS1.OTHER", "      ", false}};
  return 0;
}

int zsys_resolve_syslog(const ZSyslogRequest &request, ZSyslogOptions &opts, std::string &)
{
  opts.date = request.date.empty() ? "2024-02-29" : request.date;
  opts.time = request.time.empty() ? "12:00:00" : request.time;
  opts.max_lines = request.has_max_lines ? request.max_lines : 100;
  return 0;
}

int zsys_read_syslog(ZDIAG &diag, const ZSyslogOptions &opts, ZSyslogResponse &response)
{
  if (opts.max_lines == 2)
  {
    response.data = "partial";
    diag.e_msg[0] = 'X';
    diag.e_msg_len = -1;
    return 8;
  }
  if (opts.max_lines == 3)
  {
    response.data.assign("\xC3\x81\x86\x51", 4);
    response.data.append(1, 0);
    response.data.append(" tail\n");
    response.returned_lines = 1;
    response.has_more = true;
    response.end_date = opts.date;
    response.end_time = opts.time;
  }
  return 0;
}
