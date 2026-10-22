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

#ifndef ZSPOOL_HPP
#define ZSPOOL_HPP

#include <string>
#include "ztype.h"

struct ZSpoolOptions
{
  std::string dsname;
  unsigned char token[80] = {};
};

int zspool_allocate(ZDIAG &diag, const ZSpoolOptions &opts, std::string &ddname);
int zspool_free(ZDIAG &diag, const std::string &ddname);

#endif
