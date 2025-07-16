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

#ifndef ZUSF_PY_H
#define ZUSF_PY_H

#include <string>
#include <stdexcept>
#include "../c/zusf.hpp"

std::string list_uss_dir(const std::string &path);

#endif