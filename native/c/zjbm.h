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

#ifndef ZJBM_H
#define ZJBM_H

#include "ztype.h"
#include "zssitype.h"
#include "zjbtype.h"

#if (defined(__IBMCPP__) || defined(__IBMC__))
#if defined(__clang__)
#pragma pack(1)
#else
#pragma pack(packed)
#endif
#endif
typedef struct
{
  STATJQTR statjqtr;
  char phase_text[64 + 1];
} ZJB_JOB_INFO;

#if (defined(__IBMCPP__) || defined(__IBMC__))
#pragma pack(reset)
#endif

#if defined(__cplusplus) && (defined(__IBMCPP__) || defined(__IBMC__))
extern "OS"
{
#elif defined(__cplusplus)
extern "C"
{
#endif

  int ZJBMVIEW(ZJB *PTR64, ZJB_JOB_INFO **PTR64, int *PTR64);
  int ZJBMLIST(ZJB *PTR64, ZJB_JOB_INFO **PTR64, int *PTR64);
  int ZJBMGJQ(ZJB *, SSOB *, STAT *, STATJQ *PTR32 *PTR32);
  int ZJBMEMSG(ZJB *, STAT *PTR64, SSOB *PTR64, int);
  int ZJBMTCOM(ZJB *PTR64, STAT *PTR64 stat, ZJB_JOB_INFO **PTR64, int *PTR64);
  int ZJBMLSDS(ZJB *PTR64, STATSEVB **PTR64, int *PTR64);
  int ZJBSYMB(ZJB *PTR64, const char *PTR64, char *PTR64);
  int ZJBMPRG(ZJB *PTR64);
  int ZJBMCNL(ZJB *PTR64, int flags);
  int ZJBMHLD(ZJB *PTR64);
  int ZJBMRLS(ZJB *PTR64);
  int ZJBMMOD(ZJB *PTR64, int type, int flags);

#if defined(__cplusplus)
}
#endif

#endif
