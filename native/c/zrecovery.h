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
#include <string.h>
#include "zwto.h"
#include "zmetal.h"
#include "ihasdwa.h"
#include "ihasaver.h"

typedef struct sdwa SDWA;
typedef struct savf4sa SAVF4SA;

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

// flow
// set_env
// raise recovery
// if abend, it's like long jump

// TODO(Kelosky): save stack
#pragma reachable(set_env)
// #pragma prolog(set_env, "&CCN_NAB_STORED SETB 1 \n&CCN_RLOW SETA 14 \n&CCN_RHIGH SETA 12")
// #pragma epilog(set_env, "&CCN_NAB_STORED SETB 1 \n&CCN_RLOW SETA 14 \n&CCN_RHIGH SETA 12")
#pragma prolog(set_env, "&CCN_RLOW SETA 14 \n&CCN_RHIGH SETA 5 \n&CCN_MAIN SETB 0 \n MYPROLOG")
#pragma epilog(set_env, "&CCN_RLOW SETA 14 \n&CCN_RHIGH SETA 5 \n&CCN_MAIN SETB 0 \n MYEPILOG")

static int set_env(SAVF4SA *f4sa) ATTRIBUTE(noinline);
static int set_env(SAVF4SA *f4sa)
{
  unsigned long long int r13 = get_prev_r13();
  unsigned char *save_area = (unsigned char *)r13;
  save_area -= 128;
  zwto_debug("@TEST %llx and %x", r13, save_area);

  // zwto_debug("@TEST before %02x%02x%02x%02x%02x%02x%02x%02x",
  //            f4sa->savf4sag64rs5[0],
  //            f4sa->savf4sag64rs5[1],
  //            f4sa->savf4sag64rs5[2],
  //            f4sa->savf4sag64rs5[3],
  //            f4sa->savf4sag64rs5[4],
  //            f4sa->savf4sag64rs5[5],
  //            f4sa->savf4sag64rs5[6],
  //            f4sa->savf4sag64rs5[7]);

  memcpy(f4sa, save_area, sizeof(SAVF4SA));
  // memcpy(regs, save_area + 16, 8);  // r15
  // memcpy(regs, save_area + 24, 8);  // r0

  zwto_debug("@TEST after %x%x%x%x%x%x%x%02x",
             f4sa->savf4sag64rs5[0],
             f4sa->savf4sag64rs5[1],
             f4sa->savf4sag64rs5[2],
             f4sa->savf4sag64rs5[3],
             f4sa->savf4sag64rs5[4],
             f4sa->savf4sag64rs5[5],
             f4sa->savf4sag64rs5[6],
             f4sa->savf4sag64rs5[7]);

  zwto_debug("@TEST after %02x%02x%02x%02x%02x%02x%02x%02x",
             f4sa->savf4sag64rs6[0],
             f4sa->savf4sag64rs6[1],
             f4sa->savf4sag64rs6[2],
             f4sa->savf4sag64rs6[3],
             f4sa->savf4sag64rs6[4],
             f4sa->savf4sag64rs6[5],
             f4sa->savf4sag64rs6[6],
             f4sa->savf4sag64rs6[7]);

  zwto_debug("@TEST after %02x%02x%02x%02x%02x%02x%02x%02x",
             f4sa->savf4sag64rs7[0],
             f4sa->savf4sag64rs7[1],
             f4sa->savf4sag64rs7[2],
             f4sa->savf4sag64rs7[3],
             f4sa->savf4sag64rs7[4],
             f4sa->savf4sag64rs7[5],
             f4sa->savf4sag64rs7[6],
             f4sa->savf4sag64rs7[7]);
  // zwto_debug("@TEST save area is %x", r13);
  // memcpy(&regs

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
