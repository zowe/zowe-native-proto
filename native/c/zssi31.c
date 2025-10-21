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

#include "zmetal.h"
#include "zssi31.h"

#if defined(__IBM_METAL__)
#ifndef _IEFJESCT_DSECT
#define _IEFJESCT_DSECT
#pragma insert_asm(" IEFJESCT ")
#endif
#ifndef _CVT_DSECT
#define _CVT_DSECT
#pragma insert_asm(" CVT   LIST=NO,DSECT=YES ")
#endif
#endif

#if defined(__IBM_METAL__)
#define IEFSSREQ(ssob, rc)                                    \
  __asm(                                                      \
      "*                                                  \n" \
      " LA 1,%0        -> SSSOB                           \n" \
      "*                                                  \n" \
      " IEFSSREQ                                          \n" \
      "*                                                  \n" \
      " ST 15,%1                                          \n" \
      "*                                                    " \
      : "+m"(*(unsigned char *)ssob), "=m"(rc)                \
      :                                                       \
      : "r0", "r1", "r14", "r15");
#else
#define IEFSSREQ(ssob, rc)
#endif

// https://www.ibm.com/docs/en/zos/3.1.0?topic=subsystem-making-request-iefssreq-macro
int iefssreq(SSOB * PTR32 * PTR32 ssob)
{
  int rc = 0;
  IEFSSREQ(ssob, rc);
  return rc;
}
