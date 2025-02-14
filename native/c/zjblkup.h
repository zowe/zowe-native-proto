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

#ifndef ZJBLKUP_H
#define ZJBLKUP_H

#include "zmetal.h"
#include "zssitype.h"
#include "iaztlkdf.h"
#include "zwto.h"

#if defined(__IBM_METAL__)
#ifndef _IAZTLKDF_DSECT
#define _IAZTLKDF_DSECT
#pragma insert_asm(" IAZTLKDF ")
#endif
#endif

#if defined(__IBM_METAL__)
#define IAZTLKUP(ssob, datastr, outarea, outlen, plist, rc)   \
  __asm(                                                      \
      "*                                                  \n" \
      " SLGR  0,0       Save RC                           \n" \
      "*                                                  \n" \
      " IAZTLKUP LEVEL=1,"                                    \
      "SSOB=%2,"                                              \
      "TABLEID=PHZ,"                                          \
      "DATASTR=%3,"                                           \
      "OUTAREA=%0,"                                           \
      "OUTLEN=(%4),"                                          \
      "MF=(E,%5)                                          \n" \
      "*                                                  \n" \
      " ST    15,%1     Save RC                           \n" \
      "*                                                   "  \
      : "+m"(outarea),                                        \
        "=m"(rc)                                              \
      : "m"(ssob),                                            \
        "m"(datastr),                                         \
        "r"(outlen),                                          \
        "m"(plist)                                            \
      : "r0", "r1", "r14", "r15");
#else
#define IAZTLKUP(ssob, datastr, outarea, outlen, plist, rc)
#endif

static int iaztlkup(SSOB *ssob, STATJQ *statjq)
{
  int rc = 0;
  char response[64] = {0};
  int response_len = sizeof(response);
  struct tlkup plist = {0};
  IAZTLKUP(*ssob,
           *statjq,
           response[0],
           response_len,
           plist,
           rc);

  zwto_debug("@TEST %d %s", rc, response);

  return rc;
}

#endif
