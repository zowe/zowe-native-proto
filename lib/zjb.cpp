/*
  This program and the accompanying materials are
  made available under the terms of the Eclipse Public License v2.0 which accompanies
  this distribution, and is available at https://www.eclipse.org/legal/epl-v20.html

  SPDX-License-Identifier: EPL-2.0

  Copyright Contributors to the Zowe Project.
*/
#include <iostream>
#include <fstream>
#include "zmetal.h"
#include "zds.hpp"
#include "zjb.hpp"
#include "zjbm.h"
#include "zssitype.h"
#include <cstring>
#include <string>
#include <vector>
#include <iomanip>
#include <stdio.h>
#include <sstream>
#include <dynit.h>
#include <ctype.h>
#include <algorithm>
#include "iazbtokp.h"
#include "iefzb4d0.h"
#include "iefzb4d2.h"
#include "zut.hpp"
#include "zutm.h"
#include "zjbtype.h"
#include "zdstype.h"
#include "zdyn.h"

typedef struct iazbtokp IAZBTOKP;

using namespace std;

#define BTOKLEN (293 - 254) // 293 is the full length, minus the max optional buffer area for logs (less 254)

// NOTE(Kelosky): see struct __S99struc via 'showinc' compiler option in <stdio.h>
// NOTE(Kelosky): In the future, to allocate the logical SYSLOG concatenation for a system specify the following data set name (in DALDSNAM).
// NOTE(Kelosky): needed for dynalloc JES spool https://www.ibm.com/docs/en/zos/3.1.0?topic=programming-jes-spool-data-set-browse
// NOTE(Kelosky): needed for dynalloc https://www.ibm.com/docs/en/zos/3.1.0?topic=list-coding-dynamic-allocation-request
int zjb_read_jobs_output_by_jobid_and_key(ZJB *zjb, string jobid, int key, string &response)
{
  int rc = 0;
  string job_dsn;

  rc = zjb_get_job_dsn_by_jobid_and_key(zjb, jobid, key, job_dsn);
  if (0 != rc) return rc;

  return zjb_read_job_content_by_dsn(zjb, job_dsn, response);
}

int zjb_get_job_dsn_by_jobid_and_key(ZJB *zjb, string jobid, int key, string &job_dsn)
{
  int rc = 0;

  vector<ZJobDD> list;

  rc = zjb_list_dds_by_jobid(zjb, jobid, list);
  if (0 != rc) return rc;

  rc = RTNCD_FAILURE; // assume failure

  for (vector<ZJobDD>::iterator it = list.begin(); it != list.end(); ++it)
  {
    if (key == it->key)
    {
      job_dsn = it->dsn;
      rc = 0;
      break;
    }
  }

  if (0 != rc)
  {
    zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "Could not locate data set key '%d' on job '%s'", key, jobid.c_str());
    zjb->diag.detail_rc = ZJB_RTNCD_JOB_NOT_FOUND;
    return RTNCD_FAILURE;
  }

  return RTNCD_SUCCESS;
}

#define NUM_TEXT_UNITS 5

int zjb_read_job_content_by_dsn(ZJB *zjb, string jobdsn, string &response)
{
  int rc = 0;
  unsigned char *p = NULL;
  ZDS zds = {0};

  // calculate total size needed, obtain, & clear
  int total_size_needed = sizeof(IAZBTOKP) + (sizeof(S99TUNIT_X) * NUM_TEXT_UNITS) + (sizeof(S99TUPL) * NUM_TEXT_UNITS) + sizeof(__S99parms) + sizeof(__S99rbx_t);
  unsigned char *parms = (unsigned char *)__malloc31(total_size_needed);
  memset(parms, 0x00, total_size_needed);

  // carve up storage to needed structs
  IAZBTOKP *PTR32 iazbtokp = (IAZBTOKP * PTR32) parms;
  S99TUNIT_X *PTR32 s99tunit_x = (S99TUNIT_X * PTR32) (parms + sizeof(IAZBTOKP));
  S99TUPL *PTR32 s99tupl = (S99TUPL * PTR32) (parms + sizeof(IAZBTOKP) + (sizeof(S99TUNIT_X) * NUM_TEXT_UNITS));
  __S99parms *PTR32 s99parms = (__S99parms * PTR32) (parms + sizeof(IAZBTOKP) + (sizeof(S99TUNIT_X) * NUM_TEXT_UNITS) + (sizeof(S99TUPL) * NUM_TEXT_UNITS));
  __S99rbx_t *PTR32 s99parmsx = (__S99rbx_t * PTR32) (parms + sizeof(IAZBTOKP) + (sizeof(S99TUNIT_X) * NUM_TEXT_UNITS) + (sizeof(S99TUPL) * NUM_TEXT_UNITS) + sizeof(__S99parms));

  // https://www.ibm.com/docs/en/zos/3.1.0?topic=allocation-building-browse-token-dalbrtkn
  short int len = sizeof(iazbtokp->btokid);
  memcpy(iazbtokp->btokpl1, &len, sizeof(len));
  memcpy(iazbtokp->btokid, "BTOK", sizeof(iazbtokp->btokid));
  len = sizeof(iazbtokp->btokver);
  memcpy(iazbtokp->btokpl2, &len, sizeof(len));
  iazbtokp->btoktype = btokbrws;
  iazbtokp->btokvers = btokvrnm;
  len = sizeof(iazbtokp->btokiotp);
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
  plen = strlen(jobdsn.c_str());
  memcpy(s99tunit_x[i].s99tunit.s99tukey, &dynkey, sizeof(dynkey));
  memcpy(s99tunit_x[i].s99tunit.s99tunum, &numparms, sizeof(numparms));
  memcpy(s99tunit_x[i].s99tunit.s99tulng, &plen, sizeof(plen));
  memcpy(&s99tunit_x[i].s99tunit.s99tupar, jobdsn.c_str(), plen);

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

  // https://www.ibm.com/docs/en/zos/3.1.0?topic=guide-dynamic-allocation
  rc = svc99(s99parms);

  // TODO(Kelosky): parse s99parmsx->__S99ENMSG and free
  if (0 != rc && 0 != s99parms->__S99ERROR)
  {
    strcpy(zjb->diag.service_name, "svc99");
    zjb->diag.service_rc = rc;
    zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "Could not allocate job spool file '%s', rc: '%d' s99error: '%d' s99info: '%d'", jobdsn.c_str(), rc, s99parms->__S99ERROR, s99parms->__S99INFO);
    zjb->diag.detail_rc = ZJB_RTNCD_SERVICE_FAILURE;
    free(parms);
    return RTNCD_FAILURE;
  }

  short ddnamelen = 0;
  memcpy(&ddnamelen, s99tunit_x[4].s99tunit.s99tulng, sizeof(ddnamelen));

  char cddname[8 + 1] = {0};
  memcpy(cddname, &s99tunit_x[4].s99tunit.s99tupar, ddnamelen);
  string ddname = string(cddname);
  rc = zds_read_from_dd(&zds, ddname, response);

  free(parms);

  if (0 != rc)
  {
    memcpy(&zjb->diag, &zds.diag, sizeof(ZDIAG));
    return rc;
  }

  // free DD
  __dyn_t ip;
  dyninit(&ip);
  ip.__ddname = cddname; // e.g. SYS00001
  rc = dynfree(&ip);

  if (0 != rc)
  {
    strcpy(zjb->diag.service_name, "dynfree");
    zjb->diag.service_rc = rc;
    zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "dynfree failed with %d", rc);
    zjb->diag.detail_rc = ZJB_RTNCD_SERVICE_FAILURE;
    return RTNCD_FAILURE;
  }

  return rc;
}

int zjb_delete_by_jobid(ZJB *zjb, string jobid)
{
  zut_uppercase_pad_truncate(jobid, zjb->jobid, sizeof(zjb->jobid));
  return ZJBMPRG(zjb);
}

int zjb_submit(ZJB *zjb, string data_set, string &jobId)
{
  int rc = 0;
  string content;
  ZDS zds = {0};
  rc = zds_read_from_dsn(&zds, data_set, content);
  if (rc != 0)
  {
    memcpy(&zjb->diag, &zds.diag, sizeof(ZDIAG));
    return rc;
  }

  __dyn_t ip;
  rc = dyninit(&ip);
  if (0 != rc)
  {
    strcpy(zjb->diag.service_name, "dyninit");
    zjb->diag.service_rc = rc;
    zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "dyninit failed with %d", rc);
    zjb->diag.detail_rc = ZJB_RTNCD_SERVICE_FAILURE;
    return RTNCD_FAILURE;
  }

  string ddname = "????????"; // system generated DD name
  ip.__ddname = (char *)ddname.c_str();
  ip.__sysoutname = "INTRDR  "; // https://www.ibm.com/docs/en/zos/3.1.0?topic=control-destination-internal-reader && https://www.ibm.com/docs/en/zos/3.1.0?topic=programming-internal-reader-facility
  ip.__lrecl = 80;
  ip.__blksize = 80;
  ip.__sysout =  __DEF_CLASS;
  ip.__recfm = _FB_;

  rc = dynalloc(&ip);

  if (0 != rc)
  {
    strcpy(zjb->diag.service_name, "dynalloc");
    zjb->diag.service_rc = rc;
    zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "dynalloc failed with %d", rc);
    zjb->diag.detail_rc = ZJB_RTNCD_SERVICE_FAILURE;
    return RTNCD_FAILURE;
  }

  rc = zds_write_to_dd(&zds, ddname, content);
  if (rc != 0)
  {
    memcpy(&zjb->diag, &zds.diag, sizeof(ZDIAG));
    return rc;
  }

  char cjobid[8 + 1] = {0};
  // https://www.ibm.com/docs/en/zos/3.1.0?topic=iazsymbl-jes-system-symbols
  rc = ZJBSYMB(zjb, "SYS_LASTJOBID", cjobid);

  if (0 != rc) return rc;

  jobId = string(cjobid);

  rc = dynfree(&ip);
  if (0 != rc)
  {
    strcpy(zjb->diag.service_name, "dynfree");
    zjb->diag.service_rc = rc;
    zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "dynfree failed with %d", rc);
    zjb->diag.detail_rc = ZJB_RTNCD_SERVICE_FAILURE;
    return RTNCD_FAILURE;
  }

  return RTNCD_SUCCESS;
}

int zjb_list_dds_by_jobid(ZJB *zjb, string jobid, vector<ZJobDD> &jobDDs)
{
  int rc = 0;
  STATSEVB *PTR64 sysoutInfo = NULL;
  int entries = 0;

  if (0 == zjb->buffer_size) zjb->buffer_size = ZJB_DEFAULT_BUFFER_SIZE;
  if (0 == zjb->dds_max) zjb->dds_max = ZJB_DEFAULT_MAX_DDS;

  zut_uppercase_pad_truncate(jobid, zjb->jobid, sizeof(zjb->jobid));

  rc = ZJBMLSDS(zjb, &sysoutInfo, &entries);
  if (0 != rc) return rc;

  if (0 == entries)
  {
    ZUTMFR64(sysoutInfo);
    zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "Could not locate job '%s'", jobid.c_str());
    zjb->diag.detail_rc = ZJB_RTNCD_JOB_NOT_FOUND;
    return RTNCD_FAILURE;
  }

  STATSEVB *PTR64 sysoutInfoNext = sysoutInfo;

  for (int i = 0; i < entries; i++)
  {
    char tempDDn[9] = {0};
    char tempsn[9] = {0};
    char temppn[9] = {0};
    char tempDSN[45] = {0};

    strncpy(tempDDn, (char *)sysoutInfoNext[i].stvsddnd, sizeof(sysoutInfo->stvsddnd));
    strncpy(tempsn, (char *)sysoutInfoNext[i].stvsstpd, sizeof(sysoutInfo->stvsstpd));
    strncpy(temppn, (char *)sysoutInfoNext[i].stvsprcd, sizeof(sysoutInfo->stvsprcd));
    strncpy(tempDSN, (char *)sysoutInfoNext[i].stvsdsn, sizeof(sysoutInfo->stvsdsn));

    string ddn(tempDDn);
    string stepname(tempsn);
    string procstep(temppn);
    string dsn(tempDSN);

    ZJobDD zjobdd = {0};

    zjobdd.ddn = ddn;
    zjobdd.stepname = stepname;
    zjobdd.procstep = procstep;
    zjobdd.dsn = dsn;
    zjobdd.jobid = string(jobid);
    zjobdd.key = sysoutInfoNext[i].stvsdsky;

    jobDDs.push_back(zjobdd);
  }

  ZUTMFR64(sysoutInfo);

  return rc;
}

int zjb_list_by_owner(ZJB *zjb, string owner_name, vector<ZJob> &jobs)
{
  int rc = 0;
  STATJQTR *PTR64 jobInfo = NULL;
  int entries = 0;

  if ("" == owner_name)
  {
    rc = zut_get_current_user(owner_name);
    if (0 != rc)
    {
      strcpy(zjb->diag.service_name, "IAZXJSAB");
      zjb->diag.service_rc = rc;
      zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "Could not get current user - %d", rc);
      zjb->diag.detail_rc = ZJB_RTNCD_SERVICE_FAILURE;
      return RTNCD_FAILURE;
    }
  }

  if (0 == zjb->buffer_size) zjb->buffer_size = ZJB_DEFAULT_BUFFER_SIZE;
  if (0 == zjb->jobs_max) zjb->jobs_max = ZJB_DEFAULT_MAX_JOBS;

  zut_uppercase_pad_truncate(owner_name, zjb->owner_name, sizeof(zjb->owner_name));

  rc = ZJBMLIST(zjb, &jobInfo, &entries);
  if (0 != rc) return rc;

  STATJQTR *PTR64 jobInfoNext = jobInfo;

  for (int i = 0; i < entries; i++)
  {
    char tempJobName[9] = {0};
    char tempJobId[9] = {0};
    char tempJobOwner[9] = {0};

    strncpy(tempJobName, (char *)jobInfoNext[i].sttrname, sizeof(jobInfo->sttrname));
    strncpy(tempJobId, (char *)jobInfoNext[i].sttrjid, sizeof(jobInfo->sttrjid));
    strncpy(tempJobOwner, (char *)jobInfoNext[i].sttrouid, sizeof(jobInfo->sttrouid));

    ZJob zjob = {0};

    string jobname(tempJobName);
    string jobid(tempJobId);
    string owner(tempJobOwner);

    union cc
    {
      int full;
      unsigned char parts[4];
    } mycc = {0};
    memcpy(&mycc, &jobInfoNext[i].sttrxind, sizeof(cc));

    if ((unsigned char)jobInfoNext[i].sttrxind & sttrxab)
    {
      // for an abend, these are the bits which contain the code
      mycc.full &= 0x00FFF000; // clear uneeded bits
      unsigned char byte1 = mycc.parts[1] >> 4;
      unsigned char byte2 = mycc.parts[1] & 0x0F;
      unsigned char byte3 = mycc.parts[2] >> 4;

      string result = "ABEND ";
      result.push_back(zut_get_hex_char(byte1));
      result.push_back(zut_get_hex_char(byte2));
      result.push_back(zut_get_hex_char(byte3));
      zjob.retcode = result;
    }
    else if ((unsigned char)jobInfoNext[i].sttrxind == sttrxjcl)
    {
      zjob.retcode = "JCL ERROR";
    }
    else
    {
      mycc.full &= 0x00000FFF; // clear uneeded bits
      stringstream sscc;
      sscc << setw(4) << setfill('0') << mycc.full; // format to 4 characters
      zjob.retcode = "CC " + sscc.str();            // make it look like z/OSMF
    }

    // NOTE(Kelosky): this might need additional testing
    if (jobInfoNext[i].sttrphaz < stat___onmain)
    {
      zjob.status = "INPUT";
    }
    else if (jobInfoNext[i].sttrphaz == stat___onmain)
    {
      zjob.status = "ACTIVE";
    }
    else if (jobInfoNext[i].sttrphaz == stat___outpt)
    {
      zjob.status = "OUTPUT";
    }
    else
    {
      zjob.status = "UNKNOWN";
    }

    zjob.jobname = jobname;
    zjob.jobid = jobid;
    zjob.owner = owner;

    jobs.push_back(zjob);
  }

  ZUTMFR64(jobInfo);

  return RTNCD_SUCCESS;
}
