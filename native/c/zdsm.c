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

#include "zdstype.h"
#include "zmetal.h"
#include "zdsm.h"
#include "iggcsina.h"
#include "zwto.h"

// https://www.ibm.com/docs/en/zos/3.1.0?topic=retrieval-parameters
typedef int (*IGGCSI00)(
    int *PTR32 rsn,
    CSIFIELD *PTR32 selection,
    void *PTR32 work_area) ATTRIBUTE(amode31);

#pragma prolog(ZDSCSI00, "&CCN_MAIN SETB 1 \n MYPROLOG")
int ZDSCSI00(ZDS *zds, CSIFIELD *selection, void *work_area)
{
  int rc = 0;
  int rsn = 0;

  // load our service
  IGGCSI00 csi = (IGGCSI00)load_module31("IGGCSI00"); // EP which doesn't require R0 == 0
  if (!csi)
  {
    strcpy(zds->diag.service_name, "LOAD");
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Load failure for IGGCSI00");
    zds->diag.detail_rc = ZDS_RTNCD_SERVICE_FAILURE;
    return RTNCD_FAILURE;
  }

  rc = csi(&rsn, selection, work_area);

  delete_module("IGGCSI00");

  // https://www.ibm.com/docs/en/SSLTBW_3.1.0/com.ibm.zos.v3r1.idac100/c1055.htm
  if (0 != rc)
  {
    strcpy(zds->diag.service_name, "IGGCSI00");
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "IGGCSI00 rc was: '%d', rsn was: '%04x'", rc, rsn);
    zds->diag.service_rc = rc;
    zds->diag.service_rsn = rsn;
    zds->diag.detail_rc = ZDS_RTNCD_SERVICE_FAILURE;
    return RTNCD_FAILURE;
  }

  return rc;
}

typedef struct
{
  unsigned char data[30];
} ZSMS_DATA;

// https://www.ibm.com/docs/en/zos/3.1.0?topic=retrieval-parameters
typedef int (*IGWASMS)(
    int *rc,
    int *rsn,
    int problem_data[2],
    int *dsn_len,
    const char *dsn,
    ZSMS_DATA sms_data[3],
    int *ds_type // 1 for PDSE, 2 for HFS
    ) ATTRIBUTE(amode31);
