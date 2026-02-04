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

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

#include <stdio.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <iomanip>
#include <algorithm>
#include <iconv.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include "ztype.h"

using namespace std;

int issue_command_combined_stdoutstderr(string command, string &response)
{
  int rc = 0;
  string response_raw;
  FILE *cmd = popen(command.c_str(), "r");
  if (nullptr == cmd)
  {
    return RTNCD_FAILURE;
  }

  char buffer[256] = {0};
  while (fgets(buffer, sizeof(buffer), cmd) != nullptr)
  {
    response_raw += string(buffer);
  }

  stringstream response_ss(response_raw);

  string line;
  auto index = 0;

  while (getline(response_ss, line))
  {
    index++;
    if (index > 1)
    {
      response += line + '\n';
    }
  }

  rc = pclose(cmd);
  if (0 != rc)
  {
    return WEXITSTATUS(rc);
  }

  return rc;
}

// NOTE(Kelosky): alternatives we'll likely use / consider in the future
// - CEA, probably needed to achieve z/OSMF parity (allows starting, stopping TSO address spaces)
// - IKJEFT01, requires authorized caller
// - IKJEFTSR, limited TSO dynamic environment
// - Load TMP directly, untested, but potentially useful if we read/write SYSTSIN/SYSTSPRT
int ztso_issue(string command, string &response)
{
  // NOTE(Kelosky): for now we combined stderr and stdout as `popen` doesnt
  // appear to allow access to stderr and tsocmd always writes the input parameters
  // to stderr
  string tso_cmd = "tsocmd " + command + " 2>&1"; // combine stderr
  return issue_command_combined_stdoutstderr(tso_cmd, response);
}
