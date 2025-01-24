/*
  This program and the accompanying materials are
  made available under the terms of the Eclipse Public License v2.0 which accompanies
  this distribution, and is available at https://www.eclipse.org/legal/epl-v20.html

  SPDX-License-Identifier: EPL-2.0

  Copyright Contributors to the Zowe Project.
*/
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <cstring>
#include <vector>
#include <iomanip>
#include <stdio.h>
#include <istream>
#include <ctype.h>
#include <algorithm>
#include <unistd.h>
#include "zmetal.h"
#include "zts.hpp"
#include "ztsm.h"
#include "zut.hpp"
#include "ztstype.h"

int zts_init(ZTS *zts)
{
  int rc = 0;
  rc = ZTSINIT(zts);
  return rc;
}
int zts_invoke(ZTS *zts)
{
  int rc = 0;
  rc = ZTSINVOK(zts);
  return rc;
}
int zts_term(ZTS *zts)
{
  int rc = 0;
  rc = ZTSTERM(zts);
  return rc;
}