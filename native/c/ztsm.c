/*
  This program and the accompanying materials are
  made available under the terms of the Eclipse Public License v2.0 which accompanies
  this distribution, and is available at https://www.eclipse.org/legal/epl-v20.html

  SPDX-License-Identifier: EPL-2.0

  Copyright Contributors to the Zowe Project.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "zjbm.h"
#include "zwto.h"
#include "zstorage.h"
#include "zmetal.h"
#include "ztstype.h"

#pragma prolog(ZTSINIT, "&CCN_MAIN SETB 1 \n MYPROLOG")
int ZTSINIT(ZTS *zts)
{
  return 1;
}

#pragma prolog(ZTSINVOK, "&CCN_MAIN SETB 1 \n MYPROLOG")
int ZTSINVOK(ZTS *zts)
{
  return 2;
}

#pragma prolog(ZTSTERM, "&CCN_MAIN SETB 1 \n MYPROLOG")
int ZTSTERM(ZTS *zts)
{
  return 3;
}
