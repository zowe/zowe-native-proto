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

#ifndef ZOWED_HPP
#define ZOWED_HPP

/**
 * @brief Options structure for configuring the zowed server
 */
struct IoserverOptions
{
  int num_workers;     ///< Number of worker threads
  bool verbose;        ///< Enable verbose logging
  int request_timeout; ///< Request timeout (in seconds) for worker heartbeat

  /**
   * @brief Constructor with default values
   * @param num_workers Number of worker threads
   * @param verbose Enable verbose logging
   */
  IoserverOptions(const int num_workers = 10, const bool verbose = false, const int request_timeout_seconds = 60)
      : num_workers(num_workers), verbose(verbose), request_timeout(request_timeout_seconds)
  {
    if (this->request_timeout <= 0)
      this->request_timeout = 60;
  }
};

/**
 * @brief Alternative entry point with pre-parsed options
 *
 * This function runs the zowed server with pre-configured options,
 * bypassing command line argument parsing.
 *
 * @param options Pre-configured server options
 * @param exec_dir Executable directory for logger initialization
 * @return int Exit code (0 for success, non-zero for error)
 */
extern "C" int run_zowed_server(const IoserverOptions &options, const char *exec_dir = nullptr);

#endif
