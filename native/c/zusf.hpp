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


/**
 * @brief Read data from a z/OS USS file
 *
 * @param zusf USS file returned attributes and error information
 * @param file USS file name from which to read
 * @param response data read
 * @param encoding The desired encoding for the USS file (optional)
 * @return int 0 for success; non zero otherwise
 */
int zusf_read_from_uss_file(ZUSF *zusf, std::string file, std::string &response, std::string mode);

#endif
