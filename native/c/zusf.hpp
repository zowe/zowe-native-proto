/*
  This program and the accompanying materials are
  made available under the terms of the Eclipse Public License v2.0 which accompanies
  this distribution, and is available at https://www.eclipse.org/legal/epl-v20.html

  SPDX-License-Identifier: EPL-2.0

  Copyright Contributors to the Zowe Project.
*/
#ifndef ZUSF_HPP
#define ZUSF_HPP

#include <iostream>
#include <vector>
#include <string>
#include "zusf.hpp"
#include "zusftype.h"

int zusf_create_uss_file_or_dir(ZUSF *zusf, std::string file, std::string mode, bool createDir);
int zusf_list_uss_file_path(ZUSF *zusf, std::string file, std::string &response);
int zusf_read_from_uss_file(ZUSF *zusf, std::string file, std::string &response);
int zds_write_to_uss_file(ZUSF *zusf, std::string file, std::string &response);

#endif
