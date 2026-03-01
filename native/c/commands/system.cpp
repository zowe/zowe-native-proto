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

using namespace parser;
using namespace std;
using namespace commands::common;

namespace zsystem
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
  ZJB zjb = {};

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

void register_commands(parser::Command &root_command)
{
  auto system_cmd = command_ptr(new Command("system", "system operations"));

  // Display symbol subcommand
  auto system_display_symbol_cmd = command_ptr(new Command("display-symbol", "display system symbol"));
  system_display_symbol_cmd->add_positional_arg("symbol", "symbol to display", ArgType_Single, true);
  system_display_symbol_cmd->set_handler(handle_system_display_symbol); // TODO(Kelosky): move these
  system_cmd->add_command(system_display_symbol_cmd);

  // List-parmlib subcommand
  auto system_list_parmlib_cmd = command_ptr(new Command("list-parmlib", "list parmlib"));
  system_list_parmlib_cmd->set_handler(handle_system_list_parmlib); // TODO(Kelosky): move these
  system_cmd->add_command(system_list_parmlib_cmd);

  // List-proclib subcommand
  auto system_list_proclib_cmd = command_ptr(new Command("list-proclib", "list proclib"));
  system_list_proclib_cmd->set_handler(handle_system_list_proclib); // TODO(Kelosky): move these
  system_cmd->add_command(system_list_proclib_cmd);

  // List-subsystems subcommand
  auto system_list_subsystems_cmd = command_ptr(new Command("list-subsystems", "list subsystems"));
  system_list_subsystems_cmd->set_handler(handle_system_list_subsystems); // TODO(Kelosky): move these
  system_list_subsystems_cmd->add_keyword_arg("filter", make_aliases("--filter", "-f"), "filter subsystems", ArgType_Single, false);
  system_cmd->add_command(system_list_subsystems_cmd);

  root_command.add_command(system_cmd);
}
} // namespace zsystem
