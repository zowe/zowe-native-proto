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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "zjblkup.h"
#include "zssitype.h"
#include "zjbm.h"
#include "zwto.h"
#include "zssi31.h"
#include "zjbm31.h"
#include "zstorage.h"
#include "zjsytype.h"
#include "zmetal.h"
#include "zjbtype.h"
#include "ihapsa.h"
#include "cvt.h"
#include "iefjesct.h"

// TODO(Kelosky):
// https://www.ibm.com/docs/en/zos/3.1.0?topic=79-putget-requests
// read system log

#define SYMBOL_ENTRIES 3
typedef struct
{
  JSYTABLE jsymbolTable;
  JSYENTRY jsymbolEntry[SYMBOL_ENTRIES];
  unsigned char buffer[SYMBOL_ENTRIES * 16];
} JSYMBOLO;

typedef struct psa PSA;
typedef struct cvt CVT;
typedef struct jesct JESCT;

static int get_ssibssnm(SSIB *ssib)
{
  PSA *psa_a = (PSA *)0;
  CVT *cvt_a = psa_a->flccvt;
  JESCT *jesct_a = cvt_a->cvtjesct;

  // This might be overkill, but just making sure we have an SS name
  if (NULL == jesct_a->jespjesn)
  {
    return RTNCD_FAILURE;
  }
  // We do not support JES3 (yet)
  if (0 != jesct_a->jesjesfg & jes3actv)
  {
    return ZJB_RTNCD_JES3_NOT_SUPPORTED;
  }

  memcpy(ssib->ssibssnm, jesct_a->jespjesn, sizeof(ssib->ssibssnm));
  return RTNCD_SUCCESS;
}

static int init_ssib(SSIB *ssib)
{
  memcpy(ssib->ssibid, "SSIB", sizeof(ssib->ssibid));
  ssib->ssiblen = sizeof(SSIB);
  return get_ssibssnm(ssib);
}

static void init_ssob(SSOB *PTR32 ssob, SSIB *PTR32 ssib, void *PTR32 function_depenent_area, int function)
{
  memcpy(ssob->ssobid, "SSOB", sizeof(ssob->ssobid));
  ssob->ssoblen = sizeof(SSOB);
  ssob->ssobssib = ssib;
  ssob->ssobindv = (int)function_depenent_area;
  ssob->ssobfunc = function;
}

static void init_stat(STAT *stat)
{
  memcpy(stat->stateye, "STAT", sizeof(stat->stateye));
  stat->statverl = statcvrl;
  stat->statverm = statcvrm;
  stat->statlen = statsize;
}

#pragma prolog(ZJBSYMB, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(ZJBSYMB, " ZWEEPILG ")
int ZJBSYMB(ZJB *zjb, const char *symbol, char *value)
{
  int rc = 0;

  JSYMPARM jsym = {0};

  unsigned char *p = NULL;

  memcpy(jsym.jsymeye, "JSYM", sizeof(jsym.jsymeye));
  jsym.jsymlng = jsymsize;
  jsym.jsymrqop = jsymextr;

#define JSYMVRMC 0x0100;

  jsym.jsymvrm = JSYMVRMC;
  jsym.jsymsnmn = 1;
  jsym.jsymsnml = (int)strlen(symbol);
  jsym.jsymsnma = (void *PTR32)symbol;

  JSYMBOLO jsymbolOutput = {0};

  jsym.jsymouta = &jsymbolOutput;
  jsym.jsymouts = sizeof(JSYMBOLO);

  rc = iazsymbl(&jsym);

  if (0 != rc)
  {
    // TODO(Kelosky): read jsymerad for errors
    strcpy(zjb->diag.service_name, "iazsymbl");
    zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "IAZSYMBL RC was: '%d', JSYMRETN was: '%d', JSYMREAS: %d", rc,
                                  jsym.jsymretn, jsym.jsymreas);
    zjb->diag.detail_rc = ZJB_RTNCD_SERVICE_FAILURE;
    return RTNCD_FAILURE;
  }

  p = (unsigned char *)&jsymbolOutput.jsymbolTable;                             // --> table
  JSYENTRY *jsymbolEntry = (JSYENTRY *)(p + jsymbolOutput.jsymbolTable.jsytent1); // --> first entry

  p = p + jsymbolEntry->jsyevalo;
  memcpy(value, p, jsymbolEntry->jsyevals);

  return RTNCD_SUCCESS;
}

// purge a job
#pragma prolog(ZJBMPRG, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(ZJBMPRG, " ZWEEPILG ")
int ZJBMPRG(ZJB *zjb)
{
  // purge a job in protected (ssjmpprt) mode
  return ZJBMMOD(zjb, ssjmprg, ssjmpprt);
}

// cancel a job
#pragma prolog(ZJBMCNL, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(ZJBMCNL, " ZWEEPILG ")
int ZJBMCNL(ZJB *zjb, int flags)
{
  // cancel a job in protected (ssjmcprt) mode
  int options = ssjmcprt | flags;
  return ZJBMMOD(zjb, ssjmcanc, ssjmcprt);
}

// hold a job
#pragma prolog(ZJBMHLD, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(ZJBMHLD, " ZWEEPILG ")
int ZJBMHLD(ZJB *zjb)
{
  // Hold a job in protected (ssjmpprt) mode
  return ZJBMMOD(zjb, ssjmhold, 0);
}

// release a job
#pragma prolog(ZJBMRLS, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(ZJBMRLS, " ZWEEPILG ")
int ZJBMRLS(ZJB *zjb)
{
  // Release a job in protected (ssjmpprt) mode
  return ZJBMMOD(zjb, ssjmrls, 0);
}

// modify a job
int ZJBMMOD(ZJB *zjb, int type, int flags)
{
  int rc = 0;
  int loop_control = 0;

  // return rc
  SSOB *PTR32 ssobp = NULL;
  SSOB ssob = {0};
  SSIB ssib = {0};
  SSJM ssjm = {0};

  // https://www.ibm.com/docs/en/zos/3.1.0?topic=sfcd-modify-job-function-call-ssi-function-code-85
  init_ssob(&ssob, &ssib, &ssjm, 85);
  rc = init_ssib(&ssib);
  if (0 != rc)
  {
    strcpy(zjb->diag.service_name, "init_ssib");
    zjb->diag.detail_rc = rc;
    if (ZJB_RTNCD_JES3_NOT_SUPPORTED == rc)
    {
      zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "JES3 is not supported");
    }
    else
    {
      zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "Failed to get SSIBSSNM");
    }
    return RTNCD_FAILURE;
  }

  memcpy(ssjm.ssjmeye, "SSJMPL  ", sizeof(ssjm.ssjmeye));
  ssjm.ssjmlen = ssjmsize;
  ssjm.ssjmvrm = ssjmvrmc;

  ssjm.ssjmopt1 = ssjm.ssjmopt1 | ssjmpd64; // 64 bit storage
  ssjm.ssjmopt1 = ssjm.ssjmopt1 | ssjmpsyn; // SYNC

  ssjm.ssjmtype = type;
  if (ssjmprg == type) // purge
  {
    ssjm.ssjmpflg = ssjm.ssjmpflg | flags;
  }
  else if (ssjmcanc == type) // cancel
  {
    ssjm.ssjmcflg = ssjm.ssjmcflg | flags;
  }

  memcpy(ssjm.ssjmjbid, zjb->header.jobid, 8);

  ssobp = &ssob;

  rc = iazssji(&ssobp);

  if (0 != rc)
  {
    strcpy(zjb->diag.service_name, "iazssji");
    zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "IAZSSJI RC was: '%d', SSOB return code was: '%d', SSJM return code was: '%d'", rc,
                                  ssob.ssobretc, ssjm.ssjmretn);
    zjb->diag.detail_rc = ZJB_RTNCD_SERVICE_FAILURE;
    return RTNCD_FAILURE;
  }

  return RTNCD_SUCCESS;
}

// view a single job
#pragma prolog(ZJBMVIEW, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(ZJBMVIEW, " ZWEEPILG ")
int ZJBMVIEW(ZJB *zjb, ZJB_JOB_INFO **PTR64 job_info, int *entries)
{
  int rc = 0;

  SSOB *PTR32 ssobp = NULL;
  SSOB ssob = {0};
  SSIB ssib = {0};
  STAT stat = {0};

  init_ssob(&ssob, &ssib, &stat, 83); // get job queue info
  rc = init_ssib(&ssib);
  if (0 != rc)
  {
    strcpy(zjb->diag.service_name, "init_ssib");
    zjb->diag.detail_rc = rc;
    if (ZJB_RTNCD_JES3_NOT_SUPPORTED == rc)
    {
      zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "JES3 is not supported");
    }
    else
    {
      zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "Failed to get SSIBSSNM");
    }
    return RTNCD_FAILURE;
  }

  init_stat(&stat);

  STATJQ *PTR32 statjqp = NULL;
  rc = ZJBMGJQ(zjb, &ssob, &stat, &statjqp);
  if (0 != rc)
  {
    return rc;
  }

  rc = ZJBMTCOM(zjb, &stat, job_info, entries);

  return rc;
}

// list all jobs
#pragma prolog(ZJBMLIST, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(ZJBMLIST, " ZWEEPILG ")
int ZJBMLIST(ZJB *zjb, ZJB_JOB_INFO **PTR64 job_info, int *entries)
{
  int rc = 0;

  SSOB *PTR32 ssobp = NULL;
  SSOB ssob = {0};
  SSIB ssib = {0};
  STAT stat = {0};

  init_ssob(&ssob, &ssib, &stat, 83); // get job queue info
  rc = init_ssib(&ssib);
  if (0 != rc)
  {
    strcpy(zjb->diag.service_name, "init_ssib");
    zjb->diag.detail_rc = rc;
    if (ZJB_RTNCD_JES3_NOT_SUPPORTED == rc)
    {
      zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "JES3 is not supported");
    }
    else
    {
      zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "Failed to get SSIBSSNM");
    }
    return RTNCD_FAILURE;
  }

  init_stat(&stat);

  STATJQ *PTR32 statjqp = NULL;
  rc = ZJBMGJQ(zjb, &ssob, &stat, &statjqp);
  if (0 != rc)
  {
    return rc;
  }

  rc = ZJBMTCOM(zjb, &stat, job_info, entries);

  return rc;
}

int ZJBMGJQ(ZJB *zjb, SSOB *ssobp, STAT *statp, STATJQ *PTR32 *PTR32 statjqp)
{
  int rc = 0;

  // Now invoke the service
  statp->statfmem = statfmjq;
  statp->statflg1 = 0;
  statp->statflg2 = 0;
  statp->statflg3 = 0;
  statp->statflg4 = 0;

  if (0 != strlen(zjb->header.jobid))
  {
    statp->statflg1 = statp->statflg1 | statjobi;
    memcpy(statp->statjobn, zjb->header.jobid, 8);
  }

  rc = iazssjm(ssobp);

  if (0 != rc)
  {
    rc = ZJBMEMSG(zjb, statp, ssobp, rc);
    return rc;
  }

  *statjqp = (STATJQ *PTR32)statp->statptr1;

  return RTNCD_SUCCESS;
}

int ZJBMEMSG(ZJB *zjb, STAT *PTR64 stat, SSOB *PTR64 ssobp, int rc)
{
  strcpy(zjb->diag.service_name, "iazssjm");
  zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "IAZSSJM RC was: '%d', SSOB return code was: '%d', STAT return code was: '%d'", rc,
                                ssobp->ssobretc, stat->statretc);
  zjb->diag.detail_rc = ZJB_RTNCD_SERVICE_FAILURE;

  // write our message to the screen / log for reference
  // TODO(Kelosky): may want to remove this when stable
  char msg[100] = {0};
  sprintf(msg, "IAZSSJM RC: '%d', SSOB: '%d', STAT: '%d'", rc, ssobp->ssobretc, stat->statretc);
  wto31(msg);

  return RTNCD_FAILURE;
}

int ZJBMTCOM(ZJB *zjb, STAT *PTR64 stat, ZJB_JOB_INFO **PTR64 job_info, int *entries)
{
  int rc = 0;

  STATJQSE *statjqse = NULL;
  STATJQE *statjqe = NULL;
  STATJQE *statjqe_start = NULL;

  char *p = NULL;

  char user[9] = {0};
  char class[2] = {0};

  // ptr to extended offsets list and other information about returned job queue entries
  STATJQ *PTR32 statjq = (STATJQ *PTR32)stat->statptr1;

  *entries = statjq->statjqnr;

  ZJB_JOB_INFO *PTR64 jinfos = (ZJB_JOB_INFO *PTR64)CALLOC64(*entries, sizeof(ZJB_JOB_INFO));

  if (!jinfos)
  {
    strcpy(zjb->diag.service_name, "CALLOC64");
    zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "Memory failure allocating jobs information");
    zjb->diag.detail_rc = ZJB_RTNCD_SERVICE_FAILURE;
    return RTNCD_FAILURE;
  }

  p = (char *)statjq;                                  // get ptr to structure
  statjqse = (STATJQSE *)(p + statjq->statjqof);      // ptr to extended offsets
  statjqe_start = (STATJQE *)(p + statjqse->jqesttof); // ptr to table entries
  statjqe = statjqe_start;

  for (int i = 0; i < *entries; i++)
  {
    ZJB_JOB_INFO *job_entry = &jinfos[i];

    memcpy(job_entry->jobname, statjqe->jqejname, 8);
    memcpy(job_entry->jobid, statjqe->jqejbid, 8);

    // Remove EBCDIC trailing spaces and null terminate
    for (int j = 7; j >= 0; j--)
    {
      if (job_entry->jobname[j] != ' ')
      {
        job_entry->jobname[j + 1] = '\0';
        break;
      }
    }

    for (int j = 7; j >= 0; j--)
    {
      if (job_entry->jobid[j] != ' ')
      {
        job_entry->jobid[j + 1] = '\0';
        break;
      }
    }

    memcpy(user, statjqe->jqeuseid, 8);
    for (int j = 7; j >= 0; j--)
    {
      if (user[j] != ' ')
      {
        user[j + 1] = '\0';
        break;
      }
    }
    strcpy(job_entry->owner, user);

    memcpy(class, &statjqe->jqeclass, 1);
    class[1] = '\0';
    strcpy(job_entry->class, class);

    strcpy(job_entry->status, get_job_status(statjqe->jqestc));

    switch (statjqe->jqetype)
    {
    case jqetjob:
      strcpy(job_entry->type, "JOB");
      break;
    case jqetstc:
      strcpy(job_entry->type, "STC");
      break;
    case jqettsu:
      strcpy(job_entry->type, "TSU");
      break;
    default:
      strcpy(job_entry->type, "UNK");
      break;
    }

    statjqe = (STATJQE *)((char *)statjqe + statjqe->jqelen);
  }

  *job_info = jinfos;

  return RTNCD_SUCCESS;
}

#pragma prolog(ZJBMLSDS, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(ZJBMLSDS, " ZWEEPILG ")
int ZJBMLSDS(ZJB *PTR64 zjb, STATSEVB **PTR64 sysoutInfo, int *entries)
{
  int rc = 0;

  SSOB *PTR32 ssobp = NULL;
  SSOB ssob = {0};
  SSIB ssib = {0};
  STAT stat = {0};

  init_ssob(&ssob, &ssib, &stat, 83); // get job queue info
  rc = init_ssib(&ssib);
  if (0 != rc)
  {
    strcpy(zjb->diag.service_name, "init_ssib");
    zjb->diag.detail_rc = rc;
    if (ZJB_RTNCD_JES3_NOT_SUPPORTED == rc)
    {
      zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "JES3 is not supported");
    }
    else
    {
      zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "Failed to get SSIBSSNM");
    }
    return RTNCD_FAILURE;
  }

  init_stat(&stat);

  // Now invoke the service
  stat.statfmem = statfmse;
  stat.statflg1 = 0;
  stat.statflg2 = 0;
  stat.statflg3 = 0;
  stat.statflg4 = 0;

  if (0 != strlen(zjb->header.jobid))
  {
    stat.statflg1 = stat.statflg1 | statjobi;
    memcpy(stat.statjobn, zjb->header.jobid, 8);
  }

  ssobp = &ssob;

  rc = iazssjm(ssobp);

  if (0 != rc)
  {
    rc = ZJBMEMSG(zjb, &stat, ssobp, rc);
    return rc;
  }

  STATSE *PTR32 statse = (STATSE *PTR32)stat.statptr1; // ptr to sysout information

  *entries = statse->statseno;

  STATSEVB *PTR64 dses = (STATSEVB *PTR64)CALLOC64(*entries, sizeof(STATSEVB));

  if (!dses)
  {
    strcpy(zjb->diag.service_name, "CALLOC64");
    zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "Memory failure allocating sysout information");
    zjb->diag.detail_rc = ZJB_RTNCD_SERVICE_FAILURE;
    return RTNCD_FAILURE;
  }

  char *p = (char *)statse;                         // get ptr to statse structure
  STATSESO *statseso = (STATSESO *)(p + statse->statseof); // ptr to extended offsets
  STATSEVB *statsevb_start = (STATSEVB *)(p + statseso->sesttof); // ptr to variable table entries
  STATSEVB *statsevb = statsevb_start;

  for (int i = 0; i < *entries; i++)
  {
    STATSEVB *sysout_entry = &dses[i];

    memcpy(sysout_entry, statsevb, sizeof(STATSEVB));

    // move to next
    statsevb = (STATSEVB *)((char *)statsevb + statsevb->sevblen);
  }

  *sysoutInfo = dses;

  return RTNCD_SUCCESS;
}

char *get_job_status(unsigned char status_code)
{
  switch (status_code)
  {
  case jqsrecv:
    return "INPUT";
  case jqshold:
    return "HELD";
  case jqsexec:
    return "ACTIVE";
  case jqsout:
    return "OUTPUT";
  default:
    return "UNKNOWN";
  }
}
