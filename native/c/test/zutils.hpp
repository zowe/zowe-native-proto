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
#include "../zds.hpp"

using namespace std;

const string zowex_command = "./../build-out/zowex";
const string zoweax_command = "./../build-out/zoweax";

int execute_command_with_input(const string &command, const string &input, bool suppress_output = true);
int execute_command_with_output(const string &command, string &output);
string get_random_string(const int length = 7, const bool allNumbers = true);
string get_random_uss(const string base_dir);
string get_random_ds(const int qualifier_count = 4, const string hlq = "");
string get_user();
string parse_etag_from_output(const string &output);
vector<string> parse_rfc_response(const string input, const char *delim = ",");
// Wait for a job to be visible in JES (returns true if found, false if timeout)
bool wait_for_job(const string &jobid, int max_retries = 30, int delay_ms = 100);

/**
 * @brief RAII class to manage FILE* pointers
 *
 * Opens a file on construction and automatically closes it on destruction.
 * Provides implicit conversion to FILE* for easy use with C file APIs.
 */
class TestFileGuard
{
  FILE *fp;
  string _file;

public:
  TestFileGuard(const char *_filename, const char &mode = 'w', const char *_link = nullptr);
  ~TestFileGuard();

  TestFileGuard(const TestFileGuard &) = delete;
  TestFileGuard &operator=(const TestFileGuard &) = delete;
  TestFileGuard(TestFileGuard &&) = delete;
  TestFileGuard &operator=(TestFileGuard &&) = delete;

  void reset(const char *_filename);

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
  string _dir;

public:
  TestDirGuard(const char *_dirname, const mode_t mode = 0755);
  ~TestDirGuard();

  TestDirGuard(const TestDirGuard &) = delete;
  TestDirGuard &operator=(const TestDirGuard &) = delete;
  TestDirGuard(TestDirGuard &&) = delete;
  TestDirGuard &operator=(TestDirGuard &&) = delete;

  void reset(const char *_dirname);

  operator string() const;
};

// Data set creation helpers - convenience wrappers around zds_* functions
// that use sensible defaults for test data sets
void create_dsn_with_attrs(ZDS *zds, const std::string &dsn, DS_ATTRIBUTES &attrs, const std::string &type_name);
void create_pds(ZDS *zds, const std::string &dsn);
void create_pdse(ZDS *zds, const std::string &dsn);
void create_seq(ZDS *zds, const std::string &dsn);
void write_to_dsn(const std::string &dsn, const std::string &data);

#endif
