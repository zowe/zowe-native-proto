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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zmetal.h"
#include "asasymbp.h"
#include "zattachx.h"
#include "zstorage.h"
#include "zutm.h"
#include "zutm31.h"
#include "zecb.h"
#include "zam.h"
#include "ihapsa.h"
#include "ikjtcb.h"
#include "iezjscb.h"

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
#pragma prolog(ZUTWDYN, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(ZUTWDYN, " ZWEEPILG ")
int ZUTWDYN(BPXWDYN_PARM *parm, BPXWDYN_RESPONSE *response)
{
  int rc = 0;
  ZUTAOFF();

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

typedef struct symbfp SYMBFP;
typedef int (*ASASYMBF)(SYMBFP) ATTRIBUTE(amode31);
#pragma prolog(ZUTSYMBP, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZUTSYMBP, " ZWEEPILG ")
// symbol examples: https://www.ibm.com/docs/en/zos/3.1.0?topic=symbols-static-system
// similar tool: https://www.ibm.com/docs/en/zos/3.1.0?topic=descriptions-sysvar-display-static-system-symbols
// asasymbf callable service: https://www.ibm.com/docs/en/zos/3.1.0?topic=service-calling-asasymbm-asasymbf
int ZUTSYMBP(SYMBOL_DATA *data)
{
  int rc = 0;
  ZUTAOFF();

  SYMBFP parms = {0};

  unsigned char work_area[1024] = {0};

  parms.symbfppatternaddr = (void *PTR32)data->input;
  parms.symbfppatternlength = (int)strlen(data->input);

  parms.symbfptargetaddr = data->output;
  parms.symbfptargetlengthaddr = &data->length;

  parms.symbfpreturncodeaddr = &rc;

  parms.symbfpworkareaaddr = work_area;

  ASASYMBF substitute = (ASASYMBF)load_module31("ASASYMBF");
  substitute(parms);

  delete_module("ASASYMBF");
  return rc;
}

typedef int (*ISRSUPC)() ATTRIBUTE(amode31);
#pragma prolog(ZUTSRCH, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZUTSRCH, " ZWEEPILG ")

typedef struct
{
  short len;
  char parms[100];
} ISRSUPC_PARMS;

int ZUTSRCH(const char *parms)
{
  int rc = 0;
  ZUTAOFF();

  ISRSUPC_PARMS p = {0};
  p.len = sprintf(p.parms, "%s", parms);

  ISRSUPC search = (ISRSUPC)load_module31("ISRSUPC");
  rc = search(&p);
  delete_module("ISRSUPC");

  return rc;
}

#pragma prolog(ZUTRUN, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZUTRUN, " ZWEEPILG ")
typedef int (*PGM31)(void *) ATTRIBUTE(amode31);
typedef int (*PGM64)(void *) ATTRIBUTE(amode64);

int ZUTRUN(const char *program)
{
  int rc = 0;
  ZUTAOFF();

  char name_truncated[8 + 1] = {0};
  memset(name_truncated, ' ', sizeof(name_truncated) - 1);                                                                      // pad with spaces
  memcpy(name_truncated, program, strlen(program) > sizeof(name_truncated) - 1 ? sizeof(name_truncated) - 1 : strlen(program)); // truncate

  void *p = load_module(name_truncated);

  if (p)
  {

    long long unsigned int ifunction = (long long unsigned int)p;

    if (ifunction & 0x00000000000000001)
    {
      ifunction &= 0xFFFFFFFFFFFFFFFE; // clear low bit
      PGM64 p64 = (PGM64)ifunction;
      rc = p64(NULL); // ensure no parms
    }
    else
    {
      ifunction &= 0x000000007FFFFFFF; // clear high bit
      PGM31 p31 = (PGM31)ifunction;
      rc = p31(NULL); // ensure no parms
    }
  }
  else
  {
    return RTNCD_FAILURE;
  }

  delete_module(program);

  return rc;
}

#pragma prolog(ZUTRUN24, " ZWEPROLG NEWDSA=(YES,4),LOC24=YES ")
#pragma epilog(ZUTRUN24, " ZWEEPILG ")
int ZUTRUN24(const char *program)
{
  int rc = 0;
  ZUTAOFF();

  char name_truncated[8 + 1] = {0};
  memset(name_truncated, ' ', sizeof(name_truncated) - 1);                                                                      // pad with spaces
  memcpy(name_truncated, program, strlen(program) > sizeof(name_truncated) - 1 ? sizeof(name_truncated) - 1 : strlen(program)); // truncate

  Z31FUNC p = load_module31(name_truncated);

  if (p)
  {
    rc = ((int (*)(void))p)();
  }
  else
  {
    return RTNCD_FAILURE;
  }

  delete_module(program);

  return rc;
}

typedef struct
{
  short len;
  char parms[100];
} EDSCT_PARMS;

typedef int (*CCNEDSCT)(EDSCT_PARMS *) ATTRIBUTE(amode31);
#pragma prolog(ZUTEDSCT, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZUTEDSCT, " ZWEEPILG ")
int ZUTEDSCT()
{
  int rc = 0;
  ZUTAOFF();

  CCNEDSCT convert = (CCNEDSCT)load_module31("CCNEDSCT");
  EDSCT_PARMS p = {0};
  p.len = sprintf(p.parms, "PPCOND,EQUATE(DEF),BITF0XL,HDRSKIP,UNIQ,LP64,LEGACY,SECT(ALL)");
  rc = convert(&p);
  delete_module("CCNEDSCT");
  return rc;
}

#pragma prolog(ZUTMLPLB, " ZWEPROLG NEWDSA=(YES,128) ") // TODO(Kelosky): ensure this is large enough
#pragma epilog(ZUTMLPLB, " ZWEEPILG ")
int ZUTMLPLB(ZDIAG *diag, int *num_dsns, PARMLIB_DSNS *dsns)
{
  int rc = 0;

  ZDIAG diag31 = {0};
  int num_dsns31 = 0;
  PARMLIB_DSNS dsns31 = {0};

  rc = zutm1lpl(&diag31, &num_dsns31, &dsns31);

  memcpy(dsns->dsn, &dsns31.dsn, num_dsns31 * sizeof(dsns31.dsn[0]));

  memcpy(diag, &diag31, sizeof(ZDIAG));
  *num_dsns = num_dsns31;

  return rc;
}

// NOTE(Kelosky): this is unused in favor of `getlogin()` but retained for other usages of IAZXJSAB
#pragma prolog(ZUTMGUSR, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZUTMGUSR, " ZWEEPILG ")
int ZUTMGUSR(char user[8])
{
  char user31[8] = {0};
  int rc = zutm1gur(user31);

  if (0 != rc)
    return rc;

  memcpy(user, user31, sizeof(user31));
  return 0;
}

#pragma prolog(ZUTMFR64, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZUTMFR64, " ZWEEPILG ")
int ZUTMFR64(void *PTR64 data)
{
  storage_free64(data);
  return 0;
}

#pragma prolog(ZUTMGT64, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZUTMGT64, " ZWEEPILG ")
int ZUTMGT64(void **PTR64 data, int *len)
{
  *data = storage_get64(*len);
  return 0;
}

#pragma prolog(ZUTMGKEY, " ZWEPROLG NEWDSA=(YES,1) ")
#pragma epilog(ZUTMGKEY, " ZWEEPILG ")
unsigned char ZUTMGKEY()
{
  return get_key();
}

typedef struct psa PSA;
typedef struct tcb TCB;
typedef struct iezjscb IEZJSCB;

void ZUTAOFF()
{
  PSA *psa = (PSA *)0;
  TCB *PTR32 tcb = psa->psatold;
  IEZJSCB *PTR32 jscb = NULL;
  memcpy(&jscb, &tcb->tcbjscb, sizeof(tcb->tcbjscb));
  jscb = (IEZJSCB * PTR32)((unsigned int)jscb & 0x00FFFFFF);

  if (0 == test_auth())
  {
    PSW psw = {0};
    get_psw(&psw);
    int mode_switch = psw.p ? 1 : 0;
    unsigned char key = get_key();
    unsigned char key_zero = 0;
    if (mode_switch)
    {
      mode_sup();
    }
    set_key(&key_zero);
    jscb->jscbopts &= (0xFF - jscbauth);
    set_key(&key);
    if (mode_switch)
    {
      mode_prob();
    }
  }
}

void ZUTDBGMG(const char *msg)
{
  IO_CTRL *sysprintIoc = open_output_assert("ZOWEXDBG", 132, 132, dcbrecf + dcbrecbr);
  char writeBuf[132] = {0};
  memset(writeBuf, ' ', sizeof(132));
  int len = snprintf(writeBuf, 132, "%s", msg);

  write_sync(sysprintIoc, writeBuf);
  close_assert(sysprintIoc);
}
