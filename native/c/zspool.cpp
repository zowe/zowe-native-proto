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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include "zspool.hpp"
#include "zjbtype.h"
#include "iazbtokp.h"
#include "iefzb4d0.h"
#include "iefzb4d2.h"
#include "zdyn.h"

typedef struct iazbtokp IAZBTOKP;
#define BTOKLEN (293 - 254)

#define NUM_TEXT_UNITS 5

// Disable optimization for this function since `ibm-clang` -O2 removes required assignments.
__attribute__((optnone)) int zspool_allocate(ZDIAG &diag, const ZSpoolOptions &opts, std::string &ddname)
{
  int rc = 0;

  unsigned char *p = nullptr;

  unsigned char token[80] = {0};
  bool use_stvsctkn = false;
  if (0 != memcmp(opts.token, token, sizeof(token)))
  {
    use_stvsctkn = true;
  }

  // calculate total size needed, obtain, & clear
  int total_size_needed = sizeof(IAZBTOKP) + (sizeof(S99TUNIT_X) * NUM_TEXT_UNITS) + (sizeof(S99TUPL) * NUM_TEXT_UNITS) + sizeof(__S99parms) + sizeof(__S99rbx_t) + sizeof(token);
  unsigned char *parms = (unsigned char *)__malloc31(total_size_needed);
  if (parms == nullptr)
  {
    ZDIAG_SET_MSG(&diag, "Failed to allocate 31-bit memory for job parms when reading %s", opts.dsname.c_str());
    return RTNCD_FAILURE;
  }
  std::unique_ptr<unsigned char, decltype(&free)> storage(parms, free);
  memset(parms, 0x00, total_size_needed);

  // carve up storage to needed structs
  IAZBTOKP *PTR32 iazbtokp = (IAZBTOKP * PTR32) parms;
  S99TUNIT_X *PTR32 s99tunit_x = (S99TUNIT_X * PTR32)(parms + sizeof(IAZBTOKP));
  S99TUPL *PTR32 s99tupl = (S99TUPL * PTR32)(parms + sizeof(IAZBTOKP) + (sizeof(S99TUNIT_X) * NUM_TEXT_UNITS));
  __S99parms *PTR32 s99parms = (__S99parms * PTR32)(parms + sizeof(IAZBTOKP) + (sizeof(S99TUNIT_X) * NUM_TEXT_UNITS) + (sizeof(S99TUPL) * NUM_TEXT_UNITS));
  __S99rbx_t *PTR32 s99parmsx = (__S99rbx_t * PTR32)(parms + sizeof(IAZBTOKP) + (sizeof(S99TUNIT_X) * NUM_TEXT_UNITS) + (sizeof(S99TUPL) * NUM_TEXT_UNITS) + sizeof(__S99parms));
  unsigned char *PTR32 token31 = (unsigned char *PTR32)(parms + sizeof(IAZBTOKP) + (sizeof(S99TUNIT_X) * NUM_TEXT_UNITS) + (sizeof(S99TUPL) * NUM_TEXT_UNITS) + sizeof(__S99parms) + sizeof(__S99rbx_t));

  if (use_stvsctkn)
  {
    memcpy(token31, opts.token, sizeof(opts.token));
  }

  // https://www.ibm.com/docs/en/zos/3.1.0?topic=allocation-building-browse-token-dalbrtkn
  short int len = sizeof(iazbtokp->btokid);
  memcpy(iazbtokp->btokpl1, &len, sizeof(len));
  memcpy(iazbtokp->btokid, "BTOK", sizeof(iazbtokp->btokid));
  len = sizeof(iazbtokp->btokver);
  memcpy(iazbtokp->btokpl2, &len, sizeof(len));

  if (use_stvsctkn)
  {
    iazbtokp->btoktype = btokstkn;
  }
  else
  {
    iazbtokp->btoktype = btokbrws;
  }
  iazbtokp->btokvers = btokvrnm;
  len = sizeof(iazbtokp->btokiotp);

  if (use_stvsctkn)
  {
    memcpy(&iazbtokp->btokiotp[0], &token31, sizeof(token31));
  }

  memcpy(iazbtokp->btokpl3, &len, sizeof(len));
  len = sizeof(iazbtokp->btokjkey);
  memcpy(iazbtokp->btokpl4, &len, sizeof(len));
  len = sizeof(iazbtokp->btokasid);
  memcpy(iazbtokp->btokpl5, &len, sizeof(len));
  memset(iazbtokp->btokasid, 0xFF, sizeof(iazbtokp->btokasid));
  len = sizeof(iazbtokp->btokrcid);
  memcpy(iazbtokp->btokpl6, &len, sizeof(len));
  len = sizeof(iazbtokp->btoklogs);
  memcpy(iazbtokp->btokpl7, &len, sizeof(len));

  // --- set text units

  short int plen = 0;
  short int dynkey = 0;
  short int numparms = 1;
  int i = -1;

  i++;
  dynkey = daldsnam;
  numparms = 1;
  plen = strlen(opts.dsname.c_str());
  memcpy(s99tunit_x[i].s99tunit.s99tukey, &dynkey, sizeof(dynkey));
  memcpy(s99tunit_x[i].s99tunit.s99tunum, &numparms, sizeof(numparms));
  memcpy(s99tunit_x[i].s99tunit.s99tulng, &plen, sizeof(plen));
  memcpy(&s99tunit_x[i].s99tunit.s99tupar, opts.dsname.c_str(), plen);

  i++;
  dynkey = dalstats;
  numparms = 1;
  plen = 1;
  memcpy(s99tunit_x[i].s99tunit.s99tukey, &dynkey, sizeof(dynkey));
  memcpy(s99tunit_x[i].s99tunit.s99tunum, &numparms, sizeof(numparms));
  memcpy(s99tunit_x[i].s99tunit.s99tulng, &plen, sizeof(plen));
  s99tunit_x[i].s99tunit.s99tupar = DISP_SHR;

  i++;
  dynkey = dalbrtkn;
  numparms = 7; // "# must be 7" // https://www.ibm.com/docs/en/zos/3.1.0?topic=njdaf-spool-data-set-browse-token-specification-key-006e
  plen = BTOKLEN;
  memcpy(s99tunit_x[i].s99tunit.s99tukey, &dynkey, sizeof(dynkey));
  memcpy(s99tunit_x[i].s99tunit.s99tunum, &numparms, sizeof(numparms));
  memcpy(s99tunit_x[i].s99tunit.s99tulng, &plen, sizeof(plen)); //
  memcpy(s99tunit_x[i].s99tunit.s99tulng, iazbtokp, BTOKLEN);   // NOTE(Kelosky): not using s99tupar for data, iazbtokp starts with half word of the "FIRST" parm.

  i++;
  dynkey = daluassr;
  numparms = 0;
  plen = 0;
  memcpy(s99tunit_x[i].s99tunit.s99tukey, &dynkey, sizeof(dynkey));
  memcpy(s99tunit_x[i].s99tunit.s99tunum, &numparms, sizeof(numparms));
  memcpy(s99tunit_x[i].s99tunit.s99tulng, &plen, sizeof(plen));

  i++;
  dynkey = dalrtddn;
  numparms = 1;
  plen = 8;
  memcpy(s99tunit_x[i].s99tunit.s99tukey, &dynkey, sizeof(dynkey));
  memcpy(s99tunit_x[i].s99tunit.s99tunum, &numparms, sizeof(numparms));
  memcpy(s99tunit_x[i].s99tunit.s99tulng, &plen, sizeof(plen));

  // these need to be contiguous and can point to non contiguous storage
  for (int j = 0; j <= i; j++)
  {
    s99tupl[j].s99tuptr = (void *PTR32) & s99tunit_x[j];
  }

  // set high bit in last entry
  p = (unsigned char *PTR32) & s99tupl[i].s99tuptr;
  *p = *p | s99tupln;

  memcpy(s99parmsx->__S99EID, "S99RBX", sizeof(s99parmsx->__S99EID));
  s99parmsx->__S99EVER = s99rbxvr;
  s99parmsx->__S99EOPTS = s99parmsx->__S99EOPTS | s99ermsg; // IEFDB476 can free message blocks
  s99parmsx->__S99ESUBP = 0;
  s99parmsx->__S99EMGSV = s99parmsx->__S99EMGSV | s99xinfo;

  // --- set rb

  s99parms->__S99RBLN = sizeof(__S99parms);
  s99parms->__S99VERB = s99vrbal; // allocation
  s99parms->__S99FLAG1 = 0x4000;  // s99nocnv;
  s99parms->__S99TXTPP = s99tupl;

  // s99parms->__S99S99X = s99parmsx; // TODO(Kelosky): reenable when we look at s99parmsx->__S99ENMSG and free

  // https://www.ibm.com/docs/en/zos/3.1.0?topic=list-coding-dynamic-allocation-request
  // https://www.ibm.com/docs/en/zos/3.1.0?topic=guide-dynamic-allocation
  rc = svc99(s99parms);

  // TODO(Kelosky): parse s99parmsx->__S99ENMSG and free
  if (0 != rc && 0 != s99parms->__S99ERROR)
  {
    strcpy(diag.service_name, "svc99");
    diag.service_rc = rc;
    ZDIAG_SET_MSG(&diag, "Could not allocate job spool file '%s', rc: '%d' s99error: '%d' s99info: '%d'", opts.dsname.c_str(), rc, s99parms->__S99ERROR, s99parms->__S99INFO);
    diag.detail_rc = ZJB_RTNCD_SERVICE_FAILURE;
    diag.service_rsn = s99parms->__S99ERROR;
    diag.service_rsn_secondary = s99parms->__S99INFO;
    return RTNCD_FAILURE;
  }

  short ddnamelen = 0;
  memcpy(&ddnamelen, s99tunit_x[4].s99tunit.s99tulng, sizeof(ddnamelen));

  char cddname[8 + 1] = {0};
  memcpy(cddname, &s99tunit_x[4].s99tunit.s99tupar, ddnamelen);
  ddname = std::string(cddname);

  return rc;
}

int zspool_free(ZDIAG &diag, const std::string &ddname)
{
  int rc = 0;

  // free DD
  __dyn_t ip;
  dyninit(&ip);
  ip.__ddname = (char *)ddname.c_str(); // e.g. SYS00001
  rc = dynfree(&ip);

  if (0 != rc && 0 == diag.e_msg_len) // only set error if no error message was already set
  {
    strcpy(diag.service_name, "dynfree");
    diag.service_rc = rc;
    ZDIAG_SET_MSG(&diag, "dynfree failed with %d", rc);
    diag.detail_rc = ZJB_RTNCD_SERVICE_FAILURE;
    return RTNCD_FAILURE;
  }

  return rc;
}
