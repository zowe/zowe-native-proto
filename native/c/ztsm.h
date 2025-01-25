/*
  This program and the accompanying materials are
  made available under the terms of the Eclipse Public License v2.0 which accompanies
  this distribution, and is available at https://www.eclipse.org/legal/epl-v20.html

  SPDX-License-Identifier: EPL-2.0

  Copyright Contributors to the Zowe Project.
*/
#ifndef ZTSM_H
#define ZTSM_H

#include "ztype.h"
#include "ztstype.h"

#if defined(__cplusplus) && (defined(__IBMCPP__) || defined(__IBMC__))
extern "OS"
{
#elif defined(__cplusplus)
extern "C"
{
#endif

  int ZTSIENV(ZTS *PTR64);
  int ZTSINIT(ZTS *PTR64);
  int ZTSINVOK(ZTS *PTR64);
  int ZTSTERM(ZTS *PTR64);

#if defined(__cplusplus)
}
#endif

#endif
