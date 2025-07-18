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

#ifndef ZDSM_H
#define ZDSM_H

#include "zmetal.h"
#include "ztype.h"
#include "zdstype.h"
#include "iggcsina.h"

#if defined(__cplusplus) && (defined(__IBMCPP__) || defined(__IBMC__))
extern "OS"
{
#elif defined(__cplusplus)
extern "C"
{
#endif

  typedef struct csifield CSIFIELD;

  int ZDSSMSAT(ZDS *zds, const char *dsn);
  int ZDSCSI00(ZDS *zds, CSIFIELD *selection, void *work_area);
  void ZDSDEL(ZDS *zds);
  int ZDSRECFM(ZDS *zds, const char *dsn, const char *volser, char *recfm_buf,
               int recfm_buf_len);

#if defined(__cplusplus)
}
#endif

#if defined(__IBM_METAL__)
#define OBTAIN(params, rc)          \
  __asm(                            \
      " LA   1,%1               \n" \
      " OBTAIN (1)              \n" \
      " ST   15,%0              \n" \
      : "=m"(rc)                    \
      : "m"(params)                 \
      : "r0", "r1", "r14", "r15");
#else
#define OBTAIN(params, rc)
#endif

#if (defined(__IBMCPP__) || defined(__IBMC__))
#if defined(SWIG)
#pragma pack(1)
#else
#pragma pack(packed)
#endif
#endif

typedef struct CamlstSearchParams
{
  char *PTR32 dsname_ptr;
  char *PTR32 volume_ptr;
  void *PTR32 workarea_ptr;
} CamlstSearchParams;

typedef struct ObtainParams
{
  unsigned char reserved; // should be defined as (193) 0xC1 according to dump of parameter list
  unsigned char number_dscbs;
  unsigned char option_flags;
  unsigned char unk_pad;
  CamlstSearchParams listname_addrx;
} ObtainParams;

#if (defined(__IBMCPP__) || defined(__IBMC__))
#pragma pack(reset)
#endif

/**
 * @brief Use the OBTAIN routine through CAMLST to access the VTOC and Data Set Control Blocks (DCSBs)
 *
 * @param params parameters for the OBTAIN routine
 * @return int 0 for success; non zero otherwise
 */
static int obtain_camlst(ObtainParams params)
{
  int rc = 0;
  OBTAIN(params, rc);
  return rc;
}

#endif