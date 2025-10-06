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

#include <string>
#include <time.h>
#include "parser.hpp"

namespace ping
{

using namespace std;

// Command handler for new ping command
int handle_ping(const plugin::InvocationContext &context)
{
  string message = context.get("message");

  // Get current timestamp
  time_t now = time(0);
  char *dt = ctime(&now);

  // Remove newline from ctime output
  string timestamp(dt);
  timestamp.erase(timestamp.find_last_not_of("\n\r") + 1);

  context.output_stream() << "PONG: " << message << " at " << timestamp << endl;

  return RTNCD_SUCCESS;
}

void register_commands(Command &root_command)
{
  auto ping_cmd = command_ptr(new Command("ping", "issue a ping"));
  ping_cmd->set_handler(handle_ping);

  root_command.add_command(ping_cmd);
}
} // namespace ping