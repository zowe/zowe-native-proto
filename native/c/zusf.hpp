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

#ifndef ZUSF_HPP
#define ZUSF_HPP

#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif
#include <grp.h>
#include <pwd.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include "zusf.hpp"
#include "zusftype.h"

typedef struct _ListOptions
{
  bool all_files;
  bool long_format;
} ListOptions;

int zusf_create_uss_file_or_dir(ZUSF *zusf, const std::string &file, mode_t mode, bool createDir);
std::string zusf_format_file_entry(ZUSF *zusf, const struct stat &file_stats, const std::string &file_path, const std::string &display_name, ListOptions options, bool use_csv_format);
int zusf_list_uss_file_path(ZUSF *zusf, std::string file, std::string &response, ListOptions options = ListOptions{}, bool use_csv_format = false);
int zusf_read_from_uss_file(ZUSF *zusf, const std::string &file, std::string &response);
int zusf_read_from_uss_file_streamed(ZUSF *zusf, const std::string &file, const std::string &pipe, size_t *content_len);
int zusf_write_to_uss_file(ZUSF *zusf, const std::string &file, std::string &data);
int zusf_write_to_uss_file_streamed(ZUSF *zusf, const std::string &file, const std::string &pipe, size_t *content_len);
int zusf_chmod_uss_file_or_dir(ZUSF *zusf, std::string file, mode_t mode, bool recursive);
int zusf_delete_uss_item(ZUSF *zusf, std::string file, bool recursive);
int zusf_chown_uss_file_or_dir(ZUSF *zusf, const std::string &file, const std::string &owner, bool recursive);
short zusf_get_id_from_user_or_group(const std::string &user_or_group, bool is_user);
int zusf_chtag_uss_file_or_dir(ZUSF *zusf, const std::string &file, std::string &tag, bool recursive);
int zusf_get_file_ccsid(ZUSF *zusf, std::string file);
std::string zusf_get_ccsid_display_name(int ccsid);
int zusf_get_ccsid_from_display_name(const std::string &display_name);
const char *zusf_get_owner_from_uid(uid_t uid);
const char *zusf_get_group_from_gid(gid_t gid);
std::string zusf_format_ls_time(time_t mtime, bool use_csv_format = false);

#endif
