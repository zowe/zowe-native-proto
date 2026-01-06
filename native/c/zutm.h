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

#ifndef ZUTM_H
#define ZUTM_H

#include "ztype.h"
#include "zprmtype.h"

#if defined(__cplusplus) && (defined(__IBMCPP__) || defined(__IBMC__))
extern "OS"
{
#elif defined(__cplusplus)
extern "C"
{
#endif

#define RET_ARG_MAX_LEN 260
#define MSG_ENTRIES 25

#define ALLOC_STRING_INDEX 0
#define RTDDN_INDEX 1
#define MSG_INDEX 2

#define LAST_PARAMETER_INDEX MSG_INDEX // NOTE(Kelosky): this must be set to the last parameter index

#define INPUT_PARAMETERS LAST_PARAMETER_INDEX + 1

  typedef struct
  {
    short len;
    char str[RET_ARG_MAX_LEN];
    unsigned int rtdd : 1;
    unsigned int rtdsn : 1; // NOTE(Kelosky): not implemented yet
  } BPXWDYN_RET_ARG;

  typedef BPXWDYN_RET_ARG BPXWDYN_PARM;

  typedef struct
  {
    unsigned int code;
    char response[RET_ARG_MAX_LEN * MSG_ENTRIES + 1];
    char ddname[9];
    char dsname[45]; // NOTE(Kelosky): not implemented yet
  } BPXWDYN_RESPONSE;

  typedef struct
  {
    char input[128];
    char output[128];
    int length;
    int reserve_1;
  } SYMBOL_DATA;

  int ZUTMFR64(void *PTR64);
  int ZUTMGT64(void **PTR64, int *PTR64);
  int ZUTMGUSR(char[8]);
  int ZUTWDYN(BPXWDYN_PARM *, BPXWDYN_RESPONSE *);
  int ZUTEDSCT();
  int ZUTSYMBP(SYMBOL_DATA *);
  int ZUTSRCH(const char *);
  int ZUTRUN(const char *);
  void ZUTDBGMG(const char *);
  unsigned char ZUTMGKEY();
  int ZUTMLPLB(ZDIAG *, int *, PARMLIB_DSNS *);

#if defined(__cplusplus)
}
#endif

#endif
