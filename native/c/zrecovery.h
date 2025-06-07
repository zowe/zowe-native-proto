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
      " LGHI 15," #rc "       Set RC                          \n" \
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
#define RETURN_TO_IEAARR(r14)                                   \
  __asm(                                                        \
      "*                                                    \n" \
      " LG   14,%0            = R14                         \n" \
      " BR   14               Branch and never return       \n" \
      "*                                                      " \
      :                                                         \
      : "m"(r14)                                                \
      :);
#else
#define RETURN_TO_IEAARR(r14)
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
      "*                                                    " \
      :                                                       \
      : "r"(retry_routine), "r"(sdwa)                         \
      : "r0", "r1", "r14", "r15");
#else
#define SETRP(rc, retry_routine, sdwa)
#endif

/**
 * TODO(Kelosky): features
 * - test for TRAP(ON,NOSPIE)
 * - abend counter
 * - VRA data wrappers
 * - get abend info, code, psw, bea, load module, and offset
 */

#define RTNCD_RETRY 4
#define RTNCD_PERCOLATE 0

#define NO_SDWA 12

typedef void (*ABEND_EXIT)(SDWA *, void *); // called to enrich VRA data
typedef void (*PERCOLATE_EXIT)(void *);     // called only for percolated abends, may be called manually in "retry" logic

typedef struct
{

  // return address to IEAARR
  unsigned long long int arr_return;

  // NOTE(Kelosky): we can also dervive R13 using f4sa->saveprev->savenext so that we probably don't need to store R13
  unsigned long long int r13;

  // main line stack regs and pointer (r13)
  SAVF4SA f4sa;

  // main line stack regs and pointer (r13)
  unsigned long long int final_r13;
  SAVF4SA final_f4sa;

  ABEND_EXIT abexit;
  void *abexit_data;

  PERCOLATE_EXIT perc_exit;
  void *perc_exit_data;

  int max_retries;

  // flags
  unsigned int recovery_entered : 1;
  unsigned int request_dump : 1;
  unsigned int unconditional_percolate : 1;

} ZRCVY_ENV;

typedef void (*ROUTINE)(ZRCVY_ENV *);
typedef int (*RECOVERY_ROUTINE)(SDWA);

static void ieaarr(ROUTINE routine, void *routine_parm, RECOVERY_ROUTINE arr, void *arr_parm)
{
  IEAARR(
      routine,
      &routine_parm,
      arr,
      arr_parm);
}

#pragma prolog(ZRCVYRTY, " ZWEPROLG NEWDSA=NO ")
#pragma epilog(ZRCVYRTY, " ZWEEPILG ")
typedef void (*RETRY_ROUTINE)(ZRCVY_ENV);
static void ZRCVYRTY(ZRCVY_ENV zenv)
{
  JUMP_ENV(zenv.f4sa, zenv.r13, 4); // TODO(Kelosky): document non-zero return code
}

#pragma prolog(ZRCVYARR, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(ZRCVYARR, " ZWEEPILG ")
int ZRCVYARR(SDWA sdwa)
{
  unsigned long long int r0 = get_prev_r0(); // if r0 = 12, NO_SDWA
  unsigned long long int r2 = get_prev_r2();
  ZRCVY_ENV *zenv = NULL;
  memcpy(&zenv, &r2, sizeof(r2));

  if (NO_SDWA == r0) // no sdwa
  {
    // NOTE(Kelosky): we can use this block + RTNCD_RETRY if no SDWA to attempt retry
    // unsigned long long int return_r0 = 0;
    // memcpy(&return_r0, &retry_function, sizeof(return_r0));
    // set_prev_r0(return_r0);
    if (zenv->perc_exit)
      zenv->perc_exit(zenv->perc_exit_data);
    return RTNCD_PERCOLATE; // TODO(Kelosky): handle no SDWA, for now percolate, but we can retry
  }

  if (zenv->recovery_entered) // repeated abends
  {
    if (zenv->perc_exit)
      zenv->perc_exit(zenv->perc_exit_data);
    return RTNCD_PERCOLATE; // TODO(Kelosky): handle no SDWA, call recovery routine if provided, handle counter, & use SETRP
  }
  zenv->recovery_entered = 1;

  if (sdwa.sdwaerrd & sdwaclup) // clean up only
  {
    if (zenv->perc_exit)
      zenv->perc_exit(zenv->perc_exit_data);
    return RTNCD_PERCOLATE; // TODO(Kelosky): for now percolate, user SETRP if SDWA, call recovery routine if provided
  }

  // TODO(Kelosky): capture diag info here

  if (zenv->abexit)
    zenv->abexit(&sdwa, zenv->abexit_data);

  RETRY_ROUTINE retry_function = ZRCVYRTY;
  SETRP(
      4, // RTNCD_RETRY
      retry_function,
      &sdwa);

  return RTNCD_RETRY;
}

// router back to main routine
#pragma prolog(ZRCVYRTE, " ZWEPROLG NEWDSA=NO ")
#pragma epilog(ZRCVYRTE, " ZWEEPILG ")
static void ZRCVYRTE(ZRCVY_ENV *zenv) ATTRIBUTE(noinline);
static void ZRCVYRTE(ZRCVY_ENV *zenv)
{
  get_r14_by_ref(&zenv->arr_return);
  JUMP_ENV(zenv->f4sa, zenv->r13, 0);
}

// NOTE(Kelosky): we must not have this function inline so to save and return to the mainline
static int disable_recovery(ZRCVY_ENV *zenv) ATTRIBUTE(noinline);
static int disable_recovery(ZRCVY_ENV *zenv)
{
  // get main stack regs and stack pointer
  unsigned long long int r13 = get_prev_r13();
  unsigned char *save_area = (unsigned char *)r13;
  memcpy(&zenv->final_f4sa, save_area, sizeof(SAVF4SA));
  zenv->final_r13 = r13;

  // return to the IEAARR (ieaarr) which will then "jump" back to the stack position set just above
  RETURN_TO_IEAARR(zenv->arr_return);
}

// NOTE(Kelosky): this function may "return twice" like setjmp()
// NOTE(Kelosky): we must not have this function inline so to save and return to the mainline
#pragma reachable(enable_recovery)
static int enable_recovery(ZRCVY_ENV *zenv) ATTRIBUTE(noinline);
static int enable_recovery(ZRCVY_ENV *zenv)
{
  unsigned long long int r13 = get_prev_r13();
  unsigned char *save_area = (unsigned char *)r13;

  memcpy(&zenv->f4sa, save_area, sizeof(SAVF4SA));
  zenv->r13 = r13;

  // here we call a router routine which will route back to main line code
  // eventually, whenever we call to drop recovery, we then fall through after this
  // IEAARR invocation and jump back to where to drop was called
  ieaarr(ZRCVYRTE, zenv, ZRCVYARR, zenv);

  // jump back to main whenever drop was called
  JUMP_ENV(zenv->final_f4sa, zenv->final_r13, 0);

  return 0; // NOTE(Kelosky): this never runs
}

#endif
