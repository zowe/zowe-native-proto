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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zmetal.h"
#include "zattachx.h"
#include "zstorage.h"
#include "zwto.h"
#include "zutm.h"
#include "zutm31.h"
#include "zecb.h"

#define ZUT_BPXWDYN_SERVICE_FAILURE -2

// takes a conventional paramter list
typedef int (*BPXWDYN)(
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32,
    BPXWDYN_PARM *PTR32) ATTRIBUTE(amode31);

// Doc:
// * keywords - https://www.ibm.com/docs/en/zos/3.1.0?topic=output-requesting-dynamic-allocation
// * return codes - https://www.ibm.com/docs/en/zos/3.1.0?topic=output-bpxwdyn-return-codes
// * detail codes (high 4 hex bytes) - https://www.ibm.com/docs/en/zos/3.1.0?topic=codes-interpreting-error-reason-from-dynalloc#erc__mjfig8
// * parm list - https://www.ibm.com/docs/en/zos/3.1.0?topic=conventions-conventional-mvs-parameter-list
#pragma prolog(ZUTWDYN, "&CCN_MAIN SETB 1 \n MYPROLOG")
int ZUTWDYN(BPXWDYN_PARM *parm, BPXWDYN_RESPONSE *response)
{
  int rc = 0;

  // load our service
  BPXWDYN dynalloc = (BPXWDYN)load_module31("BPXWDY2"); // EP which doesn't require R0 == 0
  if (!dynalloc)
  {
    // TODO(Kelosky): pass diag information
    // strcpy(zds->diag.service_name, "LOAD");
    // zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Load failure for IGGCSI00");
    // zds->diag.detail_rc = ZDS_RTNCD_SERVICE_FAILURE;
    return RTNCD_FAILURE;
  }

  // allow for MSG_ENTRIES response parameters + 2 input parameters
  BPXWDYN_RET_ARG parameters[MSG_ENTRIES + 2] = {0};
  memcpy(&parameters[0], parm, sizeof(BPXWDYN_RESPONSE));
  parameters[1].len = RET_ARG_MAX_LEN - sizeof(parameters[1].len);
  strcpy(parameters[1].str, "MSG");

  int index = 0;

  // assign all response paramter stem values
  for (int i = 2; i < MSG_ENTRIES + 2; i++)
  {
    parameters[i].len = RET_ARG_MAX_LEN - sizeof(parameters[i].len);
    sprintf(parameters[i].str, "MSG.%d", ++index);
  }

  // build a contiguous list of all parameters
  BPXWDYN_RET_ARG *PTR32 parms[MSG_ENTRIES + 2] = {0};
  for (int i = 0; i <= MSG_ENTRIES + 2; i++)
  {
    parms[i] = &parameters[i];
  }

  // set the high bit on last parm
  parms[MSG_ENTRIES + 1] = (void *PTR32)((unsigned int)parms[MSG_ENTRIES + 1] | 0x80000000);

  // NOTE(Kelosky): to prevent the compiler optimizer from discarding any memory assignements,
  // we need to ensure a reference to all data is passed to this external function
  rc = dynalloc(
      parms[0],
      parms[1],
      parms[2],
      parms[3],
      parms[4],
      parms[5],
      parms[6],
      parms[7],
      parms[8],
      parms[9],
      parms[10],
      parms[11],
      parms[12],
      parms[13],
      parms[14],
      parms[15],
      parms[16],
      parms[17],
      parms[18],
      parms[19],
      parms[20],
      parms[21],
      parms[22],
      parms[23],
      parms[24],
      parms[25],
      parms[26]);

  response->code = rc;

  delete_module("BPXWDY2");

  // obtain any messages returned
  char *respp = response->response;
  for (int i = 0, j = atoi(parameters[1].str); i < j && i < MSG_ENTRIES + 2; i++)
  {
    if (parameters[i + 2].len == RET_ARG_MAX_LEN - sizeof(parameters[i + 2].len))
    {
      return (0 != rc) ? ZUT_BPXWDYN_SERVICE_FAILURE : RTNCD_SUCCESS;
    }
    int len = sprintf(respp, "%.*s\n", parameters[i + 2].len, parameters[i + 2].str);
    respp = respp + len;
  }

  return (0 != rc) ? ZUT_BPXWDYN_SERVICE_FAILURE : RTNCD_SUCCESS;
}

#pragma prolog(ZUTTEST, "&CCN_MAIN SETB 1 \n MYPROLOG")
int ZUTTEST()
{
  return 0;
}
typedef struct
{
  short len;
  char parms[100];
} EDSCT_PARMS;

typedef int (*CCNEDSCT)(EDSCT_PARMS *) ATTRIBUTE(amode31);
#pragma prolog(ZUTEDSCT, "&CCN_MAIN SETB 1 \n MYPROLOG")
int ZUTEDSCT()
{
  CCNEDSCT convert = (CCNEDSCT)load_module31("CCNEDSCT");
  EDSCT_PARMS p = {0};
  p.len = sprintf(p.parms, "PPCOND,EQUATE(DEF),BITF0XL,HDRSKIP,UNIQ,LP64,LEGACY,SECT(ALL)");
  return convert(&p);
}

// TODO(Kelosky): this should probably be `getlogin()`
#pragma prolog(ZUTMGUSR, "&CCN_MAIN SETB 1 \n MYPROLOG")
int ZUTMGUSR(char user[8])
{
  char user31[8] = {0};
  int rc = zutm1gur(user31);

  if (0 != rc)
    return rc;

  memcpy(user, user31, sizeof(user31));
  return 0;
}

#pragma prolog(ZUTMFR64, "&CCN_MAIN SETB 1 \n MYPROLOG")
int ZUTMFR64(void *PTR64 data)
{
  storageFree64(data);
  return 0;
}

#pragma prolog(ZUTMGT64, "&CCN_MAIN SETB 1 \n MYPROLOG")
int ZUTMGT64(void **PTR64 data, int *len)
{
  *data = storageGet64(*len);
  return 0;
}
