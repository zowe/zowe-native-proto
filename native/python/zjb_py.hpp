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

#ifndef ZJB_PY_HPP
#define ZJB_PY_HPP

#include "../c/zjb.hpp"
#include "../c/ztype.h"
#include <string>
#include <vector>

std::vector<ZJob> list_jobs_by_owner(std::string owner_name);

#endif