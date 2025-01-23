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

#ifndef ZATTACHX_H
#define ZATTACHX_H

#include <stdio.h>
#include <string.h>

#include "zmetal.h"
#include "zecb.h"
#include "zwto.h"

#if defined(__IBM_METAL__)
#define ATTACHX_MODEL(attachxm)                               \
  __asm(                                                      \
      "*                                                  \n" \
      " ATTACHX "                                             \
      "KEY=PROB,"                                             \
      "PLIST8=YES,"                                           \
      "ETXR=,"                                                \
      "EPLOC=,"                                               \
      "SF=L                                               \n" \
      "*                                                    " \
      : "DS"(attachxm));
#else
#define ATTACHX_MODEL(attachxm) void *attachxm;
#endif

ATTACHX_MODEL(attachx_model); // make this copy in static storage

#if defined(__IBM_METAL__)
#define ATTACHX(routine, exit, plist, rc)                     \
  __asm(                                                      \
      "*                                                  \n" \
      "*                                                  \n" \
      " ATTACHX EPLOC=(%2),"                                  \
      "PLIST8=YES,"                                           \
      "ETXR=(%3),"                                            \
      "SF=(E,%0)                                          \n" \
      "*                                                  \n" \
      " ST    15,%1     Save RC                           \n" \
      "*                                                    " \
      : "+m"(plist),                                          \
        "=m"(rc)                                              \
      : "r"(routine), "r"(exit)                               \
      : "r0", "r1", "r14", "r15");
#else
#define ATTACHX(routine, exit, plist, rc)
#endif

#pragma prolog(ZUTATTCH, "&CCN_MAIN SETB 1 \n MYPROLOG")
static void ZUTATTCH()
{
  zwto_debug("@TEST");
  return;
}

static int attachx()
{
  int rc = 0;
  ATTACHX_MODEL(dsa_attachx_model);  // stack var
  dsa_attachx_model = attachx_model; // copy model
  char *routine = "12345678";
  // char *routine = "IEFBR14 ";
  ATTACHX(routine, ZUTATTCH, dsa_attachx_model, rc);
  return rc;
}

#endif
