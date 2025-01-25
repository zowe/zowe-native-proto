/*
  This program and the accompanying materials are
  made available under the terms of the Eclipse Public License v2.0 which accompanies
  this distribution, and is available at https://www.eclipse.org/legal/epl-v20.html

  SPDX-License-Identifier: EPL-2.0

  Copyright Contributors to the Zowe Project.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "zjbm.h"
#include "zwto.h"
#include "zstorage.h"
#include "zmetal.h"
#include "ztstype.h"

typedef int (*IKJTSOEV)(unsigned int *, int *, int *, int *, void *PTR32) ATTRIBUTE(amode31);
#pragma prolog(ZTSIENV, "&CCN_MAIN SETB 1 \n MYPROLOG")
int ZTSIENV(ZTS *zts)
{
  int rc = 0;
  unsigned int reserved = 0;
  // void *PTR32 cppl = NULL;
  IKJTSOEV tso_env = (IKJTSOEV)load_module31("IKJTSOEV");
  ZTS zts31 = {0};
  memcpy(&zts31, zts, sizeof(ZTS));
  zwto_debug("IKJTSOEV");

  // unsigned char *PTR32 last_parameter = cppl;
  unsigned char *PTR32 last_parameter = (unsigned char *PTR32)((unsigned int)&zts31.cppl | 0x80000000);
  zwto_debug("@TEST last parm is %x %x", &zts31.cppl, last_parameter);

  zwto_debug("@TEST bef %d %d %p", rc, zts31.diag.service_rc, zts31.cppl);
  rc = tso_env(&reserved, &zts31.diag.service_rc, &zts31.diag.service_rsn, &zts31.diag.service_rsn_secondary, last_parameter);
  zwto_debug("@TEST after %d %d %p", rc, zts31.diag.service_rc, zts31.cppl);
  memcpy(zts, &zts31, sizeof(ZTS));

  if (RTNCD_SUCCESS != rc && RTNCD_SUCCESS != zts31.diag.service_rc)
  {
    // zwto_debug("@TEST rc %d", rc);
    strcpy(zts->diag.service_name, "IKJTSOEV");
    zts->diag.detail_rc = ZTS_RTNCD_SERVICE_FAILURE;
    zts->diag.e_msg_len = sprintf(zts->diag.e_msg, "Error, service: %s, rc: %d, service_rc: %d, service_rsn: %d, abend_rc: %d", zts->diag.service_name, zts->diag.service_rc, zts->diag.service_rsn, zts->diag.service_rsn_secondary);
    return RTNCD_FAILURE;
  }

  // zwto_debug("@TEST %p", zts31.cppl);

  return RTNCD_SUCCESS;
}

typedef int (*IKJEFTSI)(unsigned int *, unsigned int *, unsigned int *, int *, int *, int *) ATTRIBUTE(amode31);
#pragma prolog(ZTSINIT, "&CCN_MAIN SETB 1 \n MYPROLOG")
int ZTSINIT(ZTS *zts)
{
  return 0;
  // unsigned int reserved = 0;
  // int rc = 0;

  // IKJEFTSI tso_init = (IKJEFTSI)load_module31("IKJEFTSI");
  // ZTS zts31 = {0};
  // memcpy(&zts31, zts, sizeof(ZTS));

  // zwto_debug("IKJEFTSI");

  // unsigned char *PTR32 last_parameter = (unsigned char *PTR32)((unsigned int)&zts31.diag.service_rsn | 0x80000000);
  // zwto_debug("@TEST last parm is %x %x", &zts31.diag.service_rsn, last_parameter);

  // rc = tso_init(&zts31.ect, &reserved, &zts31.token[0], &zts31.diag.service_rc, &zts31.diag.service_rsn_secondary, (int *)last_parameter);
  // zwto_debug("@TEST %d", zts31.token[0]);
  // zwto_debug("@TEST %x", zts31.ect);
  // memcpy(zts, &zts31, sizeof(ZTS));

  // if (RTNCD_SUCCESS != rc && RTNCD_SUCCESS != zts31.diag.service_rc)
  // {
  //   strcpy(zts->diag.service_name, "IKJEFTSI");
  //   zts->diag.detail_rc = ZTS_RTNCD_SERVICE_FAILURE;
  //   zts->diag.e_msg_len = sprintf(zts->diag.e_msg, "Error, service: %s, rc: %d, service_rc: %d, service_rsn: %d, abend_rc: %d", zts->diag.service_name, zts->diag.service_rc, zts->diag.service_rsn, zts->diag.service_rsn_secondary);
  //   return RTNCD_FAILURE;
  // }

  // return RTNCD_SUCCESS;
}

typedef int (*IKJEFTSR)(unsigned char *, char *, int *, int *, int *, void *PTR32) ATTRIBUTE(amode31);
#pragma prolog(ZTSINVOK, "&CCN_MAIN SETB 1 \n MYPROLOG")
int ZTSINVOK(ZTS *zts)
{
  unsigned char flags[4] = {0};
  unsigned int reserved = 0;
  char buffer[256] = {0};

  int rc = 0;

  char *parm = "LISTUSER";
  // char *parm = "HELP";
  // char *parm = "FREE DD(MINEMINE)";
  // char *parm = "ALTLIB DISPLAY";
  int buffer_len = strlen(parm);

  flags[0] = 0x00;
  flags[1] = 0x01; // unauthorized
  // flags[1] = 0x01; // unauthorized
  flags[2] = 0x01; // take a dump
  flags[3] = 0x01; // TSO/E, REXX, CLIST

  IKJEFTSR tso_invoke = (IKJEFTSR)load_module31("IKJEFTSR");
  // IKJEFTSR tso_invoke = (IKJEFTSR)load_module31("IKJEFTSR");
  ZTS zts31 = {0};
  memcpy(&zts31, zts, sizeof(ZTS));
  zwto_debug("@TEST before %s", parm);
  // rc = tso_invoke(NULL, NULL, NULL, NULL, NULL, NULL);
  // unsigned char *PTR32 last_parameter = (unsigned char *PTR32) & zts31.diag.service_rsn_secondary;
  // zwto_debug("@TEST last parm is %x", last_parameter);
  // last_parameter = (unsigned char *PTR32)((unsigned int)&zts31.diag.service_rsn_secondary | 0x80000000);
  unsigned char *PTR32 last_parameter = (unsigned char *PTR32)((unsigned int)&zts31.diag.service_rsn_secondary | 0x80000000);
  zwto_debug("@TEST last parm now is %x", last_parameter);

  zwto_debug("IKJEFTSR");
  // zwto_debug("@TEST last parm is %x %x", &zts31.diag.service_rsn_secondary, last_parameter);
  // zwto_debug("@TEST flags %02x %02x %02x %02x", flags[0], flags[1], flags[2], flags[3]);

  // https://www.ibm.com/docs/en/zos/3.1.0?topic=ikjeftsr-parameter-list
  // https://www.ibm.com/docs/en/zos/3.1.0?topic=ikjeftsr-return-codes-from
  // https://www.ibm.com/docs/en/zos/3.1.0?topic=ikjeftsr-reason-codes-from
  // https://www.ibm.com/docs/en/zos/3.1.0?topic=facility-assembler-program-using-ikjeftsr-invoke-rexx-exec
  rc = tso_invoke(&flags[0], parm, &buffer_len, &zts31.diag.service_rc, &zts31.diag.service_rsn, last_parameter);
  zwto_debug("@TEST rc %d", rc);
  zwto_debug("@TEST 3rd %d", zts31.diag.service_rc);
  zwto_debug("@TEST 4th %d", zts31.diag.service_rsn);
  zwto_debug("@TEST 5th %d", zts31.diag.service_rsn_secondary);
  memcpy(zts, &zts31, sizeof(ZTS));

  if (RTNCD_SUCCESS != rc && RTNCD_SUCCESS != zts31.diag.service_rc)
  {
    strcpy(zts->diag.service_name, "IKJEFTSR");
    zts->diag.detail_rc = ZTS_RTNCD_SERVICE_FAILURE;
    zts->diag.e_msg_len = sprintf(zts->diag.e_msg, "Error, service: %s, rc: %d, service_rc: %d, service_rsn: %d, abend_rc: %d", zts->diag.service_name, zts->diag.service_rc, zts->diag.service_rsn, zts->diag.service_rsn_secondary);
    return RTNCD_FAILURE;
  }

  return RTNCD_SUCCESS;
}

typedef int (*IKJEFTST)(unsigned int *, unsigned int *, unsigned int *, int *, int *, int *) ATTRIBUTE(amode31);
#pragma prolog(ZTSTERM, "&CCN_MAIN SETB 1 \n MYPROLOG")
int ZTSTERM(ZTS *zts)
{
  return 0;
  // unsigned int reserved = 0;
  // int rc = 0;

  // IKJEFTST tso_term = (IKJEFTST)load_module31("IKJEFTST");
  // ZTS zts31 = {0};
  // memcpy(&zts31, zts, sizeof(ZTS));
  // tso_term(&zts31.ect, &reserved, &zts31.token[0], &zts31.diag.service_rc, &zts31.diag.service_rsn_secondary, &zts31.diag.service_rsn);
  // memcpy(zts, &zts31, sizeof(ZTS));

  // if (RTNCD_SUCCESS != rc && RTNCD_SUCCESS != zts31.diag.service_rc)
  // {
  //   strcpy(zts->diag.service_name, "IKJEFTST");
  //   zts->diag.detail_rc = ZTS_RTNCD_SERVICE_FAILURE;
  //   zts->diag.e_msg_len = sprintf(zts->diag.e_msg, "Error, service: %s, rc: %d, service_rc: %d, service_rsn: %d, abend_rc: %d", zts->diag.service_name, zts->diag.service_rc, zts->diag.service_rsn, zts->diag.service_rsn_secondary);
  //   return RTNCD_FAILURE;
  // }

  // return RTNCD_SUCCESS;
}
