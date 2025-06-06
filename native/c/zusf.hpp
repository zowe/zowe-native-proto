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
#include <iostream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include "zusf.hpp"
#include "zusftype.h"

typedef struct _ListOptions {
    bool all;
    bool long_format;
} ListOptions;

int zusf_create_uss_file_or_dir(ZUSF *zusf, std::string file, std::string mode, bool createDir);
int zusf_list_uss_file_path(ZUSF *zusf, std::string file, std::string &response, ListOptions options = ListOptions{});
int zusf_read_from_uss_file(ZUSF *zusf, std::string file, std::string &response);
int zusf_read_from_uss_file_streamed(ZUSF *zusf, std::string file, std::string pipe);
int zusf_write_to_uss_file(ZUSF *zusf, std::string file, std::string &data);
int zusf_write_to_uss_file_streamed(ZUSF *zusf, std::string file, std::string pipe);
int zusf_chmod_uss_file_or_dir(ZUSF *zusf, std::string file, std::string mode, bool recursive);
int zusf_delete_uss_item(ZUSF *zusf, std::string file, bool recursive);
int zusf_chown_uss_file_or_dir(ZUSF *zusf, std::string file, std::string owner, bool recursive);
short zusf_get_id_from_user_or_group(std::string user_or_group, bool is_user);
int zusf_chtag_uss_file_or_dir(ZUSF *zusf, std::string file, std::string tag, bool recursive);

#endif
