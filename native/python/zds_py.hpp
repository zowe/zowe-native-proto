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

#ifndef ZDS_PY_HPP
#define ZDS_PY_HPP

#include <string>
#include <vector>
#include "../c/zdstype.h"
#include "../c/zds.hpp"

std::vector<ZDSEntry> list_data_sets(std::string dsn);

#endif