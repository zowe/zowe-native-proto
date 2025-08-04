#pragma pack(packed)
 
#ifndef __tcbfix__
#define __tcbfix__
 
struct tcbfix {
  struct {
    double         _tcbfrs0; /* -     SAVE AREA FOR FLOATING POINT REGISTER 0 */
    double         _tcbfrs2; /* -     SAVE AREA FOR FLOATING POINT REGISTER 2 */
    double         _tcbfrs4; /* -     SAVE AREA FOR FLOATING POINT REGISTER 4 */
    double         _tcbfrs6; /* -     SAVE AREA FOR FLOATING POINT REGISTER 6 */
    } tcbfrs;
  };
 
#define tcbfrs0 tcbfrs._tcbfrs0
#define tcbfrs2 tcbfrs._tcbfrs2
#define tcbfrs4 tcbfrs._tcbfrs4
#define tcbfrs6 tcbfrs._tcbfrs6
 
/* Values for field "tcbfrs6" */
#define tcbpxlen 0x20 /* LENGTH OF PREFIX SECTION */
 
#endif
 
#ifndef __tcb__
#define __tcb__
 
struct tcb {
  void * __ptr32 tcbrbp;      /* -     ADDRESS OF THE RB FOR EXECUTING PROGRAM.  THIS   */
  void * __ptr32 tcbpie;      /* -     Address of current PIE/EPIE.  This field    @P8C */
  void * __ptr32 tcbdeb;      /* -     ADDRESS OF THE DEB QUEUE                         */
  void * __ptr32 tcbtio;      /* -     ADDRESS OF THE TASK I/O TABLE (TIOT)             */
  struct {
    unsigned char  _tcbcmpf;    /* -     INDICATOR FLAGS                        */
    unsigned char  _tcbcmpc[3]; /* -   SYSTEM COMPLETION CODE IN FIRST 12 BITS, */
    } tcbcmp;
  struct {
    unsigned char  _tcbabf;       /* -     FLAG BYTE                           */
    unsigned int   _tcbtrnb : 24; /* -   ADDRESS OF TESTRAN CONTROL CORE TABLE */
    } tcbtrn;
  struct {
    unsigned char  _filler1;      /* -     HIGH ORDER BYTE OF TCBMSS                   @H2C */
    unsigned int   _tcbmssb : 24; /* -   SAME AS TCBMSS                              @L6C   */
    } tcbmss;
  unsigned char  tcbpkf;      /* -     STORAGE PROTECTION KEY FOR THIS TASK.  IF THERE  */
  struct {
    unsigned char  _tcbflgs1; /* -     FIRST TCB FLAG BYTE                              */
    unsigned char  _tcbflgs2; /* -     SECOND FLAG BYTE                                 */
    unsigned char  _tcbflgs3; /* -     THIRD FLAG BYTE.                        @G381P9A */
    unsigned char  _tcbflgs4; /* -     NONDISPATCHABILITY FLAGS                         */
    unsigned char  _tcbflgs5; /* -     MORE NONDISPATCHABILITY FLAGS.   IF ANY BIT IN   */
    } tcbflgs;
  char           tcblmp;      /* -   TASK LIMIT PRIORITY                                */
  char           tcbdsp;      /* -   DISPATCHING PRIORITY FOR THIS TASK                 */
  void * __ptr32 tcblls;      /* -     ADDRESS OF LAST LOAD LIST ELEMENT (LLE) IN LOAD  */
  void * __ptr32 tcbjlb;      /* -     ADDRESS OF A JOBLIB DCB                          */
  struct {
    unsigned char  _tcbpurge;     /* -     HIGH ORDER BYTE                         @G860PXB */
    unsigned int   _tcbjpqb : 24; /* -   LOW ORDER 24-BITS OF TCBJPQ             @G860PXB   */
    } tcbjpq;
  struct {
    int            _tcbgrs0;  /* -     SAVE AREA FOR GENERAL REGISTER 0  */
    int            _tcbgrs1;  /* -     SAVE AREA FOR GENERAL REGISTER 1  */
    int            _tcbgrs2;  /* -     SAVE AREA FOR GENERAL REGISTER 2  */
    int            _tcbgrs3;  /* -     SAVE AREA FOR GENERAL REGISTER 3  */
    int            _tcbgrs4;  /* -     SAVE AREA FOR GENERAL REGISTER 4  */
    int            _tcbgrs5;  /* -     SAVE AREA FOR GENERAL REGISTER 5  */
    int            _tcbgrs6;  /* -     SAVE AREA FOR GENERAL REGISTER 6  */
    int            _tcbgrs7;  /* -     SAVE AREA FOR GENERAL REGISTER 7  */
    int            _tcbgrs8;  /* -     SAVE AREA FOR GENERAL REGISTER 8  */
    int            _tcbgrs9;  /* -     SAVE AREA FOR GENERAL REGISTER 9  */
    int            _tcbgrs10; /* -     SAVE AREA FOR GENERAL REGISTER 10 */
    int            _tcbgrs11; /* -     SAVE AREA FOR GENERAL REGISTER 11 */
    int            _tcbgrs12; /* -     SAVE AREA FOR GENERAL REGISTER 12 */
    int            _tcbgrs13; /* -     SAVE AREA FOR GENERAL REGISTER 13 */
    int            _tcbgrs14; /* -     SAVE AREA FOR GENERAL REGISTER 14 */
    int            _tcbgrs15; /* -     SAVE AREA FOR GENERAL REGISTER 15 */
    } tcbgrs;
  struct {
    char           _filler2;      /* -   FIRST BYTE OF TCBFSA                    @G381P9A */
    unsigned int   _tcbfsab : 24; /* -   ADDRESS OF THE FIRST PROBLEM PROGRAM SAVE AREA   */
    } tcbfsa;
  void * __ptr32 tcbtcb;      /* -     Queue of TCBs in an address space.          @L5C */
  void * __ptr32 tcbtme;      /* -     ADDRESS OF THE TIMER QUEUE ELEMENT (TQE)         */
  struct {
    unsigned char  _filler3;       /* -     HIGH ORDER BYTE OF TCBJSTCB             @G860PXB */
    unsigned int   _tcbjstca : 24; /* -   LOW ORDER 24 BITS OF TCBJSTCB           @G860PXB   */
    } tcbjstcb;
  void * __ptr32 tcbntc;      /* -     ADDRESS OF THE TCB FOR THE TASK PREVIOUSLY       */
  void * __ptr32 tcbotc;      /* -     ADDRESS OF THE TCB FOR THE TASK (THE ORIGINATING */
  void * __ptr32 tcbltc;      /* -     ADDRESS OF THE TCB FOR THE TASK LAST ATTACHED BY */
  void * __ptr32 tcbiqe;      /* -     ADDRESS OF AN INTERRUPTION QUEUE ELEMENT (IQE)   */
  void * __ptr32 tcbecb;      /* -     ADDRESS OF THE ECB THAT WILL BE POSTED BY THE    */
  unsigned char  tcbtsflg;    /* -     TIME SHARING FLAGS                               */
  char           tcbstpct;    /* -   NUMBER OF SETTASK STARTS WHICH MUST BE             */
  char           tcbtslp;     /* -   LIMIT PRIORITY OF TIME SHARING TASK                */
  unsigned char  tcbtsdp;     /* -     DISPATCHING PRIORITY OF TIME SHARING TASK        */
  void * __ptr32 tcbrd;       /* POINTER TO DPQE MINUS 8 FOR THE JOB STEP               */
  void * __ptr32 tcbae;       /* LIST ORIGIN OF AQE(S) FOR THIS TASK                    */
  struct {
    unsigned char  _tcbnstae;      /* -     FLAGS INTERNAL TO STAE ROUTINE          */
    unsigned int   _tcbstabb : 24; /* -   ADDRESS OF THE CURRENT STAE CONTROL BLOCK */
    } tcbstab;
  struct {
    unsigned char  _tcbtctgf;     /* -     FLAG BYTE FOR TIMING CONTROL TABLE        ICB318 */
    unsigned int   _tcbtctb : 24; /* -   ADDRESS OF THE TIMING CONTROL TABLE (TCT) IF       */
    } tcbtct;
  void * __ptr32 tcbuser;     /* -     A WORD AVAILABLE TO THE USER                     */
  struct {
    struct {
      unsigned char  _tcbndsp0; /* -     BYTE 0 */
      unsigned char  _tcbndsp1; /* -     BYTE 1 */
      unsigned char  _tcbndsp2; /* -     BYTE 2 */
      unsigned char  _tcbndsp3; /* -     BYTE 3 */
      } tcbndsp;
    } tcbscndy;
  int            tcbmdids;    /* -     RESERVED FOR MODEL-DEPENDENT SUPPORT AND FOR     */
  struct {
    unsigned char  _tcbrecde;      /* -     ABEND RECURSION BYTE                      ICB456 */
    unsigned int   _tcbjscbb : 24; /* -   ADDRESS OF THE JOB STEP CONTROL BLOCK              */
    } tcbjscb;
  void * __ptr32 tcbssat;     /* -     ADDRESS OF THE SUBSYSTEM AFFINITY TABLE @G381P9A */
  void * __ptr32 tcbiobrc;    /* -     ADDRESS OF IOB RESTORE CHAIN FOR I/O QUIESCED    */
  void * __ptr32 tcbexcpd;    /* -     ADDRESS OF EXCP DEBUG AREA                YM4297 */
  struct {
    unsigned char  _filler4;       /* -     RESERVED.                                   @H2C */
    unsigned int   _tcbext1a : 24; /* -   ADDRESS OF OS-OS/VS COMMON TCB EXTENSION  ICB311   */
    } tcbext1;
  struct {
    unsigned char  _tcbndsp4; /* -     SECONDARY NONDISPATCHABILITY FLAGS COMMON TO */
    unsigned char  _tcbndsp5; /* -     SECONDARY NONDISPATCHABILITY FLAGS UNIQUE TO */
    unsigned char  _tcbflgs6; /* -     TASK-RELATED FLAGS                           */
    unsigned char  _tcbflgs7; /* -     TASK-RELATED FLAGS                           */
    } tcbbits;
  unsigned char  tcbdar;      /* -     DAMAGE ASSESSMENT ROUTINE (DAR) FLAGS            */
  unsigned char  tcbrsv37;    /* -     RESERVED FOR USER                                */
  char           tcbsysct;    /* -   NUMBER OF OUTSTANDING SYSTEM-MUST-COMPLETE         */
  char           tcbstmct;    /* -   NUMBER OF OUTSTANDING STEP-MUST-COMPLETE           */
  struct {
    unsigned char  _filler5;       /* -     FIRST BYTE OF TCBEXT2                   @G381P9A */
    unsigned int   _tcbext2a : 24; /* -   ADDRESS OF OS/VS1 - OS/VS2 COMMON EXTENSION        */
    } tcbext2;
  int            tcbr0d4;     /* -     Reserved                                    @PBC */
  void * __ptr32 tcbxsb;      /* -     ADDRESS OF CURRENT XSB FOR TASK.        @G381P9A */
  void * __ptr32 tcbback;     /* -     ADDRESS OF PREVIOUS TCB ON READY QUEUE.  ZERO    */
  void * __ptr32 tcbrtwa;     /* -     POINTER TO CURRENT RTM2 WORK AREA                */
  void * __ptr32 tcbnssp;     /* -     NORMAL STACK SAVE AREA POINTER.                  */
  void * __ptr32 tcbxlas;     /* -     ASCB ADDRESS OF THE CML LOCK HELD WHILE @G381PXU */
  unsigned char  tcbabcur;    /* -     ABEND RECURSION BYTE                             */
  char           tcbfjmct;    /* -   NUMBER OF OUTSTANDING STEP-MUST-COMPLETE@YA02725   */
  unsigned char  tcbtid;      /* -     The task identifier as specified on the     @P7C */
  unsigned char  tcbflgs8;    /* -     GUPI FLAG BYTE FIELD                        @P1A */
  struct {
    unsigned char  _tcbxsct1; /* -     FLAG BYTE                               @Z40FP9A */
    unsigned char  _tcbxsct2; /* -     FLAG BYTE                               @G381P9A */
    short int      _tcbccpvi; /* -     ID OF THE CURRENT CPU RUNNING THIS TASK.  USED   */
    } tcbxsct;
  struct {
    unsigned char  _filler6;      /* -     RESERVED.                                   @H2C */
    unsigned int   _tcbfoea : 24; /* -   ADDRESS OF FIRST FIX OWNERSHIP ELEMENT (FOE)       */
    } tcbfoe;
  void * __ptr32 tcbswa;      /* -     ADDRESS OF FIRST SCHEDULER WORK AREA (SWA) SPQE  */
  void * __ptr32 tcbstawa;    /* -     ESTAE ROUTINE WORK AREA POINTER         @Z40FP9A */
  unsigned char  tcbtcbid[4]; /* -   CONTAINS BLOCK ID - 'TCB '                         */
  void * __ptr32 tcbrtm12;    /* -     POINTER TO PARAMETER AREAS PASSED FROM RTM1 TO   */
  struct {
    unsigned char  _tcbscbky; /* -     KEY IN WHICH SYNCH IS TO PASS CONTROL          */
    unsigned char  _tcbestrm; /* -     ESTAE TERM OPTIONS                             */
    char           _tcbertyp; /* -   TYPE OF ERROR CAUSING ENTRY TO THE RTM.  SET     */
    char           _tcbmode;  /* -   MASK INDICATING MODE OF SYSTEM AT TIME OF ERROR. */
    } tcbestae;
  void * __ptr32 tcbukysp;    /* -     ADDRESS OF SPQE'S FOR SUBPOOLS 229 AND 230       */
  struct {
    unsigned char  _tcbprop0; /* -  Byte 0 of TCBPROPF                         @L9A */
    unsigned char  _tcbprop1; /* -  Byte 1 of TCBPROPF                         @L9A */
    } tcbpropf;
  unsigned char  tcbaffn[2];  /* -   CPU AFFINITY INDICATOR                             */
  unsigned char  tcbfbyt1;    /* -     FLAG BYTE.                              @G381P9A */
  unsigned char  tcbfbyt2;    /* -     FLAG BYTE.                              @G381P9A */
  unsigned char  tcbfbyt3;    /* -     FLAG BYTE                               @ZA72701 */
  unsigned char  tcbfbyt4;    /* -     FLAG BYTE.                                  @L3A */
  void * __ptr32 tcbrpt;      /* -     ADDRESS OF RADIX PARTITION TREE FOR LOCAL        */
  void * __ptr32 tcbvat;      /* -     ADDRESS OF THE VAT (VSAM). THERE IS ONE @ZA63673 */
  void * __ptr32 tcbswasa;    /* -     ADDRESS OF SAVE AREA USED BY SWA MANAGER         */
  void * __ptr32 tcbsvca2;    /* -     ADDRESS OF SVC SCREENING TABLE          @G50EP9A */
  void * __ptr32 tcberd;      /* EXTENDED REGION DESCRIPTOR.             @G860PXU       */
  void * __ptr32 tcbeae;      /* EXTENDED ALLOCATED ELEMENT QUEUE        @G860PXU       */
  void * __ptr32 tcbarc;      /* REASON CODE FOR ABEND                   @G860PXB       */
  int            tcbgres;     /* -     TASK GLOBAL RESOURCE COUNT - NUMBER OF GLOBAL    */
  void * __ptr32 tcbstcb;     /* ADDRESS OF STCB                             @H2C       */
  unsigned char  tcbttime[8]; /* -  TCB'S ACCUMULATED CPU TIME                  @01A    */
  void * __ptr32 tcbcelap;    /* -  COMMON EXECUTION LIBRARY ANCHOR POINTER     @02A    */
  unsigned char  tcbr148[2];  /* -  RESERVED                                    @05C    */
  unsigned char  tcbrbyt1;    /* -     RTM Flag byte                               @05A */
  unsigned char  tcblevel;    /* -     LEVEL NUMBER OF TCB                     @ZA72701 */
  void * __ptr32 tcbbdt;      /* -     ADDRESS OF BDT'S GSD LINKAGE            @ZA72701 */
  int            tcbndaxp;    /* -     COUNT OF NUMBER OF CONSECUTIVE DISPATCHES   @H1A */
  void * __ptr32 tcbsenv;     /* -     ADDRESS OF ACEE FOR THE TASK.               @H1A */
  };
 
#define tcbcmpf  tcbcmp._tcbcmpf
#define tcbcmpc  tcbcmp._tcbcmpc
#define tcbabf   tcbtrn._tcbabf
#define tcbtrnb  tcbtrn._tcbtrnb
#define tcbmssb  tcbmss._tcbmssb
#define tcbflgs1 tcbflgs._tcbflgs1
#define tcbflgs2 tcbflgs._tcbflgs2
#define tcbflgs3 tcbflgs._tcbflgs3
#define tcbflgs4 tcbflgs._tcbflgs4
#define tcbflgs5 tcbflgs._tcbflgs5
#define tcbpurge tcbjpq._tcbpurge
#define tcbjpqb  tcbjpq._tcbjpqb
#define tcbgrs0  tcbgrs._tcbgrs0
#define tcbgrs1  tcbgrs._tcbgrs1
#define tcbgrs2  tcbgrs._tcbgrs2
#define tcbgrs3  tcbgrs._tcbgrs3
#define tcbgrs4  tcbgrs._tcbgrs4
#define tcbgrs5  tcbgrs._tcbgrs5
#define tcbgrs6  tcbgrs._tcbgrs6
#define tcbgrs7  tcbgrs._tcbgrs7
#define tcbgrs8  tcbgrs._tcbgrs8
#define tcbgrs9  tcbgrs._tcbgrs9
#define tcbgrs10 tcbgrs._tcbgrs10
#define tcbgrs11 tcbgrs._tcbgrs11
#define tcbgrs12 tcbgrs._tcbgrs12
#define tcbgrs13 tcbgrs._tcbgrs13
#define tcbgrs14 tcbgrs._tcbgrs14
#define tcbgrs15 tcbgrs._tcbgrs15
#define tcbfsab  tcbfsa._tcbfsab
#define tcbjstca tcbjstcb._tcbjstca
#define tcbnstae tcbstab._tcbnstae
#define tcbstabb tcbstab._tcbstabb
#define tcbtctgf tcbtct._tcbtctgf
#define tcbtctb  tcbtct._tcbtctb
#define tcbndsp0 tcbscndy.tcbndsp._tcbndsp0
#define tcbndsp1 tcbscndy.tcbndsp._tcbndsp1
#define tcbndsp2 tcbscndy.tcbndsp._tcbndsp2
#define tcbndsp3 tcbscndy.tcbndsp._tcbndsp3
#define tcbrecde tcbjscb._tcbrecde
#define tcbjscbb tcbjscb._tcbjscbb
#define tcbext1a tcbext1._tcbext1a
#define tcbndsp4 tcbbits._tcbndsp4
#define tcbndsp5 tcbbits._tcbndsp5
#define tcbflgs6 tcbbits._tcbflgs6
#define tcbflgs7 tcbbits._tcbflgs7
#define tcbext2a tcbext2._tcbext2a
#define tcbxsct1 tcbxsct._tcbxsct1
#define tcbxsct2 tcbxsct._tcbxsct2
#define tcbccpvi tcbxsct._tcbccpvi
#define tcbfoea  tcbfoe._tcbfoea
#define tcbscbky tcbestae._tcbscbky
#define tcbestrm tcbestae._tcbestrm
#define tcbertyp tcbestae._tcbertyp
#define tcbmode  tcbestae._tcbmode
#define tcbprop0 tcbpropf._tcbprop0
#define tcbprop1 tcbpropf._tcbprop1
 
/* Values for field "tcbcmpf" */
#define tcbcreq                          0x80 /* - A DUMP HAS BEEN REQUESTED                          */
#define tcbcstep                         0x40 /* - A STEP ABEND HAS BEEN REQUESTED                    */
#define tcbcpp                           0x20 /* - SOME PROBLEM PROGRAM STORAGE WAS OVERLAID BY THE   */
#define tcbdmpo                          0x20 /* - DUMP OPTIONS WERE PROVIDED ON CALLRTM OR SETRP     */
#define tcbstcc                          0x10 /* - COMPLETION CODE IS NOT TO BE STORED IN TCBCMPC     */
#define tcbnocc                          0x10 /* - A COMPLETION CODE WAS NOT PROVIDED ON CALLRTM      */
#define tcbcdbl                          0x08 /* - A DOUBLE ABEND HAS OCCURRED (OS/VS1)               */
#define tcbcasid                         0x08 /* - ABEND WAS SCHEDULED VIA CROSS MEMORY ABTERM        */
#define tcbcwto                          0x04 /* - A DUMP MESSAGE (WTO) IS TO BE ISSUED TO THE        */
#define tcbarcv                          0x04 /* - INDICATES REASON CODE (TCBARC) IS VALID            */
#define tcbrv316                         0x04 /* - Same as TCBARCV                                    */
#define tcbcind                          0x02 /* - ABEND TO OUTPUT AN INDICATIVE DUMP  (OS/VS1)       */
#define tcbcmsg                          0x01 /* - AN ABEND MESSAGE IS PROVIDED TO BE PRINTED BY      */
 
/* Values for field "tcbabf" */
#define tcbmod91                         0x80 /* - BOTH TESTRAN AND DECIMAL SIMULATOR ON A MOD 91     */
#define tcbnochk                         0x40 /* - SUPPRESS TAKING CHECKPOINTS FOR THIS STEP          */
#define tcbgrph                          0x20 /* - GAM/SP ACTIVE FOR THIS TASK                        */
#define tcb___refrprot___override        0x10 /* - Even if the REFRPROT option is                     */
#define tcbtcpp                          0x08 /* - TCAM POST-PENDING  (RORI)                          */
#define tcbtcp                           0x04 /* - TEST TASK - USED BY TEST SVC                       */
#define tcboltep                         0x02 /* - OLTEP FUNCTIONS REQUIRE CLEANUP BEFORE ABNORMAL    */
#define tcbdfrbp                         0x01 /* - Issue SVC 61 upon fetch. Set only when      @P9A   */
 
/* Values for field "tcbpkf" */
#define tcbpkfu                          0x80 /* When on, user key                           @LIA     */
#define tcbflag                          0xF0 /* - STORAGE PROTECTION KEY                             */
#define tcbzero                          0x0F /* - MUST BE ZERO                                       */
 
/* Values for field "tcbflgs1" */
#define tcbfa                            0x80 /* - May be on when a task is being abnormally   @LHC   */
#define tcbfe                            0x40 /* - On when the system is calling resource      @L6C   */
#define tcbfera                          0x20 /* - ENTER ABEND ERASE ROUTINE WHEN IN CONTROL AGAIN    */
#define tcbnonpr                         0x10 /* - TASK IS NON-PREEMPTABLE                            */
#define tcbpdump                         0x08 /* - PREVENT DUMP INDICATOR                             */
#define tcbft                            0x04 /* - TOP TASK IN TREE BEING ABTERMED                    */
#define tcbfs                            0x02 /* - ABTERM DUMP COMPLETED                              */
#define tcbfx                            0x01 /* - PROHIBIT QUEUEING OF ASYNCHRONOUS EXITS FOR        */
 
/* Values for field "tcbflgs2" */
#define tcbfoinp                         0x80 /* - THE TASK IS ABENDING AND IS IN THE PROCESS OF      */
#define tcbfsti                          0x40 /* - SECOND JOB STEP INTERVAL HAS EXPIRED               */
#define tcbfabop                         0x20 /* - IF 1, THE SYSABEND DUMP DATA SET HAS BEEN OPENED   */
#define tcbfsmc                          0x10 /* - TASK HAS ISSUED A SYSTEM-MUST-COMPLETE AND SET     */
#define tcbfjmc                          0x08 /* - TASK HAS ISSUED A STEP-MUST-COMPLETE AND TURNED    */
#define tcbfdsop                         0x04 /* - SYSABEND OPEN FOR JOB STEP                         */
#define tcbfetxr                         0x02 /* - ETXR TO BE SCHEDULED                               */
#define tcbfts                           0x01 /* - THIS TASK IS A MEMBER OF A TIME-SLICED GROUP       */
 
/* Values for field "tcbflgs3" */
#define tcbfsm                           0x80 /* - ALL PSW'S IN SUPERVISOR STATE                      */
#define tcbrt1s                          0x40 /* - RTM1 HAS INVOKED SLIP FOR A TASK IN EUT MODE.      */
#define tcbabtrm                         0x20 /* - ABTERM BIT TO PREVENT MULTIPLE ABENDS              */
#define tcbfxset                         0x10 /* - TCBFX WAS SET BY STATUS MCSTEP              @04A   */
#define tcbkey9                          0x04 /* - TCB was attached using KEY=NINE and so is to be    */
#define tcbenqrm                         0x02 /* - ENQ/DEQ RESOURCE MANAGER HAS RECEIVED   @G381P9A   */
#define tcbdwsta                         0x01 /* - THIS TASK WAS DETACHED WITH STAE=YES OPTION        */
 
/* Values for field "tcbflgs4" */
#define tcbndump                         0x80 /* - ABDUMP NONDISPATCHABILITY INDICATOR                */
#define tcbser                           0x40 /* - SER1 NONDISPATCHABILITY INDICATOR                  */
#define tcbrqena                         0x20 /* - I/O RQE'S EXHAUSTED                                */
#define tcbhndsp                         0x10 /* - TASK OR JOB STEP IS MOMENTARILY 'FROZEN' UNTIL     */
#define tcbuxndv                         0x08 /* - TASK IS TEMPORARILY NONDISPATCHABLE BECAUSE        */
#define tcbrbwf                          0x04 /* - TOP RB IS IN WAIT STATE                 @G50DP9A   */
#define tcbondsp                         0x01 /* - TASK TERMINATING AND NONDISPATCHABLE BECAUSE       */
 
/* Values for field "tcbflgs5" */
#define tcbfc                            0x80 /* - TASK TERMINATED                                    */
#define tcbabwf                          0x40 /* - ABNORMAL WAIT                                      */
#define tcbuxndf                         0x40 /* - TASK IS TEMPORARILY NONDISPATCHABLE BECAUSE        */
#define tcbpage                          0x20 /* - TASK IS NONDISPATCHABLE DUE TO EXCESSIVE PAGING    */
#define tcbandsp                         0x10 /* - TASK IS TEMPORARILY NONDISPATCHABLE BECAUSE IT     */
#define tcbsys                           0x08 /* - ANOTHER TASK IS IN SYSTEM-MUST-COMPLETE STATUS     */
#define tcbstp                           0x04 /* - ANOTHER TASK IN THIS JOB STEP IS IN                */
#define tcbfcd1                          0x02 /* - INITIATOR WAITING FOR REGION                       */
#define tcbpndsp                         0x01 /* - PRIMARY NONDISPATCHABILITY BIT.   THIS BIT IS      */
 
/* Values for field "tcbpurge" */
#define tcbjpqf                          0x80 /* - JPQ PURGE FLAG                                     */
 
/* Values for field "tcbtme" */
#define tcbtqet                          0x80 /* - IF ZERO, TASK TYPE TQE.                            */
 
/* Values for field "tcbtsflg" */
#define tcbtstsk                         0x80 /* - SWAPPED TIME SHARING TASK (OS/VS1)                 */
#define tcbstppr                         0x40 /* - TASK SHOULD BE MADE NONDISPATCHABLE VIA TCBSTPP    */
#define tcbatt                           0x20 /* - TASK SHOULD NOT HAVE ATTENTION EXITS SCHEDULED     */
#define tcbtiotg                         0x10 /* - PURGE TGET/TPUT AFTER ATTENTION                    */
#define tcbsmcp                          0x08 /* - STATUS MCSTEP PENDING,TASK IS SUSPENDED     @L4A   */
#define tcbdydsp                         0x02 /* - M195 TASK IS MEMBER OF DYNAMIC DISPATCHING         */
#define tcbcpubn                         0x01 /* - FOR M195, ZERO MEANS I/O BOUND AND ONE MEANS       */
 
/* Values for field "tcbnstae" */
#define tcbstabe                         0x80 /* - ABEND ENTERED BECAUSE OF ERROR IN STAE             */
#define tcbquies                         0x40 /* - STAE INVOKED PURGE I/O ROUTINE WITH QUIESCE I/O    */
#define tcb33e                           0x20 /* - A 33E ABEND HAS OCCURRED FOR TASK                  */
#define tcbppsup                         0x10 /* - 1=SUPERVISOR MODE,0=PROBLEM PROGRAM MODE           */
#define tcbhalt                          0x08 /* - PURGE I/O ROUTINE DID NOT SUCCESSFULLY QUIESCE     */
#define tcbsynch                         0x04 /* - SYNCH ISSUED BY ASIR TO SCHEDULE EXIT ROUTINE      */
#define tcbstcur                         0x01 /* - STAE RECURSION VALID                      ICB443   */
 
/* Values for field "tcbtctgf" */
#define tcbsmfgf                         0x80 /* - IF ZERO, THE TCT CORE TABLE IS NOT TO BE           */
 
/* Values for field "tcbndsp0" */
#define tcbndgsf                         0x02 /* Temporarily nondispatchable for GSF         @LKA     */
#define tcbndjl                          0x01 /* Join/Leave processing                       @0EA     */
 
/* Values for field "tcbndsp1" */
#define tcbdartn                         0x80 /* - THE TASK IS TEMPORARILY NONDISPATCHABLE -          */
#define tnondisp                         0x80 /* * ALIAS *                                            */
#define tcbdarpn                         0x40 /* - THE TASK IS PERMANENTLY NONDISPATCHABLE -          */
#define pnondisp                         0x40 /* * ALIAS *                                            */
#define tcbrstnd                         0x20 /* - THE TASK IS TEMPORARILY NONDISPATCHABLE -          */
#define tcbrspnd                         0x10 /* - THE TASK IS PERMANENTLY NONDISPATCHABLE -          */
#define tcbddrnd                         0x08 /* - THE TASK IS IN DEVICE ALLOCATION AND DYNAMIC       */
#define tcbtpsp                          0x04 /* - DISPATCHING OF TCAM TASK MUST BE DELAYED UNTIL     */
#define tcbpiend                         0x02 /* - SRB IS TO BE SCHEDULED TO PERFORM PIE/PICA         */
#define tcbabtin                         0x01 /* - THE TASK IS TEMPORARILY NONDISPATCHABLE WHILE      */
 
/* Values for field "tcbndsp2" */
#define tcbabd                           0x80 /* - ABDUMP IS PROCESSING  (OS/VS1)                     */
#define tcbstpp                          0x40 /* - TASK SET NONDISPATCHABLE BY SETTASK                */
#define tcbndsvc                         0x20 /* - TASK IS NONDISPATCHABLE BECAUSE SVC DUMP IS        */
#define tcbndts                          0x10 /* - TASK IS NONDISPATCHABLE BECAUSE IT IS BEING        */
#define tcbiwait                         0x08 /* - TASK IS NONDISPATCHABLE DUE TO AN INPUT WAIT       */
#define tcbowait                         0x04 /* - TASK IS NONDISPATCHABLE DUE TO AN OUTPUT WAIT      */
#define tcbdss                           0x02 /* - DYNAMIC SUPPORT SYSTEM (DSS) HAS SET THIS          */
#define tcbabe                           0x01 /* - ABEND ROUTINE WAS ENTERED FOR THIS TASK WHILE      */
 
/* Values for field "tcbndsp3" */
#define tcbljsnd                         0x80 /* - TASK IS ABENDING AND NONDISPATCHABLE BECAUSE IT    */
#define tcbndnyi                         0x40 /* - RCT TASK IS NONDISPATCHABLE BECAUSE ADDRESS @P4A   */
#define tcbsrbnd                         0x20 /* - TCB NONDISPATCHABLE BECAUSE SRB'S ARE STOPPED      */
#define tcbslper                         0x10 /* - SET NONDISPATCHABLE SO THAT SLIP/PER CAN ALTER     */
#define tcbs3mr                          0x08 /* - STAGE 3 EXIT EFFECTOR MUST RUN TO SYNCHRONIZE      */
#define tcbareq                          0x04 /* - TSO AUTHORIZED REQUEST PROCESSING ACTIVE           */
#define tcbndsmf                         0x02 /* - Indicates task is stopped because its CPU   @03A   */
#define tcbndint                         0x01 /* - INITIATOR SETS THIS BIT TO PREVENT JOB STEP        */
 
/* Values for field "tcbrecde" */
#define tcbrec                           0x80 /* - VALID REENTRY TO ABEND IF NON-ZERO VALUE IN        */
#define tcbopen                          0x01 /* - OPEN DUMP DATA SET                        ICB456   */
#define tcbclosd                         0x02 /* - CLOSE DIRECT SYSOUT ON TAPE               ICB456   */
#define tcbclose                         0x03 /* - CLOSE OPEN DATA SETS                      ICB456   */
#define tcbclosf                         0x04 /* - RESERVED.                                 ICB456   */
#define tcbgrec                          0x05 /* - GRAPHICS                                  ICB456   */
#define tcbadump                         0x07 /* - ABDUMP                                    ICB456   */
#define tcbptaxe                         0x08 /* - PURGE TAXE                                ICB456   */
#define tcbmesg                          0x09 /* - MESSAGE RECURSION                         ICB456   */
#define tcbdynam                         0x0A /* - DD-DYNAM TIOT CLEANUP                     ICB456   */
#define tcbdamsg                         0x0B /* - ABEND IS ISSUING A WTOR ASKING WHETHER THE JOB     */
#define tcbqtip                          0x0C /* - PURGE TSO INTERPARTITION POSTS            ICB456   */
#define tcbtcamp                         0x0D /* - PURGE TCAM INTERPARTITION POSTS           ICB456   */
#define tcbindrc                         0x0E /* - INDICATIVE DUMP (LOAD 8 OF ABEND) HAS ABENDED.     */
#define tcbsavcd                         0x0F /* - ASIR RECURSION.  SAVE OLD COMPLETION CODE ICB456   */
#define tcbtyp1w                         0x10 /* - TYPE 1 MESSAGE WRITE TO PROGRAMMER        ICB456   */
#define tcbwtpse                         0x20 /* - WRITE-TO-PROGRAMMER (WTP) FAILED.  JOB STEP        */
#define tcbvtam1                         0x21 /* - ABEND IS ENTERING FIRST VTAM INTERFACE,            */
#define tcbvtam2                         0x22 /* - ABEND IS ENTERING SECOND VTAM INTERFACE,           */
#define tcbvtam3                         0x23 /* - ABEND IS ENTERING FIRST VTAM INTERFACE,            */
#define tcbvtam4                         0x24 /* - ABEND IS ENTERING SECOND VTAM INTERFACE,           */
#define tcbnosta                         0x30 /* - STAE/STAI NOT TO BE HONORED               ICB456   */
#define tcbstret                         0x31 /* - RETURN FROM DUMP PROCESSING               ICB456   */
#define tcbconvr                         0x32 /* - CONVERT TO STEP ABEND                     ICB456   */
#define tcbdaret                         0x33 /* - RETURN FROM DAMAGE ASSESSMENT ROUTINES    ICB456   */
#define tcbtyp1r                         0x34 /* - RETURN FROM TYPE 1 MESSAGE MODULE         ICB456   */
#define tcbnewrb                         0x35 /* - ABEND ISSUED SVC 13 TO TRANSFER CONTROL (XCTL)     */
#define tcbmccns                         0x40 /* - A MUST COMPLETE TASK HAS ABNORMALLY TERMINATED     */
 
/* Values for field "tcbflgs6" */
#define tcbrv                            0x80 /* - THE PARTITION IS FIXED IN REAL STORAGE.  VIRTUAL   */
#define tcbpie17                         0x40 /* - PAGE FAULT INTERRUPT IS TO BE PASSED TO THE        */
#define tcbcpu                           0x20 /* - TASK IS CPU-BOUND MEMBER OF AUTOMATIC PRIORITY     */
#define tcbspvlk                         0x10 /* - TASK SCHEDULED FOR ABTERM WHILE OWNING             */
#define tcbhcrm                          0x08 /* - Health Checker has established a task term         */
#define tcbmigr                          0x04 /* - REGION SELECTED FOR MIGRATION FROM PRIMARY         */
#define tcbapg                           0x02 /* - TASK IS IN AUTOMATIC PRIORITY GROUP (APG)          */
#define tcbntjs                          0x01 /* - JOB STEP TASK BUT NOT HIGHEST IN FAILING TREE      */
 
/* Values for field "tcbflgs7" */
#define tcbgpecb                         0x80 /* - TASK IS IN AN ECB WAIT FOR A GETPART               */
#define tcbsenvp                         0x40 /* -                                             @LCA   */
#define tcbsvcsp                         0x20 /* - IF 1, SVC SCREENING IS TO BE PROPAGATED TO         */
#define tcbstack                         0x10 /* - SET IN JOB STEP TCB TO INDICATE THAT A TASK IN     */
#define tcbsvcs                          0x08 /* - IF 1, SVC SCREENING IS REQUIRED FOR THE TASK.      */
#define tcbrstsk                         0x04 /* - RESIDENT SYSTEM TASK                               */
#define tcbadmp                          0x02 /* - ALL OTHER TASKS IN JOB STEP HAVE BEEN SET          */
#define tcbgtofm                         0x01 /* - GENERALIZED TRACE FACILITY (GTF) TRACING HAS       */
 
/* Values for field "tcbdar" */
#define tcbdarp                          0x80 /* - PRIMARY DAR RECURSION.  DAR HAS BEEN ENTERED FOR   */
#define tcbdars                          0x40 /* - SECONDARY DAR RECURSION.  IF DAR IS REENTERED,     */
#define tcbdard                          0x20 /* - A DUMP HAS BEEN REQUESTED FOR A WRITER OR          */
#define tcbdarc                          0x10 /* - RECURSION PERMITTED IN CLOSE -                     */
#define tcbdarmc                         0x10 /* - DAR HAS BEEN ENTERED TO HANDLE A VALID RECURSION   */
#define tcbdaro                          0x08 /* - SYSTEM ERROR TASK IS FAILING.  DAR DUMP SHOULD     */
#define tcbdarwt                         0x04 /* - A WTO OPERATION WITH A 'REINSTATEMENT FAILURE'     */
#define tcbdarms                         0x02 /* - WTO OPERATION WITH A 'DAR IN PROGRESS' MESSAGE     */
#define tcbexsvc                         0x01 /* - THE DUMP SVC ROUTINE IS EXECUTING FOR THIS TASK    */
 
/* Values for field "tcbnssp" */
#define tcbnssqa                         0x80 /* - NORMAL STACK SAVED IN SQA INDICATOR.    @G38CPXB   */
 
/* Values for field "tcbtid" */
#define tcbpagid                         255  /* -   ID FOR PAGING SUPERVISOR TASK             ICB403 */
#define tcbsyerr                         254  /* -   ID FOR SYSTEM ERROR TASK                  ICB403 */
#define tcbcomm                          253  /* -   ID FOR COMMUNCIATIONS TASK                ICB403 */
#define tcbiorms                         252  /* -   ID FOR I/O RMS TASK                       ICB403 */
#define tcbmastr                         251  /* -   ID FOR MASTER SCHEDULER TASK              ICB403 */
#define tcbjes                           250  /* -   ID FOR JOB ENTRY SUBSYSTEM (JES) MONITOR TASK    */
#define tcbdssid                         249  /* -   ID FOR DYNAMIC SUPPORT SYSTEM (DSS) TASK  ICB403 */
#define tcblogid                         248  /* -   ID FOR SYSTEM LOG TASK                           */
 
/* Values for field "tcbflgs8" */
#define tcbdying                         0x80 /* - If on, indicates that this TCB will be      @0CC   */
#define tcbnoirb                         0x40 /* - If on, IRBs will not be queued to this TCB. @08A   */
#define tcbjtcba                         0x20 /* - If on, this is the attach of the JSTCB by the      */
#define tcbsdndx                         0x10 /* - If on, this task is exempt from being set          */
#define tcbendng                         0x08 /* - If on, indicates that this TCB will be      @0CA   */
#define tcbendingabnormally              0x04 /* - If On, indicates that this TCB is @LHA             */
#define tcbdebl                          0x02 /* - Serialization: TCBACTIV                     @PCM   */
#define tcbsiqeo                         0x01 /* - If on, during task termination, only IQEs set      */
 
/* Values for field "tcbxsct1" */
#define tcbactiv                         0x80 /* - BIT ON MEANS THIS TCB IS CURRENTLY ACTIVE ON A     */
#define tcbs3a                           0x40 /* - STAGE 3 EXIT EFFECTOR/RESUME/TCTL INTERSECT FLAG   */
#define tcbllreq                         0x20 /* - TASK REQUESTED LOCAL LOCK               @G50DP9A   */
#define tcbtdorm                         0x08 /* - Turned on to detect dormant Tasks by        @06A   */
 
/* Values for field "tcbxsct2" */
#define tcbcmlf                          0x80 /* - CML RESOURCE MANAGER PROCESSING         @G381P9A   */
#define tcbllnew                         0x40 /* - Lock Manager has given this task the local  @L5A   */
#define tcbrsmrc                         0x20 /* - RSM Resmgr processing is complete           @LJA   */
 
/* Values for field "tcbestrm" */
#define tcbeterm                         0x80 /* - ESTAE EXIT ENTERED WITH TERM OPTION                */
#define tcbstafx                         0x40 /* - SERIALIZED BY TCB ACTIVE                @ZA30005   */
 
/* Values for field "tcbprop0" */
#define tcbbitcb                         0x80 /* - This task is "below" the initiator TCB     @L9A    */
 
/* Values for field "tcbfbyt1" */
#define tcbeotfm                         0x80 /* - END OF TASK FLAG FOR FREEMAIN.  SET TO 1           */
#define tcbrtm1e                         0x40 /* - RTM1 IS CURRENTLY PROCESSING EUT FRR'S  @G381PXU   */
#define tcbndios                         0x20 /* - TASK HAS BEEN SET NONDISPATCHABLE VIA STATUSND     */
#define tcbpgnly                         0x10 /* - SET BY RTM2 TO INDICATE ONLY PURGE PHASE TO BE     */
#define tcbrtm2                          0x08 /* - SET BY RTM2 TO INDICATE RTM2 HAS BEEN ENTERED      */
#define tcbeot                           0x04 /* - SET BY RTM2 TO INDICATE TO EXIT THAT END OF TASK   */
#define tcbsattn                         0x02 /* - SYNCHRONIZATION OF ATTENTION INTERRUPT REQUIRED    */
#define tcbllh                           0x01 /* - Task was interrupted holding the local lock.       */
 
/* Values for field "tcbfbyt2" */
#define tcbcncb                          0x80 /* - SET BY RTM2 IN THE JOB STEP TCB WHEN IT HAS BEEN   */
#define tcbfmw                           0x40 /* - MOTHER WAITING FLAG.  TURNED ON IN A SUBTASK IN    */
#define tcbfdw                           0x20 /* - Set by RTM2 to ensure that only one daughter       */
#define tcbfprap                         0x10 /* - SET BY RTM2 TO PREVENT PERCOLATION TO THE TASK     */
#define tcbrt1nr                         0x08 /* - IF 1, ERROR PROPAGATED FROM RTM1 IS         @D1A   */
#define tcbecbnv                         0x04 /* - IF 1, ECB POINTED TO BY TCBECB IS NOT TO BE        */
#define tcbsspc                          0x02 /* - STATUS STOP PENDING,TASK HOLDS A CML LOCK          */
#define tcbrtm1c                         0x01 /* - A TASK WITH EUT FRRS HAS BEEN CANCELLED. THIS      */
 
/* Values for field "tcbfbyt3" */
#define tcbexp                           0x80 /* - EXPANDED VERSION OF THE TCB             @ZA72701   */
#define tcbnctl                          0x40 /* - IF 1, INDICATES USER PROGRAM IS LOADED  @ZA84893   */
#define tcbrtmde                         0x20 /* - DETACH HAS BEEN CALLED BY RTM           @ZA93650   */
#define tcbmtdp                          0x10 /* - MEMTERM SDUMP                               @LAA   */
 
/* Values for field "tcbfbyt4" */
#define tcbpmc                           0x80 /* - IF 1, INDICATES TASK IS IN PROCESS MUST     @D2C   */
#define tcbnojlb                         0x40 /* - If 1, indicates that TCBJLB is not to be used      */
#define tcbatskl                         0x20 /* - If 1, this task's TASKLIB was provided by   @LDA   */
#define tcb___ieavalck___1               0x10 /* -                                         @0IA       */
#define tcb___keep___ls___extent___valid 0x08 /* - If 1, do not unchain and page @0DA                 */
#define tcb___pthreadtermabnormal        0x04 /* - If 1, some z/OS Unix thread    @0FA                */
#define tcb___thisisjspgmtask            0x02 /* Jobstep Program Task                 @0JA            */
 
/* Values for field "tcbrbyt1" */
#define tcbpvict                         0x80 /* - If 1, indicates that the associated task    @05M   */
#define tcbpking                         0x40 /* - If 1, indicates that the associated Task has       */
#define tcbpcand                         0x20 /* - If 1, indicates that this Task has requested       */
#define tcbptop                          0x10 /* - If 1, indicates that this Task is at the top       */
 
/* Values for field "tcblevel" */
#define tcbvs02a                         0x02 /* - JBB2110 (NOT IN BASE)                   @ZA72701   */
#define tcbvs02b                         0x02 /* - JBB2125                                 @ZA72701   */
#define tcbvs03                          0x03 /* - JBB2133                                     @H1A   */
#define tcbvers                          0x03 /* - LEVEL OF THIS MAPPING                       @H1C   */
 
#endif
 
#ifndef __tcbxtnt2__
#define __tcbxtnt2__
 
struct tcbxtnt2 {
  struct {
    unsigned char  _tcbtflg;      /* -     GTF FLAG BYTE                             ICB312 */
    unsigned int   _tcbgtfa : 24; /* -   ADDRESS OF GTF TEMPORARY TRACE BUFFER     ICB312   */
    } tcbgtf;
  unsigned char  _filler1;   /* -     RESERVED.                                   @H2C */
  unsigned char  tcbrcmp[3]; /* -   MOST RECENT ABEND COMPLETION CODE (INCLUDING       */
  void * __ptr32 tcbevent;   /* -     ADDRESS OF EVENT TABLES QUEUE           @X31ED9A */
  int            tcbrtmct;   /* -     COUNT OF TOKENS USED FOR ESTAE.         @G381PXU */
  void * __ptr32 tcbtqe;     /* -     ADDRESS OF A REUSABLE TASK-RELATED TQE  @G50EP9A */
  void * __ptr32 tcbcauf;    /* -     ADDRESS OF SUBSYSTEM FACILITY CONTROL BLOCK      */
  void * __ptr32 tcbpercp;   /* -     POINTER TO A QUEUE OF SPIS. AN SPI      @G381P9A */
  int            tcbperct;   /* -     COUNT OF SRB MODE FRRS WAITING TO       @G381P9A */
  };
 
#define tcbtflg tcbgtf._tcbtflg
#define tcbgtfa tcbgtf._tcbgtfa
 
/* Values for field "tcbtflg" */
#define tcbasync 0x80 /* - GTF ASYNCHRONOUS GATHER ROUTINE IS IN CONTROL    */
#define tcberrtn 0x40 /* - GTF ASYNCHRONOUS GATHER ERROR ROUTINE IS IN      */
#define tcbdspit 0x20 /* - MACHINE CHECK INTERRUPTION HANDLER SHOULD        */
 
/* Values for field "tcbpercp" */
#define tcbrcvry 0x80 /* - TASK IS IN RECOVERY.                    @G381P9A */
 
#endif
 
#pragma pack(reset)
