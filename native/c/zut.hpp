/*
  This program and the accompanying materials are
  made available under the terms of the Eclipse Public License v2.0 which accompanies
  this distribution, and is available at https://www.eclipse.org/legal/epl-v20.html

  SPDX-License-Identifier: EPL-2.0

  Copyright Contributors to the Zowe Project.
*/
#ifndef ZUT_HPP
#define ZUT_HPP

#include <initializer_list>
#include <iostream>
#include <vector>
#include <string>
#include "zcntype.h"

int zut_bpxwdyn(std::string, unsigned int *, std::string &);
int zut_test();
void zut_dump_storage(std::string, const void *, size_t);
int zut_hello(std::string);
char zut_get_hex_char(int);
int zut_get_current_user(std::string &);
void zut_uppercase_pad_truncate(std::string, char *, int);
char *zut_encode_alloc(char *rawData, const std::string &encoding, ZDIAG &diag, char **bufEnd);
std::string zut_format_as_csv(const std::initializer_list<std::string>& data);

#endif
