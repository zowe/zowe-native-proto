/*
  This program and the accompanying materials are
  made available under the terms of the Eclipse Public License v2.0 which accompanies
  this distribution, and is available at https://www.eclipse.org/legal/epl-v20.html

  SPDX-License-Identifier: EPL-2.0

  Copyright Contributors to the Zowe Project.
*/
#ifndef ZTS_HPP
#define ZTS_HPP

#include <iostream>
#include <vector>
#include <string>
#include "ztstype.h"

int zts_init_env(ZTS *zts);
int zts_init(ZTS *zts);
int zts_invoke(ZTS *zts);
int zts_term(ZTS *zts);

#endif
