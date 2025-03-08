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

#if defined(__IBM_METAL__)
#define JUMP_ENV(f4sa, r13)                                       \
  __asm(                                                          \
      "*                                                      \n" \
      " LA   15,%0            -> F4SA                         \n" \
      " LG   13,%1            = prev R13                      \n" \
      " LMG  14,14,8(15)      Restore R14                     \n" \
      " LMG  0,12,24(15)      Restore R0-R12                  \n" \
      " SLGR 15,15            Clear RC                        \n" \
      " BR   14               Branch and never return         \n" \
      "*                                                        " \
      :                                                           \
      : "m"(f4sa),                                                \
        "m"(r13)                                                  \
      :);
#else
#define JUMP_ENV(f4sa, r13)
#endif

/**
 * TODO(Kelosky): tech debt
 * - ensure no memory leaks
 * - ensure RLOW and RHIGH set properly
 * - ensure matching prolog / epilog
 * - ensure CCN_NAB bit set properly for all
 *
 * TODO(Kelosky): features
 * - abend counter
 * - detect recursive abends
 * - SETRP & VRA data wrappers
 */

#define RTNCD_RETRY 0
#define RTNCD_PERCOLATE 4
#define NO_SDWA 12

typedef struct
{
  SAVF4SA f4sa;
  unsigned long long int r13;

  SAVF4SA intermediate_f4sa;
  unsigned long long int intermediate_r13;

  unsigned long long int intermediate_r14;

  SAVF4SA final_f4sa;
  unsigned long long int final_r13;

} ZRCVY_ENV;

typedef int (*ROUTINE)(ZRCVY_ENV *);
typedef int (*RECOVERY_ROUTINE)(SDWA);

static int set_recovery(ROUTINE routine, void *routine_parm, RECOVERY_ROUTINE arr, void *arr_parm)
{
  IEAARR(
      routine,
      &routine_parm, // parm for routine
      arr,
      arr_parm); // @TEST@TEST@TEST@TEST@TEST@TEST@TEST this might need another level of indirection
}

#pragma prolog(ZRCVYARR, "&CCN_MAIN SETB 1 \n MYPROLOG")
#pragma eiplog(ZRCVYARR, "&CCN_MAIN SETB 1 \n MYEPILOG")
int ZRCVYARR(SDWA sdwa)
{
  unsigned long long int r0 = get_prev_r0(); // if current r0 = 12, NO_SDWA
  unsigned long long int r2 = get_prev_r2();

  zwto_debug("@TEST recovery routine called");
  zwto_debug("@TEST r0=%llx, r2=%llx, and sdwa=%llx", r0, r2, sdwa.sdwaparm);
  zwto_debug("@TEST abend=%02x%02x%02x%02x", sdwa.sdwacmpf, sdwa.sdwacmpc[0], sdwa.sdwacmpc[1], sdwa.sdwacmpc[2]);

  if (NO_SDWA == r0)
  {
    return RTNCD_PERCOLATE; // TODO(Kelosky): handle no SDWA, for now percolate
  }
  // TODO(Kelosky): check r0 for 12, meaning no SDWA
  zwto_debug("@TEST recovery routine called");
  // __asm(" exrl 0,*");

  // zwto_debug("@TEST %llx and %llx", r0, r2);

  return RTNCD_PERCOLATE;
}

#pragma prolog(ZRCVYINT, " MYPROLOG ")
#pragma epilog(ZRCVYINT, " MYEPILOG ")
static int ZRCVYINT(ZRCVY_ENV *zenv)
{
  unsigned long long int r13 = get_prev_r13();
  unsigned long long int r14 = get_prev_r14();
  unsigned char *save_area = (unsigned char *)r13;

  memcpy(&zenv->intermediate_f4sa, save_area, sizeof(SAVF4SA));
  zenv->intermediate_r13 = r13;
  zenv->intermediate_r14 = r14;

  zwto_debug("@TEST intermediate called");

  JUMP_ENV(zenv->f4sa, zenv->r13);
}

static int recovery_drop(ZRCVY_ENV *zenv)
{
  unsigned long long int r13 = get_prev_r13();
  unsigned long long int r14 = get_prev_r14();
  unsigned char *save_area = (unsigned char *)r13;

  memcpy(&zenv->final_f4sa, save_area, sizeof(SAVF4SA));
  zenv->final_r13 = r13;

  zwto_debug("@TEST returning to IEAARR");
  __asm(" LG 14,%0 \n BR 14 \n"
        :
        : "m"(zenv->intermediate_r14)
        : "r14");
}

#pragma reachable(set_env)

static int set_env(ZRCVY_ENV *zenv) ATTRIBUTE(noinline);
static int set_env(ZRCVY_ENV *zenv)
{
  unsigned long long int r13 = get_prev_r13();
  unsigned char *save_area = (unsigned char *)r13;

  memcpy(&zenv->f4sa, save_area, sizeof(SAVF4SA));
  zenv->r13 = r13;

  zwto_debug("@TEST zenv=%llx", zenv);
  set_recovery(ZRCVYINT, zenv, ZRCVYARR, zenv);

  // TODO(Kelosky): zwto_debug() has a bug here
  WTO_BUF buf = {0};
  buf.len = sprintf(buf.msg, "ZWEX0001I @TEST returned from IEAARR");
  wto(&buf);

  JUMP_ENV(zenv->final_f4sa, zenv->final_r13);
}

// #pragma prolog(set_env, "&CCN_NAB_STORED SETB 1 \n&CCN_RLOW SETA 14 \n&CCN_RHIGH SETA 12")
// #pragma epilog(set_env, "&CCN_NAB_STORED SETB 1 \n&CCN_RLOW SETA 14 \n&CCN_RHIGH SETA 12")
// #pragma prolog(set_env, "&CCN_RLOW SETA 14 \n&CCN_RHIGH SETA 12 \n&CCN_MAIN SETB 0 \n MYPROLOG")
// #pragma epilog(set_env, "&CCN_RLOW SETA 14 \n&CCN_RHIGH SETA 12 \n&CCN_MAIN SETB 0 \n MYEPILOG")

// void deadcode()
// {
// zwto_debug("@TEST after %02x%02x%02x%02x%02x%02x%02x%02x",
//            f4sa->savf4sag64rs5[0],
//            f4sa->savf4sag64rs5[1],
//            f4sa->savf4sag64rs5[2],
//            f4sa->savf4sag64rs5[3],
//            f4sa->savf4sag64rs5[4],
//            f4sa->savf4sag64rs5[5],
//            f4sa->savf4sag64rs5[6],
//            f4sa->savf4sag64rs5[7]);

// zwto_debug("@TEST after %02x%02x%02x%02x%02x%02x%02x%02x",
//            f4sa->savf4sag64rs6[0],
//            f4sa->savf4sag64rs6[1],
//            f4sa->savf4sag64rs6[2],
//            f4sa->savf4sag64rs6[3],
//            f4sa->savf4sag64rs6[4],
//            f4sa->savf4sag64rs6[5],
//            f4sa->savf4sag64rs6[6],
//            f4sa->savf4sag64rs6[7]);

// zwto_debug("@TEST after %02x%02x%02x%02x%02x%02x%02x%02x",
//            f4sa->savf4sag64rs7[0],
//            f4sa->savf4sag64rs7[1],
//            f4sa->savf4sag64rs7[2],
//            f4sa->savf4sag64rs7[3],
//            f4sa->savf4sag64rs7[4],
//            f4sa->savf4sag64rs7[5],
//            f4sa->savf4sag64rs7[6],
//            f4sa->savf4sag64rs7[7]);
// }

#endif
