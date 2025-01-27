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

#ifndef ZUT_HPP
#define ZUT_HPP

#include <iostream>
#include <vector>
#include <string>
#include "zcli.hpp"
#include "zcntype.h"

int zut_bpxwdyn(std::string, unsigned int *, std::string &);
int zut_test();
void zut_dump_storage(std::string, const void *, size_t);
int zut_hello(std::string);
char zut_get_hex_char(int);
int zut_get_current_user(std::string &);
void zut_uppercase_pad_truncate(std::string, char *, int);
int zut_convert_dsect();
bool zut_prepare_encoding(ZCLIResult &result, ZEncode *opts);
void zut_print_string_as_bytes(std::string &input);

char *zut_encode_alloc(const std::string &bytes, const std::string &from_encoding, const std::string &to_encoding, ZDIAG &diag, char **buf_end);
std::string zut_format_as_csv(std::vector<std::string> &fields);
std::string &zut_rtrim(std::string &s, const char *t = " ");
std::string &zut_ltrim(std::string &s, const char *t = " ");
std::string &zut_trim(std::string &s, const char *t = " ");

#endif