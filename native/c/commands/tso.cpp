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

#include "tso.hpp"
#include "../ztso.hpp"

using namespace parser;
using namespace std;

namespace tso
{
int handle_tso_issue(InvocationContext &result)
{
  int rc = 0;
  string command = result.get<std::string>("command", "");
  string response;

  rc = ztso_issue(command, response);

  if (0 != rc)
  {
    cerr << "Error running command, rc '" << rc << "'" << endl;
    cerr << "  Details: " << response << endl;
  }

  cout << response;

  return rc;
}

void register_commands(parser::Command &root_command)
{
  auto tso_group = command_ptr(new Command("tso", "TSO operations"));
  {
    auto tso_issue_cmd = command_ptr(new Command("issue", "issue TSO command"));
    tso_issue_cmd->add_positional_arg("command", "command to issue", ArgType_Single, true);
    tso_issue_cmd->set_handler(handle_tso_issue);

    tso_group->add_command(tso_issue_cmd);
  }
  root_command.add_command(tso_group);
}
} // namespace tso