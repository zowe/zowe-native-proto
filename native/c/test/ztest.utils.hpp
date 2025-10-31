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

// Helper function to convert string to hex format
// TODO(Kelosky): move to zut.hpp if additional use is found
string string_to_hex(const string &input)
{
  string hex_output;
  for (char c : input)
  {
    char hex_byte[3];
    sprintf(hex_byte, "%02x", static_cast<unsigned char>(c));
    hex_output += hex_byte;
  }
  return hex_output;
}

// NOTE(Kelosky): consolidate this into ztest.hpp if additional use is found
int execute_command_with_output(const string &command, string &output)
{
  output = "";
  // TestLog("Running: " + command);

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

string get_random_string(const int length = 7, const bool allNumbers = true)
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
    ret += to_string(rand() % 10);
  }
  return ret;
}

string get_random_ds(const int qualifier_count = 4, const string hlq = "")
{
  string q = hlq;
  if (q.length() == 0)
  {
    string user;
    execute_command_with_output("whoami", user);
    q = ztst::TrimChars(user);
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
