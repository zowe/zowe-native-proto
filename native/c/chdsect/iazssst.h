#pragma pack(1)

#ifndef __stat__
#define __stat__

struct stat
{
  unsigned short statlen;   /* I.Length of status extension         */
  unsigned char stateye[4]; /* I.Eye catcher                        */
  struct
  {
    unsigned char _statverl; /* I.SSOB version level    */
    unsigned char _statverm; /* I.SSOB version modifier */
  } statver;
  short int statvero;          /* O.Subsystem version/modifier         */
  char statreas;               /* O.Reason code associated with        */
  char statrea2;               /* Secondary reason code.  This field   */
  unsigned char stattype;      /* I.Type of call                       */
  unsigned int _filler1 : 24;  /* Reserved for future use and must     */
  void *__ptr32 statstrp;      /* Storage management anchor   @Z21L64B */
  void *__ptr32 stattrkp;      /* Diagnostic response area anchor      */
  unsigned char statsel1;      /* IS.Job selection flags               */
  unsigned char statsel2;      /* IS.More Job selection flags          */
  unsigned char statsel3;      /* IS.More job selection flags          */
  unsigned char statsel4;      /* IS.More job selection flags          */
  unsigned char statssl1;      /* IS.SYSOUT selection flag    @R05LOPI */
  unsigned char statssl2;      /* IS.More SYSOUT selection    @Z07LXST */
  unsigned char statjobn[8];   /* IS*.Jobname used for selection (if   */
  unsigned char statjbil[8];   /* IS*.Low jobid used for      @Z11LTJN */
  unsigned char statjbih[8];   /* IS.High jobid used for selection     */
  unsigned char statojbi[8];   /* IS.Original job id for selection     */
  unsigned char statownr[8];   /* IS*.Owning userid used for           */
  unsigned char statsecl[8];   /* IS*.SECLABEL used for selection      */
  unsigned char statdest[18];  /* IS*.Default print or punch           */
  unsigned char statorgn[8];   /* IS.Origin node name for selection    */
  unsigned char statxeqn[8];   /* IS.Execution node name for           */
  unsigned char statclsl[8];   /* IS.Job class used for selection.     */
  unsigned char statvol[4][6]; /* IS.List of SPOOL volume serial       */
  unsigned char statsys[8];    /* IS*.MVS system name where job is     */
  unsigned char statmemb[8];   /* IS*.JES member name where job is     */
  unsigned char statprio;      /* IS.Job Priority used for selection   */
  unsigned char statphaz;      /* IS.Job phase.  Additional phases     */
  unsigned char statsrvc[8];   /* IS.WLM service class for    @R04LSDS */
  unsigned char statsenv[16];  /* IS*.WLM Scheduling environ  @R04LSDS */
  unsigned char statopt1;      /* I.Option byte               @Z09LAPF */
  unsigned char statssl3;      /* IS.More SYSOUT selection    @Z09LSTP */
  void *__ptr32 statrtn;       /* I.Data processing routine   @R04LSDS */
  int statrprm;                /* I.Routine parameter area    @R04LSDS */
  void *__ptr32 stattrsa;      /* I.STATJQ, STATSE for which  @Z07LXST */
  unsigned char statssl4;      /* IS.More SYSOUT selection    @Z11LTJN */
  unsigned char stat1chr;      /* I.Wild card matching          @Z23BA */
  unsigned char statzomo;      /* I.Wild card matching 0 or     @Z23BA */
  unsigned char statsel5;      /* IS.More job selection flags @Z21LCOR */
  int statjqlm;                /* I.Limit on how many STATJQs @Z13LSSF */
  unsigned char statopt2;      /* I.More options              @Z22LDEP */
  unsigned char statsel6;      /* IS.More job selection flags @Z23LEC2 */
  unsigned char _filler2[2];   /* Reserved for future use     @Z23LEC2 */
  void *__ptr32 statjobf;      /* O.Address of first Job Queue         */
  int statnrjq;                /* O.Number of jobs found               */
  int statnrse;                /* O.Number of SYSOUT elements   @Z23BF */
  double statperf;             /* O.Performance index for last         */
  unsigned char statofg1;      /* O.Output flags              @Z13LSSF */
  unsigned char _filler3[3];   /* Reserved for future use and must be  */
  void *__ptr32 statohld;      /* O.IAZOHLD table for         @Z23D124 */
  void *__ptr32 statohix;      /* O.IAZOHLD index table for   @Z23D124 */
  int _filler4;                /* Reserved for future use and must be  */
  void *statjobf___64;         /* O.Address of first Job Queue         */
  void *__ptr32 statphtp;      /* O.Ptr to table for          @Z21LJMO */
  void *__ptr32 statjdtp;      /* O.Ptr to table for          @Z21LJMO */
  int _filler5[2];             /* Reserved for future use and must be  */
  void *statstrp___64;         /* Storage management anchor for        */
  void *__ptr32 statctkn;      /* IS. Address of client token @R05LOPI */
  unsigned char statscre[8];   /* IS*.SYSOUT owner (creator)  @R05LOPI */
  unsigned char statsdes[18];  /* IS*.SYSOUT destination      @R05LOPI */
  unsigned char statscla[8];   /* IS. SYSOUT class for        @R05LOPI */
  unsigned char statswtr[8];   /* IS*.SYSOUT writer name for  @R05LOPI */
  unsigned char statsfor[8];   /* IS*.SYSOUT forms name for   @Z07LXST */
  unsigned char statsprm[8];   /* IS*.SYSOUT PRMODE for       @Z07LXST */
  unsigned char _filler6[2];   /* Reserved for future use              */
  unsigned char statsubr[8];   /* IS*.Submitting userid used  @Z23D021 */
  int statclsn;                /* IS.Additional job class     @Z09LSTP */
  void *__ptr32 statclsp;      /* count and pointer to    @Z09LSTP     */
  int statjbnn;                /* IS*.Additional job name     @Z13LSSF */
  void *__ptr32 statjbnp;      /* count and pointer or    @Z09LSTP     */
  int statdstn;                /* IS*.Additional job dest     @Z09LSTP */
  void *__ptr32 statdstp;      /* count and pointer to    @Z09LSTP     */
  int statphzn;                /* IS.Additional job phase     @Z09LSTP */
  void *__ptr32 statphzp;      /* count and pointer to    @Z09LSTP     */
  int statscln;                /* IS.Additional SYSOUT class  @Z09LSTP */
  void *__ptr32 statsclp;      /* count and pointer to    @Z09LSTP     */
  int statsdsn;                /* IS*.Additional SYSOUT dest  @Z09LSTP */
  void *__ptr32 statsdsp;      /* count and pointer to    @Z09LSTP     */
  void *__ptr32 statjcrp;      /* IS*.Address of job          @Z21LCOR */
  int _filler7;                /* Reserved for future use     @Z21L64B */
  void *stattrsa___64;         /* I.STATJQ, STATSE for which  @Z21L64B */
  unsigned char statgrpn[8];   /* IS*.Job group name used for @Z22LDEP */
  int statgrnn;                /* IS*.Additional Job Group    @Z22LDEP */
  void *__ptr32 statgrnp;      /* name count and pointer  @Z22LDEP     */
  unsigned char statrgpn[8];   /* IS*.Resource group name     @Z31LJRL */
  int statrgnn;                /* IS*.Additional resource     @Z31LJRL */
  void *__ptr32 statrgnp;      /* group names count and   @Z31LJRL     */
  int _filler8[2];             /* Reserved for future use     @Z31LJRL */
  unsigned char statbefn[8];   /* IS*.SCHEDULE BEFORE= job    @Z23LEC2 */
  int statbenn;                /* IS*.Additional SCHEDULE     @Z23LEC2 */
  void *__ptr32 statbefp;      /* BEFORE= job name count  @Z23LEC2     */
  unsigned char stataftn[8];   /* IS*.SCHEDULE AFTER= job     @Z23LEC2 */
  int statafnn;                /* IS*.Additional SCHEDULE     @Z23LEC2 */
  void *__ptr32 stataftp;      /* AFTER= job name count   @Z23LEC2     */
  short int stathcfv;          /* IS.JES2 NET (DJC) hold      @Z23LEC2 */
  unsigned char _filler9[2];   /* Reserved for future use     @Z23LEC2 */
  int _filler10[12];           /* and must be zero          @Z23LEC2   */
};

#define statverl statver._statverl
#define statverm statver._statverm

/* Values for field "statverm" */
#define statv010 0x100 /* Initial version of macro             */
#define statv020 0x200 /* WLM support               @R04LSDS   */
#define statv030 0x300 /* Client print support      @R05LOPI   */
#define statv040 0x400 /* VERBOSE/SLOW support      @Z07LXST   */
#define statv050 0x500 /* Added fields for SDSF     @Z09LSTA   */
#define statv060 0x600 /* Data set list support     @Z10LSDS   */
#define statv070 0x700 /* Transaction selection     @Z11LTJN   */
#define statv071 0x701 /* Transaction sel active  @Z11LTJN     */
#define statv080 0x800 /* Job correlator support,   @Z21LCOR   */
#define statv090 0x900 /* Dependent job support     @Z22LDEP   */
#define statv0a0 0xA00 /* SCHEDULE BEFORE/AFTER/    @Z23LEC2   */
#define statv0b0 0xB00 /* Limited resource support  @Z31LJRL   */
#define statcvrl 11    /* Current version level     @Z31LJRL   */
#define statcvrm 0     /* Current version modifier  @Z22LDEP   */

/* Values for field "statreas" */
#define statrlmt 4   /* Processing ended due to     @Z13LSSF */
#define statrdst 4   /* One of STATDEST or STATDSTP @Z09LSTP */
#define statrjbl 8   /* STATJBIL is not valid                */
#define statrjbh 12  /* STATJBIH is not valid                */
#define statrjlm 16  /* STATJBIH is less than STATJBIL       */
#define statrcls 20  /* One of STATCLSL or STATCLSP @Z09LSTP */
#define statrvol 24  /* STATSVOL is set but STATVOL is null  */
#define statrphz 28  /* One of STATPHAZ or STATPHZP          */
#define statrque 32  /* Unable to access job queues          */
#define statreye 36  /* STATEYE is not set to C'STAT'        */
#define statrlen 40  /* STATLEN is too short                 */
#define statrjbn 44  /* One of STATJOBN or STATJBNP @Z09LSTP */
#define statrown 48  /* STATOWNR is not a valid userid       */
#define statrsys 52  /* STATSYS is not a valid system name   */
#define statrmem 56  /* STATMEMB is not a valid member name  */
#define statrcst 60  /* STATSEL2 specifies to select only    */
#define statrojb 64  /* STATOJBI is not valid                */
#define statrsec 68  /* STATSECL is not valid                */
#define statrorg 72  /* STATORGN is not valid                */
#define statrxeq 76  /* STATXEQN is not valid                */
#define statrpri 80  /* STATPRIO is not valid                */
#define statrsvc 84  /* STATSRVC is not valid       @R04LSDS */
#define statrsen 88  /* STATSENV is not valid       @R04LSDS */
#define statrsct 92  /* STATCTKN is not valid       @R05LOPI */
#define statrscr 96  /* STATSCRE is not valid       @R05LOPI */
#define statrssd 100 /* One of STATSDES or STATSDSP @Z09LSTP */
#define statrssc 104 /* One of STATSCLA or STATSCLP @Z09LSTP */
#define statrsxw 108 /* STATSWTR is not valid       @R05LOPI */
#define statrecj 112 /* STATSCTK & STATSJBI are     @R05LOPI */
#define statrvbm 116 /* STATVRBO or STATOUTV        @Z07LXST */
#define statrbea 120 /* STATTRSA does not point to  @Z07LXST */
#define statrsfr 124 /* STATSFOR is not valid       @Z07LXST */
#define statrspr 128 /* STATSPRM is not valid       @Z07LXST */
#define statrsup 132 /* Function or filter not      @Z23D021 */
#define statrsub 136 /* STATSUBR is not valid       @Z23D021 */
#define statrnex 140 /* STATTRSA points to a        @Z23D117 */
#define statrids 144 /* STATSSDS is set with either @Z09LSTP */
#define statrtrs 148 /* STATTERS or STATOUTT        @Z11LTJN */
#define statrwil 152 /* STAT1CHR = STATZOMO and not   @Z23BA */
#define statrjil 156 /* STATSJIL is set with either @Z13LSSF */
#define statrjip 160 /* At least one of the JOBIDs  @Z21LCOR */
#define statrjiz 164 /* STATSJIL is set and either  @Z13LSSF */
#define statrjcr 168 /* STATJCRP not valid          @Z21LCOR */
#define statrjco 172 /* STATSCOR is set with        @Z21LCOR */
#define statrjst 176 /* User specified an           @Z21L64B */
#define statrgrn 180 /* One of STATGRPN or STATGRNP @Z22LDEP */
#define statrbef 184 /* One of STATBEFN or STATBEFP @Z23LEC2 */
#define statraft 188 /* One of STATAFTN or STATAFTP @Z23LEC2 */
#define statrrgn 192 /* One of STATRGPN or STATRGNP @Z31LJRL */

/* Values for field "stattype" */
#define statters 1 /* Request type of Terse/Quick @R05LOPI */
#define statvrbo 2 /* Request type Verbose/Slow   @Z07LXST */
#define statmem 3  /* Request type of memory management    */
#define statoutt 4 /* Request type of Terse/quick @R05LOPI */
#define statoutv 5 /* Request type Verbose/Slow   @Z07LXST */
#define statdlst 6 /* Request data set list for   @Z10LSDS */

/* Values for field "statsel1" */
#define statscls 0x80 /* Use STATCLSL and STATCLSP @Z09LSTP   */
#define statsdst 0x40 /* Use STATDEST and STATDSTP @Z09LSTP   */
#define statsjbn 0x20 /* Use STATJOBN and STATJBNP @Z09LSTP   */
#define statsjbi 0x10 /* Use STATJBIL and STATJBIH as         */
#define statsoji 0x08 /* Use STATOJBI as a filter             */
#define statsown 0x04 /* Use STATOWNR as a filter             */
#define statssec 0x02 /* Use STATSECL as a filter             */
#define statssub 0x01 /* Use STATSUBR as a filter  @Z23D021   */

/* Values for field "statsel2" */
#define statsstc 0x80 /* Select Started Tasks (STCs)          */
#define statstsu 0x40 /* Select Time Sharing Users (TSUs)     */
#define statsjob 0x20 /* Select batch jobs (JOBs)             */
#define statsapc 0x10 /* Select APPC Initiator                */
#define statszdn 0x08 /* Select Zone Dependency    @Z22LDEP   */
#define statstyp 0xFF /* If none of these bits is on, then    */

/* Values for field "statsel3" */
#define statspri 0x80 /* Use STATPRIO as a filter             */
#define statsvol 0x40 /* Select Jobs based on the volume      */
#define statsphz 0x20 /* Use STATPHAZ and STATPHZP @Z09LSTP   */
#define statshld 0x10 /* Select jobs which are held           */
#define statsnhl 0x08 /* Select jobs which are not held       */
#define statssys 0x04 /* Select jobs which are     @R03P067   */
#define statsmem 0x02 /* Select jobs which are     @R03P067   */
#define statspos 0x01 /* Obsolete.  WLM Service      @Z23BA   */

/* Values for field "statsel4" */
#define statsorg 0x80 /* Use STATORGN as a filter             */
#define statsxeq 0x40 /* Use STATXEQN as a filter             */
#define statssrv 0x20 /* Use STATSRVC as a filter  @R04LSDS   */
#define statssen 0x10 /* Use STATSENV as a filter  @R04LSDS   */
#define statsclx 0x08 /* STATCLSL and STATCLSP     @Z09LSTP   */
#define statsojd 0x04 /* STATJOBN, STATJBNP,       @Z09LSTP   */
#define statsqps 0x02 /* Always return current job @Z09LSTP   */
#define statsjil 0x01 /* STATJBNP is a list of     @Z13LSSF   */

/* Values for field "statssl1" */
#define statsctk 0x80 /* Use STATCTKN as a filter. @R05LOPI   */
#define statssow 0x40 /* Use STATSCRE as a filter  @R05LOPI   */
#define statssds 0x20 /* Use STATSDES and STATSDSP @Z09LSTP   */
#define statsscl 0x10 /* Use STATSCLA and STATSCLP @Z09LSTP   */
#define statsswr 0x08 /* Use STATSWTR as a filter  @R05LOPI   */
#define statsshl 0x04 /* Select held SYSOUT        @R05LOPI   */
#define statssnh 0x02 /* Select non-held SYSOUT    @R05LOPI   */

/* Values for field "statssl2" */
#define statssfr 0x80 /* Use STATSFOR as a filter  @Z07LXST   */
#define statsspr 0x40 /* Use STATSPRM as a filter  @Z07LXST   */
#define statsssp 0x20 /* Select SPIN output        @Z07LXST   */
#define statssns 0x10 /* Sel non-SPIN output       @Z07LXST   */
#define statssip 0x08 /* Select IP routed SYSOUT   @Z09LSTA   */
#define statssni 0x04 /* Select non-IP routed SYSO @Z09LSTA   */
#define statssod 0x02 /* If on with STATSSOW, also @Z09LSTP   */
#define statssjd 0x01 /* If on with STATSJBN, also @Z09LSTP   */

/* Values for field "statphaz" */
#define stat___nosub 1    /* No subchain exists                   */
#define stat___fssci 2    /* Active in CI in an FSS               */
#define stat___pscbat 3   /* Awaiting postscan (batch)            */
#define stat___pscdsl 4   /* Awaiting postscan (demsel)           */
#define stat___fetch 5    /* Awaiting volume fetch                */
#define stat___volwt 6    /* Awaiting start setup                 */
#define stat___syssel 7   /* Awaiting/active in MDS               */
#define stat___alloc 8    /* Awaiting resource allocation         */
#define stat___voluav 9   /* Awaiting unavailable VOL(s)          */
#define stat___verify 10  /* Awaiting volume mounts               */
#define stat___sysver 11  /* Awaiting/active in MDS               */
#define stat___error 12   /* Error during MDS processing          */
#define stat___select 13  /* Awaiting selection on main           */
#define stat___onmain 14  /* Scheduled on main                    */
#define stat___brkdwn 17  /* Awaiting breakdown                   */
#define stat___restrt 18  /* Awaiting MDS restart proc.           */
#define stat___done 19    /* Main and MDS proc. complete          */
#define stat___outpt 20   /* Awaiting output service              */
#define stat___outque 21  /* Awaiting output service WTR          */
#define stat___oswait 22  /* Awaiting rsvd services               */
#define stat___cmplt 23   /* Output service complete              */
#define stat___demsel 24  /* Awaiting selection on main           */
#define stat___efwait 25  /* Ending function rq waiting           */
#define stat___efbad 26   /* Ending function rq not Processed     */
#define stat___maxndx 27  /* Maximum rq index value               */
#define stat___input 128  /* Active in input processing           */
#define stat___wtconv 129 /* Awaiting conversion                  */
#define stat___conv 130   /* Active in conversion                 */
#define stat___setup 131  /* Active in SETUP                      */
#define stat___spin 132   /* Active in spin                       */
#define stat___wtbkdn 133 /* Awaiting output                      */
#define stat___wtpurg 134 /* Awaiting purge                       */
#define stat___purg 135   /* Active in purge                      */
#define stat___recv 136   /* Active on NJE sysout receiver        */
#define stat___wtxmit 137 /* Awaiting NJE transmission            */
#define stat___xmit 138   /* Active on NJE Job transmitter        */
#define stat___exec 253   /* Job has not completed     @R04LSDS   */
#define stat___postex 254 /* Job has completed         @R04LSDS   */

/* Values for field "statopt1" */
#define stat1rac 0x80 /* Do seclabel dominance check @Z09LAPF */
#define stat1lcl 0x40 /* Destinations that resolve     @Z23AP */
#define stat1wsi 0x20 /* Return one STATSE for each  @Z13LSSF */
#define stat1lmt 0x10 /* Limit the number of STATJQ  @Z13LSSF */
#define stat1ndp 0x08 /* Suppress duplicate data     @Z21LSDS */
#define stat1b64 0x04 /* Returned areas may be       @Z21L64B */
#define stat1wms 0x02 /* Wait for latest MAS level   @Z21LVER */
#define stat1wmb 0x01 /* Wait for latest member      @Z21LVER */

/* Values for field "statssl3" */
#define statsslc 0x80 /* Select SYSOUT that is     @Z09LSTP   */
#define statssnt 0x40 /* Select SYSOUT that is not @Z09LSTP   */
#define statssnj 0x10 /* NJE output as WRITE       @Z09LSTP   */
#define statswrt 0x08 /* Select OUTDISP=WRITE      @Z09LSTP   */
#define statshol 0x04 /* Select OUTDISP=HOLD       @Z09LSTP   */
#define statskep 0x02 /* Select OUTDISP=KEEP       @Z09LSTP   */
#define statslve 0x01 /* Select OUTDISP=LEAVE      @Z09LSTP   */

/* Values for field "statssl4" */
#define statstpn 0x80 /* Match STATJOBN and        @Z11LTJN   */
#define statstpi 0x40 /* Match STATJBIL and        @Z11LTJN   */
#define statstpu 0x20 /* Match STATOWNR to         @Z11LTJN   */
#define statssj1 0x10 /* See explanation above       @Z23BA   */

/* Values for field "statsel5" */
#define statscor 0x80 /* Use STATJCRP as a pointer  @Z22LDEP  */
#define statsgrp 0x40 /* Use STATGRPN and STATGRNP  @Z22LDEP  */
#define statsrgp 0x20 /* Use STATRGPN as a filter   @Z31LJRL  */

/* Values for field "statopt2" */
#define stat2dep 0x80 /* Return a list of job       @Z22LDEP  */
#define stat2zdn 0x40 /* If a job representing a    @Z22LDEP  */

/* Values for field "statsel6" */
#define statsbef 0x80 /* Use STATBEFN and STATBEFP  @Z23LEC2  */
#define statsaft 0x40 /* Use STATAFTN and STATAFTP  @Z23LEC2  */
#define statsdly 0x20 /* Select jobs that are       @Z23LEC2  */
#define statshce 0x10 /* Select jobs where current  @Z23LEC2  */
#define statshcl 0x08 /* Select jobs where current  @Z23LEC2  */
#define statshcg 0x04 /* Select jobs where current  @Z23LEC2  */

/* Values for field "statofg1" */
#define stato1cp 0x80 /* Information was obtained    @Z13LSSF */
#define stato164 0x40 /* The request to use 64-bit   @Z21L64B */

/* Values for field "_filler8" */
#define statsiz3 0x1C8 /* Version 3 size of SSOB      @R05LOPI */
#define statsiz4 0x1C8 /* Version 4 size of SSOB      @Z07LXST */
#define statsiz5 0x1C8 /* Version 5 size of SSOB      @Z09LSTA */
#define statsiz6 0x1C8 /* Version 6 size of SSOB      @Z10LSDS */
#define statsiz7 0x1C8 /* Version 7 size of SSOB      @Z11LTJN */
#define statsiz8 0x1C8 /* Version 8 size of SSOB      @Z21LCOR */
#define statsiz9 0x1C8 /* Version 9 size of SSOB      @Z21L64B */

/* Values for field "_filler10" */
#define statsiza 0x21C /* Version 10 size of SSOB     @Z23LEC2 */
#define statsizb 0x21C /* Version 11 size of SSOB     @Z23LEC2 */
#define statsize 0x21C /* Length of enhanced status SSOB ext   */
#define ssstlen8 0x23C /* Total length of SSOB                 */

#endif

#ifndef __statstor__
#define __statstor__

struct statstor
{
  unsigned char statstid[8];  /* Full eyecatcher                      */
  unsigned short statsthl;    /* Length of header area                */
  unsigned short _filler1;    /* Reserved for future use              */
  unsigned char statstsp;     /* Subpool of area                      */
  unsigned int statsttl : 24; /* Total length of area (this           */
  void *__ptr32 statstnx;     /* Pointer to next area                 */
  void *__ptr32 statstcp;     /* Pointer to 1st available             */
  void *statstnx___64;        /* Pointer to next area        @Z21L64B */
  void *statstcp___64;        /* Pointer to 1st available    @Z21L64B */
  void *__ptr32 statstb2;     /* Start of data area          @Z21L64B */
};

/* Values for field "statstsp" */
#define statstpl 230 /* Recommended subpool to use */

#endif

#ifndef __stattrak__
#define __stattrak__

struct stattrak
{
  unsigned char stattkid[8]; /* Full eyecatcher                      */
  unsigned short stattkhl;   /* Length of header area                */
  unsigned char stattkvr[2]; /* Copy of STATVERO                     */
  void *__ptr32 stattknx;    /* Address of next area                 */
  unsigned char stattksn[4]; /* Subsystem name                       */
  char stattkrs;             /* Copy of STATREAS                     */
  char stattkr2;             /* Copy of STATREA2                     */
  unsigned char _filler1[2]; /* Reserved for future use              */
  unsigned char stattktk[8]; /* Token for use by subsystem           */
  double stattkpr;           /* Copy of STATPERF                     */
  int _filler2;              /* Reserved                      @Z24BK */
  int _filler3;              /* Reserved                      @Z24BK */
};

/* Values for field "stattkrs" */
#define stattksk 0xFF /* Reason code if member skipped */

#endif

#ifndef __statparm__
#define __statparm__

struct statparm
{
  short int statpsiz;        /* Size of parameter list      @R04LSDS */
  unsigned char statpver[2]; /* Copy of STATVERO            @R04LSDS */
  int statpprm;              /* Parm passed in STATRPRM     @R04LSDS */
  int statpwrk;              /* Work area                   @R04LSDS */
  void *__ptr32 statpelm;    /* Addr of Job Queue Element   @Z21L64B */
  void *statpelm___64;       /* Addr of Job Queue Element   @Z21L64B */
};

/* Values for field "statpelm___64" */
#define statplen 0x18 /* Length of area              @R04LSDS */

#endif

#ifndef __statjq__
#define __statjq__

struct statjq
{
  unsigned char stjqeye[4];  /* Eye catcher                          */
  unsigned short stjqohdr;   /* Offset to first section              */
  unsigned char _filler1[2]; /* Reserved for future use              */
  void *__ptr32 stjqnext;    /* Address of next Job Queue Element    */
  void *__ptr32 stjqse;      /* Address of first SYSOUT     @R05LOPI */
  unsigned char stjqoss[4];  /* Name of Subsystem which created      */
  void *__ptr32 stjqvrbo;    /* Address of JOB level        @Z07LXST */
  void *__ptr32 stjqsvrb;    /* Address of 1st SYSOUT       @Z07LXST */
  int _filler2;              /* Reserved for future use     @Z21L64B */
  void *stjqnext___64;       /* Address of next Job Queue Element    */
  void *stjqse___64;         /* Address of first SYSOUT     @Z21L64B */
  void *stjqvrbo___64;       /* Address of JOB level        @Z21L64B */
  void *stjqsvrb___64;       /* Address of 1st SYSOUT       @Z21L64B */
  void *stjqdep8;            /* Pointer to associated chain @Z22LDEP */
};

/* Values for field "stjqsvrb" */
#define stjqsiz1 0x1C /* Size of prologue            @Z21L64B */

/* Values for field "stjqsvrb___64" */
#define stjqsiz2 0x40 /* Size of prologue            @Z21L64B */

#endif

#ifndef __statjqhd__
#define __statjqhd__

struct statjqhd
{
  unsigned short sthdlen; /* Length of entire job header */
  unsigned char sthdtype; /* Type of this header         */
  unsigned char sthdmod;  /* Modifier                    */
};

/* Values for field "sthdmod" */
#define sthd1mod 0    /* First Header Section modifier */
#define sthdsize 0x04 /* Size of First Header Section  */

#endif

#ifndef __statjqtr__
#define __statjqtr__

struct statjqtr
{
  unsigned short sttrlen;     /* Length of terse section              */
  unsigned char sttrtype;     /* Type of this header                  */
  unsigned char sttrmod;      /* Modifier                             */
  unsigned char sttrname[8];  /* Job Name                             */
  unsigned char sttrjid[8];   /* Job Identifier                       */
  unsigned char sttrojid[8];  /* Original Job Identifier              */
  unsigned char sttrclas[8];  /* Job Class                            */
  unsigned char sttronod[8];  /* Origin Node (node of submittal)      */
  unsigned char sttrxnod[8];  /* Execution Node                       */
  unsigned char sttrprnd[8];  /* Default Print Node                   */
  unsigned char sttrprre[8];  /* Default Print Remote Name            */
  unsigned char sttrpund[8];  /* Default Punch Node                   */
  unsigned char sttrpure[8];  /* Default Punch Remote Name            */
  unsigned char sttrouid[8];  /* Owner userid                         */
  unsigned char sttrsecl[8];  /* SECLABEL for job                     */
  unsigned char sttrsys[8];   /* MVS system on which the job is       */
  unsigned char sttrmem[8];   /* JES2 member on which the job is      */
  unsigned char sttrdevn[18]; /* Name of device job is active on      */
  unsigned char sttrphaz;     /* Phase job is in (see STAT_ equates   */
  unsigned char sttrhold;     /* Job hold indicator                   */
  unsigned char sttrjtyp;     /* Job type                             */
  unsigned char sttrprio;     /* Job priority                         */
  unsigned char sttrarms;     /* Jobs ARM status                      */
  unsigned char sttrmisc;     /* Miscellaneous indicators    @Z02LLRJ */
  struct
  {
    unsigned char _sttrxind;    /* Job completion indicator    @R04LSDS */
    unsigned char _sttrmxcc[3]; /* Completion code (set for    @R04LSDS */
  } sttrmxrc;
  int sttrqpos;               /* Position of job on class    @Z09LSTA */
  int sttrjnum;               /* Binary job number           @Z09LSTP */
  unsigned char sttrspus[8];  /* Percent SPOOL utilization   @Z09LSTP */
  unsigned char sttrslog[8];  /* If this is a SYSLOG job     @Z10LSDS */
  unsigned char sttrjcor[64]; /* Job correlator              @Z21LCOR */
  int sttrspac;               /* Number of track groups of   @Z21LSSI */
};

#define sttrxind sttrmxrc._sttrxind
#define sttrmxcc sttrmxrc._sttrmxcc

/* Values for field "sttrmod" */
#define sttrtmod 0 /* Terse section modifier               */

/* Values for field "sttrhold" */
#define sttrjnhl 1 /* Job is not held                      */
#define sttrjhld 2 /* Job is held                          */
#define sttrjdup 3 /* Job held for duplicate job name      */

/* Values for field "sttrjtyp" */
#define sttrstc 1  /* Started Task (STC)                   */
#define sttrtsu 2  /* Time Sharing User (TSU)              */
#define sttrjob 3  /* Batch job (JOB)                      */
#define sttrappc 4 /* APPC Initiator                       */
#define sttrjobg 5 /* JOBGROUP                  @Z22LDEP   */

/* Values for field "sttrarms" */
#define sttrarmr 0x80 /* Job is ARM registered                */
#define sttrarmw 0x40 /* Job is awaiting ARM restart          */

/* Values for field "sttrmisc" */
#define sttrmspn 0x80 /* JESLOG is spinable        @Z02LLRJ   */
#define sttrpeom 0x40 /* JOB is being processed    @Z09LSTA   */
#define sttrjcld 0x20 /* JESJCLIN dataset avail    @Z10LSDS   */
#define sttrsysl 0x10 /* MVS SYSLOG job            @Z10LSDS   */
#define sttrnjed 0x08 /* NJE job flagged dubious     @Z23ED   */
#define sttrnjca 0x04 /* JES cancel command        @Z31D001   */
#define sttrrlim 0x02 /* Impacted by rsrc limit    @OA65053   */
#define sttrrlmt 0x01 /* Target of RAISE_LIMITS    @OA65053   */

/* Values for field "sttrxind" */
#define sttrxab 0x80  /* ABEND code exists                    */
#define sttrxcde 0x40 /* Completion code exists               */
#define sttrxreq 0x20 /* JOBRC completion code set   @Z13LJBR */
#define sttrxinm 0x0F /* Mask to extract completion    @Z23BH */
#define sttrxunk 0    /* No completion info        @R04LSDS   */
#define sttrxnrm 1    /* Job ended normally  +     @R04LSDS   */
#define sttrxcc 2     /* Job ended by CC     +     @R04LSDS   */
#define sttrxjcl 3    /* Job had a JCL error       @R04LSDS   */
#define sttrxcan 4    /* Job was canceled          @R04LSDS   */
#define sttrxabn 5    /* Job ABENDed         +     @R04LSDS   */
#define sttrxcab 6    /* Converter ABENDed         @R04LSDS   */
#define sttrxsec 7    /* Security error            @R04LSDS   */
#define sttrxeom 8    /* Job failed in EOM   +     @R04LSDS   */
#define sttrxcnv 9    /* Converter error           @R04LSDS   */
#define sttrxsys 10   /* System failure            @R04LSDS   */
#define sttrxflu 11   /* Job has been flushed      @Z22LDEP   */

/* Values for field "sttrspac" */
#define sttrsize 0xEC /* Size of Terse Information            */

#endif

#ifndef __statj2tr__
#define __statj2tr__

struct statj2tr
{
  unsigned short stj2len;    /* Length of JES2 terse section         */
  unsigned char stj2type;    /* Type of this header                  */
  unsigned char stj2mod;     /* Modifier                             */
  unsigned char stj2flg1;    /* General flag byte                    */
  unsigned char stj2brts;    /* Number of BERTs used by     @Z23LRES */
  short int stj2imbr;        /* Member id where job went    @Z22LDEP */
  unsigned char stj2jkey[4]; /* Job key                              */
  unsigned char stj2spol[8]; /* Spool data token                     */
  int stj2spac;              /* Number of track groups of spool      */
  short int stj2dpno;        /* Print default node (binary) @Z09LSTA */
  short int stj2dprm;        /* Print default rmt (binary)  @Z09LSTA */
  unsigned char stj2dpus[8]; /* Print default userid        @Z09LSTA */
  short int stj2inpn;        /* Origin node (binary)        @Z09LSTA */
  short int stj2xeqn;        /* Execution node (binary)     @Z09LSTA */
  int stj2jqei;              /* JQE index                   @Z09LSTA */
  unsigned char stj2ofsl;    /* SPOOL offload select mask   @Z09LSTA */
  unsigned char stj2busy;    /* JQE busy byte               @Z09LSTA */
  unsigned char _filler1[2]; /* Reserved for future use     @Z23LRES */
  unsigned char stj2joes[4]; /* Number of JOEs for this job @Z23LRES */
  unsigned char stj2jbrt[4]; /* Number of BERTs used for    @Z23LRES */
  unsigned char stj2crtm[4]; /* Create time of this job     @Z24D114 */
  struct
  {
    int _stj2rtst;              /* Input start time.  This   @Z24D114 */
    unsigned char _stj2rtsd[4]; /* Input start date.  This   @Z24D114 */
  } stj2rts;
  unsigned char stj2rgrp[8]; /* Resource group name if      @Z31LJRL */
};

#define stj2rtst stj2rts._stj2rtst
#define stj2rtsd stj2rts._stj2rtsd

/* Values for field "stj2mod" */
#define stj2tmod 0 /* JES2 Terse section modifier        */

/* Values for field "stj2flg1" */
#define stj21pro 0x80 /* Job is protected                   */
#define stj21ind 0x40 /* Job is set to independent mode     */
#define stj21sys 0x20 /* Job represents a system data set   */
#define stj21cnw 0x10 /* Job can be converted only   @R03AB */
#define stj21rbl 0x08 /* Job on the rebuild queue  @Z09LSTA */
#define stj21pri 0x04 /* Job is privileged         @Z23LRES */
#define stj21rgp 0x02 /* Job is associated with a  @Z31LJRL */

/* Values for field "stj2rgrp" */
#define stj2size 0x4C /* Length of section                  */

#endif

#ifndef __statdynd__
#define __statdynd__

struct statdynd
{
  unsigned short stdylen;    /* JES2 Dynamic Dependency     @Z23LEC2 */
  unsigned char stdytype;    /* Type of this header         @Z23LEC2 */
  unsigned char stdymod;     /* Modifier                    @Z23LEC2 */
  unsigned char stdybejb[8]; /* BEFORE= Job Name -          @Z23LEC2 */
  unsigned char stdybeji[8]; /* BEFORE= Job Identifier -    @Z23LEC2 */
  unsigned char stdybejk[4]; /* BEFORE= Job Key -           @Z23LEC2 */
  unsigned char stdyafjb[8]; /* AFTER= Job Name -           @Z23LEC2 */
  unsigned char stdyafji[8]; /* AFTER= Job Identifier -     @Z23LEC2 */
  unsigned char stdyafjk[4]; /* AFTER= Job Key -            @Z23LEC2 */
  unsigned char stdyflg1;    /* General flag byte           @Z23LEC2 */
  unsigned char _filler1[3]; /* Reserved for future use.    @Z23LEC2 */
  int _filler2[4];           /* Reserved for future use.    @Z23LEC2 */
};

/* Values for field "stdymod" */
#define stdytmod 0 /* JES2 Dynamic Dependency     @Z23LEC2 */

/* Values for field "stdyflg1" */
#define stdy1dly 0x80 /* ON = Job currently DELAYed  @Z23LEC2 */

/* Values for field "_filler2" */
#define stdysize 0x40 /* Length of section           @Z23LEC2 */

#endif

#ifndef __statneti__
#define __statneti__

struct statneti
{
  unsigned short stnelen;    /* Length of JES2 NET (DJC)    @Z23LEC2 */
  unsigned char stnetype;    /* Type of this header         @Z23LEC2 */
  unsigned char stnemod;     /* Modifier                    @Z23LEC2 */
  short int stneohld;        /* Original NHOLD=XXXXX value  @Z23LEC2 */
  unsigned char stnenrid[8]; /* NETREL= NETID NAME -        @Z23LEC2 */
  unsigned char stnenrjb[8]; /* NETREL= JOB NAME -          @Z23LEC2 */
  unsigned char stnenorm;    /* NORMAL  =(D,F, or R)        @Z23LEC2 */
  unsigned char stneabnr;    /* ABNORMAL=(D,F, or R)        @Z23LEC2 */
  unsigned char stneabcm;    /* ABCMP =(KEEP or NOKP)       @Z23LEC2 */
  unsigned char stnenrcm;    /* NRCMP =(HOLD,NOHO or FLSH)  @Z23LEC2 */
  unsigned char stnephld;    /* OPHOLD=(NO or YES)          @Z23LEC2 */
  unsigned char _filler1;    /* Reserved for future use.    @Z23LEC2 */
  int _filler2[4];           /* Reserved for future use.    @Z23LEC2 */
};

/* Values for field "stnemod" */
#define stnetmod 0 /* JES2 NET (DJC) information  @Z23LEC2 */

/* Values for field "stnenorm" */
#define stnendec 0x00 /* - NORMAL=D - Decrease this @Z23LEC2  */
#define stnenflu 0x01 /* - NORMAL=F - Flush this    @Z23LEC2  */
#define stnenret 0x02 /* - NORMAL=R - Retain this   @Z23LEC2  */

/* Values for field "stneabnr" */
#define stneadec 0x00 /* - ABNORMAL=D - Decrease    @Z23LEC2  */
#define stneaflu 0x01 /* - ABNORMAL=F - Flush this  @Z23LEC2  */
#define stnearet 0x02 /* - ABNORMAL=R - Retain this @Z23LEC2  */

/* Values for field "stneabcm" */
#define stnenokp 0x00 /* - ABCMP=NOKP - Purge the   @Z23LEC2  */
#define stnekeep 0x01 /* - ABCMP=KEEP - Indicates   @Z23LEC2  */

/* Values for field "stnenrcm" */
#define stnehold 0x00 /* - NRCMP=HOLD               @Z23LEC2  */
#define stnenoho 0x01 /* - NRCMP=NOHO               @Z23LEC2  */
#define stneflsh 0x02 /* - NRCMP=FLSH               @Z23LEC2  */

/* Values for field "stnephld" */
#define stneopno 0x00 /* - OPHOLD=NO                @Z23LEC2  */
#define stneopye 0x01 /* - OPHOLD=YES               @Z23LEC2  */

/* Values for field "_filler2" */
#define stnesize 0x2C /* Length of section                    */

#endif

#ifndef __stataffs__
#define __stataffs__

struct stataffs
{
  unsigned short staflen;    /* Length of affinity section */
  unsigned char staftype;    /* Type of this header        */
  unsigned char stafmod;     /* Modifier                   */
  unsigned short stafnum;    /* Number of member names     */
  unsigned char _filler1[2]; /* Reserved                   */
  unsigned char stafmemb[8]; /* First member name          */
};

/* Values for field "stafmod" */
#define staftmod 0 /* Affinity section modifier */

/* Values for field "stafmemb" */
#define stafsize 0x08 /* Length of basic section   */

#endif

#ifndef __statschd__
#define __statschd__

struct statschd
{
  unsigned short stsclen;     /* Length of scheduling data   @R04LSDS */
  unsigned char stsctype;     /* Type of this header         @R04LSDS */
  unsigned char stscmod;      /* Modifier                    @R04LSDS */
  unsigned char stscahld;     /* Reasons why job won't run   @Z21LENQ */
  unsigned char stscflg1;     /* Flag byte                   @R04LSDS */
  unsigned char stscasid[2];  /* ASID where job is executing @Z10LSDS */
  unsigned char stscsrvc[8];  /* WLM service class           @R04LSDS */
  int stscestt;               /* Estimated time to           @R04LWLM */
  unsigned char stscsenv[16]; /* WLM Scheduling environment  @R04LSDS */
  int stscqpos;               /* Position of this job on     @R04LSDS */
  int stscqnum;               /* Number of jobs on WLM       @R04LSDS */
  int stscqact;               /* Number of active jobs in    @R04LSDS */
  int stscavgq;               /* Average queue time for      @R04P651 */
  int stscqtim;               /* Queue time for this job     @R07P243 */
  int stscpseq;               /* Minimum z/OS level that job @Z21LENQ */
  unsigned char stscahl2;     /* Reasons why job won't run   @Z21LENQ */
  unsigned char stscahl3;     /* Reasons why job won't run   @Z22LDEP */
  unsigned char stscahl4;     /* Reasons why job won't run   @Z22LDEP */
  unsigned char _filler1;     /* Reserved                    @Z22LDEP */
  int stscrcls;               /* Reporting class index       @Z22D071 */
  struct
  {
    int _stscuntt;              /* HOLDUNTL time.  This       @Z22LDEP */
    unsigned char _stscuntd[4]; /* HOLDUNTL date.  This       @Z22LDEP */
  } stscuntl;
  struct
  {
    int _stscstbt;              /* STARTBY  time.  This       @Z22LDEP */
    unsigned char _stscstbd[4]; /* STARTBY  date.  This       @Z22LDEP */
  } stscstby;
  unsigned char stscwith[8]; /* WITH= job name -            @Z22LDEP */
};

#define stscuntt stscuntl._stscuntt
#define stscuntd stscuntl._stscuntd
#define stscstbt stscstby._stscstbt
#define stscstbd stscstby._stscstbd

/* Values for field "stscmod" */
#define stsctmod 0 /* Scheduling data modifier    @R04LSDS */

/* Values for field "stscahld" */
#define stscjcls 0x80 /* Job class held            @R04LSDS   */
#define stscjclm 0x40 /* Job class limit reached   @R04LSDS   */
#define stscjsch 0x20 /* Scheduling environment    @R04LSDS   */
#define stscjaff 0x10 /* System affinity           @R04LWLM   */
#define stscjspl 0x08 /* SPOOLs not available      @R04LWLM   */
#define stscjbsy 0x04 /* Job busy on device        @R04LWLM   */
#define stscjscf 0x02 /* SECLABEL affinity         @Z05LMLS   */
#define stscnosy 0x01 /* No system with the right  @R04LWLM   */

/* Values for field "stscflg1" */
#define stsc1jcm 0x80 /* Jobclass mode of JQE      @R04LSDS   */
#define stsc1unt 0x40 /* HOLDUNTL was specified    @Z22LDEP   */
#define stsc1sby 0x20 /* STARTBY  was specified    @Z22LDEP   */
#define stsc1unu 0x10 /* HOLDUNTL time is UTC      @Z22LDEP   */
#define stsc1sbu 0x08 /* STARTBY  time is UTC      @Z22LDEP   */

/* Values for field "stscahl2" */
#define stscmlev 0x80 /* z/OS minimum system level @Z21LENQ   */
#define stschunt 0x40 /* Job held for HOLDUNTL     @Z22LDEP   */
#define stscghld 0x20 /* Job group is held         @Z22LDEP   */
#define stscshld 0x10 /* Some jobs are held        @Z22LDEP   */
#define stscgaff 0x08 /* System affinity           @Z22LDEP   */
#define stscsaff 0x04 /* System affinity           @Z22LDEP   */
#define stscgsch 0x02 /* Scheduling environment    @Z22LDEP   */
#define stscssch 0x01 /* Scheduling environment    @Z22LDEP   */

/* Values for field "stscahl3" */
#define stscgslb 0x80 /* SECLABEL affinity         @Z22LDEP   */
#define stscsslb 0x40 /* SECLABEL affinity         @Z22LDEP   */
#define stscsmlv 0x20 /* z/OS minimum system level @Z22LDEP   */
#define stscslim 0x10 /* Job class limit reached   @Z22LDEP   */
#define stscsspl 0x08 /* SPOOLs not available      @Z22LDEP   */
#define stscsbus 0x04 /* Some jobs busy on device  @Z22LDEP   */
#define stscgnsy 0x02 /* No system with the right  @Z22LDEP   */
#define stscsnsy 0x01 /* No system with the right  @Z22LDEP   */

/* Values for field "stscwith" */
#define stscsize 0x5C /* Length of basic section     @R04LSDS */

#endif

#ifndef __statschs__
#define __statschs__

struct statschs
{
  unsigned short stsslen;    /* Length of sched sys section @R04LSDS */
  unsigned char stsstype;    /* Type of this header         @R04LSDS */
  unsigned char stssmod;     /* Modifier                    @R04LSDS */
  unsigned short stssnum;    /* Number of system names      @R04LSDS */
  unsigned char _filler1[2]; /* Reserved                    @R04LSDS */
  unsigned char stsssys[8];  /* First system name           @R04LSDS */
};

/* Values for field "stssmod" */
#define stsstmod 1 /* Schedulable system section  @R04LSDS */

/* Values for field "stsssys" */
#define stsssize 0x08 /* Length of sched systems sec @Z21LJMO */

#endif

#ifndef __statjzxc__
#define __statjzxc__

struct statjzxc
{
  unsigned short stjzlen;    /* Length of scheduling data   @Z22LDEP */
  unsigned char stjztype;    /* Type of this header         @Z22LDEP */
  unsigned char stjzmod;     /* Modifier                    @Z22LDEP */
  unsigned char stjzjzna[8]; /* Associated Job Zone         @Z22LDEP */
  unsigned char stjzjzid[8]; /* Associated Job Zone         @Z22LDEP */
  unsigned char stjzjzjs[8]; /* Associated JOBSET name -    @Z22LDEP */
  unsigned char stjzjzst;    /* Status of job within the    @Z22LDEP */
  unsigned char stjzjzfl;    /* Flush Type Indicator :      @Z22LDEP */
  unsigned char stjzjzel;    /* Job Eligibility Indicator:  @Z22LDEP */
  unsigned char stjzflg1;    /* Flag byte :                          */
  short int stjzchld;        /* Current (active) NHOLD=     @Z23LEC2 */
  unsigned char stjznrid[8]; /* NETREL= NETID NAME -        @Z23LEC2 */
  unsigned char stjznrjb[8]; /* NETREL= JOB NAME -          @Z23LEC2 */
  unsigned char _filler1[2]; /* Reserved                    @Z23LEC2 */
};

/* Values for field "stjzmod" */
#define stjztmod 2 /* Job zone data modifier      @Z22LDEP */

/* Values for field "stjzjzst" */
#define stjznost 0x00 /* - Job status = NOT SET    @Z22D108   */
#define stjzpend 0x01 /* - Job status = PENDING    @Z22D108   */
#define stjzacti 0x02 /* - Job status = ACTIVE     @Z22D108   */
#define stjzcomp 0x03 /* - Job status = COMPLETE   @Z22D108   */
#define stjzflsh 0x04 /* - Job status = FLUSHED    @Z22D108   */
#define stjziner 0x05 /* - Job Status = IN ERROR   @Z22D108   */

/* Values for field "stjzjzfl" */
#define stjznofl 0x00 /* - Flush Type = NOT SET    @Z22D108   */
#define stjzallf 0x01 /* - Flush Type = ALLFLUSH   @Z22D108   */
#define stjzanyf 0x02 /* - Flush Type = ANYFLUSH   @Z22D108   */

/* Values for field "stjzjzel" */
#define stjznsel 0x00 /* - Job is not eligible for @Z22LDEP   */
#define stjzesel 0x01 /* - Job is eligible for     @Z22LDEP   */

/* Values for field "stjzflg1" */
#define stjz1noi 0x80 /* Network Origin Indicator: @Z23LEC2   */

/* Values for field "_filler1" */
#define stjzsize 0x34 /* Length of basic section     @Z22LDEP */

#endif

#ifndef __statsclf__
#define __statsclf__

struct statsclf
{
  unsigned short stsllen;    /* Length of SECLABEL aff sect @Z05LMLS */
  unsigned char stsltype;    /* Type of this header         @Z05LMLS */
  unsigned char stslmod;     /* Modifier                    @Z05LMLS */
  unsigned short stslnum;    /* Number of system names      @Z05LMLS */
  unsigned char _filler1[2]; /* Reserved                    @Z05LMLS */
  unsigned char stslsys[8];  /* First system name           @Z05LMLS */
};

/* Values for field "stslmod" */
#define stsltmod 0 /* SECLABEL affinity section   @Z05LMLS */

/* Values for field "stslsys" */
#define stslsize 0x08 /* Length of SECLABEL aff sec  @Z21LJMO */

#endif

#ifndef __statjzdn__
#define __statjzdn__

struct statjzdn
{
  unsigned short stznlen;     /* Size of Job Zone Dependency @Z22LDEP */
  unsigned char stzntype;     /* Type of this header         @Z22LDEP */
  unsigned char stznmod;      /* Modifier                    @Z22LDEP */
  unsigned char stdterfl;     /* Section Flag Byte :         @Z23LEC2 */
  unsigned char _filler1[3];  /* Reserved for future use     @Z22LDEP */
  void *stznjob8;             /* Ptr to chain of STATJQs.    @Z22LDEP */
  void *stzndep8;             /* Ptr to chain of STATDBs.    @Z22LDEP */
  unsigned char stznname[8];  /* Dependency Network name.    @Z22LDEP */
  unsigned char stznstat;     /* Dependency Network status - @Z22LDEP */
  unsigned char stznerre[64]; /* ERROR= expression value.    @Z22LDEP */
  unsigned char stznoner;     /* ONERROR= value :            @Z22LDEP */
  unsigned char stznerrs;     /* Current error status        @Z22LDEP */
  unsigned char _filler2;     /* Reserved for future use     @Z22LDEP */
};

/* Values for field "stznmod" */
#define stzntmod 0 /* JES2 Job Zone Dependency    @Z22LDEP */

/* Values for field "stdterfl" */
#define stdteptr 0x80 /* STZNERRE was not large      @Z22LDEP */
#define stdtorig 0x40 /* Network origin indicator:   @Z23LEC2 */

/* Values for field "stznstat" */
#define stznpend 0x00 /* - Status = PENDING        @Z22LDEP   */
#define stznacti 0x01 /* - Status = ACTIVE,INIT    @Z22LDEP   */
#define stznact 0x02  /* - Status = ACTIVE         @Z22LDEP   */
#define stznsusi 0x03 /* - Status = SUSPENDING     @Z22LDEP   */
#define stznsusd 0x04 /* - Status = SUSPENDED      @Z22LDEP   */
#define stznheld 0x05 /* - Status = HELD           @Z22LDEP   */
#define stznflsh 0x06 /* - Status = FLUSHING       @Z22LDEP   */
#define stzncani 0x07 /* - Status = CANCELLING     @Z22LDEP   */
#define stzncomp 0x08 /* - Status = COMPLETE       @Z22LDEP   */

/* Values for field "stznoner" */
#define stznoest 0x01 /* - On Error = STOP         @Z22LDEP   */
#define stznoesu 0x02 /* - On Error = SUSPEND      @Z22LDEP   */
#define stznoefl 0x03 /* - On Error = FLUSH        @Z22LDEP   */

/* Values for field "stznerrs" */
#define stznerne 0x00 /* - Err stat = NOT_IN_ERROR @Z22LDEP   */
#define stznerst 0x01 /* - Err stat = STOPPED      @Z22LDEP   */
#define stznersu 0x02 /* - Err stat = SUSPENDED    @Z22LDEP   */
#define stznerfl 0x03 /* - Err stat = FLUSHED      @Z22LDEP   */

/* Values for field "_filler2" */
#define stznsize 0x64 /* Length of section           @Z22LDEP */

#endif

#ifndef __statj3tr__
#define __statj3tr__

struct statj3tr
{
  unsigned short stj3len;        /* Length of JES3 terse sect.    @Z23AA */
  unsigned char stj3type;        /* Type of this header           @Z23AA */
  unsigned char stj3mod;         /* Modifier                      @Z23AA */
  unsigned char stj3spol[8];     /* Spool data token or zero      @Z23AA */
  unsigned char stj3jstt[32];    /* List of reasons, by system, @Z07LXST */
  unsigned char stj3jstm[32][8]; /* List of system names        @Z07LXST */
};

/* Values for field "stj3mod" */
#define stj3tmod 0 /* JES3 Terse section modifier   @Z23AA */

/* Values for field "stj3jstm" */
#define stj3size 0x12C /* Length of section             @Z23AA */

#endif

#ifndef __statve__
#define __statve__

struct statve
{
  unsigned char stveeye[4];  /* Eye catcher                 @Z07LXST */
  unsigned short stveohdr;   /* Offset to first section     @Z07LXST */
  unsigned char _filler1[2]; /* Reserved for future use     @Z07LXST */
  void *__ptr32 stvejob;     /* Address of associated job   @Z07LXST */
  int _filler2;              /* Reserved for future use     @Z21L64B */
  void *stvejob___64;        /* Address of associated job   @Z21L64B */
};

/* Values for field "stvejob" */
#define stvesiz1 0x0C /* Size of prologue            @Z21L64B */

/* Values for field "stvejob___64" */
#define stvesiz2 0x18 /* Size of prologue            @Z21L64B */
#define stvesize 0x18 /* Current size of prologue    @Z21L64B */

#endif

#ifndef __statjvhd__
#define __statjvhd__

struct statjvhd
{
  unsigned short stjvlen; /* Len of entire Job verbose   @Z07LXST */
  unsigned char stjvtype; /* Type of this header         @Z07LXST */
  unsigned char stjvmod;  /* Modifier                    @Z07LXST */
};

/* Values for field "stjvmod" */
#define stjv1mod 0    /* 1st Header Section modifier @Z07LXST */
#define stjvsize 0x04 /* Size of 1st Header Section  @Z07LXST */

#endif

#ifndef __statjqvb__
#define __statjqvb__

struct statjqvb
{
  unsigned short stvblen;     /* Length of verbose section   @Z07LXST */
  unsigned char stvbtype;     /* Type of this header         @Z07LXST */
  unsigned char stvbmod;      /* Modifier                    @Z07LXST */
  unsigned char stvbflg1;     /* Section flag byte           @Z07LXST */
  unsigned char _filler1;     /* Reserved for future use     @Z07LXST */
  short int stvbjcpy;         /* Job Copy count              @Z07LXST */
  short int stvblnct;         /* Job line count (lines per            */
  unsigned char stvbidev[18]; /* Input device name           @Z07LXST */
  unsigned char stvbisid[8];  /* Input system/member         @Z07LXST */
  int stvbjcin;               /* Job input count             @Z07LXST */
  int stvbjlin;               /* Job line count              @Z07LXST */
  int stvbjpag;               /* Job page count              @Z07LXST */
  int stvbjpun;               /* Job card (output) count     @Z07LXST */
  struct
  {
    int _stvbrtst;              /* Input start time.  This   @Z07LXST */
    unsigned char _stvbrtsd[4]; /* Input start date.  This   @Z07LXST */
  } stvbrts;
  struct
  {
    int _stvbrtet;              /* Input end time.  This is  @Z07LXST */
    unsigned char _stvbrted[4]; /* Input end date.  This is  @Z07LXST */
  } stvbrte;
  unsigned char stvbsys[8]; /* Execution MVS system name   @Z07LXST */
  unsigned char stvbmbr[8]; /* Execution JES2 member name  @Z07LXST */
  struct
  {
    int _stvbxtst;              /* Execution start time.     @Z07LXST */
    unsigned char _stvbxtsd[4]; /* Execution start date.     @Z07LXST */
  } stvbxts;
  struct
  {
    int _stvbxtet;              /* Execution end time.  This @Z07LXST */
    unsigned char _stvbxted[4]; /* Execution end date.  This @Z07LXST */
  } stvbxte;
  unsigned char stvbjusr[8];  /* JMRUSEID field              @Z07LXST */
  unsigned char stvbmcls[8];  /* Message class (Job card)    @Z07LXST */
  unsigned char stvbnotn[8];  /* Notify Node                 @Z07LXST */
  unsigned char stvbnotu[8];  /* Notify Userid               @Z07LXST */
  unsigned char stvbpnam[20]; /* Programmer's name (from Job @Z07LXST */
  unsigned char stvbacct[8];  /* Account number (from Job    @Z07LXST */
  unsigned char stvbdept[8];  /* NJE department              @Z07LXST */
  unsigned char stvbbldg[8];  /* NJE building                @Z07LXST */
  unsigned char stvbroom[8];  /* Job card room number        @Z07LXST */
  unsigned char stvbjdvt[8];  /* JDVT name for job           @Z07LXST */
  unsigned char stvbsubu[8];  /* Submitting userid           @Z07LXST */
  unsigned char stvbsubg[8];  /* Submitting group            @Z09LSTA */
  unsigned char stvbmlrc[2];  /* Max LRECL of JCLIN stream   @Z10D034 */
  struct
  {
    unsigned char _stvbevsf; /* EVENTLOG data set flags   @Z22LEVT   */
    unsigned char _stvbfeas; /* Feature suppression flags   @Z22LEVT */
  } stvbsupf;
  struct
  {
    unsigned char _stvbxind;    /* Job completion indicator    @Z13LJBR */
    unsigned char _stvbmxcc[3]; /* Completion code (set for    @Z13LJBR */
  } stvbmxrc;
  unsigned char stvbnact[8]; /* Net account (from NETACCT   @Z24D145 */
};

#define stvbrtst stvbrts._stvbrtst
#define stvbrtsd stvbrts._stvbrtsd
#define stvbrtet stvbrte._stvbrtet
#define stvbrted stvbrte._stvbrted
#define stvbxtst stvbxts._stvbxtst
#define stvbxtsd stvbxts._stvbxtsd
#define stvbxtet stvbxte._stvbxtet
#define stvbxted stvbxte._stvbxted
#define stvbevsf stvbsupf._stvbevsf
#define stvbfeas stvbsupf._stvbfeas
#define stvbxind stvbmxrc._stvbxind
#define stvbmxcc stvbmxrc._stvbmxcc

/* Values for field "stvbmod" */
#define stvbvmod 0 /* Verbose section modifier    @Z07LXST */

/* Values for field "stvbflg1" */
#define stvb1err 0x80 /* Error obtaining verbose   @Z07LXST   */

/* Values for field "stvbevsf" */
#define stvbesmf 0x80 /* Suppress EVENTLOG SMF rec @Z22LEVT   */

/* Values for field "stvbfeas" */
#define stvbevtw 0x80 /* Suppress EVENTLOG writes  @Z22LEVT   */
#define stvbnnje 0x40 /* Suppress non-printable    @Z22LEVT   */

/* Values for field "stvbxind" */
#define stvbxab 0x80  /* ABEND code exists           @Z13LJBR */
#define stvbxcde 0x40 /* Completion code exists      @Z13LJBR */
#define stvbxreq 0x20 /* JOBRC completion code set   @Z13LJBR */
#define stvbxinm 0x0F /* Mask to extract completion    @Z23BH */
#define stvbxunk 0    /* No completion info        @Z13LJBR   */
#define stvbxnrm 1    /* Job ended normally  +     @Z13LJBR   */
#define stvbxcc 2     /* Job ended by CC     +     @Z13LJBR   */
#define stvbxjcl 3    /* Job had a JCL error       @Z13LJBR   */
#define stvbxcan 4    /* Job was canceled          @Z13LJBR   */
#define stvbxabn 5    /* Job ABENDed         +     @Z13LJBR   */
#define stvbxcab 6    /* Converter ABENDed         @Z13LJBR   */
#define stvbxsec 7    /* Security error            @Z13LJBR   */
#define stvbxeom 8    /* Job failed in EOM   +     @Z13LJBR   */
#define stvbxcnv 9    /* Converter error           @Z13LJBR   */
#define stvbxsys 10   /* System failure            @Z13LJBR   */
#define stvbxflu 11   /* Job has been flushed      @Z22LDEP   */

/* Values for field "stvbnact" */
#define stvbsize 0xE0 /* Size of verbose Information @Z07LXST */

#endif

#ifndef __statrlhd__
#define __statrlhd__

struct statrlhd
{
  unsigned short strllen; /* Length of resource limits   @Z31LJRL */
  struct
  {
    unsigned char _filler1; /* Type of this header (verbose) */
  } strltype;
  unsigned char strlmod;     /* Modifier                    @Z31LJRL */
  short int strlecnt;        /* Number of resource limits   @Z31LJRL */
  short int strleofs;        /* Offset to first resource    @Z31LJRL */
  short int strlelen;        /* Length of each resource     @Z31LJRL */
  unsigned char _filler2[2]; /* Reserved for future use     @Z31LJRL */
  int strlfent;              /* First resource limits entry @Z31LJRL */
};

/* Values for field "strlmod" */
#define strl1mod 0 /* Header section modifier     @Z31LJRL */

/* Values for field "strlfent" */
#define strlsize 0x0C /* Size of resource limits     @Z31LJRL */

#endif

#ifndef __statrent__
#define __statrent__

struct statrent
{
  unsigned char streresn[8]; /* Resource name               @Z31LJRL */
  unsigned char stretype[2]; /* Resource type               @Z31LJRL */
  unsigned char streact;     /* Limit enforcement type      @Z31LJRL */
  unsigned char streflg1;    /* Flag byte:                  @OA65053 */
  short int streplim;        /* Percent (% * 100) of total  @Z31LJRL */
  unsigned char _filler1[2]; /* Reserved for future use     @Z31LJRL */
  long int strecnt;          /* Number of resources used by @Z31LJRL */
  long int stremax;          /* Maximum number of resources @Z31LJRL */
  int strenext;              /* Next entry (internal use    @Z31LJRL */
};

/* Values for field "stretype" */
#define strettg 1  /* TG (SPOOL) usage          @Z31LJRL   */
#define stretjqe 2 /* JQE usage                 @Z31LJRL   */
#define stretjoe 3 /* JOE usage                 @Z31LJRL   */
#define stretbrt 4 /* BERT usage                @Z31LJRL   */

/* Values for field "streact" */
#define streatrk 1 /* Tracking only             @Z31LJRL   */
#define streawai 2 /* Wait when limit exceeded  @Z31LJRL   */
#define streafai 3 /* Fail when limit exceeded  @Z31LJRL   */

/* Values for field "streflg1" */
#define stre1dfa 0x80 /* Action is system default  @OA65053   */
#define stre1dfl 0x40 /* Limit is system default   @OA65053   */
#define stre1rli 0x20 /* Resource limit impact     @OA65053   */
#define stre1rlw 0x10 /* Resource limit wait       @OA65053   */

/* Values for field "strenext" */
#define stresize 0x20 /* Size of resource limits     @Z31LJRL */

#endif

#ifndef __statdb__
#define __statdb__

struct statdb
{
  unsigned char stdbeye[4];  /* Eye catcher                 @Z22LDEP */
  unsigned short stdbohdr;   /* Offset to first section     @Z22LDEP */
  unsigned char _filler1[2]; /* Reserved for future use     @Z22LDEP */
  void *stdbnxt8;            /* Pointer to next Job         @Z22LDEP */
};

#endif

#ifndef __statdbhd__
#define __statdbhd__

struct statdbhd
{
  unsigned short stdhlen; /* Length of entire Job        @Z22LDEP */
  unsigned char stdhtype; /* Type of this section        @Z22LDEP */
  unsigned char stdhmod;  /* Modifier                    @Z22LDEP */
};

/* Values for field "stdhmod" */
#define stdh1mod 0    /* Job Dependency Block -      @Z22LDEP */
#define stdhdsiz 0x04 /* Size of Job Dependency      @Z22LDEP */

#endif

#ifndef __statdbte__
#define __statdbte__

struct statdbte
{
  unsigned short stdtlen;     /* Length of Job Dependency    @Z22LDEP */
  unsigned char stdttype;     /* Type of this section        @Z22LDEP */
  unsigned char stdtmod;      /* Modifier                    @Z22LDEP */
  unsigned char stdtdtyp;     /* Dependency type -           @Z22LDEP */
  unsigned char stdtstat;     /* Job Dependency Status -     @Z22LDEP */
  unsigned char stdtcsta;     /* Dependency Complete Status: @Z22LDEP */
  unsigned char stdtwhfl;     /* WHEN= print text flag       @Z22LDEP */
  unsigned char stdtwhen[64]; /* WHEN= expression value.     @Z22LDEP */
  unsigned char stdtactn;     /* ACTION= value :             @Z22LDEP */
  unsigned char stdtothr;     /* OTHERWISE= value :          @Z22LDEP */
  unsigned char stdtpjbn[8];  /* Parent Job name/ID or       @Z22LDEP */
  unsigned char stdtpjid[8];  /* concurent job 1 name/ID.  @Z22LDEP   */
  unsigned char stdtdjbn[8];  /* Dependent Job name/ID or    @Z22LDEP */
  unsigned char stdtdjid[8];  /* concurent job 2 name/ID.  @Z22LDEP   */
};

/* Values for field "stdtmod" */
#define stdttmod 0 /* Job Dependency Block -      @Z22LDEP */

/* Values for field "stdtdtyp" */
#define stdtdep 0x01 /* - PARENT/DEPENDENT        @Z22LDEP   */
#define stdtcon 0x02 /* - CONCURRENT              @Z22LDEP   */

/* Values for field "stdtstat" */
#define stdtpend 0x00 /* - Status = PENDING        @Z22LDEP   */
#define stdtcomp 0x01 /* - Status = COMPLETE       @Z22LDEP   */
#define stdtudef 0x02 /* - Status = UNDEFINED (can @Z24D040   */

/* Values for field "stdtcsta" */
#define stdtcsat 0x00 /* - Compl Status = SATISFY  @Z22LDEP   */
#define stdtcflu 0x01 /* - Compl Status = FLUSH    @Z22LDEP   */
#define stdtcfai 0x02 /* - Compl Status = FAIL     @Z22LDEP   */
#define stdtcdef 0x03 /* - Action = DEFER (maps    @Z24D040   */

/* Values for field "stdtwhfl" */
#define stdtwptr 0x80 /* STDTWHEN was not large      @Z22LDEP */

/* Values for field "stdtactn" */
#define stdtasat 0x00 /* - Action = SATISFY        @Z22LDEP   */
#define stdtaflu 0x01 /* - Action = FLUSH          @Z22LDEP   */
#define stdtafai 0x02 /* - Action = FAIL           @Z22LDEP   */
#define stdtadef 0x03 /* - Action = DEFER (maps    @Z24D040   */

/* Values for field "stdtothr" */
#define stdtosat 0x00 /* - Otherwise = SATISFY     @Z22LDEP   */
#define stdtoflu 0x01 /* - Otherwise = FLUSH       @Z22LDEP   */
#define stdtofai 0x02 /* - Otherwise = FAIL        @Z22LDEP   */
#define stdtodef 0x03 /* - Otherwise = DEFER (maps @Z24D040   */

/* Values for field "stdtdjid" */
#define stdtsize 0x6A /* Size of Job Dependency      @Z22LDEP */

#endif

#ifndef __statjqse__
#define __statjqse__

struct statjqse
{
  unsigned short stselen;  /* Length of security section  @Z07LXST */
  unsigned char stsetype;  /* Type of this header         @Z07LXST */
  unsigned char stsemod;   /* Modifier                    @Z07LXST */
  unsigned char stseflg1;  /* Security section flags      @Z07LXST */
  unsigned char _filler1;  /* Reserved for future use     @Z07LXST */
  unsigned short stseoffs; /* Offset to SAF token     @Z07LXST     */
  unsigned char stsetokn;  /* Mapped SAF token            @Z07LXST */
};

/* Values for field "stsemod" */
#define stsesmod 0 /* Security section modifier   @Z07LXST */

/* Values for field "stseflg1" */
#define stse1err 0x80 /* Error obtaining verbose   @Z07LXST   */
#define stse1jb 0x40  /* Token represents a job    @Z07LXST   */

#endif

#ifndef __statjqac__
#define __statjqac__

struct statjqac
{
  unsigned short staclen; /* Len of accounting section   @Z07LXST */
  unsigned char stactype; /* Type of this header         @Z07LXST */
  unsigned char stacmod;  /* Modifier                    @Z07LXST */
  unsigned char stacflg1; /* Flags                       @Z07LXST */
  unsigned char _filler1; /* Reserved for future use     @Z07LXST */
  short int stacoffs;     /* Offset to beginning of      @Z07LXST */
};

/* Values for field "stacmod" */
#define stacamod 0 /* Accounting section modifier @Z07LXST */

/* Values for field "stacflg1" */
#define stac1err 0x80 /* Error obtaining verbose   @Z07LXST   */
#define stac1ov 0x40  /* Accounting string can be  @Z07LXST   */

/* Values for field "stacoffs" */
#define stacflen 0x08 /* Length of fixed portion     @Z07LXST */

#endif

#ifndef __stacntry__
#define __stacntry__

struct stacntry
{
  short int stacjlen;     /* Length of job accounting    @Z07LXST */
  char stacjnr;           /* Number of sub-strings       @Z07LXST */
  unsigned char stacjac1; /* First sub-string            @Z07LXST */
};

#endif

#ifndef __statjqem__
#define __statjqem__

struct statjqem
{
  unsigned short stemlen;      /* Length of the section       @Z23LNFY */
  unsigned char stemtype;      /* Section type                @Z23LNFY */
  unsigned char stemmod;       /* Section modifier            @Z23LNFY */
  unsigned short stemelen;     /* Length of email address   @Z23LNFY   */
  unsigned short stemoffs;     /* Offset to start of email  @Z23LNFY   */
  unsigned char _filler1[4];   /* Reserved                    @Z23LNFY */
  unsigned char stememid[256]; /* Email address               @Z23LNFY */
};

/* Values for field "stemmod" */
#define stemamod 1 /* Alternative identifier      @Z23LNFY */

/* Values for field "stememid" */
#define stemsize 0x0C /* Size of email section       @Z23LNFY */

#endif

#ifndef __statse__
#define __statse__

struct statse
{
  unsigned char stseeye[4];  /* Eye catcher                 @R05LOPI */
  unsigned short stseohdr;   /* Offset to first section     @R05LOPI */
  unsigned char _filler1[2]; /* Reserved for future use     @R05LOPI */
  void *__ptr32 stsejnxt;    /* Address of next SYSOUT data @R05LOPI */
  void *__ptr32 stsejob;     /* Address of associated job   @R05LOPI */
  void *__ptr32 stsevrbo;    /* Address of 1st SYSOUT       @Z07LXST */
  int _filler2;              /* Reserved for future use     @Z21L64B */
  void *stsejnxt___64;       /* Address of next SYSOUT data @Z21L64B */
  void *stsejob___64;        /* Address of associated job   @Z21L64B */
  void *stsevrbo___64;       /* Address of 1st SYSOUT       @Z21L64B */
};

/* Values for field "stsevrbo" */
#define stsesiz1 0x14 /* Size of prologue            @Z21L64B */

/* Values for field "stsevrbo___64" */
#define stsesiz2 0x30 /* Size of prologue            @Z21L64B */
#define stsesize 0x30 /* Current size of prologue    @Z21L64B */

#endif

#ifndef __statsehd__
#define __statsehd__

struct statsehd
{
  unsigned short stshlen; /* Length of entire SYSOUT     @R05LOPI */
  unsigned char stshtype; /* Type of this header         @R05LOPI */
  unsigned char stshmod;  /* Modifier                    @R05LOPI */
};

/* Values for field "stshmod" */
#define stsh1mod 0    /* 1st Header Section modifier @R05LOPI */
#define stshsize 0x04 /* Size of 1st Header Section  @R05LOPI */

#endif

#ifndef __statsetr__
#define __statsetr__

struct statsetr
{
  unsigned short ststlen;       /* Length of terse section     @R05LOPI */
  unsigned char ststtype;       /* Type of this header         @R05LOPI */
  unsigned char ststmod;        /* Modifier                    @R05LOPI */
  unsigned char ststouid[8];    /* SYSOUT owner (creator)      @R05LOPI */
  unsigned char ststsecl[8];    /* SECLABEL for SYSOUT         @R05LOPI */
  unsigned char ststdest[18];   /* SYSOUT destination          @R05LOPI */
  unsigned char ststclas[8];    /* SYSOUT class                @R05LOPI */
  int ststnrec;                 /* Record count                @R05LOPI */
  int ststpage;                 /* Page count                  @R05LOPI */
  int ststlnct;                 /* Line count                  @R05P404 */
  int ststbyct[2];              /* Byte count of consumed      @Z13LSSI */
  unsigned char ststform[8];    /* Forms                       @R05LOPI */
  unsigned char ststfcb[8];     /* FCB                         @R05LOPI */
  unsigned char ststucs[8];     /* UCS                         @R05LOPI */
  unsigned char ststxwtr[8];    /* External writer name        @R05LOPI */
  unsigned char ststpmde[8];    /* Processing mode             @R05LOPI */
  unsigned char ststflsh[8];    /* Flash                       @R05LOPI */
  unsigned char ststchar[4][4]; /* Printer translate table     @R05P404 */
  unsigned char ststmodf[4];    /* MODIFY=(modname)            @R05P404 */
  unsigned char ststmodc;       /* MODIFY=(,trc)               @R05P404 */
  unsigned char ststflg2;       /* General flag byte             @Z23AZ */
  unsigned char _filler1[2];    /* Reserved for future use       @Z23AZ */
  unsigned char ststsys[8];     /* MVS system on which the     @R05LOPI */
  unsigned char ststmem[8];     /* JES member on which the     @R05LOPI */
  unsigned char ststdevn[18];   /* Name of device on which the @R05LOPI */
  unsigned char ststhsta;       /* SYSOUT hold state           @R05LOPI */
  unsigned char ststhrsn;       /* Reason for system hold (see @R05LOPI */
  unsigned char ststdisp;       /* Output disposition          @R05P404 */
  unsigned char ststflg1;       /* General flag byte           @R05LOPI */
  unsigned char ststprio;       /* SYSOUT priority             @R05LOPI */
  unsigned char ststsoid[44];   /* EBCDIC SYSOUT identifier    @R05LOPI */
  unsigned char ststctkn[80];   /* SYSOUT client token         @R05LOPI */
  unsigned char ststlncu[4];    /* Current line active on      @Z22LSSF */
  unsigned char ststpgcu[4];    /* Current page active on      @Z22LSSF */
  unsigned char _filler2[3];    /* Reserved for future use     @Z31LJRL */
};

/* Values for field "ststmod" */
#define ststtmod 0 /* Terse section modifier      @R05LOPI */

/* Values for field "ststflg2" */
#define stst2civ 0x80 /* STSTCTKN is not usable      @Z23AZ   */
#define stst2dmn 0x40 /* Data sets represented by    @Z23BJ   */

/* Values for field "ststhsta" */
#define ststhopr 0x80 /* SYSOUT is held due to     @R05LOPI   */
#define ststhusr 0x40 /* SYSOUT is currently held  @R05LOPI   */
#define ststhsys 0x20 /* SYSOUT is in a system     @R05LOPI   */
#define ststhtso 0x10 /* SYSOUT is held for TSO,   @R05P404   */
#define ststhxwt 0x08 /* SYSOUT is held for        @R05P404   */
#define ststhbdt 0x04 /* SYSOUT is held on the     @Z10LSDS   */
#define ststhtcp 0x02 /* SYSOUT is held on the     @Z10LSDS   */

/* Values for field "ststdisp" */
#define ststdhld 0x80 /* OUTDISP=HOLD              @R05P404   */
#define ststdlve 0x40 /* OUTDISP=LEAVE             @R05P404   */
#define ststdwrt 0x20 /* OUTDISP=WRITE             @R05P404   */
#define ststdkep 0x10 /* OUTDISP=KEEP              @R05P404   */

/* Values for field "ststflg1" */
#define stst1brt 0x80 /* BURST=YES                 @R05LOPI   */
#define stst1dsi 0x40 /* 3540 held data set        @R05LOPI   */
#define stst1ipa 0x20 /* Destination has an IPADDR @R05LOPI   */
#define stst1cpd 0x10 /* Schedulable element has   @R05LOPI   */
#define stst1spn 0x08 /* SPIN data set             @R05P404   */
#define stst1nsl 0x04 /* Not selectable            @Z23D021   */
#define stst1apc 0x02 /* SYSOUT has job level      @Z09LSTP   */
#define stst1ctk 0x01 /* When SYSOUT was allocated @Z09LSTP   */

/* Values for field "_filler2" */
#define ststsize 0x138 /* Size of Terse Information   @R05LOPI */

#endif

#ifndef __statsj2t__
#define __statsj2t__

struct statsj2t
{
  unsigned short sts2len;     /* Len of JES2 terse section   @R05LOPI */
  unsigned char sts2type;     /* Type of this header         @R05LOPI */
  unsigned char sts2mod;      /* Modifier                    @R05LOPI */
  unsigned char sts2flg1;     /* General flags               @R05LOPI */
  unsigned char sts2ognm[26]; /* SYSOUT group name           @R05LOPI */
  unsigned char sts2crtm[4];  /* JOE creation time (STCK     @R05LOPI */
  unsigned char sts2spol[8];  /* Spool data token (IOT addr) @Z07LXST */
  unsigned char sts2gnam[8];  /* Group name                  @Z07LXST */
  unsigned char sts2jid1[2];  /* JOE ID 1                    @Z07LXST */
  unsigned char sts2rnod[2];  /* Dest node number (binary)   @Z09LSTA */
  unsigned char sts2rrmt[2];  /* Dest remote number (binary) @Z09LSTA */
  unsigned char sts2rusr[8];  /* Dest user field             @Z09LSTA */
  unsigned char sts2tswb[8];  /* JOE level SWB MTTR          @Z09LSTA */
  unsigned char sts2chkt[8];  /* JOE CHK MTTR if CHK         @Z09LSTA */
  unsigned char sts2joei[4];  /* Work JOE index              @Z09LSTA */
  unsigned char sts2ofsl;     /* SPOOL offload select mask   @Z09LSTA */
  unsigned char sts2busy;     /* JOE busy byte               @Z09LSTA */
  unsigned char sts2brts;     /* Number of BERTs used by     @Z23LRES */
};

/* Values for field "sts2mod" */
#define sts2tmod 0 /* JES2 Terse section modifier @R05LOPI */

/* Values for field "sts2flg1" */
#define sts21dsh 0x80 /* JOE has been cloned (all  @R05P404   */
#define sts21tso 0x40 /* JOE is available for TSO  @R05P404   */
#define sts21usr 0x20 /* JOE is on userid queue    @Z09LSTA   */

/* Values for field "sts2brts" */
#define sts2size 0x58 /* Length of section           @R05LOPI */

#endif

#ifndef __statsj3t__
#define __statsj3t__

struct statsj3t
{
  unsigned short sts3len;    /* Len of JES3 terse section   @R05LOPI */
  unsigned char sts3type;    /* Type of this header         @R05LOPI */
  unsigned char sts3mod;     /* Modifier                    @R05LOPI */
  unsigned char sts3flg1;    /* General flags               @R05P404 */
  unsigned char _filler1[3]; /* Reserved                    @Z13LSSF */
  unsigned char sts3wsi[4];  /* Work Selection Identifier   @Z13LSSF */
};

/* Values for field "sts3mod" */
#define sts3tmod 0 /* JES3 Terse section modifier @R05LOPI */

/* Values for field "sts3flg1" */
#define sts31xsy 0x80 /* Extended keywords used    @R05P404   */
#define sts31wsi 0x40 /* One STATSE returned for   @Z13LSSF   */
#define sts31fmt 0x20 /* FORMAT JECL statements      @Z13LSSF */

/* Values for field "sts3wsi" */
#define sts3size 0x0C /* Length of section           @R05LOPI */

#endif

#ifndef __statsatr__
#define __statsatr__

struct statsatr
{
  unsigned short stsalen;    /* Length of transaction sect  @Z11LTJN */
  unsigned char stsatype;    /* Type of this header         @Z11LTJN */
  unsigned char stsamod;     /* Modifier                    @Z11LTJN */
  unsigned char stsajobn[8]; /* Transaction (APPC) Program  @Z11LTJN */
  unsigned char stsajid[8];  /* Transaction (APPC) Program  @Z11LTJN */
};

/* Values for field "stsamod" */
#define stsatmod 0 /* Transaction sect modifier   @Z11LTJN */

/* Values for field "stsajid" */
#define stsasize 0x14 /* Length of section           @Z11LTJN */

#endif

#ifndef __statvo__
#define __statvo__

struct statvo
{
  unsigned char stvoeye[4];  /* Eye catcher                 @Z07LXST */
  unsigned short stvoohdr;   /* Offset to first section     @Z07LXST */
  unsigned char _filler1[2]; /* Reserved for future use     @Z07LXST */
  void *__ptr32 stvojob;     /* Address of associated job   @Z07LXST */
  void *__ptr32 stvojnxt;    /* Address of next verbose     @Z07LXST */
  void *__ptr32 stvosout;    /* Address of associated       @Z07LXST */
  void *__ptr32 stvosnxt;    /* Address of next verbose     @Z07LXST */
  void *stvojob___64;        /* Address of associated job   @Z21L64B */
  void *stvojnxt___64;       /* Address of next verbose     @Z21L64B */
  void *stvosout___64;       /* Address of associated       @Z21L64B */
  void *stvosnxt___64;       /* Address of next verbose     @Z21L64B */
};

/* Values for field "stvosnxt" */
#define stvosiz1 0x18 /* Size of prologue            @Z21L64B */

/* Values for field "stvosnxt___64" */
#define stvosiz2 0x38 /* Size of prologue            @Z21L64B */
#define stvosize 0x38 /* Current size of prologue    @Z21L64B */

#endif

#ifndef __statsvhd__
#define __statsvhd__

struct statsvhd
{
  unsigned short stsvlen; /* Length of entire SYSOUT     @Z07LXST */
  unsigned char stsvtype; /* Type of this header         @Z07LXST */
  unsigned char stsvmod;  /* Modifier                    @Z07LXST */
};

/* Values for field "stsvmod" */
#define stsv1mod 0    /* 1st Header Section modifier @Z07LXST */
#define stsvsize 0x04 /* Size of 1st Header Section  @Z07LXST */

#endif

#ifndef __statsevb__
#define __statsevb__

struct statsevb
{
  unsigned short stvslen;       /* Length of verbose section   @Z07LXST */
  unsigned char stvstype;       /* Type of this header         @Z07LXST */
  unsigned char stvsmod;        /* Modifier                    @Z07LXST */
  unsigned char stvsflg1;       /* Section flag byte           @Z07LXST */
  unsigned char stvsrecf;       /* Record format               @Z23D117 */
  unsigned char stvsprcd[8];    /* Procname for the step       @Z07LXST */
  unsigned char stvsstpd[8];    /* Stepname for the step       @Z07LXST */
  unsigned char stvsddnd[8];    /* DDNAME for the data set     @Z07LXST */
  unsigned char stvstjn[8];     /* TP (APPC) jobname(depricated)        */
  unsigned char stvstjid[8];    /* TP (APPC) jobid (depricated)         */
  unsigned char stvstod[4];     /* Date and time of data set   @Z07LXST */
  int stvssegm;                 /* Segment id (zero if data    @Z07LXST */
  int stvsdsky;                 /* Data set number (key)       @Z23D021 */
  short int stvsmlrl;           /* Maximum logical record      @Z07LXST */
  int stvslnct;                 /* Line count                  @Z07LXST */
  int stvspgct;                 /* Page count                  @Z07LXST */
  int stvsbyct[2];              /* Byte count after blank      @Z07LXST */
  int stvsrcct;                 /* Record count (JES3 only)    @Z07LXST */
  unsigned char stvsdsn[44];    /* SYSOUT data set name        @Z07LXST */
  char stvscopy;                /* Data set copy count         @Z23D121 */
  char stvsflsc;                /* Number of flash copies      @Z10LSDS */
  unsigned char stvsflg2;       /* Section flag byte             @Z23BA */
  char stvsstpn;                /* Step number for the step    @Z22LEVT */
  unsigned char stvscpyg[8];    /* Data set copy groups        @Z23LSDS */
  unsigned char _filler1[4];    /* Reserved for future use     @Z23LSDS */
  unsigned char stvsctkn[80];   /* SYSOUT data set token       @Z07LXST */
  unsigned char stvschar[4][4]; /* Printer translate table     @Z10LSDS */
  unsigned char stvsmodf[4];    /* MODIFY=(modname)            @Z10LSDS */
  unsigned char stvsmodc;       /* MODIFY=(,trc)               @Z10LSDS */
};

/* Values for field "stvsmod" */
#define stvsvmod 0 /* Verbose section modifier    @Z07LXST */

/* Values for field "stvsflg1" */
#define stvs1err 0x80 /* Error obtaining verbose   @Z07LXST   */
#define stvsdscl 0x40 /* Line count, page count,   @Z07LXST   */
#define stvs1spn 0x20 /* SPIN data set                        */
#define stvs1jsl 0x10 /* Spin-any/JESLOG spin D S  @Z13LSPN   */
#define stvs1sys 0x08 /* System data set           @Z10LSDS   */
#define stvs1sin 0x04 /* Instream data set (SYSIN) @Z10LSDS   */
#define stvs1dum 0x02 /* Dummy data set (SYSOUT    @Z10LSDS   */
#define stvs1enf 0x01 /* All ENF58 signals are     @Z12LENF   */

/* Values for field "stvsflg2" */
#define stvs2civ 0x80 /* STVSCTKN is not usable      @Z23BA   */
#define stvs2spn 0x40 /* Spinnable file            @Z23D123   */
#define stvs2opj 0x20 /* OPTCD=J specified         @Z24D145   */

/* Values for field "stvsmodc" */
#define stvssize 0xF1 /* Length of section           @Z07LXST */

#endif

#ifndef __statseo2__
#define __statseo2__

struct statseo2
{
  unsigned short sto2len;    /* Len of JES2 verbose section @Z07LXST */
  unsigned char sto2type;    /* Type of this header         @Z07LXST */
  unsigned char sto2mod;     /* Modifier                    @Z07LXST */
  unsigned char sto2flg1;    /* General flags               @Z07LXST */
  unsigned char _filler1[3]; /* Reserved                    @Z07LXST */
  unsigned char sto2spst[8]; /* Data set SPOOL data token   @Z07LXST */
  unsigned char sto2form[8]; /* Forms                         @Z23BJ */
  unsigned char sto2fcb[4];  /* FCB                           @Z23BJ */
  unsigned char sto2ucs[4];  /* UCS                           @Z23BJ */
  unsigned char sto2flsh[4]; /* Flash                         @Z23BJ */
  unsigned char sto2flg2;    /* General flag byte             @Z23BJ */
};

/* Values for field "sto2mod" */
#define sto2tmod 0 /* JES2 Verbose section mod    @Z07LXST */

/* Values for field "sto2flg1" */
#define sto21err 0x80 /* Error obtaining verbose   @Z07LXST   */
#define sto21ori 0x40 /* Demand select overrides     @Z23BJ   */

/* Values for field "sto2flg2" */
#define sto21brt 0x80 /* BURST=YES                   @Z23BJ   */
#define sto2size 0x25 /* Length of section           @Z07LXST */

#endif

#ifndef __statseo3__
#define __statseo3__

struct statseo3
{
  unsigned short sto3len;     /* Len of JES3 verbose section @Z10LSDS */
  unsigned char sto3type;     /* Type of this header         @Z10LSDS */
  unsigned char sto3mod;      /* Modifier                    @Z10LSDS */
  unsigned char sto3flg1;     /* General flags               @Z10LSDS */
  unsigned char _filler1[3];  /* Reserved                    @Z10LSDS */
  unsigned char sto3cmtk[80]; /* *MODIFY,U command token     @Z10LSDS */
};

/* Values for field "sto3mod" */
#define sto3tmod 0 /* JES3 Verbose section mod    @Z10LSDS */

/* Values for field "sto3flg1" */
#define sto31err 0x80 /* Error obtaining verbose   @Z10LSDS   */

/* Values for field "sto3cmtk" */
#define sto3size 0x58 /* Length of section           @Z10LSDS */

#endif

#ifndef __statseso__
#define __statseso__

struct statseso
{
  unsigned short stsolen;  /* Length of security section  @Z07LXST */
  unsigned char stsotype;  /* Type of this header         @Z07LXST */
  unsigned char stsomod;   /* Modifier                    @Z07LXST */
  unsigned char stsoflg1;  /* Security section flags      @Z07LXST */
  unsigned char _filler1;  /* Reserved for future use     @Z07LXST */
  unsigned short stsooffs; /* Offset to SAF token     @Z09LSTA     */
  unsigned char stsotokn;  /* Mapped SAF token            @Z07LXST */
};

/* Values for field "stsomod" */
#define stsosmod 0 /* Security section modifier   @Z07LXST */

/* Values for field "stsoflg1" */
#define stso1err 0x80 /* Error obtaining verbose   @Z07LXST   */

#endif

#ifndef __statsees__
#define __statsees__

struct statsees
{
  unsigned short steslen;    /* Length of security section  @Z24LENZ */
  unsigned char stestype;    /* Type of this header         @Z24LENZ */
  unsigned char stesmod;     /* Modifier                    @Z24LENZ */
  unsigned char stesflg1;    /* Security section flags      @Z24LENZ */
  unsigned char _filler1[9]; /* Reserved for future use     @Z24LENZ */
  unsigned short steslabo;   /* Data set key label off  @Z24LENZ     */
  short int steslabl;        /* Data set key label length   @Z24LENZ */
  unsigned char stesbyte[8]; /* Byte size of data set pre   @Z25LENZ */
  unsigned char stesbcmp[8]; /* Byte size of data set post  @Z25LENZ */
  unsigned char steslab1;    /* Data set key label          @Z25LENZ */
};

/* Values for field "stesmod" */
#define stessmod 1 /* Security section modifier   @Z24LENZ */

/* Values for field "stesflg1" */
#define stes1enc 0x80 /* Data set is encrypted     @Z24LENZ   */
#define stes1cmp 0x40 /* Data set is compressed    @Z24LENZ   */

#endif

#ifndef __statseot__
#define __statseot__

struct statseot
{
  unsigned short stotlen;    /* Length of transaction sect  @Z11LTJN */
  unsigned char stottype;    /* Type of this header         @Z09LSTA */
  unsigned char stotmod;     /* Modifier                    @Z09LSTA */
  unsigned char stotjobn[8]; /* Transaction (APPC) Program  @Z11LTJN */
  unsigned char stotjid[8];  /* Transaction (APPC) Program  @Z11LTJN */
  unsigned char stotstrt[4]; /* Trans entry start time      @Z11LTJN */
  unsigned char stotstrd[4]; /* Trans entry start date      @Z11LTJN */
  unsigned char stotexst[8]; /* Trans execution start time  @Z11LTJN */
  unsigned char stotacto[4]; /* Trans account number        @Z11LTJN */
};

/* Values for field "stotmod" */
#define stotsmod 0 /* Transaction sect modifier   @Z11LTJN */

/* Values for field "stotacto" */
#define stotsize 0x28 /* Length of section           @Z09LSTA */

#endif

#ifndef __ssob__
#define __ssob__

struct ssob
{
  unsigned char ssobid[4];   /* CONTROL BLOCK IDENTIFIER              */
  unsigned short ssoblen;    /* LENGTH OF SSOB HEADER                 */
  short int ssobfunc;        /* FUNCTION ID                           */
  void *__ptr32 ssobssib;    /* ADDRESS OF SSIB OR ZERO               */
  int ssobretn;              /* RETURN CODE FROM SUBSYSTEM            */
  int ssobindv;              /* FUNCTION DEPENDENT AREA POINTER       */
  void *__ptr32 ssobreta;    /* USED BY SSI TO SAVE RETURN ADDRESS    */
  unsigned char ssobflg1;    /* Flag Byte                        @01A */
  unsigned char ssobrsv1[3]; /* RESERVED                         @01C */
};

/* Values for field "ssobretn" */
#define ssrtok 0    /* SUCCESSFUL COMPLETION - REQUEST WENT  */
#define ssrtnsup 4  /* SUBSYSTEM DOES NOT SUPPORT THIS       */
#define ssrtntup 8  /* SUBSYSTEM EXISTS, BUT IS NOT UP       */
#define ssrtnoss 12 /* SUBSYSTEM DOES NOT EXIST              */
#define ssrtdist 16 /* FUNCTION NOT COMPLETED-DISASTROUS     */
#define ssrtlerr 20 /* LOGICAL ERROR (BAD SSOB FORMAT,       */
#define ssrtnssi 24 /* SSI not initialized              @L1A */

/* Values for field "ssobflg1" */
#define ssobrtry 0x80 /* Retry Requested                  @01A */

/* Values for field "ssobrsv1" */
#define ssobhsiz 0x1C /* SSOB HEADER LENGTH                    */

#endif

#pragma pack(reset)
