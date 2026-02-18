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

  for (int i = 0; i < length; ++i)
  {
    if (allNumbers)
    {
      ret += to_string(rand() % 10);
    }
    else
    {
      static const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
      ret += letters[rand() % 26];
    }
  }
  return ret;
}

string get_basename(const string fspath)
{
  if (fspath.empty()) return "";

  auto lastSlash = fspath.find_last_of("/\\");

  return (lastSlash == std::string::npos) ? fspath : fspath.substr(lastSlash + 1);
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
