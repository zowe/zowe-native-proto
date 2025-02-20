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

#ifndef ZRECOVERY_H
#define ZRECOVERY_H
#include <stdio.h>
#include "zmetal.h"
#include "ihasdwa.h"

typedef struct sdwa SDWA;

// https://www.ibm.com/docs/en/zos/3.1.0?topic=ixg-ieaarr-establish-associated-recovery-routine-arr
#if defined(__IBM_METAL__)
#define IEAARR(routine, parm, arr, arr_parm)                      \
  __asm(                                                          \
      "*                                                      \n" \
      " IEAARR "                                                  \
      "DYNSTORAGE=NOTAVAIL,"                                      \
      "ARRPARAM64=(%3),"                                          \
      "PARAM64=(%1),"                                             \
      "TARGETSTATE=PROB,"                                         \
      "ARR=(%2),"                                                 \
      "TARGET=(%0)                                            \n" \
      "*                                                      \n" \
      "*                                                        " \
      :                                                           \
      : "r"(routine),                                             \
        "r"(parm),                                                \
        "r"(arr),                                                 \
        "r"(arr_parm)                                             \
      : "r0", "r1", "r14", "r15");
#else
#define IEAARR(routine, parm, arr, arr_parm)
#endif

// TODO(Kelosky): save stack
#pragma reachable(set_env)
#pragma prolog(set_env, "&CCN_RLOW SETA 14 \n&CCN_RHIGH SETA 12 \n MYPROLOG")
#pragma epilog(set_env, "&CCN_RLOW SETA 14 \n&CCN_RHIGH SETA 12 \n MYEPILOG")
static int set_env(unsigned long long int regs[16]) ATTRIBUTE(noinline);
static int set_env(unsigned long long int regs[16])
{
  unsigned long long int r13 = get_prev_r13();
  // memcpy registers
  // set r15 = 0
}

typedef struct
{
  unsigned long long int regs[16];
  // TODO(Kelosky): add a recovery routine to be called on abend
  // TODO(Kelosky): add a parameter to be passed to user recovery routine
  // TODO(Kelosky): add flag to percolate
} ZRCVY;

#define RTNCD_RETRY 0
#define RTNCD_PERCOLATE 4
#define NO_SDWA 12

typedef int (*ROUTINE)(void);
typedef int (*RECOVERY_ROUTINE)(SDWA *);

static int set_recovery(ROUTINE routine, ROUTINE arr, void *data)
{

  IEAARR(
      routine,
      NULL, // parm for routine
      arr,
      data);
}

#endif
