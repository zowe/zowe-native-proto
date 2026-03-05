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

#include <cstdlib>
#include <iostream>
#include "server.hpp"
#include "../server/server_main.hpp"

using namespace parser;

namespace server
{

static std::string g_exec_dir = ".";

void set_exec_dir(const std::string &dir)
{
  g_exec_dir = dir;
}

static int handle_server(plugin::InvocationContext &context)
{
  server::Options opts;
  opts.num_workers = context.get<long long>("num-workers", opts.num_workers);
  opts.verbose = context.get<bool>("verbose", opts.verbose);
  opts.request_timeout = context.get<long long>("request-timeout", opts.request_timeout);
  opts.exec_dir = g_exec_dir;

  const auto *num_workers_env = getenv("ZOWED_NUM_WORKERS");
  if (num_workers_env != nullptr)
  {
    opts.num_workers = atoi(num_workers_env);
  }

  if (opts.num_workers <= 0)
  {
    context.error_stream() << "Number of workers must be greater than 0" << std::endl;
    return 1;
  }

  if (opts.request_timeout <= 0)
  {
    context.error_stream() << "Request timeout must be greater than 0 seconds" << std::endl;
    return 1;
  }

  return server::run(opts);
}

void register_commands(Command &root_command)
{
  auto server_cmd = command_ptr(new Command("server", "start the Zowe native IO server"));
  server_cmd->add_keyword_arg("num-workers",
                               make_aliases("-w", "--num-workers"),
                               "number of worker threads",
                               ArgType_Single, false,
                               ArgValue(10LL));
  server_cmd->add_keyword_arg("verbose",
                               make_aliases("-v", "--verbose"),
                               "enable verbose logging",
                               ArgType_Flag, false,
                               ArgValue(false));
  server_cmd->add_keyword_arg("request-timeout",
                               make_aliases("-t", "--request-timeout"),
                               "request timeout in seconds before a worker is restarted",
                               ArgType_Single, false,
                               ArgValue(60LL));
  server_cmd->set_handler(handle_server);
  root_command.add_command(server_cmd);
}

} // namespace server
