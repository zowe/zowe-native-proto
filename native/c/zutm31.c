/*
  This program and the accompanying materials are
  made available under the terms of the Eclipse Public License v2.0 which accompanies
  this distribution, and is available at https://www.eclipse.org/legal/epl-v20.html

  SPDX-License-Identifier: EPL-2.0

  Copyright Contributors to the Zowe Project.
*/
#include "zmetal.h"
#include "zutm31.h"
#include "zwto.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__IBM_METAL__)
#ifndef _IAZJSAB_DSECT
#define _IAZJSAB_DSECT
#pragma insert_asm(" IAZJSAB ")
#endif
#ifndef _IHAASCB_DSECT
#define _IHAASCB_DSECT
#pragma insert_asm(" IHAASCB ")
#endif
#ifndef _IHAASSB_DSECT
#define _IHAASSB_DSECT
#pragma insert_asm(" IHAASSB ")
#endif
#ifndef _IHAPSA_DSECT
#define _IHAPSA_DSECT
#pragma insert_asm(" IHAPSA ")
#endif
#ifndef _IKJTCB_DSECT
#define _IKJTCB_DSECT
#pragma insert_asm(" IKJTCB ")
#endif
#ifndef _IHASTCB_DSECT
#define _IHASTCB_DSECT
#pragma insert_asm(" IHASTCB ")
#endif
#endif

#if defined(__IBM_METAL__)
#define IAZXJSAB(user, rc)                                    \
  __asm(                                                      \
      "*                                                  \n" \
      " L 2,%0         -> User                            \n" \
      "*                                                  \n" \
      " IAZXJSAB READ,USERID=(2)                          \n" \
      "*                                                  \n" \
      " ST 15,%1                                          \n" \
      "*                                                    " \
      : "+m"(user), "=m"(rc)                                  \
      :                                                       \
      : "r0", "r1", "r2", "r14", "r15");
#else
#define IAZXJSAB(ssob, rc)
#endif

// https://www.ibm.com/docs/en/zos/3.1.0?topic=ixg-iazxjsab-obtain-information-about-currently-running-job
int zutm1gur(char user[8])
{
  int rc = 0;
  IAZXJSAB(user, rc);
  return rc;
}
