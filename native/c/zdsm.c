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
#include "zdbg.h"
#include "zwto.h"
#include <ctype.h>
#include <string.h>

// OBTAIN option parameters for CAMLST
const unsigned char OPTION_EADSCB = 0x08;  // EADSCB=OK
const unsigned char OPTION_NOQUEUE = 0x04; // NOQUEUE=OK

#ifndef MAX_DSCBS
#define MAX_DSCBS 12
#endif

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
// Full PDF for DFSMSdfp advanced services: https://www.ibm.com/docs/en/SSLTBW_2.5.0/pdf/idas300_v2r5.pdf
// Doc page: https://www.ibm.com/docs/en/zos/3.1.0?topic=macros-reading-dscbs-from-vtoc-using-obtain
int ZDSRECFM(ZDS *zds, const char *dsn, const char *volser, char *recfm_buf,
             int recfm_buf_len)
{
  // workarea: each DSCB is 140 bytes, we need enough space for format-1 DSCB, format-8 DSCB and max possible format-3 DSCBs (10)
  // adding 140 bytes for the workarea itself
  char workarea[sizeof(IndexableDSCBFormat1) * MAX_DSCBS];
  ObtainParams params = {0};

  // We're using OBTAIN through CAMLST SEARCH to get the DSCBs, see here for more info:
  // https://www.ibm.com/docs/en/zos/3.1.0?topic=obtain-reading-dscb-by-data-set-name

  // OBTAIN by data set name
  params.reserved = 0xC1;
  params.number_dscbs = MAX_DSCBS;
  params.option_flags = OPTION_EADSCB;
  // Allow lookup of format-1 or format-8 DSCB
  char dsname[44] = {0};
  memset(dsname, ' ', sizeof(dsname));
  memcpy(dsname, dsn, strlen(dsn));
  params.listname_addrx.dsname_ptr = dsname;
  char volume[6] = {0};
  memset(volume, ' ', sizeof(volume));
  memcpy(volume, volser, strlen(volser));
  params.listname_addrx.volume_ptr = volume;
  params.listname_addrx.workarea_ptr = workarea;

  int rc = obtain_camlst(params);
  if (0 != rc)
  {
    strcpy(zds->diag.service_name, "OBTAIN");
    zds->diag.e_msg_len =
        sprintf(zds->diag.e_msg, "OBTAIN SVC failed for %s on %s with rc=%d, workarea_ptr=%p",
                dsn, volser, rc, workarea);
    zds->diag.service_rc = rc;
    zds->diag.detail_rc = ZDS_RTNCD_SERVICE_FAILURE;
    return RTNCD_FAILURE;
  }

  IndexableDSCBFormat1 indexable_dscb = {0};
  for (int i = 0; i < MAX_DSCBS - 1; i++)
  {
    memcpy(&indexable_dscb, workarea + (i * (sizeof(IndexableDSCBFormat1) - 1)), sizeof(indexable_dscb));
    // The returned DSCB does not include the key, but we can infer the returned variables by re-aligning the struct
    DSCBFormat1 *dscb = (DSCBFormat1 *)&indexable_dscb;

    // '1' or '8' in EBCDIC
    if (dscb == NULL || (dscb->ds1fmtid != '1' && dscb->ds1fmtid != '8'))
    {
      continue;
    }

    char temp_recfm[8] = {0};
    int len = 0;
    char main_fmt = 0;

    // Bitmasks translated from binary to hex from "DFSMSdfp advanced services" PDF, Chapter 1 page 7 (PDF page 39)
    // Fixed: First bit is set
    if ((dscb->ds1recfm & 0xC0) == 0x80)
    {
      temp_recfm[len++] = 'F';
      main_fmt = 'F';
    }
    // Variable: Second bit is set
    else if ((dscb->ds1recfm & 0xC0) == 0x40)
    {
      temp_recfm[len++] = 'V';
      main_fmt = 'V';
    }
    // Undefined: First and second bits are set
    else if ((dscb->ds1recfm & 0xC0) == 0xC0)
    {
      temp_recfm[len++] = 'U';
      main_fmt = 'U';
    }

    // Blocked records: Fourth bit is set
    if ((dscb->ds1recfm & 0x10) > 0)
    {
      temp_recfm[len++] = 'B';
    }

    // Sequential: Fifth bit is set
    if ((dscb->ds1recfm & 0x08) > 0)
    {
      // Fixed length: standard blocks, no truncated or unfilled tracks
      // Variable length: spanned records
      if (main_fmt == 'F' || main_fmt == 'V')
      {
        temp_recfm[len++] = 'S';
      }
    }

    // ANSI control characters/ASA: Sixth bit is set
    if ((dscb->ds1recfm & 0x04) > 0)
    {
      temp_recfm[len++] = 'A';
    }

    // Machine-control characters: Seventh bit is set
    if ((dscb->ds1recfm & 0x02) > 0)
    {
      temp_recfm[len++] = 'M';
    }

    if (len == 0)
    {
      temp_recfm[len++] = 'U';
    }

    memcpy(recfm_buf, temp_recfm, len);
    return RTNCD_SUCCESS;
  }

  strcpy(zds->diag.service_name, "OBTAIN");
  zds->diag.e_msg_len = sprintf(
      zds->diag.e_msg, "Could not find Format-1 or Format-8 DSCB, OBTAIN rc=%d, sizeof(dscb)=%d", rc, sizeof(IndexableDSCBFormat1));
  zds->diag.detail_rc = ZDS_RTNCD_UNEXPECTED_ERROR;
  return RTNCD_FAILURE;
}
