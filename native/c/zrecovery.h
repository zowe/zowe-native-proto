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
typedef struct sdwaptrs SDWAPTRS;
typedef struct sdwarc4 SDWARC4;

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
#define JUMP_ENV(f4sa, r13, rc)                                   \
  __asm(                                                          \
      "*                                                      \n" \
      " LA   15,%0            -> F4SA                         \n" \
      " LG   13,%1            = prev R13                      \n" \
      " LMG  14,14,8(15)      Restore R14                     \n" \
      " LMG  0,12,24(15)      Restore R0-R12                  \n" \
      " LGHI 15," #rc "            Clear RC                   \n" \
      " BR   14               Branch and never return         \n" \
      "*                                                        " \
      :                                                           \
      : "m"(f4sa),                                                \
        "m"(r13)                                                  \
      :);
#else
#define JUMP_ENV(f4sa, r13, rc)
#endif

#if defined(__IBM_METAL__)
#define RETURN_ARR(r14)                                         \
  __asm(                                                        \
      "*                                                    \n" \
      " LG   14,%0            = R14                         \n" \
      " BR   14               Branch and never return       \n" \
      "*                                                      " \
      :                                                         \
      : "m"(r14)                                                \
      :);
#else
#define RETURN_ARR(r14)
#endif

#if defined(__IBM_METAL__)
#ifndef _IHASDWA_DSECT
#define _IHASDWA_DSECT
#pragma insert_asm(" IHASDWA ")
#endif
#endif

#if defined(__IBM_METAL__)
#define SETRP(rc, retry_routine, sdwa)                        \
  __asm(                                                      \
      "*                                                  \n" \
      " SETRP RC=" #rc ","                                    \
      "RETREGS=NO,"                                           \
      "RETADDR=(%0),"                                         \
      "FRESDWA=YES,"                                          \
      "WKAREA=(%1),"                                          \
      "RECORD=YES                                         \n" \
      "*                                                  \n" \
      "*                                                    " \
      :                                                       \
      : "r"(retry_routine), "r"(sdwa)                         \
      : "r0", "r1", "r14", "r15");
#else
#define SETRP(rc, retry_routine, sdwa)
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

#define RTNCD_RETRY 4
#define RTNCD_PERCOLATE 0

#define NO_SDWA 12
typedef struct
{
  SAVF4SA f4sa;
  unsigned long long int r13;

  unsigned long long int intermediate_r14;

  SAVF4SA final_f4sa;
  unsigned long long int final_r13;

  unsigned int recovery_entered : 1;

} ZRCVY_ENV;

typedef int (*ROUTINE)(ZRCVY_ENV *);
typedef int (*RECOVERY_ROUTINE)(SDWA);

static int set_recovery(ROUTINE routine, void *routine_parm, RECOVERY_ROUTINE arr, void *arr_parm)
{
  IEAARR(
      routine,
      &routine_parm,
      arr,
      arr_parm);
}

#pragma prolog(ZRCVYRTY, " MYPROLOG ")
#pragma epilog(ZRCVYRTY, " MYEPILOG ")
// TODO(Kelosky): memory leak bc of JUMP_ENV
typedef void (*RETRY_ROUTINE)(ZRCVY_ENV *);
static void ZRCVYRTY(ZRCVY_ENV zenv)
{
  zwto_debug("@TEST retry routine entered zenv=%llx", zenv);
  JUMP_ENV(zenv.f4sa, zenv.r13, 4);
}

#pragma prolog(ZRCVYARR, "&CCN_MAIN SETB 1 \n MYPROLOG")
#pragma epilog(ZRCVYARR, "&CCN_MAIN SETB 1 \n MYEPILOG")
int ZRCVYARR(SDWA sdwa)
{
  unsigned long long int r0 = get_prev_r0(); // if current r0 = 12, NO_SDWA
  unsigned long long int r2 = get_prev_r2();
  ZRCVY_ENV *zenv = NULL;
  memcpy(&zenv, &r2, sizeof(r2));

  if (zenv->recovery_entered)
  {
    return RTNCD_PERCOLATE; // TODO(Kelosky): handle no SDWA, call recovery routine if provided, handle counter, & use SETRP
  }
  zenv->recovery_entered = 1;

  zwto_debug("@TEST recovery routine called");
  zwto_debug("@TEST r0=%llx, r2=%llx, zenv=%llx and sdwa=%llx", r0, r2, zenv, sdwa.sdwaparm);
  zwto_debug("@TEST abend=%02x%02x%02x%02x", sdwa.sdwacmpf, sdwa.sdwacmpc[0], sdwa.sdwacmpc[1], sdwa.sdwacmpc[2]);

  if (NO_SDWA == r0)
  {
    return RTNCD_PERCOLATE; // TODO(Kelosky): handle no SDWA, for now percolate, but we can retry
  }

  if (sdwa.sdwaerrd & sdwaclup) // clean up only
  {
    return RTNCD_PERCOLATE; // TODO(Kelosky): for now percolate, user SETRP if SDWA, call recovery routine if provided
  }

  zwto_debug("@TEST recovery preparing for retry called");

  // NOTE(Kelosky): needed for FRR
  // SDWAPTRS *PTR32 sdwaptrs = NULL;
  // memcpy(&sdwaptrs, &sdwa.sdwaxpad, sizeof(SDWAPTRS * PTR32));
  // zwto_debug("@TEST part1");
  // SDWARC4 *PTR32 sdwarc4 = NULL;
  // memcpy(&sdwarc4, &sdwaptrs->sdwaxeme, sizeof(SDWAPTRS * PTR32));
  // zwto_debug("@TEST part2");
  // memcpy(
  //     &sdwarc4->sdwag6401,
  //     &r2,
  //     sizeof(r2));

  RETRY_ROUTINE retry_function = ZRCVYRTY;

  SETRP(
      4, // RTNCD_RETRY
      retry_function,
      &sdwa);

  // NOTE(Kelosky): this is technically only needed if no SDWA
  unsigned long long int return_r0 = 0;
  memcpy(&return_r0, &retry_function, sizeof(return_r0));
  set_prev_r0(return_r0);

  return RTNCD_RETRY;
}

// TODO(Kelosky): memory leak #1... we need a custom prolog for this instance where we take storage from a work area
// anchored off ZRCVY_ENV.  We can then use this prolog here and other locations where we have a memory leak
// when we use the prolog and bypass our epilog
#pragma prolog(ZRCVYINT, " MYPROLOG ")
#pragma epilog(ZRCVYINT, " MYEPILOG ")
static int ZRCVYINT(ZRCVY_ENV *zenv)
{
  // unsigned long long int r13 = get_prev_r13();
  unsigned long long int r14 = get_prev_r14();
  // unsigned char *save_area = (unsigned char *)r13;

  zenv->intermediate_r14 = r14;

  zwto_debug("@TEST intermediate called");

  JUMP_ENV(zenv->f4sa, zenv->r13, 0);
}

static int recovery_drop(ZRCVY_ENV *zenv)
{
  unsigned long long int r13 = get_prev_r13();
  unsigned long long int r14 = get_prev_r14();
  unsigned char *save_area = (unsigned char *)r13;

  memcpy(&zenv->final_f4sa, save_area, sizeof(SAVF4SA));
  zenv->final_r13 = r13;

  zwto_debug("@TEST returning to IEAARR");
  RETURN_ARR(zenv->intermediate_r14);
}

#pragma reachable(set_env)

static int set_env(ZRCVY_ENV *zenv) ATTRIBUTE(noinline);
static int set_env(ZRCVY_ENV *zenv)
{
  unsigned long long int r13 = get_prev_r13();
  unsigned char *save_area = (unsigned char *)r13;

  memcpy(&zenv->f4sa, save_area, sizeof(SAVF4SA));
  zenv->r13 = r13;

  // NOTE(Kelosky): this code block shows that the previous R13 via get_prev_r13() is identical
  // f4sa->saveprev->savenext so that we probably don't need to store R13
  zwto_debug("@TEST prev r13 is %llx", r13);
  SAVF4SA *save_prev = NULL; // save_area + 144;
  memcpy(&save_prev, zenv->f4sa.savf4saprev, 8);
  SAVF4SA *save_next = NULL;
  memcpy(&save_next, save_prev->savf4sanext, 8);
  zwto_debug("@TEST prev comparison value is %llx", save_next);

  zwto_debug("@TEST zenv=%llx", zenv);
  set_recovery(ZRCVYINT, zenv, ZRCVYARR, zenv);

  // TODO(Kelosky): zwto_debug() has a bug here
  WTO_BUF buf = {0};
  buf.len = sprintf(buf.msg, "ZWEX0001I @TEST returned from IEAARR");
  wto(&buf);

  JUMP_ENV(zenv->final_f4sa, zenv->final_r13, 0);
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
