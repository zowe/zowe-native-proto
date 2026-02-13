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

#ifndef ZUTILS_HPP
#define ZUTILS_HPP
#include <string>
#include <vector>
#include "../zut.hpp"
#include <sys/stat.h>

const std::string zowex_command = "./../build-out/zowex";
const std::string zoweax_command = "./../build-out/zoweax";

int execute_command_with_input(const std::string &command, const std::string &input, bool suppress_output = true);
int execute_command_with_output(const std::string &command, std::string &output);
std::string get_random_string(const int length = 7, const bool allNumbers = true);
std::string get_random_uss(const std::string base_dir);
std::string get_random_ds(const int qualifier_count = 4, const std::string hlq = "");
std::string get_user();
std::string parse_etag_from_output(const std::string &output);
std::vector<std::string> parse_rfc_response(const std::string input, const char *delim = ",");
// Wait for a job to be visible in JES (returns true if found, false if timeout)
bool wait_for_job(const std::string &jobid, int max_retries = 30, int delay_ms = 100);

/**
 * @brief RAII class to manage FILE* pointers
 *
 * Opens a file on construction and automatically closes it on destruction.
 * Provides implicit conversion to FILE* for easy use with C file APIs.
 */
class TestFileGuard
{
  FILE *fp;
  std::string filename;

public:
  TestFileGuard(const char *_filename, const char &mode = 'w');
  ~TestFileGuard();

  // Delete copy and move since ownership is non-transferable
  TestFileGuard(const TestFileGuard &) = delete;
  TestFileGuard &operator=(const TestFileGuard &) = delete;
  TestFileGuard(TestFileGuard &&) = delete;
  TestFileGuard &operator=(TestFileGuard &&) = delete;

  // Allow reassignment via reset
  void reset(const char *_filename, const char &mode = 'w');
  void reset();

  operator FILE *() const;
  operator bool() const;
};

/**
 * @brief RAII class to manage directory pointers
 *
 * Creates a directory on construction and automatically deletes it on destruction.
 */
class TestDirGuard
{
  const char *dirname;

public:
  TestDirGuard(const char *_dirname, const mode_t mode = 0755);
  ~TestDirGuard();

  // Delete copy and move since ownership is non-transferable
  TestDirGuard(const TestDirGuard &) = delete;
  TestDirGuard &operator=(const TestDirGuard &) = delete;
  TestDirGuard(TestDirGuard &&) = delete;
  TestDirGuard &operator=(TestDirGuard &&) = delete;

  void reset(const char *_dirname, const mode_t mode = 0755);
  void reset();

  operator std::string() const;
};

#endif
