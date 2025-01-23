/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0 & Apache-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

#ifndef ZCNM_H
#define ZCNM_H

#include "ztype.h"
#include "zcntype.h"

#if defined(__cplusplus) && (defined(__IBMCPP__) || defined(__IBMC__))
extern "OS"
{
#elif defined(__cplusplus)
extern "C"
{
#endif

  int ZCNACT(ZCN *);
  int ZCNPUT(ZCN *, const char *);
  int ZCNGET(ZCN *, char *);
  int ZCNDACT(ZCN *);

#if defined(__cplusplus)
}
#endif

#endif