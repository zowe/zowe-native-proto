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
#include "zshmem.hpp"
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
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
#include "zbase64.h"
#include "iefzb4d2.h"

#ifndef _XPLATFORM_SOURCE
#define _XPLATFORM_SOURCE
#endif
#include <sys/xattr.h>
#include <vector>
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
int zusf_create_uss_file_or_dir(ZUSF *zusf, string file, mode_t mode, bool createDir)
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
        if (0 != rc)
        {
          return rc;
        }
      }
    }
    const auto rc = mkdir(file.c_str(), mode);
    if (0 != rc)
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
      chmod(file.c_str(), mode);
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
int zusf_list_uss_file_path(ZUSF *zusf, string file, string &response, ListOptions options)
{
  // TODO(zFernand0): Handle `*` and other bash-expansion rules
  struct stat file_stats;
  if (stat(file.c_str(), &file_stats) == -1)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' does not exist", file.c_str());
    return RTNCD_FAILURE;
  }

  // TODO: Hide hidden paths by default
  // TODO(zFernand0): Add option to list full file paths
  // TODO(zFernand0): Add option to list file tags

  if (S_ISREG(file_stats.st_mode))
  {
    response.clear();
    const auto file_name = file.substr(file.find_last_of("/") + 1);
    vector<string> fields;
    fields.push_back(file_name);
    if (options.long_format)
    {
      string mode;
      mode += (S_ISDIR(file_stats.st_mode) ? "d" : "-");
      mode += (file_stats.st_mode & S_IRUSR ? "r" : "-");
      mode += (file_stats.st_mode & S_IWUSR ? "w" : "-");
      mode += (file_stats.st_mode & S_IXUSR ? "x" : "-");
      mode += (file_stats.st_mode & S_IRGRP ? "r" : "-");
      mode += (file_stats.st_mode & S_IWGRP ? "w" : "-");
      mode += (file_stats.st_mode & S_IXGRP ? "x" : "-");
      mode += (file_stats.st_mode & S_IROTH ? "r" : "-");
      mode += (file_stats.st_mode & S_IWOTH ? "w" : "-");
      mode += (file_stats.st_mode & S_IXOTH ? "x" : "-");
      fields.push_back(mode);
    }
    response = zut_format_as_csv(fields) + "\n";
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
    if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0))
    {
      string child_path = file[file.length() - 1] == '/' ? file + string(entry->d_name) : file + string("/") + string(entry->d_name);
      struct stat child_stats;
      stat(child_path.c_str(), &child_stats);

      vector<string> fields;
      string name = entry->d_name;
      if (name.at(0) == '.' && !options.all_files)
      {
        continue;
      }

      fields.push_back(entry->d_name);
      if (options.long_format)
      {
        string mode;
        mode += (S_ISDIR(child_stats.st_mode) ? "d" : "-");
        mode += (child_stats.st_mode & S_IRUSR ? "r" : "-");
        mode += (child_stats.st_mode & S_IWUSR ? "w" : "-");
        mode += (child_stats.st_mode & S_IXUSR ? "x" : "-");
        mode += (child_stats.st_mode & S_IRGRP ? "r" : "-");
        mode += (child_stats.st_mode & S_IWGRP ? "w" : "-");
        mode += (child_stats.st_mode & S_IXGRP ? "x" : "-");
        mode += (child_stats.st_mode & S_IROTH ? "r" : "-");
        mode += (child_stats.st_mode & S_IWOTH ? "w" : "-");
        mode += (child_stats.st_mode & S_IXOTH ? "x" : "-");
        fields.push_back(mode);
      }
      response += zut_format_as_csv(fields) + "\n";
    }
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

  vector<char> raw_data(size);
  in.read(&raw_data[0], size);

  response.assign(raw_data.begin(), raw_data.end());
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
 * Reads data from a USS file.
 *
 * @param zusf pointer to a ZUSF object
 * @param file name of the USS file
 * @param pipe name of the output pipe
 * @param content_len pointer where the length of the file contents will be stored
 *
 * @return RTNCD_SUCCESS on success, RTNCD_FAILURE on failure
 */
int zusf_read_from_uss_file_streamed(ZUSF *zusf, string file, string pipe, size_t *content_len)
{
  if (content_len == nullptr)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "content_len must be a valid size_t pointer");
    return RTNCD_FAILURE;
  }

  FILE *fin = fopen(file.c_str(), zusf->encoding_opts.data_type == eDataTypeBinary ? "rb" : "r");
  if (!fin)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open file '%s'", file.c_str());
    return RTNCD_FAILURE;
  }

  struct stat st;
  if (stat(file.c_str(), &st) != 0)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not stat file '%s'", file.c_str());
    return RTNCD_FAILURE;
  }
  set_content_length((uint64_t)st.st_size);

  int fifo_fd = open(pipe.c_str(), O_WRONLY);
  FILE *fout = fdopen(fifo_fd, "w");
  if (!fout)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open output pipe '%s'", pipe.c_str());
    return RTNCD_FAILURE;
  }

  // TODO(traeok): Finish support for encoding auto-detection
  // char tagged_encoding[16] = {0};
  // ssize_t xattr_result = getxattr(file.c_str(), "system.filetag", &tagged_encoding);

  const auto hasEncoding = zusf->encoding_opts.data_type == eDataTypeText && strlen(zusf->encoding_opts.codepage) > 0;
  const auto codepage = string(zusf->encoding_opts.codepage);

  const size_t chunk_size = FIFO_CHUNK_SIZE * 3 / 4;
  std::vector<char> buf(chunk_size);
  size_t bytes_read;

  while ((bytes_read = fread(&buf[0], 1, chunk_size, fin)) > 0)
  {
    int chunk_len = bytes_read;
    const char *chunk = &buf[0];
    std::vector<char> temp_encoded;

    if (hasEncoding)
    {
      try
      {
        temp_encoded = zut_encode(chunk, chunk_len, codepage, "UTF-8", zusf->diag);
        chunk = &temp_encoded[0];
        chunk_len = temp_encoded.size();
      }
      catch (std::exception &e)
      {
        zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to convert input data from UTF-8 to %s", codepage.c_str());
        return RTNCD_FAILURE;
      }
    }

    *content_len += chunk_len;
    temp_encoded = zbase64::encode(chunk, chunk_len, false);
    fwrite(&temp_encoded[0], 1, temp_encoded.size(), fout);
  }

  const auto padding = 4 - (*content_len % 4);
  if (padding > 0)
  {
    fwrite("===", 1, padding, fout);
  }

  fflush(fout);
  fclose(fin);
  fclose(fout);
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
  struct stat file_stats;

  int stat_result = stat(file.c_str(), &file_stats);
  if (strlen(zusf->etag) > 0 && stat_result != -1)
  {
    const auto current_etag = zut_build_etag(file_stats.st_mtime, file_stats.st_size);
    if (current_etag != zusf->etag)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Etag mismatch: expected %s, actual %s", zusf->etag, current_etag.c_str());
      return RTNCD_FAILURE;
    }
  }
  if (stat_result == -1 && errno != ENOENT)
    return RTNCD_FAILURE;
  zusf->created = stat_result == -1;

  std::string temp = data;
  if (hasEncoding)
  {
    try
    {
      const auto bytes_with_encoding = zut_encode(temp, "UTF-8", codepage, zusf->diag);
      temp = bytes_with_encoding;
    }
    catch (std::exception &e)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to convert input data from UTF-8 to %s", codepage.c_str());
      return RTNCD_FAILURE;
    }
  }
  const char *mode = (zusf->encoding_opts.data_type == eDataTypeBinary) ? "wb" : "w";
  FILE *fp = std::fopen(file.c_str(), mode);
  if (!fp)
  {
    zusf->diag.e_msg_len = std::sprintf(zusf->diag.e_msg, "Could not open '%s' for writing", file.c_str());
    return RTNCD_FAILURE;
  }

  if (!temp.empty())
    std::fwrite(temp.data(), 1, temp.size(), fp);
  std::fclose(fp);

  struct stat new_stats;
  if (stat(file.c_str(), &new_stats) == -1)
  {
    zusf->diag.e_msg_len = std::sprintf(
        zusf->diag.e_msg,
        "Could not stat file '%s' after writing",
        file.c_str());
    return RTNCD_FAILURE;
  }

  const string new_tag = zut_build_etag(new_stats.st_mtime, new_stats.st_size);
  std::strcpy(zusf->etag, new_tag.c_str());

  return RTNCD_SUCCESS; // success
}

/**
 * Writes data to a USS file.
 *
 * @param zusf pointer to a ZUSF object
 * @param file name of the USS file
 * @param pipe name of the input pipe
 * @param content_len pointer where the length of the file contents will be stored
 *
 * @return RTNCD_SUCCESS on success, RTNCD_FAILURE on failure
 */
int zusf_write_to_uss_file_streamed(ZUSF *zusf, string file, string pipe, size_t *content_len)
{
  // TODO(zFernand0): Avoid overriding existing files
  if (content_len == nullptr)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "content_len must be a valid size_t pointer");
    return RTNCD_FAILURE;
  }

  struct stat file_stats;
  const auto hasEncoding = zusf->encoding_opts.data_type == eDataTypeText && strlen(zusf->encoding_opts.codepage) > 0;
  const auto codepage = string(zusf->encoding_opts.codepage);

  int stat_result = stat(file.c_str(), &file_stats);
  if (strlen(zusf->etag) > 0 && stat_result != -1)
  {
    const auto current_etag = zut_build_etag(file_stats.st_mtime, file_stats.st_size);
    if (current_etag != zusf->etag)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Etag mismatch: expected %s, actual %s", zusf->etag, current_etag.c_str());
      return RTNCD_FAILURE;
    }
  }
  if (stat_result == -1 && errno != ENOENT)
    return RTNCD_FAILURE;
  zusf->created = stat_result == -1;

  FILE *fout = fopen(file.c_str(), zusf->encoding_opts.data_type == eDataTypeBinary ? "wb" : "w");
  if (!fout)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open '%s'", file.c_str());
    return RTNCD_FAILURE;
  }

  int fifo_fd = open(pipe.c_str(), O_RDONLY);
  FILE *fin = fdopen(fifo_fd, "r");
  if (!fin)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open input pipe '%s'", pipe.c_str());
    fclose(fout);
    return RTNCD_FAILURE;
  }

  std::vector<char> buf(FIFO_CHUNK_SIZE);
  size_t bytes_read;

  while ((bytes_read = fread(&buf[0], 1, FIFO_CHUNK_SIZE, fin)) > 0)
  {
    std::vector<char> temp_encoded = zbase64::decode(&buf[0], bytes_read);
    const char *chunk = &temp_encoded[0];
    int chunk_len = temp_encoded.size();
    *content_len += chunk_len;

    if (hasEncoding)
    {
      try
      {
        temp_encoded = zut_encode(chunk, chunk_len, "UTF-8", codepage, zusf->diag);
        chunk = &temp_encoded[0];
        chunk_len = temp_encoded.size();
      }
      catch (std::exception &e)
      {
        zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to convert input data from UTF-8 to %s", codepage.c_str());
        fclose(fin);
        fclose(fout);
        return RTNCD_FAILURE;
      }
    }

    size_t bytes_written = fwrite(chunk, 1, chunk_len, fout);
    if (bytes_written != chunk_len)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to write to '%s' (possibly out of space)", file.c_str());
      fclose(fin);
      fclose(fout);
      return RTNCD_FAILURE;
    }
  }

  fflush(fout);
  fclose(fin);
  fclose(fout);

  if (stat(file.c_str(), &file_stats) == -1)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' does not exist", file.c_str());
    return RTNCD_FAILURE;
  }

  // Print new e-tag to stdout as response
  string etag_str = zut_build_etag(file_stats.st_mtime, file_stats.st_size);
  strcpy(zusf->etag, etag_str.c_str());

  return RTNCD_SUCCESS;
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
int zusf_chmod_uss_file_or_dir(ZUSF *zusf, string file, mode_t mode, bool recursive)
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

  chmod(file.c_str(), mode);
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
        const string child_path = file[file.length() - 1] == '/' ? file + string((const char *)entry->d_name)
                                                                 : file + string("/") + string((const char *)entry->d_name);
        struct stat file_stats;
        stat(child_path.c_str(), &file_stats);

        const auto rc = zusf_chmod_uss_file_or_dir(zusf, child_path, mode, S_ISDIR(file_stats.st_mode));
        if (0 != rc)
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

  if (is_dir)
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
        const string child_path = file[file.length() - 1] == '/' ? file + string((const char *)entry->d_name)
                                                                 : file + string("/") + string((const char *)entry->d_name);
        struct stat file_stats;
        stat(child_path.c_str(), &file_stats);

        const auto rc = zusf_delete_uss_item(zusf, child_path, S_ISDIR(file_stats.st_mode));
        if (0 != rc)
        {
          return rc;
        }
      }
    }
    closedir(dir);
  }

  const auto rc = is_dir ? rmdir(file.c_str()) : remove(file.c_str());
  if (0 != rc)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not delete '%s', rc: %d", file.c_str(), errno);
    return RTNCD_FAILURE;
  }

  return 0;
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

  if (0 != rc)
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
        const string child_path = file[file.length() - 1] == '/' ? file + string((const char *)entry->d_name)
                                                                 : file + string("/") + string((const char *)entry->d_name);
        struct stat file_stats;
        stat(child_path.c_str(), &file_stats);

        const auto rc = zusf_chown_uss_file_or_dir(zusf, child_path, owner, S_ISDIR(file_stats.st_mode));
        if (0 != rc)
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
  const auto is_dir = S_ISDIR(file_stats.st_mode);
  if (!is_dir)
  {
    attrib_t attr;
    memset(&attr, 0, sizeof(attr));
    attr.att_filetagchg = 1;
    attr.att_filetag.ft_ccsid = ccsid;
    attr.att_filetag.ft_txtflag = int(ccsid != 65535);

    const auto rc = __chattr((char *)file.c_str(), &attr, sizeof(attr));
    if (0 != rc)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to update attributes for path '%s'", file.c_str());
      return RTNCD_FAILURE;
    }
  }
  else if (recursive)
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
        const string child_path = file[file.length() - 1] == '/' ? file + string((const char *)entry->d_name)
                                                                 : file + string("/") + string((const char *)entry->d_name);
        struct stat file_stats;
        stat(child_path.c_str(), &file_stats);

        const auto rc = zusf_chtag_uss_file_or_dir(zusf, child_path, tag, S_ISDIR(file_stats.st_mode));
        if (0 != rc)
        {
          return rc;
        }
      }
    }
  }
  return 0;
}
