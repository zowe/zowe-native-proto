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

#include "zdsm.h"
#include "iggcsina.h"
#include "zdstype.h"
#include "zmetal.h"
#include "zwto.h"
#include <ctype.h>
#include <string.h>

// OBTAIN option parameters for CAMLST
const unsigned char OPTION_EADSCB = 0b00010000;  // EADSCB=OK
const unsigned char OPTION_NOQUEUE = 0b10000000; // NOQUEUE=OK

#pragma prolog(ZDSDEL, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZDSDEL, " ZWEEPILG ")
void ZDSDEL(ZDS *zds)
{
  if (zds->csi)
  {
    delete_module("IGGCSI00");
  }

  zds->csi = NULL;
}

// https://www.ibm.com/docs/en/zos/3.1.0?topic=retrieval-parameters
typedef int (*IGGCSI00)(int *PTR32 rsn, CSIFIELD *PTR32 selection,
                        void *PTR32 work_area) ATTRIBUTE(amode31);

#pragma prolog(ZDSCSI00, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(ZDSCSI00, " ZWEEPILG ")
int ZDSCSI00(ZDS *zds, CSIFIELD *selection, void *work_area)
{
  int rc = 0;
  int rsn = 0;

  // load our service on first call
  if (!zds->csi)
  {
    zds->csi = (void *PTR64)load_module31("IGGCSI00"); // EP which doesn't require R0 == 0
  }

  if (!zds->csi)
  {
    strcpy(zds->diag.service_name, "LOAD");
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Load failure for IGGCSI00");
    zds->diag.detail_rc = ZDS_RTNCD_SERVICE_FAILURE;
    return RTNCD_FAILURE;
  }

  rc = ((IGGCSI00)(zds->csi))(&rsn, selection, work_area);

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

#pragma prolog(ZDSRECFM, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(ZDSRECFM, " ZWEEPILG ")

// Obtain the record format for a data set, given its name and volser
// https://www.ibm.com/docs/en/SSLTBW_2.2.0/pdf/dgt3s310.pdf
int ZDSRECFM(ZDS *zds, const char *dsn, const char *volser, char *recfm_buf,
             int recfm_buf_len)
{
  int rc;
  char dsn_upper[45] = {0};
  char vol_upper[7] = {0};

  struct DSCBFormat1 dscb;
  struct ObtainCamlstSearchParams params;
  memset(&params, 0, sizeof(params));
  memset(&dscb, 0, sizeof(dscb));

  int i;
  // Prepare the data set name and volume serial in uppercase for comparison
  for (i = 0; i < 44 && dsn[i]; i++)
  {
    dsn_upper[i] = toupper(dsn[i]);
  }
  for (; i < 44; i++)
  {
    dsn_upper[i] = ' ';
  }

  for (i = 0; i < 6 && volser[i]; i++)
  {
    vol_upper[i] = toupper(volser[i]);
  }
  for (; i < 6; i++)
  {
    vol_upper[i] = ' ';
  }

  // OBTAIN by data set name
  params.function_code = 0xC1;
  // Search for a format-1 or format-8 DSCB
  params.option_flags = OPTION_EADSCB | OPTION_NOQUEUE;
  params.dsname_ptr = dsn_upper;
  params.volume_ptr = vol_upper;
  params.workarea_ptr = &dscb;
#if defined(__IBM_METAL__)
  __asm(" LR    1,%1 \n\t"
        " SVC   27 \n\t"
        " ST    15,%0 \n\t"
        : "=m"(rc)
        : "r"(&list)
        : "r1", "r15");
#endif

  if (rc != 0)
  {
    strcpy(zds->diag.service_name, "OBTAIN");
    zds->diag.e_msg_len =
        sprintf(zds->diag.e_msg, "OBTAIN SVC failed for %s on %s with rc=%d",
                dsn, volser, rc);
    zds->diag.service_rc = rc;
    zds->diag.detail_rc = ZDS_RTNCD_SERVICE_FAILURE;
    return RTNCD_FAILURE;
  }

  if (dscb.ds1fmtid != '1' && dscb.ds1fmtid != '8')
  {
    strcpy(zds->diag.service_name, "OBTAIN");
    zds->diag.e_msg_len = sprintf(
        zds->diag.e_msg, "Expected Format-1 or Format-8 DSCB but got Format-%c",
        dscb.ds1fmtid);
    zds->diag.detail_rc = ZDS_RTNCD_UNEXPECTED_ERROR;
    return RTNCD_FAILURE;
  }

  char temp_recfm[8] = {0};
  int len = 0;
  char main_fmt = 0;

  if ((dscb.ds1recfm & 0xC0) == 0x40)
  {
    temp_recfm[len++] = 'F';
    main_fmt = 'F';
  }
  else if ((dscb.ds1recfm & 0xC0) == 0x80)
  {
    temp_recfm[len++] = 'V';
    main_fmt = 'V';
  }
  else if ((dscb.ds1recfm & 0xC0) == 0x20)
  {
    temp_recfm[len++] = 'U';
    main_fmt = 'U';
  }

  if (dscb.ds1recfm & 0x10)
  {
    temp_recfm[len++] = 'B';
  }

  if (dscb.ds1recfm & 0x08)
  {
    if (main_fmt == 'F' || main_fmt == 'V')
    {
      temp_recfm[len++] = 'S';
    }
  }

  if (dscb.ds1recfm & 0x04)
  {
    temp_recfm[len++] = 'A';
  }

  if (dscb.ds1recfm & 0x02)
  {
    temp_recfm[len++] = 'M';
  }

  if (len == 0)
  {
    temp_recfm[len++] = '?';
  }

  if (recfm_buf_len > 0)
  {
    strncpy(recfm_buf, temp_recfm, recfm_buf_len - 1);
    recfm_buf[recfm_buf_len - 1] = '\0';
  }

  return RTNCD_SUCCESS;
}
