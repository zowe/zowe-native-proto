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

#ifndef _LARGE_TIME_API
#define _LARGE_TIME_API
#endif
#ifndef _OPEN_SYS_FILE_EXT
#define _OPEN_SYS_FILE_EXT 1
#endif
#include <sys/stat.h>
#include <stdio.h>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <string>
#include <iomanip>
#include <algorithm>
#include <iconv.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>
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
#include <dirent.h>
// #include "zusfm.h"

using namespace std;

/**
 * Creates a USS file or directory.
 *
 * @param zusf pointer to a ZUSF object
 * @param file name of the USS file
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

  if (createDir)
  {
    const auto last_trailing_slash = file.find_last_of("/");
    if (last_trailing_slash != std::string::npos)
    {
      const auto parent_path = file.substr(0, last_trailing_slash);
      const auto exists = stat(parent_path.c_str(), &file_stats) == 0;
      if (!exists)
      {
        const auto rc = zusf_create_uss_file_or_dir(zusf, parent_path, mode, true);
        if (rc != 0)
        {
          return rc;
        }
      }
    }
    const auto rc = mkdir(file.c_str(), strtol(mode.c_str(), nullptr, 8));
    if (rc != 0)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to create directory '%s', errno: %d", file.c_str(), errno);
    }
    return rc;
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
int zusf_chmod_uss_file_or_dir(ZUSF *zusf, string file, string mode, bool recursive)
{
  // TODO(zFernand0): Add recursive option for directories
  struct stat file_stats;
  if (stat(file.c_str(), &file_stats) == -1)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' does not exist", file.c_str());
    return RTNCD_FAILURE;
  }

  if (!recursive && S_ISDIR(file_stats.st_mode))
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' is a folder and recursive is false", file.c_str());
    return RTNCD_FAILURE;
  }

  chmod(file.c_str(), strtol(mode.c_str(), nullptr, 8));
  if (recursive && S_ISDIR(file_stats.st_mode))
  {
    DIR *dir;
    if ((dir = opendir(file.c_str())) == nullptr)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open directory '%s'", file.c_str());
      return RTNCD_FAILURE;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
      {
        const string child_path = file[file.length() - 1] == '/' ? file.append((const char *)entry->d_name) : file.append(string("/") + (const char *)entry->d_name);
        struct stat file_stats;
        stat(child_path.c_str(), &file_stats);

        const auto rc = zusf_chmod_uss_file_or_dir(zusf, child_path, mode, S_ISDIR(file_stats.st_mode));
        if (rc != 0)
        {
          return rc;
        }
      }
    }
  }
  return 0;
}

int zusf_delete_uss_item(ZUSF *zusf, string file, bool recursive)
{
  struct stat file_stats;
  if (stat(file.c_str(), &file_stats) == -1)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' does not exist", file.c_str());
    return RTNCD_FAILURE;
  }

  const auto is_dir = S_ISDIR(file_stats.st_mode);
  if (is_dir && !recursive)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' is a directory and recursive was false", file.c_str());
    return RTNCD_FAILURE;
  }
  return is_dir ? rmdir(file.c_str()) : remove(file.c_str());
}

short zusf_get_id_from_user_or_group(string user_or_group, bool is_user)
{
  const auto is_numeric = user_or_group.find_first_not_of("0123456789") == std::string::npos;
  if (is_numeric)
  {
    return (short)atoi(user_or_group.c_str());
  }

  auto *meta = is_user ? (void *)getpwnam(user_or_group.c_str()) : (void *)getgrnam(user_or_group.c_str());
  if (meta)
  {
    return is_user ? ((passwd *)meta)->pw_uid : ((group *)meta)->gr_gid;
  }

  return -1;
}

int zusf_chown_uss_file_or_dir(ZUSF *zusf, string file, string owner, bool recursive)
{
  struct stat file_stats;
  if (stat(file.c_str(), &file_stats) == -1)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' does not exist", file.c_str());
    return RTNCD_FAILURE;
  }

  if (S_ISDIR(file_stats.st_mode) && !recursive)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' is a folder and recursive is false", file.c_str());
    return RTNCD_FAILURE;
  }

  const auto uid = zusf_get_id_from_user_or_group(owner, true);
  const auto colon_pos = owner.find_first_of(":");
  const auto group = colon_pos != std::string::npos ? owner.substr(colon_pos + 1) : std::string();
  const auto gid = group.empty() ? file_stats.st_gid : zusf_get_id_from_user_or_group(group, false);
  const auto rc = chown(file.c_str(), uid, gid);

  if (rc != 0)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "chmod failed for path '%s', errno %d", file.c_str(), errno);
    return RTNCD_FAILURE;
  }

  if (recursive && S_ISDIR(file_stats.st_mode))
  {
    DIR *dir;
    if ((dir = opendir(file.c_str())) == nullptr)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open directory '%s'", file.c_str());
      return RTNCD_FAILURE;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
      {
        const string child_path = file[file.length() - 1] == '/' ? file.append((const char *)entry->d_name) : file.append(string("/") + (const char *)entry->d_name);
        struct stat file_stats;
        stat(child_path.c_str(), &file_stats);

        const auto rc = zusf_chown_uss_file_or_dir(zusf, child_path, owner, S_ISDIR(file_stats.st_mode));
        if (rc != 0)
        {
          return rc;
        }
      }
    }
  }

  return 0;
}

int zusf_chtag_uss_file_or_dir(ZUSF *zusf, string file, string tag, bool recursive)
{
  struct stat file_stats;
  if (stat(file.c_str(), &file_stats) == -1)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' does not exist", file.c_str());
    return RTNCD_FAILURE;
  }

  const auto ccsid = strtol(tag.c_str(), nullptr, 10);
  if (ccsid == LONG_MAX || ccsid == LONG_MIN)
  {
    // TODO(traeok): Get CCSID from encoding name
  }
  attrib64_t attr;
  memset(&attr, 0, sizeof(attr));
  attr.att_filetagchg = 1;
  attr.att_filetag.ft_ccsid = ccsid;
  attr.att_filetag.ft_txtflag = int(ccsid != 65535);

  const auto rc = __chattr64((char *)file.c_str(), &attr, sizeof(attr));
  if (rc != 0)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to update attributes for path '%s'", file.c_str());
    return RTNCD_FAILURE;
  }

  if (recursive && S_ISDIR(file_stats.st_mode))
  {
    DIR *dir;
    if ((dir = opendir(file.c_str())) == nullptr)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open directory '%s'", file.c_str());
      return RTNCD_FAILURE;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
      {
        const string child_path = file[file.length() - 1] == '/' ? file.append((const char *)entry->d_name) : file.append(string("/") + (const char *)entry->d_name);
        struct stat file_stats;
        stat(child_path.c_str(), &file_stats);

        const auto rc = zusf_chtag_uss_file_or_dir(zusf, child_path, tag, S_ISDIR(file_stats.st_mode));
        if (rc != 0)
        {
          return rc;
        }
      }
    }
  }
  return rc;
}
