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
#include <sys/types.h>
#include <unistd.h>
#include <unistd.h>
#include "zcn.hpp"
#include "zut.hpp"
#include "parser.hpp"
#include "zjb.hpp"
#include "zds.hpp"
#include "zusf.hpp"
#include "ztso.hpp"
#include "zuttype.h"

#ifndef TO_STRING
#define TO_STRING(x) static_cast<std::ostringstream &>(           \
                         (std::ostringstream() << std::dec << x)) \
                         .str()
#endif

using namespace parser;
using namespace std;

int handle_ping(const ParseResult &result);

int main(int argc, char *argv[])
{
  ArgumentParser arg_parser(argv[0], "Zowe Native Protocol CLI");

  // Add interactive mode flag to root command
  arg_parser.get_root_command().add_keyword_arg("interactive",
                                                make_aliases("--interactive", "--it"),
                                                "interactive (REPL) mode", ArgType_Flag, false,
                                                ArgValue(false));

  // Ping command
  auto ping_cmd = command_ptr(new Command("ping", "ping the server to test connectivity"));
  ping_cmd->add_keyword_arg("message",
                            make_aliases("--message", "-m"),
                            "optional message to include in ping", ArgType_Single, false,
                            ArgValue(std::string("Hello from zowex!")));
  ping_cmd->set_handler(handle_ping);
  arg_parser.get_root_command().add_command(ping_cmd);

  // rest of code is in native/c/zowex.cpp, but omitted in the example for brevity
}

// Command handler for new ping command
int handle_ping(const ParseResult &result)
{
  string message = result.find_kw_arg_string("message");

  // Get current timestamp
  time_t now = time(0);
  char *dt = ctime(&now);

  // Remove newline from ctime output
  string timestamp(dt);
  timestamp.erase(timestamp.find_last_not_of("\n\r") + 1);

  cout << "PONG: " << message << " at " << timestamp << endl;

  return RTNCD_SUCCESS;
}