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

#include "system.hpp"
#include "common_args.hpp"
#include "../zut.hpp"
#include "../zjb.hpp"
#include <unistd.h>
#include <ctime>

using namespace parser;
using namespace std;
using namespace commands::common;

namespace sys
{

int handle_system_display_symbol(InvocationContext &context)
{
  int rc = 0;
  string symbol = context.get<std::string>("symbol", "");
  transform(symbol.begin(), symbol.end(), symbol.begin(), ::toupper);
  symbol = "&" + symbol;
  string value;
  rc = zut_substitute_symbol(symbol, value);
  if (0 != rc)
  {
    context.error_stream() << "Error: asasymbf with parm '" << symbol << "' rc: '" << rc << "'" << endl;
    return RTNCD_FAILURE;
  }
  context.output_stream() << value << endl;

  return RTNCD_SUCCESS;
}

int handle_system_list_parmlib(InvocationContext &context)
{
  int rc = 0;
  ZDIAG diag = {};
  std::vector<std::string> parmlibs;
  rc = zut_list_parmlib(diag, parmlibs);
  if (0 != rc)
  {
    context.error_stream() << "Error: could not list parmlibs rc: '" << rc << "'" << endl;
    context.error_stream() << "  Details: " << diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  for (vector<string>::iterator it = parmlibs.begin(); it != parmlibs.end(); ++it)
  {
    context.output_stream() << *it << endl;
  }

  return rc;
}

int handle_system_list_proclib(InvocationContext &context)
{
  int rc = 0;
  ZJB zjb = {};

  vector<string> proclib;
  rc = zjb_list_proclib(&zjb, proclib);
  if (0 != rc)
  {
    context.error_stream() << "Error: could not list proclib, rc: '" << rc << "'" << endl;
    context.error_stream() << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  for (vector<string>::iterator it = proclib.begin(); it != proclib.end(); it++)
  {
    context.output_stream() << *it << endl;
  }

  return RTNCD_SUCCESS;
}

int handle_system_list_subsystems(InvocationContext &context)
{
  int rc = 0;

  string filter = context.get<std::string>("filter", "*");
  transform(filter.begin(), filter.end(), filter.begin(), ::toupper);

  vector<string> subsystems;
  ZDIAG diag = {};
  rc = zut_list_subsystems(diag, subsystems, filter);
  if (0 != rc)
  {
    context.error_stream() << "Error: could not list subsystems, rc: '" << rc << "'" << endl;
    context.error_stream() << "  Details: " << diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  for (vector<string>::iterator it = subsystems.begin(); it != subsystems.end(); it++)
  {
    context.output_stream() << *it << endl;
  }

  return RTNCD_SUCCESS;
}

int handle_system_view_syslog(InvocationContext &context)
{
  int rc = 0;

  string time_stamp = context.get<std::string>("timestamp", "");
  string date = context.get<std::string>("date", "");
  auto max_lines = context.get<long long>("max-lines", 5);

  time_t now = time(nullptr);
  struct tm *tm_now = localtime(&now);
  char buf[32];

  if (time_stamp.empty())
  {
    strftime(buf, sizeof(buf), "%H:%M:%S.00", tm_now);
    time_stamp = buf;
  }
  else
  {
    int hh = -1, mm = -1, ss = -1, cs = -1;
    int parsed = sscanf(time_stamp.c_str(), "%d:%d:%d.%d", &hh, &mm, &ss, &cs);
    if (parsed < 3 || hh < 0 || hh > 23 || mm < 0 || mm > 59 || ss < 0 || ss > 59 || (parsed == 4 && (cs < 0 || cs > 99)))
    {
      context.error_stream() << "Error: invalid timestamp '" << time_stamp << "', expected hh:mm:ss.cs e.g. 12:01:30.03" << endl;
      return RTNCD_FAILURE;
    }
    if (parsed == 3)
      time_stamp += ".00";
  }

  if (date.empty())
  {
    strftime(buf, sizeof(buf), "%Y-%m-%d", tm_now);
    date = buf;
  }
  else
  {
    int yyyy = -1, mm = -1, dd = -1;
    int parsed = sscanf(date.c_str(), "%d-%d-%d", &yyyy, &mm, &dd);
    if (parsed != 3 || yyyy < 1900 || yyyy > 2185 || mm < 1 || mm > 12 || dd < 1 || dd > 31)
    {
      context.error_stream() << "Error: invalid date '" << date << "', expected yyyy-mm-dd e.g. 2026-03-13" << endl;
      return RTNCD_FAILURE;
    }
  }

  string response;
  ZJB zjb = {};

  rc = zjb_read_syslog(&zjb, response, date, time_stamp, max_lines);
  if (0 != rc)
  {
    context.error_stream() << "Error: could not view syslog, rc: '" << rc << "'" << endl;
    context.error_stream() << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  context.output_stream() << response << endl;
  return RTNCD_SUCCESS;
}

void register_commands(parser::Command &root_command)
{
  auto system_cmd = command_ptr(new Command("system", "system operations"));

  // Display symbol subcommand
  auto system_display_symbol_cmd = command_ptr(new Command("display-symbol", "display system symbol"));
  system_display_symbol_cmd->add_positional_arg("symbol", "symbol to display", ArgType_Single, true);
  system_display_symbol_cmd->set_handler(handle_system_display_symbol);
  system_cmd->add_command(system_display_symbol_cmd);

  // List-parmlib subcommand
  auto system_list_parmlib_cmd = command_ptr(new Command("list-parmlib", "list parmlib"));
  system_list_parmlib_cmd->set_handler(handle_system_list_parmlib);
  system_cmd->add_command(system_list_parmlib_cmd);

  // List-proclib subcommand
  auto system_list_proclib_cmd = command_ptr(new Command("list-proclib", "list proclib"));
  system_list_proclib_cmd->set_handler(handle_system_list_proclib);
  system_cmd->add_command(system_list_proclib_cmd);

  // List-subsystems subcommand
  auto system_list_subsystems_cmd = command_ptr(new Command("list-subsystems", "list subsystems"));
  system_list_subsystems_cmd->set_handler(handle_system_list_subsystems);
  system_list_subsystems_cmd->add_keyword_arg("filter", make_aliases("--filter", "-f"), "filter subsystems", ArgType_Single, false);
  system_cmd->add_command(system_list_subsystems_cmd);

  // View-syslog subcommand
  auto system_view_syslog_cmd = command_ptr(new Command("view-syslog", "view syslog"));
  system_view_syslog_cmd->set_handler(handle_system_view_syslog);
  system_view_syslog_cmd->add_keyword_arg("timestamp", make_aliases("--timestamp", "-ts"), "specify timestamp, e.g. --ts 10:41:00.15", ArgType_Single, false);
  system_view_syslog_cmd->add_keyword_arg("date", make_aliases("--date", "-d"), "specify date yyyy-mm-dd, e.g. --date 2026-03-13", ArgType_Single, false);
  system_view_syslog_cmd->add_keyword_arg("max-lines", make_aliases("--max-lines", "-ml"), "specify maximum number of lines to display, e.g. --max-lines 100", ArgType_Single, false);
  system_view_syslog_cmd->add_example("View syslog for a specifc date and timestamp", "zowex system view-syslog --date 2026-03-13 --timestamp 10:41:00.15");
  system_cmd->add_command(system_view_syslog_cmd);

  root_command.add_command(system_cmd);
}
} // namespace sys
