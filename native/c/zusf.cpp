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
#include <sys/stat.h>
// #include "zusfm.h"

using namespace std;

/**
 * Creates a USS file or directory.
 *
 * @param zusf pointer to a ZUSF object
 * @param file name of the USS file
 * @param response reference to a string where the read data will be stored
 * @param mode mode of the file or directory
 * @param createDir flag indicating whether to create a directory
 *
 * @return 0 on success, -1 on failure
 */
int zusf_create_uss_file_or_dir(ZUSF *zusf, std::string file, std::string &response, std::string mode, bool createDir)
{
  struct stat file_stats;
  if (stat(file.c_str(), &file_stats) != -1)
  {
    if (S_ISREG(file_stats.st_mode))
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "File '%s' already exists", file.c_str());
    }
    else if (S_ISDIR(file_stats.st_mode))
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Directory '%s' already exists", file.c_str());
    }
    else
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' already exists! Mode: '%08o'", file.c_str(), file_stats.st_mode);
    }
    return RTNCD_FAILURE;
  }

  if (createDir)
  {
    mkdir(file.c_str(), strtol(mode.c_str(), NULL, 8));
    return 0;
  }
  else
  {
    ofstream out(file.c_str());
    if (out.is_open())
    {
      out.close();
      chmod(file.c_str(), strtol(mode.c_str(), NULL, 8));
      return 0;
    }
  }

  zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not create '%s'", file.c_str());
  return RTNCD_FAILURE;
}

/**
 * Reads data from a USS file.
 *
 * @param zusf pointer to a ZUSF object
 * @param file name of the USS file
 * @param response reference to a string where the read data will be stored
 *
 * @return 0 on success, -1 on failure
 */
int zusf_read_from_uss_file(ZUSF *zusf, std::string file, std::string &response)
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
