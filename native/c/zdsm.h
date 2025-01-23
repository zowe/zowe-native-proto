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

#ifndef ZDSM_H
#define ZDSM_H

#include "ztype.h"
#include "zdstype.h"
#include "iggcsina.h"

#if defined(__cplusplus) && (defined(__IBMCPP__) || defined(__IBMC__))
extern "OS"
{
#elif defined(__cplusplus)
extern "C"
{
#endif

  typedef struct csifield CSIFIELD;

  int ZDSSMSAT(ZDS *zds, const char *dsn);
  int ZDSCSI00(ZDS *zds, CSIFIELD *selection, void *work_area);

#if defined(__cplusplus)
}
#endif

#endif