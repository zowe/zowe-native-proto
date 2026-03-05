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

#ifndef SERVER_MAIN_HPP
#define SERVER_MAIN_HPP

#include <string>

namespace server
{

struct Options
{
  long long num_workers = 10;
  bool verbose = false;
  long long request_timeout = 60;
  std::string exec_dir = ".";
};

/**
 * @brief Run the JSON-RPC server with the given options.
 *
 * @param options Pre-configured server options
 * @return int Exit code (0 for success, non-zero for error)
 */
int run(const Options &options);

} // namespace server

#endif
