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
#ifndef _XPLATFORM_SOURCE
#define _XPLATFORM_SOURCE
#endif
#include <sys/xattr.h>
#include <sys/stat.h>
#include <dirent.h>
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
 * @return RTNCD_SUCCESS on success, RTNCD_FAILURE on failure
 */
int zusf_create_uss_file_or_dir(ZUSF *zusf, string file, string mode, bool createDir)
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

  // TODO(zFernand0): Implement `mkdirp` by default
  // TODO(zFernand0): `mkdirp` when creatnig a file in a directory that doesn't exist
  if (createDir)
  {
    mkdir(file.c_str(), strtol(mode.c_str(), nullptr, 8));
    return RTNCD_SUCCESS;
  }
  else
  {
    ofstream out(file.c_str());
    if (out.is_open())
    {
      out.close();
      chmod(file.c_str(), strtol(mode.c_str(), nullptr, 8));
      return RTNCD_SUCCESS;
    }
  }

  zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not create '%s'", file.c_str());
  return RTNCD_FAILURE;
}

/**
 * Lists the USS file path.
 *
 * @param zusf pointer to a ZUSF object
 * @param file name of the USS file or directory
 * @param response reference to a string where the read data will be stored
 *
 * @return RTNCD_SUCCESS on success, RTNCD_FAILURE on failure
 */
int zusf_list_uss_file_path(ZUSF *zusf, string file, string &response)
{
  // TODO(zFernand0): Handle `*` and other bash-expansion rules
  struct stat file_stats;
  if (stat(file.c_str(), &file_stats) == -1)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' does not exist", file.c_str());
    return RTNCD_FAILURE;
  }

  if (S_ISREG(file_stats.st_mode))
  {
    response.clear();
    response += file.substr(file.find_last_of("/") + 1);
    response.push_back('\n');
    return RTNCD_SUCCESS;
  }

  if (!S_ISDIR(file_stats.st_mode))
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' is not a directory", file.c_str());
    return RTNCD_FAILURE;
  }

  DIR *dir;
  if ((dir = opendir(file.c_str())) == nullptr)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open directory '%s'", file.c_str());
    return RTNCD_FAILURE;
  }

  struct dirent *entry;
  response.clear();
  while ((entry = readdir(dir)) != nullptr)
  {
    // TODO(zFernand0): Skip hidden files
    if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0))
    {
      // TODO(zFernand0): Add option to list full file paths
      // TODO(zFernand0): Add option to list file tags
      response += entry->d_name;
      response.push_back('\n');
    }
    // TODO(zFernand0): Sort in alphabetical order
  }
  closedir(dir);

  return RTNCD_SUCCESS;
}

/**
 * Reads data from a USS file.
 *
 * @param zusf pointer to a ZUSF object
 * @param file name of the USS file
 * @param response reference to a string where the read data will be stored
 *
 * @return RTNCD_SUCCESS on success, RTNCD_FAILURE on failure
 */
int zusf_read_from_uss_file(ZUSF *zusf, string file, string &response)
{
  ifstream in(file.c_str(), zusf->encoding_opts.data_type == eDataTypeBinary ? ifstream::in | ifstream::binary : ifstream::in);
  if (!in.is_open())
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open file '%s'", file.c_str());
    return RTNCD_FAILURE;
  }

  in.seekg(0, ios::end);
  size_t size = in.tellg();
  in.seekg(0, ios::beg);

  char *raw_data = new char[size];
  in.read(raw_data, size);

  response.assign(raw_data);
  delete[] raw_data;

  in.close();

  // TODO(traeok): Finish support for encoding auto-detection
  // char tagged_encoding[16] = {0};
  // ssize_t xattr_result = getxattr(file.c_str(), "system.filetag", &tagged_encoding);

  const auto encodingProvided = zusf->encoding_opts.data_type == eDataTypeText && strlen(zusf->encoding_opts.codepage) > 0;

  if (size > 0 && encodingProvided)
  {
    // const encoding = encodingProvided ? string(zusf->encoding_opts.codepage) : string(tagged_encoding);
    std::string temp = response;
    try
    {
      const auto bytes_with_encoding = zut_encode(temp, string(zusf->encoding_opts.codepage), "UTF-8", zusf->diag);
      temp = bytes_with_encoding;
    }
    catch (std::exception &e)
    {
      // TODO: error handling
    }
    if (!temp.empty())
    {
      response = temp;
    }
  }

  return RTNCD_SUCCESS;
}

/**
 * Writes data to a USS file.
 *
 * @param zusf pointer to a ZUSF object
 * @param file name of the USS file
 * @param data string to be written to the file
 *
 * @return RTNCD_SUCCESS on success, RTNCD_FAILURE on failure
 */
int zusf_write_to_uss_file(ZUSF *zusf, string file, string &data)
{
  // TODO(zFernand0): Avoid overriding existing files
  const auto hasEncoding = zusf->encoding_opts.data_type == eDataTypeText && strlen(zusf->encoding_opts.codepage) > 0;
  const auto codepage = string(zusf->encoding_opts.codepage);

  if (!data.empty() && hasEncoding)
  {
    ofstream out(file.c_str(), zusf->encoding_opts.data_type == eDataTypeBinary ? ios::out | ios::binary : ios::out);
    if (!out.is_open())
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open '%s'", file.c_str());
      return RTNCD_FAILURE;
    }

    std::string temp = data;
    try
    {
      const auto bytes_with_encoding = zut_encode(temp, "UTF-8", codepage, zusf->diag);
      temp = bytes_with_encoding;
    }
    catch (std::exception &e)
    {
      // TODO: error handling
    }
    if (!temp.empty())
    {
      out << temp;
    }
    out.close();
  }

  return 0;
}

/**
 * Changes the permissions of a USS file or directory.
 *
 * @param zusf pointer to a ZUSF object
 * @param file name of the USS file
 * @param mode new permissions in octal format
 *
 * @return RTNCD_SUCCESS on success, RTNCD_FAILURE on failure
 */
int zusf_chmod_uss_file_or_dir(ZUSF *zusf, string file, string mode)
{
  // TODO(zFernand0): Add recursive option for directories
  struct stat file_stats;
  if (stat(file.c_str(), &file_stats) == -1)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' does not exist", file.c_str());
    return RTNCD_FAILURE;
  }
  chmod(file.c_str(), strtol(mode.c_str(), nullptr, 8));
  return 0;
}
