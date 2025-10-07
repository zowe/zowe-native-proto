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
  int numWorkers; ///< Number of worker threads
  bool verbose;   ///< Enable verbose logging

  /**
   * @brief Constructor with default values
   * @param workers Number of worker threads
   * @param verb Enable verbose logging
   */
  IoserverOptions(const int workers = 10, const bool verb = false) : numWorkers(workers), verbose(verb)
  {
  }
};

/**
 * @brief Alternative entry point with pre-parsed options
 *
 * This function runs the zowed server with pre-configured options,
 * bypassing command line argument parsing.
 *
 * @param options Pre-configured server options
 * @param execDir Executable directory for logger initialization
 * @return int Exit code (0 for success, non-zero for error)
 */
extern "C" int run_zowed_server(const IoserverOptions &options, const char *execDir = nullptr);

#endif
