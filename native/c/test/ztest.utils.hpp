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

#ifndef ZTEST_UTILS_HPP
#define ZTEST_UTILS_HPP

#include "ztest.hpp"
#include <iostream>

using namespace std;

// NOTE(Kelosky): consolidate this into ztest.hpp if additional use is found
int execute_command_with_output(const string &command, string &output)
{
  output = "";

  FILE *pipe = popen((command + " 2>&1").c_str(), "r");
  if (!pipe)
  {
    throw runtime_error("Failed to open pipe");
  }

  char buffer[256];
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
  {
    output += buffer;
  }

  int exit_code = pclose(pipe);
  return WEXITSTATUS(exit_code);
}

static std::string s_user = "";
string get_user()
{
  if (s_user.empty())
  {
    string user;
    execute_command_with_output("whoami", user);
    s_user = ztst::TrimChars(user);
  }
  return s_user;
}

string get_random_string(const int length = 7, const bool allNumbers = true)
{
  static bool seeded = false;
  if (!seeded)
  {
    srand(static_cast<unsigned int>(time(NULL)));
    seeded = true;
  }
  string ret = "";
  // A simple array to map an index (0-25) to an EBCDIC uppercase character code
  const unsigned char EBCDIC_UPPERCASE_LETTERS[] = {
      0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, // A-I
      0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, // J-R
      0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9        // S-Z
  };

  for (int i = 0; i < length; ++i)
  {
    if (allNumbers)
    {
      ret += to_string(rand() % 10);
    }
    else
    {
      ret += EBCDIC_UPPERCASE_LETTERS[rand() % 26];
    }
  }
  return ret;
}

string get_random_ds(const int qualifier_count = 4, const string hlq = "")
{
  string q = hlq;
  if (q.length() == 0)
  {
    q = get_user();
  }
  string ret = q + ".ZNP#TEST";
  for (int i = 0; i < qualifier_count - 2; ++i)
  {
    ret += ".Z" + get_random_string();
  }
  return ret;
}

vector<string> split_rfc_response(const string input, const char *delim)
{
  vector<string> ret;
  char *cstr = new char[input.length() + 1];
  strcpy(cstr, input.c_str());
  char *token_ptr = strtok(cstr, delim);
  while (token_ptr != NULL)
  {
    string token = token_ptr;
    ret.push_back(ztst::TrimChars(token));
    token_ptr = strtok(NULL, delim);
  }
  delete[] cstr;
  return ret;
}

#endif
