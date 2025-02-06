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
#define _XOPEN_SOURCE

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

using namespace std;

int ztso_issue(string command, string &response)
{
  int rc = 0;
  string data = "tsocmd " + command + " 2>&1"; // combine stderr
  string response_raw;

  FILE *tso = popen(data.c_str(), "r");
  if (nullptr == tso)
  {
    return -1; // TODO(Kelosky)
  }

  char buffer[256] = {0};
  while (fgets(buffer, sizeof(buffer), tso) != nullptr)
  {
    response_raw += string(buffer);
  }

  stringstream response_ss(response_raw);

  string line;
  auto index = 0;

  while (getline(response_ss, line))
  {
    index++;
    if (1 == index)
    {
      // do nothing
    }
    else
    {
      response += line + '\n';
    }
  }

  rc = pclose(tso);
  if (0 != rc)
  {
    return WEXITSTATUS(rc);
  }

  return rc;
}
