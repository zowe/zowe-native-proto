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

#include "ztest.hpp"
#include "zutils.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <cstring>
#include <chrono>
#include <thread>
#include <random>
#include <stdexcept>

using namespace std;

int execute_command_with_input(const std::string &command, const std::string &input, bool suppress_output)
{
  std::string final_command = command;
  if (suppress_output)
  {
    final_command += " > /dev/null";
  }

  FILE *pipe = popen(final_command.c_str(), "w");
  if (!pipe)
  {
    throw std::runtime_error("Failed to open pipe for writing");
  }

  if (!input.empty())
  {
    if (fprintf(pipe, "%s", input.c_str()) < 0)
    {
      pclose(pipe);
      throw std::runtime_error("Failed to write to pipe");
    }
  }

  int exit_status = pclose(pipe);
  return WEXITSTATUS(exit_status);
}

int execute_command_with_output(const std::string &command, std::string &output)
{
  output = "";

  // Open the pipe in "read" mode and redirect stderr to stdout
  FILE *pipe = popen((command + " 2>&1").c_str(), "r");
  if (!pipe)
  {
    throw std::runtime_error("Failed to open pipe for reading");
  }

  char buffer[256];
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
  {
    output += buffer;
  }

  int exit_status = pclose(pipe);
  return WEXITSTATUS(exit_status);
}

string get_random_string(const int length, const bool allNumbers)
{
  static bool seeded = false;
  if (!seeded)
  {
    srand(static_cast<unsigned int>(time(NULL)));
    seeded = true;
  }
  string ret = "";
  static const unsigned char EBCDIC_A = 0xC1;

  for (int i = 0; i < length; ++i)
  {
    if (allNumbers)
    {
      ret += to_string(rand() % 10);
    }
    else
    {
      const auto rand_num = rand() % 26;
      ret += EBCDIC_A + (rand_num / 10 * 6) + (rand_num % 10);
    }
  }
  return ret;
}

string get_random_uss(const string base_dir)
{
  static bool seeded = false;
  if (!seeded)
  {
    srand(static_cast<unsigned int>(time(NULL)));
    seeded = true;
  }

  string ret = base_dir;
  if (ret.back() != '/')
  {
    ret += "/";
  }

  ret += "test_" + get_random_string(10);

  return ret;
}

static std::string s_user = "";
string get_user()
{
  if (s_user.empty())
  {
    string user;
    // Note: using `basename $HOME` instead of `whoami` to get the current user
    // because `whoami` may be mapped to a kernel user instead of a real one.
    execute_command_with_output("basename $HOME | tr '[:lower:]' '[:upper:]'", user);
    s_user = ztst::TrimChars(user);
  }
  return s_user;
}

string get_random_ds(const int qualifier_count, const string hlq)
{
  const auto q = hlq.length() == 0 ? get_user() : hlq;
  string ret = q + ".ZNP#TEST";
  for (int i = 0; i < qualifier_count - 2; ++i)
  {
    ret += ".Z" + get_random_string();
  }
  return ret;
}

// Helper function to get etag from command response
string parse_etag_from_output(const string &output)
{
  const string label = "etag: ";
  size_t etag_label_pos = output.find(label);

  if (etag_label_pos == string::npos)
  {
    return "";
  }

  size_t start_value_pos = etag_label_pos + label.length();

  size_t end_value_pos = output.find_first_of("\r\n", start_value_pos);

  if (end_value_pos == string::npos)
  {
    end_value_pos = output.length();
  }

  string etag = output.substr(start_value_pos, end_value_pos - start_value_pos);

  return etag;
}

vector<string> parse_rfc_response(const string input, const char *delim)
{
  vector<string> ret;
  string current;
  char delimiter = delim[0];

  for (size_t i = 0; i < input.length(); ++i)
  {
    if (input[i] == delimiter)
    {
      ret.push_back(ztst::TrimChars(current));
      current.clear();
    }
    else
    {
      current += input[i];
    }
  }
  ret.push_back(ztst::TrimChars(current));

  return ret;
}

bool wait_for_job(const string &jobid, int max_retries, int delay_ms)
{
  string output;
  for (int i = 0; i < max_retries; ++i)
  {
    int rc = execute_command_with_output(zowex_command + " job view-status " + jobid, output);
    if (rc == 0 && output.find(jobid) != string::npos)
    {
      return true;
    }
    this_thread::sleep_for(chrono::milliseconds(delay_ms));
  }
  return false;
}

// Data set creation helpers

string get_test_dsn()
{
  static int counter = 0;
  counter++;
  static random_device rd;
  static mt19937 gen(rd()); // NOSONAR: safe for generating unique test names, not for security
  uniform_int_distribution<> dist(0, 99999);
  int random_num = dist(gen);
  return get_user() + ".ZDSTEST.T" + to_string(random_num) + to_string(counter);
}

void create_dsn_with_attrs(ZDS *zds, const string &dsn, DS_ATTRIBUTES &attrs, const string &type_name)
{
  memset(zds, 0, sizeof(ZDS));
  string response;
  int rc = zds_create_dsn(zds, dsn, attrs, response);
  if (rc != 0)
  {
    string err = zds->diag.e_msg_len > 0 ? string(zds->diag.e_msg)
                 : response.length() > 0 ? response
                                         : "rc=" + to_string(rc);
    throw runtime_error("Failed to create " + type_name + ": " + err);
  }
}

void create_pds(ZDS *zds, const string &dsn)
{
  DS_ATTRIBUTES attrs = {0};
  attrs.dsorg = "PO";
  attrs.recfm = "F,B";
  attrs.lrecl = 80;
  attrs.blksize = 800;
  attrs.dirblk = 5;
  create_dsn_with_attrs(zds, dsn, attrs, "PDS");
}

void create_pdse(ZDS *zds, const string &dsn)
{
  DS_ATTRIBUTES attrs = {0};
  attrs.dsorg = "PO";
  attrs.dsntype = "LIBRARY";
  attrs.recfm = "F,B";
  attrs.lrecl = 80;
  attrs.blksize = 800;
  attrs.dirblk = 5;
  create_dsn_with_attrs(zds, dsn, attrs, "PDSE");
}

void create_seq(ZDS *zds, const string &dsn)
{
  DS_ATTRIBUTES attrs = {0};
  attrs.dsorg = "PS";
  attrs.recfm = "F,B";
  attrs.lrecl = 80;
  attrs.blksize = 800;
  attrs.primary = 1;
  attrs.secondary = 1;
  create_dsn_with_attrs(zds, dsn, attrs, "sequential data set");
}

void write_to_dsn(const string &dsn, const string &content)
{
  ZDS zds = {0};
  string data = content;
  zds_write_to_dsn(&zds, dsn, data);
}
