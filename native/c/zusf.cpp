/*
  This program and the accompanying materials are
  made available under the terms of the Eclipse Public License v2.0 which accompanies
  this distribution, and is available at https://www.eclipse.org/legal/epl-v20.html

  SPDX-License-Identifier: EPL-2.0

  Copyright Contributors to the Zowe Project.
*/
#include <stdio.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <iomanip>
#include <algorithm>
#include <iconv.h>
#include <stdlib.h>
#include "zusf.hpp"
#include "zdyn.h"
#include "zusftype.h"
#include "zut.hpp"
#include "iefzb4d2.h"
// #include "zusfm.h"

using namespace std;

int zusf_read_from_uss_file(ZUSF *zusf, std::string file, std::string &response, std::string mode)
{
  ifstream in(file.c_str());
  if (!in.is_open())
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open file '%s'", file.c_str());
    return RTNCD_FAILURE;
  }

  in.seekg(0, ios::end);
  size_t size = in.tellg();
  in.seekg(0, ios::beg);

  char *rawData = new char[size];
  in.read(rawData, size);
  in.seekg(0, ios::beg);

  response.assign(rawData);
  in.close();

  return 0;
}
