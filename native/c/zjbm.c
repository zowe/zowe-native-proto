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
typedef struct {
  JSYTABLE jsymbolTable;
  JSYENTRY jsymbolEntry[SYMBOL_ENTRIES];
  unsigned char buffer[SYMBOL_ENTRIES * 16];
} JSYMBOLO;

typedef struct psa PSA;
typedef struct cvt CVT;
typedef struct jesct JESCT;

static int get_ssibssnm(SSIB* ssib) {
  PSA* psa_a = (PSA*)0;
  CVT* cvt_a = psa_a->flccvt;
  JESCT* jesct_a = cvt_a->cvtjesct;

  // This might be overkill, but just making sure we have an SS name
  if (NULL == jesct_a->jespjesn) {
    return RTNCD_FAILURE;
  }
  // We do not support JES3 (yet)
  if (0 != jesct_a->jesjesfg & jes3actv) {
    return ZJB_RTNCD_JES3_NOT_SUPPORTED;
  }

  memcpy(ssib->ssibssnm, jesct_a->jespjesn, sizeof(ssib->ssibssnm));
  return RTNCD_SUCCESS;
}

static int init_ssib(SSIB* ssib) {
  memcpy(ssib->ssibid, "SSIB", sizeof(ssib->ssibid));
  ssib->ssiblen = sizeof(SSIB);
  return get_ssibssnm(ssib);
}

static void init_ssob(SSOB* PTR32 ssob, SSIB* PTR32 ssib, void* PTR32 function_depenent_area, int function) {
  memcpy(ssob->ssobid, "SSOB", sizeof(ssob->ssobid));
  ssob->ssoblen = sizeof(SSOB);
  ssob->ssobssib = ssib;
  ssob->ssobindv = (int)function_depenent_area;
  ssob->ssobfunc = function;
}

static void init_stat(STAT* stat) {
  memcpy(stat->stateye, "STAT", sizeof(stat->stateye));
  stat->statverl = statcvrl;
  stat->statverm = statcvrm;
  stat->statlen = statsize;
}

#pragma prolog(ZJBSYMB, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(ZJBSYMB, " ZWEEPILG ")
int ZJBSYMB(ZJB* zjb, const char* symbol, char* value) {
  int rc = 0;

  JSYMPARM jsym = {0};

  unsigned char* p = NULL;

  memcpy(jsym.jsymeye, "JSYM", sizeof(jsym.jsymeye));
  jsym.jsymlng = jsymsize;
  jsym.jsymrqop = jsymextr;

#define JSYMVRMC 0x0100;

  jsym.jsymvrm = JSYMVRMC;
  jsym.jsymsnmn = 1;
  jsym.jsymsnml = (int)strlen(symbol);
  jsym.jsymsnma = (void* PTR32)symbol;

  JSYMBOLO jsymbolOutput = {0};

  jsym.jsymouta = &jsymbolOutput;
  jsym.jsymouts = sizeof(JSYMBOLO);

  rc = iazsymbl(&jsym);

  if (0 != rc) {
    // TODO(Kelosky): read jsymerad for errors
    strcpy(zjb->diag.service_name, "iazsymbl");
    zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "IAZSYMBL RC was: '%d', JSYMRETN was: '%d', JSYMREAS: %d", rc,
                                  jsym.jsymretn, jsym.jsymreas);
    zjb->diag.detail_rc = ZJB_RTNCD_SERVICE_FAILURE;
    return RTNCD_FAILURE;
  }

  p = (unsigned char*)&jsymbolOutput.jsymbolTable;                               // --> table
  JSYENTRY* jsymbolEntry = (JSYENTRY*)(p + jsymbolOutput.jsymbolTable.jsytent1); // --> first entry

  p = p + jsymbolEntry->jsyevalo;
  memcpy(value, p, jsymbolEntry->jsyevals);

  return RTNCD_SUCCESS;
}

// purge a job
#pragma prolog(ZJBMPRG, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(ZJBMPRG, " ZWEEPILG ")
int ZJBMPRG(ZJB* zjb) {
  // purge a job in protected (ssjmpprt) mode
  return ZJBMMOD(zjb, ssjmprg, ssjmpprt);
}

// cancel a job
#pragma prolog(ZJBMCNL, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(ZJBMCNL, " ZWEEPILG ")
int ZJBMCNL(ZJB* zjb, int flags) {
  // cancel a job in protected (ssjmcprt) mode
  int options = ssjmcprt | flags;
  return ZJBMMOD(zjb, ssjmcanc, ssjmcprt);
}

// hold a job
#pragma prolog(ZJBMHLD, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(ZJBMHLD, " ZWEEPILG ")
int ZJBMHLD(ZJB* zjb) {
  // Hold a job in protected (ssjmpprt) mode
  return ZJBMMOD(zjb, ssjmhold, 0);
}

// release a job
#pragma prolog(ZJBMRLS, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(ZJBMRLS, " ZWEEPILG ")
int ZJBMRLS(ZJB* zjb) {
  // Release a job in protected (ssjmpprt) mode
  return ZJBMMOD(zjb, ssjmrls, 0);
}

// modify a job
int ZJBMMOD(ZJB* zjb, int type, int flags) {
  int rc = 0;
  int loop_control = 0;

  // return rc
  SSOB* PTR32 ssobp = NULL;
  SSOB ssob = {0};
  SSIB ssib = {0};
  SSJM ssjm = {0};

  // https://www.ibm.com/docs/en/zos/3.1.0?topic=sfcd-modify-job-function-call-ssi-function-code-85
  init_ssob(&ssob, &ssib, &ssjm, 85);
  rc = init_ssib(&ssib);
  if (0 != rc) {
    strcpy(zjb->diag.service_name, "init_ssib");
    zjb->diag.detail_rc = rc;
    if (ZJB_RTNCD_JES3_NOT_SUPPORTED == rc) {
      zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "JES3 is not supported");
    } else {
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
  } else if (ssjmcanc == type) // cancel
  {
    ssjm.ssjmcflg = ssjm.ssjmcflg | flags;
  } else if (ssjmhold == type) // hold
  {
    // no flags needed
  } else if (ssjmrls == type) // release
  {
    // no flags needed
  } else if (ssjmrst == type) // restart
  {                           // TODO(zFernand0): not implemented
    ssjm.ssjmeflg = ssjm.ssjmeflg | flags;
  } else if (ssjmspin == type) // spin
  {                            // TODO(zFernand0): not implemented
    ssjm.ssjmtsfl = ssjm.ssjmtsfl | flags;
    // ssjm.ssjmtsdn = ddname to spin
  }

  if (zjb->jobid[0] != 0x00) {
    ssjm.ssjmsel1 = ssjm.ssjmsel1 | ssjmsoji; //@TEST
    memcpy(ssjm.ssjmojbi, zjb->jobid, sizeof(ssjm.ssjmojbi));
  } else {
    char correlator31[64] = {0};
    memcpy(correlator31, zjb->correlator, sizeof(zjb->correlator));
    ssjm.ssjmsel5 = ssjmscor;
    ssjm.ssjmjcrp = &correlator31[0];
  }

  // ssjm.ssjmsel2 = ssjm.ssjmsel2 | ssjmsjob; // batch jobs
  // ssjm.ssjmsel2 = ssjm.ssjmsel2 | ssjmsstc; // stcs
  // ssjm.ssjmsel2 = ssjm.ssjmsel2 | ssjmstsu; // time sharing users

  ssobp = &ssob;
  ssobp = (SSOB * PTR32)((unsigned int)ssobp | 0x80000000);
  rc = iefssreq(&ssobp); // TODO(Kelosky): recovery

  if (0 != rc || 0 != ssob.ssobretn) {
    strcpy(zjb->diag.service_name, "IEFSSREQ");
    zjb->diag.service_rc = ssob.ssobretn;
    zjb->diag.service_rsn = ssjm.ssjmretn;
    zjb->diag.service_rsn_secondary = ssjm.ssjmret2;
    // Understanding reason codes from this SSOB: https://www.ibm.com/docs/en/zos/3.1.0?topic=85-output-parameters
    zjb->diag.e_msg_len =
      sprintf(zjb->diag.e_msg, "IEFSSREQ rc was: '%d' SSOBRETN was: '%d', SSJMRETN was: '%d', SSJMRET2 was: '%d'", rc,
              ssob.ssobretn, ssjm.ssjmretn, ssjm.ssjmret2);
    return RTNCD_FAILURE;
  }

  if (0 == ssjm.ssjmnsjf) {
    zwto_debug("@TEST ssjmsjf8 %llx and flag is %02x", ssjm.ssjmnsjf8, ssjm.ssjmofg1);
    if (zjb->jobid[0] != 0x00) {
      zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "No jobs found matching jobid '%.8s'", zjb->jobid);
    } else {
      zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "No jobs found matching correlator '%.64s'", zjb->correlator);
    }
    zjb->diag.detail_rc = ZJB_RTNCD_JOB_NOT_FOUND;
    return RTNCD_FAILURE;
  }

  return RTNCD_SUCCESS;
}

// view job
#pragma prolog(ZJBMVIEW, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(ZJBMVIEW, " ZWEEPILG ")
int ZJBMVIEW(ZJB* zjb, ZJB_JOB_INFO** PTR64 job_info, int* entries) {
  STAT stat = {0};
  init_stat(&stat);

  if (zjb->jobid[0] != 0x00) {
    stat.statsel1 = statsjbi;
    memcpy(stat.statjbil, zjb->jobid, sizeof((stat.statojbi)));
    memcpy(stat.statjbih, zjb->jobid, sizeof((stat.statojbi)));
  } else {
    char correlator31[64] = {0};
    memcpy(correlator31, zjb->correlator, sizeof(zjb->correlator));
    stat.statsel5 = statscor;
    stat.statjcrp = &correlator31[0];
  }

  stat.stattype = statters;

  return ZJBMTCOM(zjb, &stat, job_info, entries);
}

// list jobs
#pragma prolog(ZJBMLIST, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(ZJBMLIST, " ZWEEPILG ")
int ZJBMLIST(ZJB* zjb, ZJB_JOB_INFO** PTR64 job_info, int* entries) {
  STAT stat = {0};
  init_stat(&stat);

  stat.statsel1 = statsown;
  stat.stattype = statters;
  memcpy(stat.statownr, zjb->owner_name, sizeof((stat.statownr)));

  if (0 == strcmp(zjb->prefix_name, "        ")) {
    // do nothing
  } else {
    stat.statsel1 |= statsjbn; // add job name filter
    memcpy(stat.statjobn, zjb->prefix_name, sizeof((stat.statjobn)));
  }

  return ZJBMTCOM(zjb, &stat, job_info, entries);
}

int ZJBMGJQ(ZJB* zjb, SSOB* ssobp, STAT* statp, STATJQ* PTR32* PTR32 statjqp) {
  int rc = 0;

  SSOB* PTR32 ssobp2 = NULL;

  ssobp2 = ssobp;
  ssobp2 = (SSOB * PTR32)((unsigned int)ssobp2 | 0x80000000);
  rc = iefssreq(&ssobp2); // TODO(Kelosky): recovery

  if (0 != rc || 0 != ssobp->ssobretn) {
    ZJBMEMSG(zjb, statp, ssobp, rc);
    statp->stattype = statmem; // free storage
    rc = iefssreq(&ssobp2);
    return RTNCD_FAILURE;
  }

  *statjqp = (STATJQ * PTR32) statp->statjobf;

  return RTNCD_SUCCESS;
}

int ZJBMEMSG(ZJB* zjb, STAT* PTR64 stat, SSOB* PTR64 ssobp, int rc) {
  strcpy(zjb->diag.service_name, "IEFSSREQ");
  zjb->diag.detail_rc = ZJB_RTNCD_SERVICE_FAILURE;
  zjb->diag.service_rc = ssobp->ssobretn;
  zjb->diag.service_rsn = stat->statreas;
  zjb->diag.service_rsn_secondary = stat->statrea2;
#define STATLERR 8
  if (STATLERR == ssobp->ssobretn && statrojb == stat->statreas) // skip if invalid job id
  {
    zjb->diag.e_msg_len =
      sprintf(zjb->diag.e_msg, "Job ID '%.8s' was not valid", stat->statojbi); // STATREAS contains the reason
  } else {
    zjb->diag.e_msg_len =
      sprintf(zjb->diag.e_msg, "IEFSSREQ rc was: '%d' SSOBRETN was: '%d', STATREAS was: '%d', STATREA2 was: '%d'", rc,
              ssobp->ssobretn, stat->statreas, stat->statrea2); // STATREAS contains the reason
  }
}

int ZJBMTCOM(ZJB* zjb, STAT* PTR64 stat, ZJB_JOB_INFO** PTR64 job_info, int* entries) {
  int rc = 0;

  SSOB* PTR32 ssobp = NULL;
  SSOB ssob = {0};
  SSIB ssib = {0};

  // https://www.ibm.com/docs/en/zos/3.1.0?topic=sfcd-extended-status-function-call-ssi-function-code-80
  init_ssob(&ssob, &ssib, stat, 80);
  if (0 != init_ssib(&ssib)) {
    strcpy(zjb->diag.service_name, "init_ssib");
    zjb->diag.detail_rc = ZJB_RTNCD_SERVICE_FAILURE;
    return RTNCD_FAILURE;
  }

  ssobp = &ssob;
  ssobp = (SSOB * PTR32)((unsigned int)ssobp | 0x80000000);
  rc = iefssreq(&ssobp); // TODO(Kelosky): recovery

  if (0 != rc || 0 != ssob.ssobretn) {
    ZJBMEMSG(zjb, stat, &ssob, rc);
    stat->stattype = statmem; // free storage
    rc = iefssreq(&ssobp);
    return RTNCD_FAILURE;
  }

  STATJQ* PTR32 statjqp = (STATJQ * PTR32) stat->statjobf;

  ZJB_JOB_INFO* statjqtrsp = storage_get64(zjb->buffer_size);
  *job_info = statjqtrsp;

  STATJQHD* PTR32 statjqhdp = NULL;
  STATJQTR* PTR32 statjqtrp = NULL;

  int total_size = 0;
  int loop_control = 0;

  while (statjqp) {
    if (loop_control >= zjb->jobs_max) {
      zjb->diag.detail_rc = ZJB_RSNCD_MAX_JOBS_REACHED;
      zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "Reached maximum returned jobs requested %d", zjb->jobs_max);
      stat->stattype = statmem; // free storage
      rc = iefssreq(&ssobp);
      return RTNCD_WARNING;
      break;
    }

    total_size += (int)sizeof(ZJB_JOB_INFO);

    if (total_size <= zjb->buffer_size) {
      *entries = *entries + 1;

      statjqhdp = (STATJQHD * PTR32)((unsigned char* PTR32)statjqp + statjqp->stjqohdr);
      statjqtrp = (STATJQTR * PTR32)((unsigned char* PTR32)statjqhdp + sizeof(STATJQHD));

      memcpy(statjqtrsp, statjqtrp, sizeof(STATJQTR));
      int rc = iaztlkup(&ssob, statjqtrsp, zjb);
      if (0 != rc) {
        strcpy(zjb->diag.service_name, "iaztlkup");
        // For information about the reason code, look for `tlkretcd` in "native/c/chdsect/iaztlkdf.h"
        // https://www.ibm.com/docs/en/zos/3.1.0?topic=80-text-lookup-service-iaztlkup
        zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "IAZTLKUP RC: '%d' reason: '%d'", statjqtrsp->statjqtr.sttrjid,
                                      rc, zjb->diag.detail_rc);
        zjb->diag.detail_rc = ZJB_RTNCD_SERVICE_FAILURE;
        storage_free64(statjqtrsp);
        return RTNCD_FAILURE;
      }

      statjqtrsp++;
    } else {
      zjb->diag.detail_rc = ZJB_RTNCD_INSUFFICIENT_BUFFER;
    }

    statjqp = (STATJQ * PTR32) statjqp->stjqnext;

    loop_control++;
  }

  zjb->buffer_size_needed = total_size;

  stat->stattype = statmem; // free storage
  rc = iefssreq(&ssobp);    // TODO(Kelosky): recovery

  return RTNCD_SUCCESS;
}

#define LOOP_MAX 100

// list data sets for a job
#pragma prolog(ZJBMLSDS, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(ZJBMLSDS, " ZWEEPILG ")
int ZJBMLSDS(ZJB* PTR64 zjb, STATSEVB** PTR64 sysoutInfo, int* entries) {
  int rc = 0;
  int loop_control = 0;

  // return rc
  SSOB* PTR32 ssobp = NULL;
  SSOB ssob = {0};
  SSIB ssib = {0};
  STAT stat = {0};

  STATJQ* PTR32 statjqp = NULL;
  STATJQHD* PTR32 statjqhdp = NULL;
  STATJQTR* PTR32 statjqtrp = NULL;

  STATVO* PTR32 statvop = NULL;
  STATSVHD* PTR32 statsvhdp = NULL;
  STATSEVB* PTR32 statsevbp = NULL;

  // https://www.ibm.com/docs/en/zos/3.1.0?topic=sfcd-extended-status-function-call-ssi-function-code-80
  if (0 != init_ssib(&ssib)) {
    strcpy(zjb->diag.service_name, "init_ssib");
    zjb->diag.detail_rc = ZJB_RTNCD_SERVICE_FAILURE;
    return RTNCD_FAILURE;
  }
  init_ssob(&ssob, &ssib, &stat, 80);
  init_stat(&stat);

  stat.stattype = statters;

  if (zjb->jobid[0] != 0x00) {
    stat.statsel1 = statsoji;
    memcpy(stat.statojbi, zjb->jobid, sizeof((stat.statojbi)));
  } else {
    char correlator31[64] = {0};
    memcpy(correlator31, zjb->correlator, sizeof(zjb->correlator));
    stat.statsel5 = statscor;
    stat.statjcrp = &correlator31[0];
  }

  // NOTE(Kelosky): we first locate the STATJQ via jobid or job correlator because verbose data which containts SYSOUT
  // info cannot be obtained directly from the jobid or job correlator as documented by the JES SSI API.
  rc = ZJBMGJQ(zjb, &ssob, &stat, &statjqp);

  if (0 != rc) {
    return rc;
  }
  stat.statsel1 = 0;
  stat.statsel5 = 0;

  ssobp = &ssob;
  ssobp = (SSOB * PTR32)((unsigned int)ssobp | 0x80000000);

  if (NULL == statjqp) {
    zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "No jobs found matching correlator '%.64s'", zjb->correlator);
    zjb->diag.detail_rc = ZJB_RTNCD_JOB_NOT_FOUND;
    stat.stattype = statmem; // free storage
    rc = iefssreq(&ssobp);   // TODO(Kelosky): recovery
    return RTNCD_FAILURE;
  }

  stat.stattrsa = statjqp;
  stat.stattype = statoutv;

  rc = iefssreq(&ssobp); // TODO(Kelosky): recovery

  if (0 != rc || 0 != ssob.ssobretn) {
    strcpy(zjb->diag.service_name, "IEFSSREQ");
    zjb->diag.service_rc = ssob.ssobretn;
    zjb->diag.service_rsn = stat.statreas;
    zjb->diag.service_rsn_secondary = stat.statrea2;
    zjb->diag.e_msg_len =
      sprintf(zjb->diag.e_msg, "IEFSSREQ rc was: '%d' SSOBRETN was: '%d', STATREAS was: '%d', STATREA2 was: '%d'", rc,
              ssob.ssobretn, stat.statreas, stat.statrea2); // STATREAS contains the reason
    return RTNCD_FAILURE;
  }

  statjqp = (STATJQ * PTR32) stat.statjobf;
  statvop = (STATVO * PTR32) statjqp->stjqsvrb;

  if (NULL == statjqp) {
    statjqp = stat.stattrsa;
  }

  if (NULL == statjqp) {
    if (zjb->jobid[0] != 0x00) {
      zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "No jobs found matching jobid '%.8s'", zjb->jobid);
    } else {
      zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "No jobs found matching correlator '%.64s'", zjb->correlator);
    }
    zjb->diag.detail_rc = ZJB_RTNCD_JOB_NOT_FOUND;
    stat.stattype = statmem; // free storage
    rc = iefssreq(&ssobp);   // TODO(Kelosky): recovery
    return RTNCD_FAILURE;
  }

  STATSEVB* statsetrsp = storage_get64(zjb->buffer_size);
  *sysoutInfo = statsetrsp;

  int total_size = 0;

  while (statjqp) {
    statjqhdp = (STATJQHD * PTR32)((unsigned char* PTR32)statjqp + statjqp->stjqohdr);
    statjqtrp = (STATJQTR * PTR32)((unsigned char* PTR32)statjqhdp + sizeof(STATJQHD));

    while (statvop) {

      if (loop_control >= zjb->dds_max) {
        stat.stattype = statmem; // free storage
        rc = iefssreq(&ssobp);   // TODO(Kelosky): recovery
        zjb->diag.detail_rc = ZJB_RSNCD_MAX_JOBS_REACHED;
        zjb->diag.e_msg_len = sprintf(zjb->diag.e_msg, "max DDs reached '%d', results truncated", zjb->dds_max);
        return RTNCD_WARNING;
      }

      total_size += (int)sizeof(STATSEVB);

      if (total_size <= zjb->buffer_size) {
        *entries = *entries + 1;

        statsvhdp = (STATSVHD * PTR32)((unsigned char* PTR32)statvop + statvop->stvoohdr);
        statsevbp = (STATSEVB * PTR32)((unsigned char* PTR32)statsvhdp + sizeof(STATSVHD));

        memcpy(statsetrsp, statsevbp, sizeof(STATSEVB));
        statsetrsp++;
      } else {
        zjb->diag.detail_rc = ZJB_RTNCD_INSUFFICIENT_BUFFER;
      }

      statvop = (STATVO * PTR32) statvop->stvojnxt;

      loop_control++;
    }

    statjqp = (STATJQ * PTR32) statjqp->stjqnext;
  }

  zjb->buffer_size_needed = total_size;

  stat.stattype = statmem; // free storage
  rc = iefssreq(&ssobp);   // TODO(Kelosky): recovery

  return RTNCD_SUCCESS;
}
