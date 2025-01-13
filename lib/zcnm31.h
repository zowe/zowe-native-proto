/*
  This program and the accompanying materials are
  made available under the terms of the Eclipse Public License v2.0 which accompanies
  this distribution, and is available at https://www.eclipse.org/legal/epl-v20.html

  SPDX-License-Identifier: EPL-2.0

  Copyright Contributors to the Zowe Project.
*/
#ifndef ZCNM31_H
#define ZCNM31_H

#include "ztype.h"
#include "zcntype.h"

int zcnm1act(ZCN *) ATTRIBUTE(amode31);
int zcnm1put(ZCN *, const char *) ATTRIBUTE(amode31);
int zcnm1get(ZCN *, char *) ATTRIBUTE(amode31,armode);
int zcnm1dea(ZCN *) ATTRIBUTE(amode31);

#endif