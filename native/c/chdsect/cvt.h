#pragma pack(1)

#ifndef __cvt__
#define __cvt__

struct cvt
{
  void *__ptr32 cvttcbp;  /* -  Address of PSATNEW.                   @PTC          */
  void *__ptr32 cvt0ef00; /* - ADDRESS OF ROUTINE TO SCHEDULE                       */
  void *__ptr32 cvtlink;  /* -  ADDRESS OF DCB FOR SYS1.LINKLIB DATA SET.           */
  void *__ptr32 cvtauscb; /* - ADDRESS OF ASSIGN/UNASSIGN SERVICE    @H1A           */
  void *__ptr32 cvtbuf;   /* -        ADDRESS OF THE BUFFER OF THE RESIDENT         */
  void *__ptr32 cvtxapg;  /* -  ADDRESS OF I/O SUPERVISOR APPENDAGE VECTOR          */
  void *__ptr32 cvt0vl00; /* - ADDRESS OF ENTRY POINT OF THE TASK                   */
  void *__ptr32 cvtpcnvt; /* - ADDRESS OF ENTRY POINT OF THE ROUTINE                */
  void *__ptr32 cvtprltv; /* - ADDRESS OF ENTRY POINT OF THE ROUTINE                */
  void *__ptr32 cvtllcb;  /* -  ADDRESS OF THE LLCB.                  @P1M          */
  void *__ptr32 cvtlltrm; /* - ADDRESS OF LLA'S MEMORY TERNIMATION   @P1M           */
  void *__ptr32 cvtxtler; /* - ADDRESS OF ERROR RECOVERY PROCEDURE (ERP)            */
  void *__ptr32 cvtsysad; /* -        UCB ADDRESS FOR THE SYSTEM RESIDENCE          */
  void *__ptr32 cvtbterm; /* - ADDRESS OF ENTRY POINT OF THE ABTERM                 */
  int cvtdate;            /* -        CURRENT DATE IN PACKED DECIMAL                */
  void *__ptr32 cvtmslt;  /* -  ADDRESS OF THE MASTER COMMON AREA IN                */
  void *__ptr32 cvtzdtab; /* - ADDRESS OF I/O DEVICE                                */
  void *__ptr32 cvtxitp;  /* -  ADDRESS OF ERROR INTERPRETER ROUTINE                */
  void *__ptr32 cvt0ef01; /* ENTRY POINT IN STAGE II EXIT EFFECTOR @02C             */
  struct
  {
    short int _cvtvss;  /* VECTOR SECTION SIZE                   @D5A */
    short int _cvtvpsm; /* VECTOR PARTIAL SUM NUMBER             @D5A */
  } cvtvprm;
  short int cvtexit;      /* -           An SVC 3 instruction. Exit to         @PBA */
  short int cvtbret;      /* -           A BR 14 INSTRUCTION.  RETURN TO       @PBA */
  void *__ptr32 cvtsvdcb; /* - ADDRESS OF THE DCB FOR THE SYS1.SVCLIB               */
  void *__ptr32 cvttpc;   /* -   ADDRESS OF THE TIMER SUPERVISOR WORK AREA          */
  struct
  {
    unsigned char _cvtflgc0; /* -   Flags                                 @0IA */
    unsigned char _cvtflgc1; /* -   Flags                                 @0IA */
    short int _cvticpid;     /* -   IPL'ED CPU PHYSICAL ID            @ZA51978 */
  } cvtflgcs;
  unsigned char cvtcvt[4];   /* -   CVT ACRONYM IN EBCDIC (EYE-CATCHER)   @PCA         */
  void *__ptr32 cvtcucb;     /* -  ADDRESS OF THE UNIT CONTOL MODULE (UCM)             */
  void *__ptr32 cvtqte00;    /* - ADDRESS OF THE TIMER ENQUEUE ROUTINE FOR             */
  void *__ptr32 cvtqtd00;    /* - ADDRESS OF THE TIMER DEQUEUE ROUTINE FOR             */
  void *__ptr32 cvtstb;      /* -   ADDRESS OF THE I/O DEVICE             @LDC         */
  unsigned char cvtdcb;      /* -     OPERATING SYSTEM FOR                  @LGC       */
  unsigned int cvtdcba : 24; /* - ADDRESS OF THE DCB FOR THE SYS1.LOGREC               */
  int cvtsv76m;              /* -        SVC 76 MESSAGE COUNT FIELD (OS/VS2)           */
  void *__ptr32 cvtixavl;    /* - ADDRESS OF THE I/O SUPERVISOR'S FREELIST             */
  void *__ptr32 cvtnucb;     /* -        RESERVED                (MDCXXX)  @YA19285    */
  void *__ptr32 cvtfbosv;    /* - ADDRESS OF PROGRAM FETCH ROUTINE  @G860P46           */
  void *__ptr32 cvt0ds;      /* -   ADDRESS OF ENTRY POINT OF THE DISPATCHER           */
  void *__ptr32 cvtecvt;     /* -  POINTER TO THE EXTENDED CVT           @LJC          */
  void *__ptr32 cvtdairx;    /* -        ADDRESS OF THE 31- BIT ENTRY          @L4A    */
  void *__ptr32 cvtmser;     /* -  ADDRESS OF DATA AREA OF MASTER SCHEDULER            */
  void *__ptr32 cvt0pt01;    /* - ADDRESS OF BRANCH ENTRY POINT OF POST                */
  void *__ptr32 cvttvt;      /* -        ADDRESS OF TSO VECTOR TABLE       @ZA65725    */
  int cvt040id;              /* -        IFB040I WTO MESSAGE ID.               @09C    */
  unsigned char cvtmz00[4];  /* - HIGHEST ADDRESS IN VIRTUAL STORAGE FOR               */
  void *__ptr32 cvt1ef00;    /* -        ADDRESS OF ROUTINE WHICH CREATES IRB'S        */
  void *__ptr32 cvtqocr;     /* -        GRAPHICS INTERFACE TASK (GFX) FIELD.          */
  void *__ptr32 cvtqmwr;     /* -  ADDRESS OF THE ALLOCATION COMMUNICATION             */
  short int cvtsnctr;        /* -        SERIAL NUMBER COUNTER FOR ASSIGNING SERIAL    */
  unsigned char cvtopta;     /* -     OPTION INDICATORS                     @LBC       */
  unsigned char cvtoptb;     /* -     MISCELLANEOUS FLAGS                   @LBC       */
  void *__ptr32 cvtqcdsr;    /* - CDE SEARCH ROUTINE ADDRESS  (OS/VS2)                 */
  void *__ptr32 cvtqlpaq;    /* - ADDRESS OF POINTER TO MOST RECENT ENTRY              */
  void *__ptr32 cvtenfct;    /* EVENT NOTIFICATION CONTROL TABLE                       */
  void *__ptr32 cvtsmca;     /* -        ADDRESS OF THE SYSTEM MANAGEMENT CONTROL      */
  void *__ptr32 cvtabend;    /* -  ADDRESS OF SECONDARY CVT FOR ABEND IN EOT           */
  void *__ptr32 cvtuser;     /* -        A WORD AVAILABLE TO THE USER                  */
  void *__ptr32 cvtmdlds;    /* -        RESERVED FOR MODEL-DEPENDENT SUPPORT          */
  short int cvtqabst;        /* -           AN SVC 13 (ABEND) INVOCATION (OS/VS2) @PDA */
  short int cvtlnksc;        /* -           AN SVC  6 (LINK) INVOCATION           @PDA */
  void *__ptr32 cvttsce;     /* -        ADDRESS OF THE FIRST TIME SLICE CONTROL       */
  void *__ptr32 cvtpatch;    /* - ADDRESS OF A 200-BYTE FE PATCH AREA                  */
  void *__ptr32 cvtrms;      /* -   RECOVERY MANAGEMENT SUPPORT (RMS)                  */
  void *__ptr32 cvtspdme;    /* -        SERVICE PROCESSOR DAMAGE MONITOR  @ZA57793    */
  void *__ptr32 cvt0scr1;    /* - ADDRESS OF THE SECTOR CALCULATION ROUTINE            */
  struct
  {
    unsigned char _cvtgtfst;    /* -      GTF FLAG BYTES                      ICB312 */
    unsigned int _cvtgtfa : 24; /* -ADDRESS OF MAIN MONITOR CALL ROUTING             */
  } cvtgtf;
  struct
  {
    unsigned char _cvttcmfg;     /* -       TCAM FLAGS            */
    unsigned int _cvtaqavb : 24; /* -      SAME AS CVTAQAVT ABOVE */
  } cvtaqavt;
  unsigned char cvtflag5; /* Flags, refreshed upon error, set                       */
  unsigned char cvtflag6; /* More flags                            @NUA             */
  unsigned char cvtflag7; /* More flags                            @NUA             */
  unsigned char cvtflag8; /* More flags. IBM use only              @O3C             */
  void *__ptr32 cvtsaf;   /* -        ADDRESS OF ROUTER VECTOR TABLE.       @L5A    */
  void *__ptr32 cvtext1;  /* - ADDRESS OF OS - OS/VS COMMON EXTENSION               */
  void *__ptr32 cvtcbsp;  /* -        ADDRESS OF ACCESS METHOD CONTROL BLOCK        */
  struct
  {
    unsigned char _filler1;      /* -     RESERVED - FIRST BYTE OF CVTPURG      @LFC  */
    unsigned int _cvtpurga : 24; /* -      ADDRESS OF SUBSYSTEM PURGE ROUTINE  ICB330 */
  } cvtpurg;
  unsigned char cvtamff[4]; /* -     RESERVED FOR ACCESS METHOD FLAGS    ICB436       */
  void *__ptr32 cvtqmsg;    /* - ADDRESS OF INFORMATION TO BE      @G860P46           */
  struct
  {
    unsigned char _cvtdmsrf;     /* -     OPEN/CLOSE/EOV FLAG BYTE.  SETTING BOTH    */
    unsigned int _cvtdmsra : 24; /* -      ADDRESS OF THE OPEN/CLOSE/EOV SUPERVISORY */
  } cvtdmsr;
  void *__ptr32 cvtsfr;      /* - ADDRESS OF SETFRR ROUTINE         @G383P9A           */
  void *__ptr32 cvtgxl;      /* - ADDRESS OF CONTENTS SUPERVISION   @G860P46           */
  void *__ptr32 cvtreal;     /* -        ADDRESS OF THE VIRTUAL STORAGE    @G860P46    */
  void *__ptr32 cvtptrv;     /* -  ADDRESS OF PAGING SUPERVISOR GENERAL  @L1C          */
  void *__ptr32 cvtihvp;     /* - POINTER TO IHV$COMM. INITIALIZED TO   @06C           */
  void *__ptr32 cvtjesct;    /* - ADDRESS OF JOB ENTRY SUBSYSTEM (JES)                 */
  unsigned char cvtrs12c[4]; /* -     RESERVED                              @LFC       */
  int cvttz;                 /* -        Difference between local time and UTC         */
  void *__ptr32 cvtmchpr;    /* -        ADDRESS OF MACHINE CHECK PARAMETER LIST       */
  void *__ptr32 cvteorm;     /* -        POTENTIAL REAL HIGH STORAGE ADDRESS.  @LBC    */
  void *__ptr32 cvtptrv3;    /* - ADDRESS OF PAGING SUPERVISOR ROUTINE  @L1A           */
  void *__ptr32 cvtlkrm;     /* ADDRESS OF CML LOCK RESOURCE      @G860P46             */
  struct
  {
    unsigned char _filler2;     /* -     RESERVED - FIRST BYTE OF CVTAPF       @LFC */
    unsigned int _cvtapfa : 24; /* ADDRESS OF BRANCH ENTRY POINT IN                 */
  } cvtapf;
  struct
  {
    unsigned char _filler3;      /* -     RESERVED - FIRST BYTE OF CVTEXT2      @LFC */
    unsigned int _cvtext2a : 24; /* SAME AS CVTEXT2 ABOVE               ICB330       */
  } cvtext2;
  struct
  {
    unsigned char _filler4;      /* -     RESERVED - FIRST BYTE OF CVTHJES      @LFC */
    unsigned int _cvthjesa : 24; /* -      ADDRESS OF OPTIONAL JOB ENTRY SUBSYSTEM   */
  } cvthjes;
  struct
  {
    unsigned char _cvtrs150; /* -     Reserved.  Was CVTRSTCP:                   */
    unsigned char _cvtrstrs; /* -     RESTART REASON.                   @ZA84941 */
    short int _cvtrcp2b;     /* -        Logical CPU address of target of the    */
  } cvtrstw2;
  unsigned char cvtsname[8]; /* SYSTEM NAME FOR CURRENT SYSTEM.                        */
  void *__ptr32 cvtgetl;     /* -        ADDRESS OF IKJGETL, TSO GET LINE ROUTINE      */
  void *__ptr32 cvtlpdsr;    /* - ADDRESS OF LINK PACK AREA (LPA) DIRECTORY            */
  void *__ptr32 cvtpvtp;     /* -  ADDRESS OF PAGE VECTOR TABLE                        */
  struct
  {
    unsigned char _cvtdirst;     /* -       FLAG BYTE                         @ZA65734 */
    unsigned int _cvtlpdir : 24; /* -      ADDRESS OF LINK PACK AREA (LPA) DIRECTORY   */
  } cvtlpdia;
  void *__ptr32 cvtrbcb;     /* -        ADDRESS OF THE RECORD BUFFER'S        @LEC    */
  unsigned char cvtrs170[4]; /* -     RESERVED                              @LFC       */
  struct
  {
    unsigned char _cvtsylk;     /* -       SUPERVISOR LOCK.  ONLY ENABLED TASKS MAY  */
    unsigned int _cvtslid : 24; /* -      SAME AS CVTSLIDA ABOVE            @ZA65734 */
  } cvtslida;
  struct
  {
    unsigned char _cvtflag1; /* -     FLAG BYTE                             @0CC */
    unsigned char _cvtflag2; /* -     FLAG BYTE                             @0CA */
    unsigned char _cvtflag3; /* -     FLAG BYTE refreshed upon error, set        */
    unsigned char _cvtflag4; /* -     Flag byte This is an interface for      CA */
  } cvtflags;
  void *__ptr32 cvtrt03;     /* - ADDRESS OF SRB TIMING             @G383P9A           */
  unsigned char cvtrs180[8]; /* - RESERVED                              @LOC           */
  void *__ptr32 cvtexsnr;    /* - ADDRESS OF EXCESSIVE SPIN NOTIFICATION               */
  unsigned char cvtexsnl;    /* -     SERIALIZATION BYTE FOR EXCESSIVE SPIN            */
  unsigned char cvtspvlk;    /* -      NUMBER OF TASKS WHICH HAVE TERMINATED           */
  unsigned char cvtctlfg;    /* -     SYSTEM CONTROL FLAGS                  @LBC       */
  char cvtapg;               /* -      DISPATCHING PRIORITY OF AUTOMATIC PRIORITY      */
  unsigned char cvttrace[2]; /* -          RESERVED             @(DCR670)    @G860P46  */
  unsigned char cvttrac2[2]; /* -          RESERVED             @(DCR670)    @G860P46  */
  void *__ptr32 cvtrscn;     /* - ADDRESS OF ROUTINE TO SCAN TCB TREE                  */
  void *__ptr32 cvttas;      /* -        ADDRESS OF ROUTINE TO TRANSFER ADDRESS        */
  void *__ptr32 cvttrcrm;    /* -        ADDRESS POINTER OF THE SYSTEM     @G860P46    */
  void *__ptr32 cvtshrvm;    /* -        LOWEST ADDRESS OF SHARED VIRTUAL STORAGE      */
  void *__ptr32 cvt0vl01;    /* - ENTRY POINT ADDRESS OF VALIDITY CHECK                */
  void *__ptr32 cvtppgmx;    /* -  ADDRESS POINTER FOR MVS/370-XA.   @G860P46          */
  unsigned char cvtgrsst;    /* -     GRS status. SERIALIZATION: None.      @0DA       */
  unsigned char cvtflag9;    /* -     Function-available flags              @0QC       */
  unsigned char cvtbsm0f[2]; /* -     Return via reg 15, BSM 0,15           @NJA       */
  void *__ptr32 cvtgvt;      /* -      ADDRESS OF THE GRS VECTOR TABLE   @G383P9A      */
  void *__ptr32 cvtascrf;    /* -        CREATED ASCB QUEUE HEADER                     */
  void *__ptr32 cvtascrl;    /* -        CREATED ASCB QUEUE TRAILER                    */
  void *__ptr32 cvtputl;     /* -        ADDRESS OF IKJPUTL, TSO PUT LINE ROUTINE      */
  void *__ptr32 cvtsrbrt;    /* - DISPATCHER RETURN ADDRESS FOR SRB ROUTINES           */
  void *__ptr32 cvtolt0a;    /* - BRANCH ENTRY TO OLTEP MEMORY TERMINATION             */
  void *__ptr32 cvtsmfex;    /* - BRANCH ENTRY TO SYSTEM MANAGEMENT                    */
  void *__ptr32 cvtcspie;    /* -        ENTRY POINT ADDRESS OF THE SUPERVISOR         */
  void *__ptr32 cvtptgt;     /* -        ADDRESS OF IKJPTGT, TSO PUTGET ROUTINE        */
  unsigned char cvtsigpt;    /* -     SIGP TIMEOUT VALUE.                   @07A       */
  unsigned char cvtspdmc;    /* -     SERVICE PROCESSOR DAMAGE MACHINE  @ZA57793       */
  unsigned char cvtdssac;    /* -     DYNAMIC SUPPORT SYSTEM (DSS) ACTIVATED           */
  unsigned char cvtrs1d7;    /* -     RESERVED                              @LFC       */
  void *__ptr32 cvtstck;     /* -        ADDRESS OF IKJSTCK, TSO STACK ROUTINE         */
  short int cvtmaxmp;        /* -        Maximum CPU address available for             */
  unsigned char cvtbsm2[2];  /* -     RETURN VIA REG 2, BSM 0,2.            @L4A       */
  void *__ptr32 cvtscan;     /* -        ADDRESS OF IKJSCAN, TSO SCAN ROUTINE          */
  void *__ptr32 cvtauthl;    /* -        POINTER TO AUTHORIZED LIBRARY TABLE.  @LWC    */
  void *__ptr32 cvtbldcp;    /* - BRANCH ENTRY TO BUILD POOL          MDC003           */
  void *__ptr32 cvtgetcl;    /* - BRANCH ENTRY TO GET CELL            MDC004           */
  void *__ptr32 cvtfrecl;    /* - BRANCH ENTRY TO FREE CELL           MDC005           */
  void *__ptr32 cvtdelcp;    /* - BRANCH ENTRY TO DELETE POOL         MDC006           */
  void *__ptr32 cvtcrmn;     /* - BRANCH ENTRY TO SVC 120 (GETMAIN/FREEMAIN            */
  void *__ptr32 cvtcras;     /* - POINTER DEFINED ADDRESS OF BRANCH  @G860PVB          */
  void *__ptr32 cvtqsas;     /* - POINTER DEFINED ADDRESS OF        @G860PVB           */
  void *__ptr32 cvtfras;     /* - POINTER DEFINED ENTRY TO TASK      @G860PVB          */
  void *__ptr32 cvts1ee;     /* - BRANCH ENTRY TO STAGE 1 EXIT EFFECTOR                */
  void *__ptr32 cvtpars;     /* -        ADDRESS OF IKJPARS, TSO PARSE ROUTINE         */
  void *__ptr32 cvtquis;     /* - BRANCH ENTRY TO QUIESCE             MDC013           */
  void *__ptr32 cvtstxu;     /* -        BRANCH ENTRY TO ATTENTION EXIT EPILOGUE       */
  void *__ptr32 cvtopte;     /* - BRANCH ENTRY ADDRESS TO SYSEVENT    MDC015           */
  void *__ptr32 cvtsdrm;     /* -        BRANCH ENTRY ADDRESS OF THE RESOURCE          */
  void *__ptr32 cvtcsrt;     /* - POINTER TO CALLABLE SERVICE REQUEST   @D9C           */
  void *__ptr32 cvtaqtop;    /* - POINTER TO THE ALLOCATION QUEUE   @G860P46           */
  void *__ptr32 cvtvvmdi;    /* -        CONSTANT USED BY PAGED LINK PACK AREA         */
  void *__ptr32 cvtasvt;     /* -        POINTER TO ADDRESS SPACE VECTOR TABLE         */
  void *__ptr32 cvtgda;      /* -        POINTER TO GLOBAL DATA AREA (GDA) IN SQA      */
  void *__ptr32 cvtascbh;    /* - POINTER TO HIGHEST PRIORITY ADDRESS SPACE            */
  void *__ptr32 cvtascbl;    /* - POINTER TO LOWEST PRIORITY ASCB ON THE               */
  void *__ptr32 cvtrtmct;    /* -        POINTER TO RECOVERY/TERMINATION CONTROL       */
  void *__ptr32 cvtsv60;     /* - BRANCH ENTRY ADDRESS FOR              @D8C           */
  void *__ptr32 cvtsdmp;     /* - ADDRESS OF SVC DUMP BRANCH        @G860P46           */
  void *__ptr32 cvtscbp;     /* - ADDRESS OF SCB PURGE RESOURCE     @G860P46           */
  unsigned char cvtsdbf[4];  /* - Address of 4K SQA buffer used by SVC                 */
  void *__ptr32 cvtrtms;     /* -        ADDRESS OF SERVICABILITY LEVEL INDICATOR      */
  void *__ptr32 cvttpios;    /* -        ADDRESS OF THE TELEPROCESSING I/O             */
  void *__ptr32 cvtsic;      /* -        BRANCH ADDRESS OF THE ROUTINE TO SCHEDULE     */
  void *__ptr32 cvtopctp;    /* - ADDRESS OF SYSTEM RESOURCES MANAGER (SRM)            */
  void *__ptr32 cvtexpro;    /* - ADDRESS OF EXIT PROLOGUE/TYPE 1 EXIT                 */
  void *__ptr32 cvtgsmq;     /* -  ADDRESS OF GLOBAL SERVICE MANAGER QUEUE             */
  void *__ptr32 cvtlsmq;     /* -  ADDRESS OF LOCAL SERVICE MANAGER QUEUE              */
  unsigned char cvtrs26c[4]; /* -  RESERVED.                             @LZC          */
  void *__ptr32 cvtvwait;    /* - ADDRESS OF WAIT ROUTINE             MDC048           */
  void *__ptr32 cvtparrl;    /* - ADDRESS OF PARTIALLY LOADED       @G860P46           */
  void *__ptr32 cvtapft;     /* -        ADDRESS OF AUTHORIZED PROGRAM FACILITY        */
  void *__ptr32 cvtqcs01;    /* - BRANCH ENTRY ADDRESS TO PROGRAM MANAGER              */
  int cvtfqcb;               /* -        FORMERLY USED BY ENQ/DEQ.         @G383P9A    */
  int cvtlqcb;               /* -        FORMERLY USED BY ENQ/DEQ.         @G383P9A    */
  void *__ptr32 cvtrenq;     /* - RESOURCE MANAGER ADDRESS FOR ENQ  @G860P46           */
  void *__ptr32 cvtrspie;    /* -  RESOURCE MANAGER FOR              @G860P46          */
  void *__ptr32 cvtlkrma;    /* - RESOURCE MANAGER ADDRESS FOR LOCK @G860P46           */
  void *__ptr32 cvtcsd;      /* -        VIRTUAL ADDRESS OF COMMON SYSTEM DATA AREA    */
  void *__ptr32 cvtdqiqe;    /* - RESOURCE MANAGER FOR EXIT         @G860P46           */
  void *__ptr32 cvtrpost;    /* - RESOURCE MANAGER FOR POST.        @G860P46           */
  void *__ptr32 cvt062r1;    /* - BRANCH ENTRY TO DETACH              MDC060           */
  void *__ptr32 cvtveac0;    /* - ASCBCHAP BRANCH ENTRY               MDC061           */
  void *__ptr32 cvtglmn;     /* - GLOBAL BRANCH ENTRY ADDRESS FOR                      */
  void *__ptr32 cvtspsa;     /* - POINTER TO GLOBAL WORK/SAVE AREA VECTOR              */
  void *__ptr32 cvtwsal;     /* - ADDRESS OF TABLE OF LENGTHS OF LOCAL                 */
  void *__ptr32 cvtwsag;     /* - ADDRESS OF TABLE OF LENGTHS OF GLOBAL                */
  void *__ptr32 cvtwsac;     /* - ADDRESS OF TABLE OF LENGTHS OF CPU                   */
  void *__ptr32 cvtrecrq;    /* - ADDRESS OF THE RECORDING REQUEST  @G860P46           */
  void *__ptr32 cvtasmvt;    /* -    POINTER TO AUXILIARY STORAGE MANAGEMENT           */
  void *__ptr32 cvtiobp;     /* - ADDRESS OF THE BLOCK PROCESSOR CVT                   */
  void *__ptr32 cvtspost;    /* - POST RESOURCE MANAGER TERMINATION ROUTINE            */
  struct
  {
    short int _cvtrstci;        /* -        CPU ID OF THE CPU HOLDING THE     @ZA84941 */
    unsigned char _cvtrstri[2]; /* -     IDENTIFIER OF OWNING ROUTINE      @ZA84941    */
  } cvtrstwd;
  void *__ptr32 cvtfetch;    /* - ADDRESS OF ENTRY POINT FOR BASIC  @G860P46           */
  void *__ptr32 cvt044r2;    /* - ADDRESS OF IGC044R2 IN CHAP SERVICE                  */
  void *__ptr32 cvtperfm;    /* -        ADDRESS OF THE PERFORMANCE WORK AREA.         */
  void *__ptr32 cvtdair;     /* -        ADDRESS OF IKJDAIR, TSO DYNAMIC               */
  void *__ptr32 cvtehdef;    /* -        ADDRESS OF IKJEHDEF, TSO DEFAULT  @G860P46    */
  void *__ptr32 cvtehcir;    /* -        ADDRESS OF IKJEHCIR, TSO CATALOG  @G860P46    */
  void *__ptr32 cvtssap;     /* -        ADDRESS OF SYSTEM SAVE AREA       @ZA65734    */
  void *__ptr32 cvtaidvt;    /* -        POINTER TO APPENDAGE ID VECTOR TABLE          */
  void *__ptr32 cvtipcds;    /* - BRANCH ENTRY FOR DIRECT SIGNAL                       */
  void *__ptr32 cvtipcri;    /* - BRANCH ENTRY FOR REMOTE IMMEDIATE                    */
  void *__ptr32 cvtipcrp;    /* - BRANCH ENTRY FOR REMOTE PENDABLE                     */
  void *__ptr32 cvtpccat;    /* -        POINTER TO PHYSICAL CCA VECTOR TABLE          */
  void *__ptr32 cvtlccat;    /* -        POINTER TO LOGICAL CCA VECTOR TABLE           */
  void *__ptr32 cvtxsft;     /* - ADDRESS OF SYSTEM FUNCTION TABLE  @G383P9A           */
  void *__ptr32 cvtxstks;    /* - ADDRESS OF PCLINK STACK           @G383P9A           */
  void *__ptr32 cvtxstkn;    /* - ADDRESS OF PCLINK STACK (SAVE=NO) @G383P9A           */
  void *__ptr32 cvtxunss;    /* - ADDRESS OF PCLINK UNSTACK         @G383P9A           */
  void *__ptr32 cvtpwi;      /* -        ADDRESS OF THE WINDOW INTERCEPT ROUTINE       */
  void *__ptr32 cvtpvbp;     /* -        ADDRESS OF THE VIRTUAL BLOCK PROCESSOR        */
  void *__ptr32 cvtmfctl;    /* -        POINTER TO MEASUREMENT FACILITY CONTROL       */
  void *__ptr32 cvtmfrtr;    /* -  IF MEASUREMENT FACILITY IS ACTIVE,                  */
  void *__ptr32 cvtvpsib;    /* - BRANCH ENTRY TO PAGE SERVICES     @G860PVB           */
  void *__ptr32 cvtvsi;      /* - POINTER DEFINED, BRANCH ENTRY     @G860P46           */
  void *__ptr32 cvtexcl;     /* - ADDRESS POINTER TO THE EXCP           @D3A           */
  void *__ptr32 cvtxunsn;    /* - ADDRESS OF PCLINK UNSTACK         @G383P9A           */
  void *__ptr32 cvtisnbr;    /* - ENTRY POINT ADDRESS OF DISABLED       @LLA           */
  void *__ptr32 cvtxextr;    /* - ADDRESS OF PCLINK EXTRACT ROUTINE                    */
  void *__ptr32 cvtmsfrm;    /* - ADDRESS OF THE PROCESSOR          @G860P46           */
  void *__ptr32 cvtscpin;    /* -        ADDRESS OF IPL-TIME SCPINFO DATA      @NAC    */
  void *__ptr32 cvtwsma;     /* ADDRESS OF WAIT STATE MESSAGE AREA    @LLC             */
  void *__ptr32 cvtrmbr;     /* - ADDRESS OF REGMAIN BRANCH ENTRY.    MDC123           */
  void *__ptr32 cvtlfrm;     /* - LIST FORMAT FREEMAIN BRANCH ENTRY   MDC124           */
  void *__ptr32 cvtgmbr;     /* - LIST FORMAT GETMAIN BRANCH ENTRY    MDC125           */
  void *__ptr32 cvt0tc0a;    /* -        ADDRESS OF TASK CLOSE MODULE        MDC128    */
  int cvtrlstg;              /* -        SIZE OF ACTUAL REAL STORAGE ONLINE  @0GC      */
  void *__ptr32 cvtspfrr;    /* - 'SUPER FRR' ADDRESS (ADDRESS OF   @G860P46           */
  unsigned char cvtrs360[4]; /* - RESERVED.                             @LZC           */
  void *__ptr32 cvtsvt;      /* - ADDRESS POINTER FOR FETCH                            */
  void *__ptr32 cvtirecm;    /* -        ADDRESS OF INITIATOR RESOURCE       MDC158    */
  void *__ptr32 cvtdarcm;    /* -        ADDRESS OF DEVICE ALLOCATION        MDC159    */
  void *__ptr32 cvt0pt02;    /* - ADDRESS OF POST ENTRY POINT         MDC160           */
  unsigned char cvtrs374[4]; /* RESERVED                              @LOC             */
  void *__ptr32 cvtwtcb;     /* -  ADDRESS OF WAIT STATE TCB.          MDC164          */
  void *__ptr32 cvtvacr;     /* -        ACR/VARY CPU CHANNEL RECOVERY       MDC178    */
  void *__ptr32 cvtrecon;    /* -        VARY CPU SHUTDOWN ROUTINE         @G860PVB    */
  void *__ptr32 cvtgtfr8;    /* - GENERALIZED TRACE FACILITY (GTF)    MDC180           */
  void *__ptr32 cvtvstop;    /* - ADDRESS OF VARY CPU STOP CPU        MDC169           */
  void *__ptr32 cvtvpsa;     /* -        ADDRESS OF COPY OF SYSGEN'ED PSA -  MDC170    */
  void *__ptr32 cvtrmptt;    /* -        ADDRESS OF ISTRAMA1, THE VTAM     @Z30AP9A    */
  void *__ptr32 cvtrmpmt;    /* -        ADDRESS OF ISTRAMA2, THE VTAM     @Z30AP9A    */
  void *__ptr32 cvtexp1;     /* - ADDRESS OF EXIT PROLOGUE WHICH      MDC173           */
  void *__ptr32 cvtcsdrl;    /* -        REAL ADDRESS OF COMMON SYSTEM DATA  MDC174    */
  void *__ptr32 cvtssrb;     /* - STATUS STOP SRB ENTRY.              MDC175           */
  unsigned char cvtrs3a4[4]; /* -     RESERVED                              @LFC       */
  void *__ptr32 cvtqv1;      /* - ADDRESS OF QUEUE VERIFICATION FOR   MDC181           */
  void *__ptr32 cvtqv2;      /* - ADDRESS OF QUEUE VERIFICATION FOR   MDC182           */
  void *__ptr32 cvtqv3;      /* - ADDRESS OF QUEUE VERIFICATION FOR   MDC183           */
  void *__ptr32 cvtgsda;     /* - ADDRESS OF GLOBAL SYSTEM DUPLEX       @L7C           */
  void *__ptr32 cvtadv;      /* - ADDRESS OF ADDRESS VERIFICATION     MDC186           */
  void *__ptr32 cvttpio;     /* -   ADDRESS OF VTAM TPIO (SVC 124)      MDC193         */
  unsigned char cvtrs3c0[4]; /* -     RESERVED                              @LFC       */
  void *__ptr32 cvtevent;    /* - BRANCH ENTRY ADDRESS TO EVENTS    @G860P46           */
  void *__ptr32 cvtsscr;     /* -        ADDRESS OF STORAGE SYSTEM         @Z30LP9A    */
  void *__ptr32 cvtcbbr;     /* - BRANCH ENTRY ADDRESS TO           @Z30EN9A           */
  void *__ptr32 cvteff02;    /* -        ADDRESS OF IKJEFF02, TSO MESSAGE  @ZM30972    */
  void *__ptr32 cvtlsch;     /* - ADDRESS OF LOCAL SCHEDULE.        @G50EP9A           */
  void *__ptr32 cvtcdeq;     /* -        ADDRESS OF PROGRAM MANAGER        @G50EP9A    */
  void *__ptr32 cvthsm;      /* -        POINTER TO HIERARCHICAL STORAGE   @ZA27771    */
  void *__ptr32 cvtrac;      /* -        ADDRESS OF ACCESS CONTROL CVT.    @Z40RP9A    */
  void *__ptr32 cvtcgk;      /* -  ADDRESS OF ROUTINE USED TO CHANGE @G860P46          */
  void *__ptr32 cvtsrm;      /* - ADDRESS OF ENTRY TABLE FOR SRM,   @G50IP9A           */
  void *__ptr32 cvt0pt0e;    /* - ENTRY POINT TO IDENTIFY POST EXIT @Z40EP9A           */
  void *__ptr32 cvt0pt03;    /* - POST REINVOCATION ENTRY POINT     @Z40EP9A           */
  void *__ptr32 cvttcasp;    /* -        POINTER TO THE TSO/VTAM TERMINAL  @Z40DQ9A    */
  void *__ptr32 cvtcttvt;    /* -        CTT VT                                @PVC    */
  void *__ptr32 cvtjterm;    /* - POINTER DEFINED ADDRESS OF        @G860PVB           */
  void *__ptr32 cvtrsume;    /* - ADDRESS OF RESUME FUNCTION.       @G383P9A           */
  void *__ptr32 cvttctl;     /* - ADDRESS OF TRANSFER CONTROL       @Z40FP9A           */
  void *__ptr32 cvtrmt;      /* -        ADDRESS OF RESOURCE MANAGER           @LGC    */
  void *__ptr32 cvtt6svc;    /* - ENTRY POINT ADDRESS FOR TYPE 6    @Z40FP9A           */
  void *__ptr32 cvtsusp;     /* - ADDRESS OF SUSPEND ROUTINE.       @Z40FP9A           */
  void *__ptr32 cvtihasu;    /* - ADDRESS OF BIT STRING.  (MDC355)  @Z404P9A           */
  void *__ptr32 cvtsfv;      /* - ADDRESS OF SETFRR ROUTINE ABOVE 16M   @N3A           */
  void *__ptr32 cvtidevn;    /* - ADDRESS OF DEVICE NUMBER CONVERSION   @LGC           */
  void *__ptr32 cvtsmf83;    /* -        ADDRESS OF BRANCH ENTRY TO        @G741P9A    */
  void *__ptr32 cvtsmfsp;    /* - ADDRESS OF SMF SUSPEND HANDLER.   @G860P46           */
  void *__ptr32 cvtmsfcb;    /* -        ADDRESS OF MAINTENANCE AND        @G871A9A    */
  void *__ptr32 cvthid;      /* -  ADDRESS OF SCP HOST ID. @(DCR819) @G860P46          */
  void *__ptr32 cvtpsxm;     /* - ADDRESS OF CROSS MEMORY PAGE      @G860PVB           */
  void *__ptr32 cvtucbsc;    /* - ADDRESS OF UCB SCAN               @G860P46           */
  void *__ptr32 cvttpur;     /* -        DDR QUEUE OF TAPE UNIT-RECORD         @L2A    */
  void *__ptr32 cvtdpur;     /* -        DDR QUEUE OF DASD SWAP REQUESTS.      @L2A    */
  void *__ptr32 cvttrpos;    /* -        DDR QUEUE OF TAPES TO BE              @L2A    */
  void *__ptr32 cvtrs444;    /* -        Reserved, must always be 0. Was CVTRESTX,     */
  short int cvtxcpct;        /* -      MAXIMUM EXCP COUNT PER ADDRESS        @L2A      */
  short int cvtcall;         /* -           A BASSM 14,15 INSTRUCTION.  POINTER   @PDA */
  void *__ptr32 cvtvfind;    /* - THE POINTER TO VIRTUAL FETCH BUILD    @L8A           */
  void *__ptr32 cvtvfget;    /* - THE POINTER TO VIRTUAL FETCH GET      @L8A           */
  void *__ptr32 cvtvfmem;    /* RESERVED. THIS FIELD IS NO LONGER    @MLC              */
  void *__ptr32 cvtvfcb;     /* -        THE POINTER TO VIRTUAL FETCH INTERNAL @L8A    */
  void *__ptr32 cvtpgser;    /* - POINTER DEFINED ADDRESS OF ENTRY   @G860P46          */
  void *__ptr32 cvttski;     /* - POINTER DEFINED ADDRESS OF TASK   @G860PVB           */
  void *__ptr32 cvtcpgub;    /* - POINTER DEFINED ADDRESS OF CPOOL  @G860PVB           */
  void *__ptr32 cvtcpgup;    /* - POINTER DEFINED ADDRESS OF CPOOL  @G860PVB           */
  void *__ptr32 cvtcpgtc;    /* - POINTER DEFINED ADDRESS OF GET    @G860PVB           */
  void *__ptr32 cvtcpfre;    /* - POINTER DEFINED ADDRESS OF CPOOL  @G860PVB           */
  void *__ptr32 cvtslist;    /* - POINTER DEFINED ADDRESS OF VSM    @G860PVB           */
  void *__ptr32 cvtsregn;    /* - POINTER DEFINED ADDRESS OF VSM    @G860PVB           */
  void *__ptr32 cvtsloc;     /* - POINTER DEFINED ADDRESS OF VSM     @G860PVB          */
  void *__ptr32 cvtcpbdb;    /* - POINTER DEFINED ADDRESS OF CPOOL  @G860PVB           */
  void *__ptr32 cvtcpdlb;    /* - POINTER DEFINED ADDRESS OF CPOOL  @G860PVB           */
  void *__ptr32 cvtdoffs;    /* -        STARTING REAL ADDRESS OF DAT-OFF  @G860PVB    */
  void *__ptr32 cvtdoffe;    /* -        ENDING REAL ADDRESS OF DAT-OFF    @G860PVB    */
  void *__ptr32 cvtrcep;     /* - ADDRESS OF THE RSM CONTROL AND    @G860PVB           */
  void *__ptr32 cvtcpgus;    /* - ADDRESS OF CPOOL GET              @G860P46           */
  void *__ptr32 cvtgrrgn;    /* - POINTER DEFINED ADDRESS OF GET    @G860PVB           */
  void *__ptr32 cvtgvrgn;    /* - POINTER DEFINED ADDRESS OF GET    @G860PVB           */
  unsigned char cvtionlv;    /* -     DEFAULT VALUE OF                  @G860P46       */
  unsigned char cvtrs4a1[3]; /* -     RESERVED                              @LFC       */
  unsigned char cvtrczrt[2]; /* -       SET REGISTER 15 TO ZERO.          @G860PVB     */
  unsigned char _filler5[2]; /* -   RETURN VIA A BSM.                 @G860PVB         */
  unsigned char cvtfunc[4];  /* -     Reserved for solution/offering use.   @M6A       */
  void *__ptr32 cvtsmext;    /* - ADDRESS OF STORAGE MAP EXTENSION. @G860PVB           */
  void *__ptr32 cvtnucmp;    /* -        ADDRESS OF NUCLEUS MAP.           @G860PVB    */
  unsigned char cvtxafl;     /* -     FLAG BYTE FOR MVS/XA PROCESSING.  @G860P46       */
  unsigned char cvtrs4b5[3]; /* -     RESERVED                              @LFC       */
  void *__ptr32 cvtvtam;     /* -        ADDRESS OF VTAM COMMAND PROCESSOR @G860P46    */
  void *__ptr32 cvtspip;     /* -        ADDRESS OF RTM INTERFACE TO       @ZA65734    */
  struct
  {
    void *__ptr32 _cvtdfa; /* - ADDRESS OF DFP ID TABLE, MAPPED BY    @P8A */
  } cvtckras;
  void *__ptr32 cvtnvt0;  /* - ADDRESS OF DATA IN DAT-ON NUCLEUS @G860P46           */
  void *__ptr32 cvtcsomf; /* -        OWNER OF CHANNEL MEASUREMENT      @G860P46    */
  void *__ptr32 cvtcsoal; /* -        OWNER OF ADDRESS LIMIT FACILITY.  @G860P46    */
  void *__ptr32 cvtichpt; /* -        ADDRESS OF THE INSTALLED CHANNEL  @G860P46    */
  void *__ptr32 cvtcsocr; /* -        CHANNEL SUBSYSTEM OWNER - CHANNEL @G860P46    */
  void *__ptr32 cvtcsocs; /* -        CHANNEL SUBSYSTEM OWNER - CHANNEL @G860P46    */
  void *__ptr32 cvtllta;  /* -        LINK LIST TABLE ADDRESS.          @G860P46    */
  void *__ptr32 cvtdcqa;  /* -   ADDRESS OF DEVICE CLASS QUEUE         @LDC         */
  void *__ptr32 cvtucba;  /* -   ADDRESS OF THE FIRST UCB IN THE       @LDC         */
  void *__ptr32 cvtvestu; /* - ADDRESS OF THE ENTRY POINT OF THE @G860P46           */
  void *__ptr32 cvtnuclu; /* - ADDRESS TO SUPPORT THE NUCLEUS    @G860P46           */
  struct
  {
    unsigned char _cvtoslv0; /* BYTE 0 OF CVTOSLVL                    @LRC */
    unsigned char _cvtoslv1; /* BYTE 1 OF CVTOSLVL                    @MBC */
    unsigned char _cvtoslv2; /* BYTE 2 OF CVTOSLVL                    @PTC */
    unsigned char _cvtoslv3; /* BYTE 3 OF CVTOSLVL                    @NDC */
    unsigned char _cvtoslv4; /* BYTE 4 OF CVTOSLVL                    @0KC */
    unsigned char _cvtoslv5; /* BYTE 5 OF CVTOSLVL                    @NIC */
    unsigned char _cvtoslv6; /* BYTE 6 OF CVTOSLVL                    @O9C */
    unsigned char _cvtoslv7; /* BYTE 7 OF CVTOSLVL                    @LGA */
    unsigned char _cvtoslv8; /* BYTE 8 OF CVTOSLVL                    @0PC */
    unsigned char _cvtoslv9; /* BYTE 9 OF CVTOSLVL                    @LGA */
    unsigned char _cvtoslva; /* BYTE 10 OF CVTOSLVL                   @LGA */
    unsigned char _cvtoslvb; /* BYTE 11 OF CVTOSLVL                   @LGA */
    unsigned char _cvtoslvc; /* BYTE 12 OF CVTOSLVL                   @LGA */
    unsigned char _cvtoslvd; /* BYTE 13 OF CVTOSLVL                   @LGA */
    unsigned char _cvtoslve; /* BYTE 14 OF CVTOSLVL                   @LGA */
    unsigned char _cvtoslvf; /* BYTE 15 OF CVTOSLVL                   @LGA */
  } cvtoslvl;
};

#define cvtvss cvtvprm._cvtvss
#define cvtvpsm cvtvprm._cvtvpsm
#define cvtflgc0 cvtflgcs._cvtflgc0
#define cvtflgc1 cvtflgcs._cvtflgc1
#define cvticpid cvtflgcs._cvticpid
#define cvtgtfst cvtgtf._cvtgtfst
#define cvtgtfa cvtgtf._cvtgtfa
#define cvttcmfg cvtaqavt._cvttcmfg
#define cvtaqavb cvtaqavt._cvtaqavb
#define cvtpurga cvtpurg._cvtpurga
#define cvtdmsrf cvtdmsr._cvtdmsrf
#define cvtdmsra cvtdmsr._cvtdmsra
#define cvtapfa cvtapf._cvtapfa
#define cvtext2a cvtext2._cvtext2a
#define cvthjesa cvthjes._cvthjesa
#define cvtrs150 cvtrstw2._cvtrs150
#define cvtrstrs cvtrstw2._cvtrstrs
#define cvtrcp2b cvtrstw2._cvtrcp2b
#define cvtdirst cvtlpdia._cvtdirst
#define cvtlpdir cvtlpdia._cvtlpdir
#define cvtsylk cvtslida._cvtsylk
#define cvtslid cvtslida._cvtslid
#define cvtflag1 cvtflags._cvtflag1
#define cvtflag2 cvtflags._cvtflag2
#define cvtflag3 cvtflags._cvtflag3
#define cvtflag4 cvtflags._cvtflag4
#define cvtrstci cvtrstwd._cvtrstci
#define cvtrstri cvtrstwd._cvtrstri
#define cvtdfa cvtckras._cvtdfa
#define cvtoslv0 cvtoslvl._cvtoslv0
#define cvtoslv1 cvtoslvl._cvtoslv1
#define cvtoslv2 cvtoslvl._cvtoslv2
#define cvtoslv3 cvtoslvl._cvtoslv3
#define cvtoslv4 cvtoslvl._cvtoslv4
#define cvtoslv5 cvtoslvl._cvtoslv5
#define cvtoslv6 cvtoslvl._cvtoslv6
#define cvtoslv7 cvtoslvl._cvtoslv7
#define cvtoslv8 cvtoslvl._cvtoslv8
#define cvtoslv9 cvtoslvl._cvtoslv9
#define cvtoslva cvtoslvl._cvtoslva
#define cvtoslvb cvtoslvl._cvtoslvb
#define cvtoslvc cvtoslvl._cvtoslvc
#define cvtoslvd cvtoslvl._cvtoslvd
#define cvtoslve cvtoslvl._cvtoslve
#define cvtoslvf cvtoslvl._cvtoslvf

/* Values for field "cvtflgc0" */
#define cvtmulnf 0x80 /* -   For users of IFAUSAGE, REQUEST=FUNCTIONxxx     */

/* Values for field "cvtdcb" */
#define cvtmvse 0x80  /* -       S/370-XA MODE EXECUTION           @G860PVB */
#define cvt1sss 0x40  /* -       OPTION 1 (PCP) SSS. ALSO, LANGUAGE    @H3C */
#define cvt2sps 0x20  /* -       OPTION 2 (MFT) SPS, OS/VS1, VSE       @PHC */
#define cvt4ms1 0x10  /* -       OPTION 4 (MVT) MS1, OS/VS2                 */
#define cvtosext 0x08 /* -       INDICATOR THAT THE CVTOSLVL AREA IS   @LGA */
#define cvt4mps 0x04  /* -       MODEL 65 MULTIPROCESSING                   */
#define cvt6dat 0x02  /* -       DYNAMIC ADDRESS TRANSLATION BY CPU         */
#define cvtmvs2 0x01  /* -       MULTIPLE MEMORY OPTION OF OS/VS2 IS        */
#define cvt8aos2 0x12 /* - OS/VS2 SYSTEM                                    */

/* Values for field "cvtopta" */
#define cvtcch 0x80     /* -       CHANNEL CHECK HANDLER (CCH) OPTION PRESENT */
#define cvtapr 0x40     /* -       ALTERNATE PATH RETRY (APR) OPTION PRESENT  */
#define cvtddr 0x20     /* -       DYNAMIC DEVICE RECONFIGURATION (DDR)       */
#define cvtnip 0x10     /* -       NIP IS EXECUTING                           */
#define cvtwarnund 0x08 /* WARNUND processing is in effect       @NXA         */
#define cvt121tr 0x04   /* - DO NOT TRANSLATE EXCP V=R.        @ZA45816       */
#define cvtascii 0x02   /* -       ASCII TAPE PROCESSING IS GENERATED IN      */
#define cvtxpfp 0x01    /* -       CPU HAS EXTENDED PRECISION FLOATING POINT  */

/* Values for field "cvtoptb" */
#define cvtprot 0x80  /* -       CPU HAS STORE PROTECTION FEATURE (OS/VS1)  */
#define cvtctims 0x40 /* -       IF ON, HARDWARE HAS THE CLOCK COMPARATOR   */
#define cvttod 0x20   /* -       CPU HAS TIME-OF-DAY CLOCK FEATURE          */
#define cvtnlog 0x10  /* -       SYS1.LOGREC IS UNAVAILABLE FOR ERROR       */
#define cvtapthr 0x08 /* -       NIP SETS THIS BIT TO 1 WHEN DEVICE TESTING */
#define cvtfp 0x04    /* -       CPU HAS FETCH PROTECTION FEATURE (OS/VS1)  */
#define cvtvs1a 0x02  /* -       VS1 ASSIST IS AVAILABLE FOR USE (OS/VS1)   */
#define cvtvs1b 0x01  /* -       VS1 ASSIST SUBSET IS AVAILABLE FOR USE     */

/* Values for field "cvtgtfst" */
#define cvtgtfav 0x80 /* -       IF ZERO, GTF NOT ACTIVE.  IF ONE, GTF      */
#define cvtspd 0x40   /* -       SERVICE PROCESSOR DAMAGE.         @ZA57793 */
#define cvtwspr 0x20  /* -       WAITING FOR SERVICE PROCESSOR     @ZA57793 */
#define cvtusr 0x04   /* -       TRACE=USR SPECIFIED.  USER-REQUESTED TRACE */
#define cvtrnio 0x02  /* -       GTF IS ACTIVE AND TRACING RNIO EVENTS      */

/* Values for field "cvttcmfg" */
#define cvttcrdy 0x80 /* -       TCAM IS READY TO ACCEPT USERS              */
#define cvtldev 0x40  /* -       LOCAL DEVICE ATTACHED TO TCAM              */
#define cvtnwtcm 0x20 /* -       MULTIPLE TCAM FEATURE ACTIVE.         @D1A */

/* Values for field "cvtflag5" */
#define cvtvef 0x80   /* Vector Extension Facility             @NUA         */
#define cvtz1 0x80    /* Z1                                    @NUA         */
#define cvteec 0x40   /* EEC                                   @0OA         */
#define cvtnnpaf 0x20 /* NNPA facility                         @0SA         */

/* Values for field "cvtflag6" */
#define cvtsoled 0x80 /* Solution Edition                      @NWA         */

/* Values for field "cvtflag7" */
#define cvtsysplexzaaponline 0x80 /* The sysplex has one or more zAAP                   */
#define cvtsysplexzcbponline 0x40 /* The sysplex has one or more zCBP                   */

/* Values for field "cvtflag8" */
#define cvtpqap 0x80 /* @O3A                                               */

/* Values for field "cvtdmsrf" */
#define cvtsdump 0x80 /* -       SET BY COREZAP.  WILL CAUSE AN SDUMP TO BE */
#define cvtudump 0x40 /* -       SET BY COREZAP.  WILL CAUSE AN ABEND DUMP  */

/* Values for field "cvtdirst" */
#define cvtdicom 0x80 /* -       LPA DIRECTORY HAS BEEN INITIALIZED BY NIP  */

/* Values for field "cvtsylk" */
#define cvtsylks 0xFF /* -       SET LOCK BYTE                              */
#define cvtsylkr 0x00 /* -       RESET LOCK BYTE                            */

/* Values for field "cvtflag1" */
#define cvtrsmwd 0x80 /* -       IF ONE REAL STORAGE MANAGER WINDOW    @L1A */
#define cvtsvprc 0x40 /* -       SERVICE PROCESSOR ARCHITECTURE        @H3C */
#define cvtcuse 0x20  /* -       CUSE. SET BY NIP                      @03A */
#define cvtmvpg 0x10  /* -       IF ONE, MOVEPAGE CAPABILITY IS        @LIA */
#define cvtover 0x08  /* -       SUBPOOL OVERRIDE IS SUPPORTED.        @LRA */
#define cvtcstr 0x04  /* -       IF ONE, CSTRING FACILITY IS PRESENT   @DCA */
#define cvtsubsp 0x02 /* -       IF ONE, SUBSPACE FACILITY IS PRESENT  @LVA */
#define cvtkpar 0x01  /* -       RESERVED FOR USE BY RTM ONLY.         @M2A */

/* Values for field "cvtflag2" */
#define cvtcmpsc 0x80 /* -       IF ONE, INDICATES PRESENCE OF MVS     @0CA */
#define cvtcmpsh 0x40 /* -       IF ONE, INDICATES PRESENCE OF CMPSC   @0CA */
#define cvtsopf 0x20  /* -       IF ONE, INDICATES PRESENCE OF THE     @M9A */
#define cvtbfph 0x10  /* -       If one, indicates presence of              */
#define cvtper2 0x08  /* -       If one, indicates presence of              */
#define cvtiqd 0x04   /* -       If one, indicates that Internal       @MRC */
#define cvtalr 0x02   /* -       If one, indicates ASN and LX Reuse    @MXA */
#define cvtedat 0x01  /* -       If one, indicates that the Enhanced DAT    */

/* Values for field "cvtflag3" */
#define cvtesame 0x80            /* -                                                  */
#define cvtzarch 0x80            /* -       If one, indicates presence of              */
#define cvtprocascore 0x40       /* A processor resource is viewed                     */
#define cvtmulticpuspercore 0x20 /* When CvtProcAsCore is on, this indicates           */
#define cvtcpuasaliastocore 0x10 /* When CvtProcAsCore is on, the term "CPU"           */
#define cvtflag3diag 0x08        /* Diagnostic data for IBM use only      @O4C         */
#define cvtflag3rsvd 0x04        /* Reserved for IBM use                  @O4C         */
#define cvtzcbp 0x02             /* When bit is on, system fields with zCBP            */
#define cvtgsf 0x01              /* GSF is available                      @02A         */

/* Values for field "cvtflag4" */
#define cvtp001i 0x80 /* If one, indicates that P001 support                */
#define cvtp001a 0x40 /* If one, indicates that the system is               */
#define cvtznalc 0x20 /* zNALC                                 @0JA         */
#define cvtdcpa 0x10  /* Dynamic CPU Addition is enabled       @NAA         */
#define cvttx 0x08    /* TX support is enabled                 @NMA         */
#define cvtp002 0x08  /* P002 support is enabled               @NMA         */
#define cvttxc 0x04   /* TXC support is enabled                @NMA         */
#define cvtp002c 0x04 /* P002C support is enabled              @NMA         */
#define cvtri 0x02    /* RI support is enabled                 @NNA         */
#define cvtedat2 0x01 /* EDAT2 is enabled                      @NMA         */

/* Values for field "cvtctlfg" */
#define cvttxte 0x80      /* A Transactional Execution test environment         */
#define cvttxj 0x80       /* Not a programming interface           @NMA         */
#define cvtctlfgdiag 0x60 /* Diagnostic data for IBM use only      @O4C         */
#define cvtdstat 0x10     /* -       DEVSTAT OPTION IN EFFECT.  DEVICE ADDRESS  */
#define cvtdrmod 0x08     /* -       Set on when DRMODE=YES was specified. @MZA */
#define cvtnomp 0x04      /* -       MULTIPROCESSING CODE IS NOT IN THE SYSTEM. */
#define cvtgtrce 0x02     /* -       GENERALIZED TRACE FACILITY (GTF) HAS       */
#define cvtsdtrc 0x01     /* -       SVC DUMP HAS SUPPRESSED SUPERVISOR TRACE.  */

/* Values for field "cvtgrsst" */
#define cvte51gn 0x80 /* -       When on, global resource contention        */
#define cvte51ln 0x40 /* -       When on, local resource contention         */

/* Values for field "cvtflag9" */
#define cvt___llacopy___deblockexclok 0x40 /* LLACOPY supports DEBLOCKEXCLOK=YES                 */
#define cvtifawicavailable 0x02            /* IFAWIC service is available for use   @0QA         */
#define cvtifawicinstalled 0x01            /* IFAWIC service is installed           @0QA         */

/* Values for field "cvtmfrtr" */
#define cvtmfact 0x80 /* -       IF ONE, I/O SUPERVISOR AND TIMER SECOND    */

/* Values for field "cvtgsda" */
#define cvtgsdab 0x80 /* -       IF HIGH-ORDER BIT IS ONE, THERE IS  MDC185 */

/* Values for field "cvtfunc" */
#define cvtsoln 0x80 /* -       If high order bit is on, this is not  @M6A */

/* Values for field "cvtxafl" */
#define cvtcsrim 0x80 /* -       EXPLICIT LOAD PROCESSING REQUIRED @G860PVB */

/* Values for field "cvtoslv0" */
#define cvth3310 0x80 /* HBB3310 FUNCTIONS ARE PRESENT         @DBA         */
#define cvtesa 0x80   /* ESA/370 IS SUPPORTED                  @LQA         */
#define cvtxax 0x80   /* ESA/370 IS SUPPORTED (XAX - OLD NAME) @LQC         */
#define cvth4420 0x40 /* HBB4420 FUNCTIONS ARE PRESENT.        @LPA         */
#define cvtj3313 0x20 /* JBB3313 FUNCTIONS ARE PRESENT         @LHA         */
#define cvtj3311 0x10 /* JBB3311 FUNCTIONS ARE PRESENT         @DBA         */
#define cvthiper 0x10 /* HIPERSPACES ARE SUPPORTED             @DAA         */
#define cvth4410 0x08 /* HBB4410 FUNCTIONS ARE PRESENT.        @LKA         */
#define cvtlkr 0x08   /* SPIN LOCK RESTRUCTURE INDICATOR.      @LJA         */
#define cvtucbsv 0x08 /* UCB SERVICES INSTALLED.               @LMA         */
#define cvtcads 0x04  /* SCOPE=COMMON DATA SPACES SUPPORTED    @04A         */
#define cvtcrptl 0x02 /* ENCRYPTION ASYMMETRIC FEATURE IS      @05A         */
#define cvtj4422 0x01 /* JBB4422 FUNCTIONS ARE PRESENT         @LTA         */

/* Values for field "cvtoslv1" */
#define cvth4430 0x80          /* HBB4430 FUNCTIONS ARE PRESENT         @LQA         */
#define cvtdyapf 0x80          /* DYNAMIC APF, THROUGH CSVAPF, PRESENT  @LQA         */
#define cvtwlm 0x40            /* WORKLOAD MANAGER IS INSTALLED         @LYA         */
#define cvth5510 0x20          /* HBB5510 FUNCTIONS ARE PRESENT         @M1A         */
#define cvtdynex 0x20          /* CSVDYNEX FOR DYNAMIC EXITS IS PRESENT @LXA         */
#define cvth5520 0x10          /* HBB5520 FUNCTIONS ARE PRESENT         @M3A         */
#define cvtenclv 0x10          /* ENCLAVES FUNCTION IS PRESENT          @M4A         */
#define cvtj5522 0x08          /* JBB5522 FUNCTIONS ARE PRESENT         @M8A         */
#define cvth5530 0x04          /* HBB6603 FUNCTIONS ARE PRESENT         @MBA         */
#define cvth6603 0x04          /* HBB6603 FUNCTIONS ARE PRESENT         @MDA         */
#define cvtos390___010300 0x04 /* OS/390 R3                             @MEA         */
#define cvtos390___r3 0x04     /* OS/390 R3                             @MEA         */
#define cvtdynl 0x04           /* Dynamic LNKLST, via CSVDYNL, is present            */
#define cvth6601 0x02          /* OS/390 release 1                      @M9A         */
#define cvtos390 0x02          /* OS/390 release 1                      @M9A         */
#define cvtos390___010100 0x02 /* OS/390 R1                             @MEA         */
#define cvtos390___r1 0x02     /* OS/390 R1                             @MEA         */
#define cvtprded 0x02          /* Product enable/disable (IFAEDxxx)     @M9A         */
#define cvtj6602 0x01          /* OS/390 release 2                      @MAA         */
#define cvtos390___010200 0x01 /* OS/390 R2                             @MEA         */
#define cvtos390___r2 0x01     /* OS/390 R2                             @MEA         */
#define cvtparmc 0x01          /* Logical Parmlib Service is available               */

/* Values for field "cvtoslv2" */
#define cvtos390___010400 0x80 /* OS/390 R4                             @MEA         */
#define cvtos390___020400 0x80 /* OS/390 R4                             @PPA         */
#define cvtos390___r4 0x80     /* OS/390 R4                             @MEA         */
#define cvtj6604 0x80          /* OS/390 R4                             @MEA         */
#define cvtdylpa 0x80          /* Dynamic LPA (CSVDYLPA) available      @MEA         */
#define cvtrtls 0x80           /* Runtime Library Services (CSVRTLS)    @MEA         */
#define cvtos390___020500 0x40 /* OS/390 R5                             @MFA         */
#define cvtos390___r5 0x40     /* OS/390 R5                             @MFA         */
#define cvth6605 0x40          /* OS/390 R5                             @MFA         */
#define cvtos390___020600 0x20 /* OS/390 R6                             @MGA         */
#define cvtos390___r6 0x20     /* OS/390 R6                             @MGA         */
#define cvth6606 0x20          /* OS/390 R6                             @MGA         */
#define cvtbfp 0x10            /* Binary Floating Point support         @MGA         */
#define cvtos390___020700 0x08 /* OS/390 R7                             @MHA         */
#define cvtos390___r7 0x08     /* OS/390 R7                             @MHA         */
#define cvtj6607 0x08          /* OS/390 R7                             @MHA         */
#define cvtos390___020800 0x04 /* OS/390 R8                             @MIA         */
#define cvtos390___r8 0x04     /* OS/390 R8                             @MIA         */
#define cvth6608 0x04          /* OS/390 R8                             @MIA         */
#define cvtos390___020900 0x02 /* OS/390 R9                             @MJA         */
#define cvtos390___r9 0x02     /* OS/390 R9                             @MJA         */
#define cvtj6609 0x02          /* OS/390 R9                             @MJA         */
#define cvth6609 0x02          /* OS/390 R9                             @MJA         */
#define cvtos390___021000 0x01 /* OS/390 R10 functions are present      @PUC         */
#define cvtos390___r10 0x01    /* OS/390 R10 functions are present      @MLA         */
#define cvth7703 0x01          /* OS/390 R10 functions are present      @MLA         */

/* Values for field "cvtoslv3" */
#define cvtpause 0x80        /* Pause/Release services are present    @MKA         */
#define cvtpaus2 0x40        /* IEAVAPE2 and related services, and    @N5A         */
#define cvtzos 0x20          /* z/OS V1R1  functions are present      @MMA         */
#define cvtzos___010100 0x20 /* z/OS V1R1  functions are present      @MMA         */
#define cvtzos___v1r1 0x20   /* z/OS V1R1  functions are present      @MMA         */
#define cvtj7713 0x20        /* JBB7713                               @MMA         */
#define cvtlparc 0x20        /* LPAR Clustering is present.           @PWA         */
#define cvtzos___010200 0x10 /* z/OS V1R2  functions are present      @MQA         */
#define cvtzos___v1r2 0x10   /* z/OS V1R2  functions are present      @MQA         */
#define cvth7705 0x10        /* HBB7705    functions are present      @MQA         */
#define cvtv64 0x10          /* 64-bit virtual services are present.  @MQA         */
#define cvtzos___010300 0x08 /* z/OS V1R3  functions are present      @MSA         */
#define cvtzos___v1r3 0x08   /* z/OS V1R3  functions are present      @MSA         */
#define cvth7706 0x08        /* HBB7706    functions are present      @MSA         */
#define cvtzos___010400 0x04 /* z/OS V1R4  functions are present      @MUA         */
#define cvtzos___v1r4 0x04   /* z/OS V1R4  functions are present      @MUA         */
#define cvth7707 0x04        /* HBB7707    functions are present      @MUA         */
#define cvtzos___010500 0x02 /* z/OS V1R5  functions are present      @MVA         */
#define cvtzos___v1r5 0x02   /* z/OS V1R5  functions are present      @MVA         */
#define cvth7708 0x02        /* HBB7708    functions are present      @MVA         */
#define cvtzos___010600 0x01 /* z/OS V1R6  functions are present      @MWA         */
#define cvtzos___v1r6 0x01   /* z/OS V1R6  functions are present      @MWA         */
#define cvth7709 0x01        /* HBB7709    functions are present      @MWA         */

/* Values for field "cvtoslv4" */
#define cvtcsrsi 0x80 /* CSRSI service is available            @0EA         */
#define cvtunics 0x40 /* Unicode callable services available   @MLA         */
#define cvtcsrun 0x20 /* CSRUNIC callable service available    @MLA         */
#define cvtilm 0x10   /* IBM License Manager functions are     @PYM         */
#define cvtalrs 0x08  /* ASN-and-LX-Reuse architecture is      @MXA         */
#define cvttocp 0x04  /* TIMEUSED TIME_ON_CP                   @0KA         */
#define cvtziip 0x02  /* zIIP support is present               @H5A         */
#define cvtsup 0x02   /* zIIP support is present               @H5A         */
#define cvtifar 0x01  /* IFA routine is present                @H4A         */

/* Values for field "cvtoslv5" */
#define cvtzose 0x80                       /* z/OS.e                                @MTA         */
#define cvtzosas 0x80                      /* z/OS.e                                @MTA         */
#define cvtpuma 0x80                       /* z/OS.e                                @MTA         */
#define cvtzos___010700 0x40               /* z/OS V1R7  functions are present      @Q3C         */
#define cvtzos___v1r7 0x40                 /* z/OS V1R7  functions are present      @Q3C         */
#define cvth7720 0x40                      /* HBB7720    functions are present      @MYA         */
#define cvtzos___010800 0x20               /* z/OS V1R8  functions are present      @N2A         */
#define cvtzos___v1r8 0x20                 /* z/OS V1R8  functions are present      @N2A         */
#define cvth7730 0x20                      /* HBB7730    functions are present      @N2A         */
#define cvtzos___010900 0x10               /* z/OS V1R9  functions are present      @N6A         */
#define cvtzos___v1r9 0x10                 /* z/OS V1R9  functions are present      @N6A         */
#define cvth7740 0x10                      /* HBB7740    functions are present      @N6A         */
#define cvtzos___011000 0x08               /* z/OS V1R10 functions are present      @N8A         */
#define cvtzos___v1r10 0x08                /* z/OS V1R10 functions are present      @N8A         */
#define cvth7750 0x08                      /* HBB7750    functions are present      @N8A         */
#define cvtzos___011100 0x04               /* z/OS V1R11 functions are present      @NBA         */
#define cvtzos___v1r11 0x04                /* z/OS V1R11 functions are present      @NBA         */
#define cvt___g64cpu___infrastructure 0x04 /* G64CPU Infrastructure present  @NCA                */
#define cvth7760 0x04                      /* HBB7760    functions are present      @NBA         */
#define cvtzos___011200 0x02               /* z/OS V1R12 functions are present      @NFA         */
#define cvtzos___v1r12 0x02                /* z/OS V1R12 functions are present      @NFA         */
#define cvth7770 0x02                      /* HBB7770    functions are present      @NFA         */
#define cvtzos___011300 0x01               /* z/OS V1R13 functions are present      @NIA         */
#define cvtzos___v1r13 0x01                /* z/OS V1R13 functions are present      @NIA         */
#define cvth7780 0x01                      /* HBB7780    functions are present      @NIA         */

/* Values for field "cvtoslv6" */
#define cvtzos___020100 0x80           /* z/OS V2R1 functions are present       @NLC         */
#define cvtzos___v2r1 0x80             /* z/OS V2R1 functions are present       @NLC         */
#define cvth7790 0x80                  /* HBB7790   functions are present       @NKA         */
#define cvtzos___020200 0x40           /* z/OS V2R2 functions are present       @NQA         */
#define cvtzos___v2r2 0x40             /* z/OS V2R2 functions are present       @NQA         */
#define cvtpausemultiple 0x40          /* Pause Multiple                        @NYA         */
#define cvth77a0 0x40                  /* HBB77A0   functions are present       @NQA         */
#define cvtj778h 0x20                  /* JBB778H   functions are present       @NTA         */
#define cvtzos___v1r13___jbb778h 0x20  /* JBB778H functions are present       @NTA           */
#define cvtzos___011300___jbb778h 0x20 /* JBB778H                            @NTA            */
#define cvtzos___020300 0x10           /* z/OS V2R3 functions are present       @O1A         */
#define cvtzos___v2r3 0x10             /* z/OS V2R3 functions are present       @O1A         */
#define cvth77b0 0x10                  /* HBB77B0   functions are present       @O1A         */
#define cvtzos___020400 0x08           /* z/OS V2R4 functions are present       @O3A         */
#define cvtzos___v2r4 0x08             /* z/OS V2R4 functions are present       @O3A         */
#define cvth77c0 0x08                  /* HBB77C0   functions are present       @O3A         */
#define cvtzos___020500 0x04           /* z/OS V2R5 functions are present       @O6A         */
#define cvtzos___v2r5 0x04             /* z/OS V2R5 functions are present       @O6A         */
#define cvth77d0 0x04                  /* HBB77D0   functions are present       @O6A         */
#define cvtzos___030100 0x02           /* z/OS V3R1 functions are present       @O9A         */
#define cvtzos___v3r1 0x02             /* z/OS V3R1 functions are present       @O9A         */
#define cvth77e0 0x02                  /* HBB77E0   functions are present       @O9A         */

/* Values for field "cvtoslv8" */
#define cvtpaus3 0x80  /* IEA4xxxx                              @N9A         */
#define cvtpaus4 0x40  /* Pause with checkpoint-OK              @0MA         */
#define cvtect1 0x20   /* TIMEUSED ECT=YES with TIME_ON_CP,                  */
#define cvtoocp 0x10   /* TIMEUSED with TIME_ON_CP and                       */
#define cvtiefopz 0x08 /* IEFOPZ                                @NZA         */
#define cvtboost 0x04  /* Support for BOOST system parameter is              */

#endif

#ifndef __cvtvstgx__
#define __cvtvstgx__

struct cvtvstgx
{
  int cvtbldls;           /* -        RESERVED - WAS STARTING ADDRESS OF    @P3C */
  int cvtbldle;           /* -        RESERVED - WAS ENDING ADDRESS OF      @P3C */
  void *__ptr32 cvtmlpas; /* -        STARTING VIRTUAL ADDRESS OF MLPA. @G860PVB */
  void *__ptr32 cvtmlpae; /* -        ENDING VIRTUAL ADDRESS OF MLPA.   @G860PVB */
  void *__ptr32 cvtflpas; /* -        STARTING VIRTUAL ADDRESS OF FLPA. @G860PVB */
  void *__ptr32 cvtflpae; /* -        ENDING VIRTUAL ADDRESS OF FLPA.   @G860PVB */
  void *__ptr32 cvtplpas; /* -        STARTING VIRTUAL ADDRESS OF PLPA. @G860PVB */
  void *__ptr32 cvtplpae; /* -        ENDING VIRTUAL ADDRESS OF PLPA.   @G860PVB */
  void *__ptr32 cvtrwns;  /* -        STARTING VIRTUAL ADDRESS OF       @G860PVB */
  void *__ptr32 cvtrwne;  /* -        ENDING VIRTUAL ADDRESS OF         @G860PVB */
  void *__ptr32 cvtrons;  /* -        STARTING VIRTUAL ADDRESS OF       @G860PVB */
  void *__ptr32 cvtrone;  /* -        ENDING VIRTUAL ADDRESS OF         @G860PVB */
  void *__ptr32 cvterwns; /* -        STARTING EXTENDED ADDRESS         @G860P46 */
  void *__ptr32 cvterwne; /* -        ENDING EXTENDED ADDRESS           @G860P46 */
  void *__ptr32 cvteplps; /* -        Starting virtual address of       @G860PVB */
  void *__ptr32 cvteplpe; /* -        ENDING VIRTUAL ADDRESS OF         @G860PVB */
  void *__ptr32 cvteflps; /* -        STARTING VIRTUAL ADDRESS OF       @G860PVB */
  void *__ptr32 cvteflpe; /* -        ENDING VIRTUAL ADDRESS OF         @G860PVB */
  void *__ptr32 cvtemlps; /* -        STARTING VIRTUAL ADDRESS OF       @G860PVB */
  void *__ptr32 cvtemlpe; /* -        ENDING VIRTUAL ADDRESS OF         @G860PVB */
};

#endif

#ifndef __cvtxtnt1__
#define __cvtxtnt1__

struct cvtxtnt1
{
  void *__ptr32 cvtfachn;    /* -        ADDRESS OF CHAIN OF DCB FIELD AREAS */
  unsigned char cvt1r004[8]; /* RESERVED                              @LFC   */
};

#endif

#ifndef __cvtxtnt2__
#define __cvtxtnt2__

struct cvtxtnt2
{
  unsigned char cvt2r000[4]; /* RESERVED                              @LFC          */
  unsigned char cvtnucls;    /* -      IDENTIFICATION OF THE NUCLEUS MEMBER         */
  unsigned char cvtflgbt;    /* -     Flag byte. This byte is an interface only     */
  unsigned char cvtiocid[2]; /* -     EBCDIC IDENTIFIER OF THE ACTIVE I/O   @LBA    */
  void *__ptr32 cvtdebvr;    /* - ADDRESS OF BRANCH ENTRY POINT OF DEB              */
  void *__ptr32 cvtcvaf;     /* -        POINTER TO THE CVAF TABLE, WHICH CONTAINS  */
  void *__ptr32 cvtmmvt;     /* ADDRESS OF THE MEDIA MANAGER VECTOR TABLE           */
  void *__ptr32 cvtncvp;     /* ADDRESS OF CSA BUFFER POOL - USED BY                */
  struct
  {
    unsigned char _filler1;     /* -     RESERVED - FIRST BYTE OF CVTQID       @LFC */
    unsigned int _cvtqida : 24; /* -      ADDRESS OF QUEUE IDENTIFICATION (QID)     */
  } cvtqid;
  void *__ptr32 cvtoltep;    /* -        POINTER TO CONTROL BLOCK CREATED BY SVC 59 */
  unsigned char cvt2r020[4]; /* -     RESERVED                              @LFC    */
  struct
  {
    unsigned char _filler2[4]; /* @P5A */
  } cvtavvt;
  void *__ptr32 cvtccvt;     /* -        ADDRESS OF CRYPTOGRAPHIC FACILITY CVT      */
  void *__ptr32 cvtskta;     /* -        ADDRESS OF STORAGE KEY TABLE (VM           */
  void *__ptr32 cvticb;      /* -        ADDRESS OF MASS STORAGE SYSTEM (MSS)       */
  unsigned char cvtfbyt1;    /* -     FLAG BYTE                                     */
  unsigned char cvt2r035[3]; /* -     RESERVED                              @LFC    */
  struct
  {
    unsigned char _cvtldtol[4]; /* HIGH WORD                             @08A */
    unsigned char _cvtldtor[4]; /* LOW WORD                              @08A */
  } cvtldto;
  void *__ptr32 cvtatcvt;    /* -        POINTER TO VTAM'S CVT             @ZA65734 */
  unsigned char cvt2r044[4]; /* -     RESERVED                              @LFC    */
  int cvtbclmt;              /* -        NUMBER OF 130-BYTE RECORDS SET ASIDE  @LBA */
  int cvt2r04c;              /* RESERVED                              @0BA          */
  struct
  {
    unsigned char _cvtlsoh[4]; /* HIGH WORD                             @0BA */
    unsigned char _cvtlsol[4]; /* LOW WORD                              @0BA */
  } cvtlso;
  unsigned char cvt2r058[44]; /* RESERVED                              @0BC          */
};

#define cvtqida cvtqid._cvtqida
#define cvtldtol cvtldto._cvtldtol
#define cvtldtor cvtldto._cvtldtor
#define cvtlsoh cvtlso._cvtlsoh
#define cvtlsol cvtlso._cvtlsol

/* Values for field "cvtflgbt" */
#define cvtnpe 0x80      /* -       INDICATES NON-PAGING ENVIRONMENT (VM       */
#define cvtvme 0x40      /* -       INDICATES MACHINE IS OPERATING IN VM       */
#define cvtbah 0x20      /* -       INDICATES THAT THE VM/370 - OS/VS1 BTAM    */
#define cvtundzvm 0x10   /* -       Running under z/VM (this is not the same   */
#define cvtundvm 0x10    /* -       Same as CVTUNDzVM                     @0TA */
#define cvtvicom 0x08    /* -       Running under VICOM                        */
#define cvtzpdt 0x04     /* -       Running on zPDT (includes running     @O7A */
#define cvtundaltvm 0x02 /* -      Running under an alternate virtual machine  */
#define cvtvmenv 0x01    /* -       Running as a virtual machine environment.  */

/* Values for field "cvtavvt" */
#define cvtavin 0x80 /* INDICATES AVM INSTALLED               @LAA         */

/* Values for field "cvtfbyt1" */
#define cvtrde 0x80 /* -       RELIABILITY DATA EXTRACTOR INDICATOR  @D7A */

/* Values for field "cvtatcvt" */
#define cvtatact 0x80 /* IF ON, VTAM IS ACTIVE               MDC081         */

#endif

#pragma pack()
