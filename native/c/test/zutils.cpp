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
#include <unistd.h>

using namespace std;

int execute_command_with_input(const string &command, const string &input, bool suppress_output)
{
  string final_command = command;
  if (suppress_output)
  {
    final_command += " > /dev/null";
  }

  FILE *pipe = popen(final_command.c_str(), "w");
  if (!pipe)
  {
    throw runtime_error("Failed to open pipe for writing");
  }

  if (!input.empty())
  {
    if (fprintf(pipe, "%s", input.c_str()) < 0)
    {
      pclose(pipe);
      throw runtime_error("Failed to write to pipe");
    }
  }

  int exit_status = pclose(pipe);
  return WEXITSTATUS(exit_status);
}

int execute_command_with_output(const string &command, string &output)
{
  output = "";

  // Open the pipe in "read" mode and redirect stderr to stdout
  FILE *pipe = popen((command + " 2>&1").c_str(), "r");
  if (!pipe)
  {
    throw runtime_error("Failed to open pipe for reading");
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

static string s_user = "";
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

TestFileGuard::TestFileGuard(const char *_filename, const char &mode, const char *_link)
    : fp()
{
  if (mode == 'p')
  {
    mkfifo(_filename, 0666);
    _file = string(_filename);
  }
  else if (mode == 'l')
  {
    symlink(_link, _filename);
    _file = string(_filename);
  }
  else
  {
    _file = string(_filename);
    fp = FileGuard(_filename, string(1, mode).c_str());
  }
}

void TestFileGuard::reset(const char *_filename)
{
  struct stat file_stats;
  if (stat(_file.c_str(), &file_stats) == 0)
  {
    if (S_ISDIR(file_stats.st_mode))
    {
      rmdir(_file.c_str());
    }
    else
    {
      unlink(_file.c_str());
    }
  }
  _file = string(_filename);
  if (stat(_file.c_str(), &file_stats) == -1)
  {
    fp = FileGuard(_file.c_str(), "w");
  }
}

TestFileGuard::~TestFileGuard()
{
  unlink(_file.c_str());
}

TestFileGuard::operator FILE *() const
{
  return fp;
}

TestFileGuard::operator bool() const
{
  return fp != nullptr;
}

TestDirGuard::TestDirGuard(const char *_dirname, const mode_t mode)
    : _dir(string(_dirname))
{
  struct stat dir_stats;
  if (stat(_dir.c_str(), &dir_stats) == 0 && S_ISDIR(dir_stats.st_mode))
  {
    rmdir(_dir.c_str());
  }
  mkdir(_dir.c_str(), mode);
}

TestDirGuard::~TestDirGuard()
{
  struct stat dir_stats;
  if (stat(_dir.c_str(), &dir_stats) == 0)
  {
    if (S_ISDIR(dir_stats.st_mode))
    {
      rmdir(_dir.c_str());
    }
    else
    {
      unlink(_dir.c_str());
    }
  }
}

void TestDirGuard::reset(const char *_dirname)
{
  struct stat dir_stats;
  if (stat(_dir.c_str(), &dir_stats) == 0)
  {
    if (S_ISDIR(dir_stats.st_mode))
    {
      rmdir(_dir.c_str());
    }
    else
    {
      unlink(_dir.c_str());
    }
  }
  _dir = string(_dirname);
  if (stat(_dir.c_str(), &dir_stats) == -1)
  {
    mkdir(_dir.c_str(), 0755);
  }
}

TestDirGuard::operator string() const
{
  return _dir;
}
