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

#define RTNCD_RETRY 0
#define RTNCD_PERCOLATE 4
#define NO_SDWA 12

typedef struct
{
  SAVF4SA f4sa;
  unsigned long long int r13;
} ZRCVY_ENV;

typedef int (*ROUTINE)(ZRCVY_ENV *);
typedef int (*RECOVERY_ROUTINE)(SDWA *);

static int set_recovery(ROUTINE routine, void *routine_parm, RECOVERY_ROUTINE arr, void *arr_parm)
{
  IEAARR(
      routine,
      &routine_parm, // parm for routine
      arr,
      arr_parm); // @TEST@TEST@TEST@TEST@TEST@TEST@TEST this might need another level of indirection
}

#pragma prolog(ZRCVYARR, "&CCN_MAIN SETB 1 \n MYPROLOG")
int ZRCVYARR(SDWA *sdwa)
{
  unsigned long long int r0 = get_r0(); // if current r0 = 12, NO_SDWA
  unsigned long long int r2 = get_r2(); // TODO(Kelosky): this should not be prev, it is EASTEX parm

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
// flow
// set_env
// raise recovery
// if abend, long jump

// @TEST@TEST@TEST@TEST@TEST@TEST@TEST memory leak if we never return to this routine and skip epilog!!!!
#pragma prolog(ZRCVYINT, "&CCN_MAIN SETB 1 \n MYPROLOG")
// static int ZRCVYINT(char *message)
static int ZRCVYINT(ZRCVY_ENV *zenv)
{
  zwto_debug("@TEST intermediate called");
  // zwto_debug("@TEST %llx", message);
  // zwto_debug("@TEST %s", message);

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

  // unsigned char *save_prev = NULL;
  // memcpy(&save_prev, f4sa->savf4saprev, 8);
  // zwto_debug("@TEST saveprev + 88 %llx", save_prev + 88);

  zwto_debug("@TEST r13prev %02x%02x%02x%02x%02x%02x%02x%02x",
             zenv->f4sa.savf4sanext[0],
             zenv->f4sa.savf4sanext[1],
             zenv->f4sa.savf4sanext[2],
             zenv->f4sa.savf4sanext[3],
             zenv->f4sa.savf4sanext[4],
             zenv->f4sa.savf4sanext[5],
             zenv->f4sa.savf4sanext[6],
             zenv->f4sa.savf4sanext[7]);

  zwto_debug("@TEST r13 %llx", zenv->r13);

  __asm(" LG 13,%1 \n LMG 14,14,8(13) \n LMG 0,12,24(13) \n SLGR 15,15 \n BR 14 \n"
        // __asm(" LG 13,%1 \n LA 13,%0 \n LMG 14,14,8(13) \n LMG 0,14,24(13) \n LGR 13,15 SLGR 15,15 \n BR 14 \n"
        :
        : "m"(zenv->f4sa), "m"(zenv->r13)
        : "r13");
}

static int recovery_drop()
{
  __asm(" PR ");
}

#pragma reachable(set_env)
// #pragma prolog(set_env, "&CCN_NAB_STORED SETB 1 \n&CCN_RLOW SETA 14 \n&CCN_RHIGH SETA 12")
// #pragma epilog(set_env, "&CCN_NAB_STORED SETB 1 \n&CCN_RLOW SETA 14 \n&CCN_RHIGH SETA 12")

// #pragma prolog(set_env, "&CCN_RLOW SETA 14 \n&CCN_RHIGH SETA 12 \n&CCN_MAIN SETB 0 \n MYPROLOG")
// #pragma epilog(set_env, "&CCN_RLOW SETA 14 \n&CCN_RHIGH SETA 12 \n&CCN_MAIN SETB 0 \n MYEPILOG")

static int set_env(SAVF4SA *f4sa) ATTRIBUTE(noinline);
static int set_env(SAVF4SA *f4sa)
{
  ZRCVY_ENV zenv = {0};
  unsigned long long int r13 = get_prev_r13();
  unsigned char *save_area = (unsigned char *)r13;

  // unsigned long long int regs[5] = {0};
  // regs[0] = get_r5();
  // regs[1] = get_r6();
  // regs[2] = get_r7();
  // regs[3] = get_r13();

  // char *dumb_data = "happy";

  memcpy(f4sa, save_area, sizeof(SAVF4SA));
  memcpy(&zenv.f4sa, f4sa, sizeof(SAVF4SA));
  zenv.r13 = r13;

  zwto_debug("@TEST inside %llx", r13);

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

  // set_recovery(ZRCVYINT, dumb_data, ZRCVYARR, NULL);
  set_recovery(ZRCVYINT, &zenv, ZRCVYARR, NULL);

  // we saved the caller's stack
  // next we establish an ARR
  // we need to save r14 from the ARR so that we can later "drop"
  // after ARR is established and r14 is saved, restore stack position with rc = 0
  // if there is an abend and we recover w/retry we restore stack position with non-rc = 0;
}

// typedef struct
// {
//   unsigned long long int regs[16];
//   // TODO(Kelosky): add a recovery routine to be called on abend
//   // TODO(Kelosky): add a parameter to be passed to user recovery routine
//   // TODO(Kelosky): add flag to percolate
// } ZRCVY;

#endif
