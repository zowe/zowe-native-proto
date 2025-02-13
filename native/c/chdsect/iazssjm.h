#pragma pack(packed)

#ifndef __ssjm__
#define __ssjm__

struct ssjm
{
  unsigned char ssjmeye[8]; /* I.Eye catcher                        */
  unsigned short ssjmlen;   /* I.Length of parameter list           */
  unsigned short ssjmvrm;   /* I.Parm list ver/mod                  */
  unsigned short ssjmsvrm;  /* O.Subsystem ver/mod                  */
  unsigned char ssjmopt1;   /* I.Processing options:                */
  unsigned char _filler1;   /* Reserved for future use              */
  int ssjmretn;             /* O.Reason code for a return           */
  int ssjmret2;             /* O.Secondary reason code.             */
  struct
  {
    unsigned char _ssjmtype;   /* I.Requested function:   */
    unsigned char _filler2[3]; /* Reserved for future use */
  } ssjmreqp;
  unsigned char ssjmpflg;      /* I.Job purge (SSJMPRG)       @Z21LJMO */
  unsigned char _filler3[3];   /* Reserved for future use     @Z21LJMO */
  unsigned char ssjmcflg;      /* I.Job cancel (SSJMCANC)              */
  unsigned char _filler4[3];   /* Reserved for future use              */
  unsigned char ssjmeflg;      /* I.Job restart (SSJMRST)              */
  unsigned char _filler5[3];   /* Reserved for future use              */
  unsigned char ssjmtsfl;      /* I.Job SPIN (SSJMSPIN)                */
  unsigned char _filler6[3];   /* Reserved for future use              */
  unsigned char ssjmtsdn[8];   /* I.If SSJMTSDD is ON,                 */
  unsigned char ssjmrnod[8];   /* I.Node name to route the             */
  unsigned char _filler7[4];   /* Reserved for future use              */
  unsigned char ssjmcop1;      /* I.Change Characteristics             */
  unsigned char ssjmcop2;      /* I.2nd characteristics       @Z31D001 */
  unsigned char _filler8[2];   /* Reserved for future use     @Z31D001 */
  unsigned char ssjmcaff;      /* I.Change Characteristics             */
  unsigned char _filler9[3];   /* Reserved for future use              */
  unsigned char ssjmjbcl[8];   /* I.Change job class of                */
  unsigned char ssjmsvcl[8];   /* I.Change service class of            */
  unsigned char ssjmscev[16];  /* I.Change WLM scheduling              */
  unsigned char ssjmolst[8];   /* I.List of offload device             */
  unsigned char ssjmjpri;      /* I.Change priority of the             */
  unsigned char _filler10[15]; /* Reserved for future use              */
  int ssjmcmbn;                /* I.Member name count and              */
  void *__ptr32 ssjmcmbp;      /* pointer to 4 byte                    */
  int _filler11[8];            /* Reserved for future use              */
  unsigned char ssjm1chr;      /* I.Wild card matching                 */
  unsigned char ssjmzomo;      /* I.Wild card matching 0 or            */
  unsigned char ssjmsel1;      /* IS.Job selection flags               */
  unsigned char ssjmsel2;      /* IS.More Job selection flags          */
  unsigned char ssjmsel3;      /* IS.More job selection flags          */
  unsigned char ssjmsel4;      /* IS.More job selection flags          */
  unsigned char ssjmsel5;      /* IS.More job selection flags          */
  unsigned char ssjmsel6;      /* IS.More job selection flags @Z23LEC2 */
  unsigned char _filler12[4];  /* Reserved for future use     @Z23LEC2 */
  unsigned char ssjmssl1;      /* IS.SYSOUT filtering flags            */
  unsigned char ssjmssl2;      /* IS.More SYSOUT filtering             */
  unsigned char ssjmssl3;      /* IS.More SYSOUT filtering             */
  unsigned char ssjmssl4;      /* IS.More SYSOUT filtering             */
  unsigned char _filler13[4];  /* Reserved for future use              */
  unsigned char ssjmjobn[8];   /* IS*.Jobname used for selection (if   */
  unsigned char ssjmjbil[8];   /* IS*.Low jobid used for               */
  unsigned char ssjmjbih[8];   /* IS.High jobid used for selection     */
  unsigned char ssjmojbi[8];   /* IS.Original job id for selection     */
  unsigned char ssjmownr[8];   /* IS*.Owning userid used for           */
  unsigned char ssjmsecl[8];   /* IS*.SECLABEL used for selection      */
  unsigned char ssjmorgn[8];   /* IS.Origin node name for selection    */
  unsigned char ssjmxeqn[8];   /* IS.Execution node name for           */
  unsigned char ssjmclsl[8];   /* IS.Job class used for selection.     */
  unsigned char ssjmsys[8];    /* IS*.MVS system name where job is     */
  unsigned char ssjmmemb[8];   /* IS*.JES member name where job is     */
  unsigned char ssjmsrvc[8];   /* IS.WLM service class for             */
  unsigned char ssjmsenv[16];  /* IS*.WLM Scheduling environ           */
  unsigned char ssjmdest[18];  /* IS*.Default print or punch           */
  unsigned char ssjmvol[4][6]; /* IS.List of SPOOL volume serial       */
  unsigned char ssjmprio;      /* IS.Job Priority used for selection   */
  unsigned char ssjmphaz;      /* IS.Job phase.  Additional            */
  unsigned char ssjmgrpn[8];   /* IS*.Job group name used for @Z22LDEP */
  unsigned char ssjmbefn[8];   /* IS*.SCHEDULE BEFORE= job    @Z23LEC2 */
  unsigned char ssjmaftn[8];   /* IS*.SCHEDULE AFTER= job     @Z23LEC2 */
  short int ssjmhcfv;          /* IS.JES2 NET (DJC) hold      @Z23LEC2 */
  unsigned char _filler14[6];  /* Reserved for future use     @Z23LEC2 */
  void *__ptr32 ssjmctkn;      /* IS. Address of client token          */
  unsigned char ssjmscre[8];   /* IS*.SYSOUT owner (creator)           */
  unsigned char ssjmscla[8];   /* IS. SYSOUT class for                 */
  unsigned char ssjmswtr[8];   /* IS*.SYSOUT writer name for           */
  unsigned char ssjmsfor[8];   /* IS*.SYSOUT forms name for            */
  unsigned char ssjmsprm[8];   /* IS*.SYSOUT PRMODE for                */
  unsigned char ssjmsdes[18];  /* IS*.SYSOUT destination               */
  unsigned char _filler15[2];  /* Reserved for future use              */
  int _filler16[2];            /* Reserved for future use     @OA64772 */
  unsigned char ssjmrgpn[8];   /* IS*.Resource group name     @OA64772 */
  int ssjmbenn;                /* IS.Additional SCHEDULE      @Z23LEC2 */
  void *__ptr32 ssjmbefp;      /* BEFORE= job name count  @Z23LEC2     */
  int ssjmafnn;                /* IS.Additional SCHEDULE      @Z23LEC2 */
  void *__ptr32 ssjmaftp;      /* AFTER= job name count   @Z23LEC2     */
  int ssjmclsn;                /* IS.Additional job class              */
  void *__ptr32 ssjmclsp;      /* count and pointer to                 */
  int ssjmjbnn;                /* IS*.Additional job name              */
  void *__ptr32 ssjmjbnp;      /* count and pointer or                 */
  int ssjmdstn;                /* IS*.Additional job dest              */
  void *__ptr32 ssjmdstp;      /* count and pointer to                 */
  int ssjmphzn;                /* IS.Additional job phase              */
  void *__ptr32 ssjmphzp;      /* count and pointer to                 */
  int ssjmscln;                /* IS.Additional SYSOUT class           */
  void *__ptr32 ssjmsclp;      /* count and pointer to                 */
  int ssjmsdsn;                /* IS*.Additional SYSOUT dest           */
  void *__ptr32 ssjmsdsp;      /* count and pointer to                 */
  void *__ptr32 ssjmjcrp;      /* IS*.Pointer to job                   */
  int ssjmgrnn;                /* IS*.Additional Job Group    @Z22LDEP */
  void *__ptr32 ssjmgrnp;      /* name count and pointer  @Z22LDEP     */
  int ssjmrgnn;                /* IS*.Additional resource     @OA64772 */
  void *__ptr32 ssjmrgnp;      /* group names count and   @OA64772     */
  int _filler17;               /* Reserved for future use     @OA64772 */
  void *ssjmsjf8;              /* O.Pointer to a list of               */
  int ssjmnsjf;                /* O.Number of job feedback             */
  unsigned char ssjmofg1;      /* O.Output flags                       */
  unsigned char _filler18[3];  /* Reserved for future use              */
  void *ssjmstrp;              /* O.Storage management anchor          */
  int _filler19[16];           /* Reserved for future use              */
};

#define ssjmtype ssjmreqp._ssjmtype

/* Values for field "ssjmvrm" */
#define ssjmvrmc 0x300 /* - latest   ver/mod         @Z23LEC2  */
#define ssjmvrm1 0x100 /* - original ver/mod         @Z22LEVT  */
#define ssjmvrm2 0x200 /* - JOBGROUP support         @Z22LDEP  */
#define ssjmvrm3 0x300 /* - SCHEDULE BEFORE/AFTER/   @Z23LEC2  */

/* Values for field "ssjmopt1" */
#define ssjmpd64 0x04 /* ON - Return output in                */
#define ssjmpsyn 0x02 /* SYNC request (ON) or                 */

/* Values for field "ssjmtype" */
#define ssjmhold 4   /* Hold    selected job(s)              */
#define ssjmrls 8    /* Release selected job(s)              */
#define ssjmprg 12   /* Purge   selected job(s)              */
#define ssjmcanc 16  /* Cancel  selected job(s)              */
#define ssjmstrt 20  /* Start   selected Job(s)              */
#define ssjmrst 24   /* Restart selected job(s)              */
#define ssjmspin 28  /* SPIN    selected job(s)              */
#define ssjmjchr 32  /* Change Characteristics               */
#define ssjmnode 36  /* Change execution node                */
#define ssjmrstg 128 /* Release Storage                      */

/* Values for field "ssjmpflg" */
#define ssjmpprt 0x10 /* Perform a protected       @Z21LJMO   */

/* Values for field "ssjmcflg" */
#define ssjmcdmp 0x40 /* Dump the cancelled job(s) @Z21LJMO   */
#define ssjmcprt 0x10 /* Perform a protected       @Z21LJMO   */
#define ssjmcfrc 0x08 /* Force cancel the job,     @Z21LJMO   */
#define ssjmcarm 0x04 /* Request ARM restart the   @Z21LJMO   */
#define ssjmcprg 0x01 /* Purge output of the       @Z21LJMO   */

/* Values for field "ssjmeflg" */
#define ssjmecan 0x80 /* Cancel and hold the                  */
#define ssjmeres 0x40 /* Restart the selected                 */
#define ssjmesth 0x20 /* Hold and re-queue the                */

/* Values for field "ssjmtsfl" */
#define ssjmtsdd 0x80 /* SPIN only the dataset                */

/* Values for field "ssjmcop1" */
#define ssjmcjbc 0x80 /* Change the job class of              */
#define ssjmcsvc 0x40 /* Change the service class             */
#define ssjmcsch 0x20 /* Change WLM scheduling                */
#define ssjmcpra 0x10 /* Change the priority of               */
#define ssjmcprr 0x08 /* Change the priority of               */
#define ssjmcofl 0x04 /* Mark that selected job(s)            */
#define ssjmcnof 0x02 /* Mark that selected job(s)            */

/* Values for field "ssjmcop2" */
#define ssjmcocr 0x80 /* Mark that selected job(s) @Z31D001   */
#define ssjmcncr 0x40 /* Mark that selected job(s) @Z31D001   */

/* Values for field "ssjmcaff" */
#define ssjmcany 0x80 /* Selected job(s) are                  */
#define ssjmcrpl 0x40 /* REPLACE the affinity                 */
#define ssjmcadd 0x20 /* ADD TO the current                   */
#define ssjmcdel 0x10 /* DELETE FROM the current              */

/* Values for field "_filler10" */
#define ssjmcmbs 4 /* Size of a member name in             */

/* Values for field "_filler11" */
#define ssjmrsiz 0x90 /* Size of request parameters  @Z21LJMO */

/* Values for field "ssjmsel1" */
#define ssjmscls 0x80 /* Use SSJMCLSL and SSJMCLSP            */
#define ssjmsdst 0x40 /* Use SSJMDEST and SSJMDSTP            */
#define ssjmsjbn 0x20 /* Use SSJMJOBN and SSJMJBNP            */
#define ssjmsjbi 0x10 /* Use SSJMJBIL and SSJMJBIH            */
#define ssjmsoji 0x08 /* Use SSJMOJBI as a filter             */
#define ssjmsown 0x04 /* Use SSJMOWNR as a filter             */
#define ssjmssec 0x02 /* Use SSJMSECL as a filter             */
#define ssjmssub 0x01 /* Invalid for job modify    @Z22LBLD   */

/* Values for field "ssjmsel2" */
#define ssjmsstc 0x80 /* Select Started Tasks (STCs)          */
#define ssjmstsu 0x40 /* Select Time Sharing Users (TSUs)     */
#define ssjmsjob 0x20 /* Select batch jobs (JOBs)             */
#define ssjmsapc 0x10 /* Select APPC Initiator                */
#define ssjmszdn 0x08 /* Select Zone Dependency    @Z22LDEP   */
#define ssjmstyp 0xFF /* If none of these bits are on,        */

/* Values for field "ssjmsel3" */
#define ssjmspri 0x80 /* Use SSJMPRIO as a filter             */
#define ssjmsvol 0x40 /* Select Jobs based on the volume      */
#define ssjmsphz 0x20 /* Use SSJMPHAZ and SSJMPHZP            */
#define ssjmshld 0x10 /* Select jobs which are                */
#define ssjmsnhl 0x08 /* Select jobs which are                */
#define ssjmssys 0x04 /* Select jobs which are                */
#define ssjmsmem 0x02 /* Select jobs which are                */

/* Values for field "ssjmsel4" */
#define ssjmsorg 0x80 /* Use SSJMORGN as a filter             */
#define ssjmsxeq 0x40 /* Use SSJMXEQN as a filter             */
#define ssjmssrv 0x20 /* Use SSJMSRVC as a filter             */
#define ssjmssen 0x10 /* Use SSJMSENV as a filter             */
#define ssjmsclx 0x08 /* SSJMCLSL and SSJMCLSP                */
#define ssjmsojd 0x04 /* SSJMJOBN, SSJMJBNP,                  */
#define ssjmsjil 0x01 /* SSJMJBNP is a list of                */

/* Values for field "ssjmsel5" */
#define ssjmscor 0x80 /* Use SSJMJCRP as a pointer            */
#define ssjmsgrp 0x40 /* Use SSJMGRPN and SSJMGRNP  @Z22LDEP  */
#define ssjmsrgp 0x20 /* Use SSJMRGPN and SSJMRGNP  @OA64772  */

/* Values for field "ssjmsel6" */
#define ssjmsbef 0x80 /* Use SSJMBEFN and SSJMBEFP  @Z23LEC2  */
#define ssjmsaft 0x40 /* Use SSJMAFTN and SSJMAFTP  @Z23LEC2  */
#define ssjmsdly 0x20 /* Select jobs that are       @Z23LEC2  */
#define ssjmshce 0x10 /* Select jobs where current  @Z23LEC2  */
#define ssjmshcl 0x08 /* Select jobs where current  @Z23LEC2  */
#define ssjmshcg 0x04 /* Select jobs where current  @Z23LEC2  */

/* Values for field "ssjmssl1" */
#define ssjmsctk 0x80 /* Use SSJMCTKN as a filter.            */
#define ssjmssow 0x40 /* Use SSJMSCRE as a filter             */
#define ssjmssds 0x20 /* Use SSJMSDES and SSJMSDSP            */
#define ssjmsscl 0x10 /* Use SSJMSCLA and SSJMSCLP            */
#define ssjmsswr 0x08 /* Use SSJMSWTR as a filter             */
#define ssjmsshl 0x04 /* Select held SYSOUT                   */
#define ssjmssnh 0x02 /* Select non-held SYSOUT               */

/* Values for field "ssjmssl2" */
#define ssjmssfr 0x80 /* Use SSJMSFOR as a filter             */
#define ssjmsspr 0x40 /* Use SSJMSPRM as a filter             */
#define ssjmsssp 0x20 /* Select SPIN output                   */
#define ssjmssns 0x10 /* Sel non-SPIN output                  */
#define ssjmssip 0x08 /* Select IP routed SYSOUT              */
#define ssjmssni 0x04 /* Select non-IP routed SYSO            */
#define ssjmssod 0x02 /* If on with SSJMSSOW, also            */
#define ssjmssjd 0x01 /* If on with SSJMSJBN, also            */

/* Values for field "ssjmssl3" */
#define ssjmsslc 0x80 /* Select SYSOUT that is                */
#define ssjmssnt 0x40 /* Select SYSOUT that is not            */
#define ssjmssnj 0x10 /* NJE output as WRITE                  */
#define ssjmswrt 0x08 /* Select OUTDISP=WRITE                 */
#define ssjmshol 0x04 /* Select OUTDISP=HOLD                  */
#define ssjmskep 0x02 /* Select OUTDISP=KEEP                  */
#define ssjmslve 0x01 /* Select OUTDISP=LEAVE                 */

/* Values for field "ssjmssl4" */
#define ssjmstpn 0x80 /* Match SSJMJOBN and                   */
#define ssjmstpi 0x40 /* Match SSJMJBIL and                   */
#define ssjmstpu 0x20 /* Match SSJMOWNR to                    */
#define ssjmssj1 0x10 /* See explanation above                */

/* Values for field "ssjmphaz" */
#define ssjm___nosub 1    /* No subchain exists                   */
#define ssjm___fssci 2    /* Active in CI in an FSS               */
#define ssjm___pscbat 3   /* Awaiting postscan (batch)            */
#define ssjm___pscdsl 4   /* Awaiting postscan (demsel)           */
#define ssjm___fetch 5    /* Awaiting volume fetch                */
#define ssjm___volwt 6    /* Awaiting start setup                 */
#define ssjm___syssel 7   /* Awaiting/active in MDS               */
#define ssjm___alloc 8    /* Awaiting resource allocation         */
#define ssjm___voluav 9   /* Awaiting unavailable VOL(s)          */
#define ssjm___verify 10  /* Awaiting volume mounts               */
#define ssjm___sysver 11  /* Awaiting/active in MDS               */
#define ssjm___error 12   /* Error during MDS processing          */
#define ssjm___select 13  /* Awaiting selection on main           */
#define ssjm___onmain 14  /* Scheduled on main                    */
#define ssjm___brkdwn 17  /* Awaiting breakdown                   */
#define ssjm___restrt 18  /* Awaiting MDS restart proc.           */
#define ssjm___done 19    /* Main and MDS proc. complete          */
#define ssjm___outpt 20   /* Awaiting output service              */
#define ssjm___outque 21  /* Awaiting output service WTR          */
#define ssjm___oswait 22  /* Awaiting rsvd services               */
#define ssjm___cmplt 23   /* Output service complete              */
#define ssjm___demsel 24  /* Awaiting selection on main           */
#define ssjm___efwait 25  /* Ending function rq waiting           */
#define ssjm___efbad 26   /* Ending function rq not Processed     */
#define ssjm___maxndx 27  /* Maximum rq index value               */
#define ssjm___input 128  /* Active in input processing           */
#define ssjm___wtconv 129 /* Awaiting conversion                  */
#define ssjm___conv 130   /* Active in conversion                 */
#define ssjm___setup 131  /* Active in SETUP                      */
#define ssjm___spin 132   /* Active in spin                       */
#define ssjm___wtbkdn 133 /* Awaiting output                      */
#define ssjm___wtpurg 134 /* Awaiting purge                       */
#define ssjm___purg 135   /* Active in purge                      */
#define ssjm___recv 136   /* Active on NJE sysout receiver        */
#define ssjm___wtxmit 137 /* Awaiting NJE transmission            */
#define ssjm___xmit 138   /* Active on NJE Job transmitter        */
#define ssjm___exec 253   /* Job has not completed                */
#define ssjm___postex 254 /* Job has completed                    */

/* Values for field "ssjmofg1" */
#define ssjmo1cp 0x80 /* Job selection indicator :            */

/* Values for field "_filler19" */
#define ssjmsze1 0x278 /* Version 1 length                     */
#define ssjmsze2 0x278 /* Version 2 length            @Z22LDEP */
#define ssjmsize 0x278 /* Current version length               */

#endif

#ifndef __ssjf__
#define __ssjf__

struct ssjf
{
  unsigned char ssjfeye[4];   /* Eye catcher             */
  unsigned char _filler1[4];  /* Reserved for future use */
  void *ssjfnxt8;             /* Address of next job     */
  unsigned char ssjfname[8];  /* Job Name                */
  unsigned char ssjfjid[8];   /* Job Identifier          */
  unsigned char ssjfojid[8];  /* Original Job Identifier */
  unsigned char ssjfouid[8];  /* Owner userid            */
  unsigned char ssjfmem[8];   /* Member name             */
  unsigned char ssjfsys[8];   /* System name             */
  unsigned char ssjfjcor[64]; /* Job correlator          */
  int ssjfstat;               /* Job processing status   */
  int ssjfintc;               /* If SSJFSTAT = SSJFINTP, */
  unsigned char _filler2[4];  /* Reserved for future use */
  union
  {
    unsigned char _ssjfemsg[80]; /* Status message. */
    struct
    {
      int _ssjftrak; /* MTTR of associated JCT.     @Z21LJMO */
      int _ssjfjkey; /* Job key.                    @Z21LJMO */
      unsigned char _filler3[72];
    } _ssjf_struct1;
  } _ssjf_union1;
};

#define ssjfemsg _ssjf_union1._ssjfemsg
#define ssjftrak _ssjf_union1._ssjf_struct1._ssjftrak
#define ssjfjkey _ssjf_union1._ssjf_struct1._ssjfjkey

/* Values for field "ssjfstat" */
#define ssjf___mok 0   /* SYNC request processed               */
#define ssjf___mcl 128 /* Class authorization         @Z21LJMO */
#define ssjf___mrd 132 /* Reroute destination         @Z21LJMO */
#define ssjfjlck 136   /* Request ignored, job locked @Z21LJMO */
#define ssjfjntf 140   /* Request ignored, job not    @Z21LJMO */
#define ssjfbjid 144   /* Bad jobID                   @Z21LJMO */
#define ssjffqlo 148   /* Failed QLOC                 @Z21LJMO */
#define ssjfbjcr 152   /* Job correlator mismatch     @Z21LJMO */
#define ssjfjncn 156   /* Job not cancellable or      @Z21LJMO */
#define ssjfnpur 160   /* Job not cancellable or      @Z21LJMO */
#define ssjfpcon 164   /* Job not cancellable due to  @Z21LJMO */
#define ssjfnxeq 168   /* Job not routable for exec   @Z21LJMO */
#define ssjfbadj 172   /* Job class invalid           @Z21LJMO */
#define ssjfnsvc 176   /* Job service class cannot be @Z21LJMO */
#define ssjfbads 180   /* Service class invalid       @Z21LJMO */
#define ssjfpoex 184   /* Request failed - job is     @Z21LJMO */
#define ssjfjobc 188   /* Job change request failed   @Z21LJMO */
#define ssjfisch 192   /* WLM scheduling env invalid  @Z21LJMO */
#define ssjfioff 196   /* Request failed - offload    @Z21LJMO */
#define ssjfbaff 200   /* Request failed - invalid    @Z21LJMO */
#define ssjfbmbr 204   /* Request failed - invalid    @Z21LJMO */
#define ssjf0mbr 208   /* Request failed - no member  @Z21LJMO */
#define ssjfnobr 212   /* Request failed - no BERTs   @Z21LJMO */
#define ssjfnoex 216   /* Request failed - job not    @Z21LJMO */
#define ssjfddin 220   /* Request failed - DDNAME     @Z21LJMO */
#define ssjfinte 224   /* Request failed - internal   @Z21LJMO */
#define ssjfipri 228   /* Request failed - invalid    @Z21LJMO */
#define ssjfnote 232   /* Job not eligible for start  @Z21LJMO */
#define ssjfsdrn 236   /* Job not eligible for start. @Z21LJMO */
#define ssjfdupj 240   /* Job not eligible for start. @Z21LJMO */
#define ssjfsena 244   /* Job not eligible for start. @Z21LJMO */
#define ssjfinde 248   /* Job not eligible for start. @Z21LJMO */
#define ssjfspol 252   /* Job not eligible for start. @Z21LJMO */
#define ssjfintp 256   /* Request failed - internal   @Z21LJMO */
#define ssjfex49 260   /* Job not eligible for start. @Z21LJMO */
#define ssjfsecl 264   /* Job not eligible for start. @Z21LJMO */
#define ssjfnaff 268   /* Job not eligible for start. @Z21LJMO */
#define ssjfarmr 272   /* Job not eligible for start. @Z21LJMO */
#define ssjfbusy 276   /* Job not eligible for start. @Z21LJMO */
#define ssjfnbat 280   /* Job not eligible for start. @Z21LJMO */
#define ssjfnexq 284   /* Job not eligible for start. @Z21LJMO */
#define ssjfnoj2 288   /* Job not eligible for start. @Z21LJMO */
#define ssjfnj2s 292   /* Job not eligible for start. @Z21LJMO */
#define ssjfminl 296   /* Job not eligible for start. @Z21LJMO */
#define ssjfnbbm 300   /* Job not executing on        @Z21LJMO */
#define ssjfnbtc 304   /* Request failed. Not a       @Z21LJMO */
#define ssjfnsjb 308   /* Request failed. No SJB, no  @Z21LJMO */
#define ssjfjgbo 312   /* Request failed. Invalid     @Z22LDEP */
#define ssjfnojg 316   /* Request failed. Invalid     @Z22LDEP */
#define ssjfncom 320   /* Request failed. JOBGROUP    @Z22LDEP */
#define ssjfnzod 324   /* Request failed. JOBGROUP    @Z22LDEP */
#define ssjfnocs 328   /* Request failed. Invalid     @Z23D179 */
#define ssjfnosj 332   /* Request failed. Invalid     @Z23D179 */

/* Values for field "ssjfjkey" */
#define ssjfsize 0xDC /* Current size of job                  */

#endif

#ifndef __ssjmstor__
#define __ssjmstor__

struct ssjmstor
{
  unsigned char ssjmstid[8]; /* Eye-catcher                          */
  unsigned short ssjmsthl;   /* Length of storage header             */
  unsigned char ssjmstsp;    /* Subpool of this block       @Z24D111 */
  unsigned char _filler1;    /* Do not use (was SSJMSTSP)   @Z24D111 */
  int ssjmsttl;              /* Total length of this block           */
  void *ssjmstnx;            /* Pointer to next block                */
  void *ssjmstcp;            /* Ptr to 1st available byte            */
  void *__ptr32 ssjmdata;    /* Start of data in the block           */
};

/* Values for field "ssjmstsp" */
#define ssjmstpl 230 /* Recommended subpool        */

/* Values for field "ssjmdata" */
#define ssjmstsz 0x20 /* Size of Storage management */

#endif

#pragma pack(reset)
