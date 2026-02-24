#ifdef __open_xl__
#pragma pack(1)
#else
#pragma pack(packed)
#endif
 
#ifndef __iezjscb__
#define __iezjscb__
 
struct iezjscb {
  unsigned char  _filler1[188];
  int            jscrsv01;      /* -            RESERVED                                  */
  struct {
    unsigned char  _jscrsv32;      /* -            RESERVED                           ICB459 */
    unsigned int   _jschpcea : 24; /* -          ADDRESS OF OPTIONAL JOB ENTRY SUBSYSTEM     */
    } jschpce;
  void * __ptr32 jscbshr;       /* -            ADDRESS OF ASSEMBLY CHAIN (VSAM)   ICB434 */
  void * __ptr32 jscbtcp;       /* -            ADDRESS OF TIOT CHAINING ELEMENT CHAIN    */
  void * __ptr32 jscbpcc;       /* -            ADDRESS OF PRIVATE CATALOG CONTROL BLOCK  */
  void * __ptr32 jscbtcbp;      /* -            ADDRESS OF INITIATOR'S TCB (VSAM)  ICB434 */
  void * __ptr32 jscbijsc;      /* -            ADDRESS OF JSCB OF THE INITIATOR THAT     */
  void * __ptr32 jscbdbtb;      /* -            ADDRESS OF THE DEB TABLE FOR THIS JOB     */
  unsigned char  jscbid[4];     /* -          JOB SERIAL NUMBER (OS/VS1)                  */
  struct {
    unsigned char  _jscrsv02;      /* -            RESERVED                             */
    unsigned int   _jscbdcba : 24; /* -          ADDRESS OF DCB FOR DATA SET CONTAINING */
    } jscbdcb;
  char           jscbstep;      /* -          CURRENT STEP NUMBER.  THE FIRST STEP IS     */
  unsigned char  jscrsv03[3];   /* -          RESERVED                                    */
  void * __ptr32 jscbsecb;      /* -            ECB FOR COMMUNICATION BETWEEN    @ZMC1264 */
  unsigned char  jscbopts;      /* -            OPTION SWITCHES                           */
  unsigned char  jscbcrb6[6];   /* -          LOW ORDER 6 BYTES OF THE SHR RBA USED       */
  unsigned char  jscbswt1;      /* -            STATUS SWITCHES  (OS/VS2)          ICB351 */
  void * __ptr32 jscbqmpi;      /* -            ADDRESS OF THE QUEUE MANAGER PARAMETER    */
  void * __ptr32 jscbjesw;      /* -            ADDRESS OF THE JES WORKAREA      @YA01530 */
  struct {
    unsigned char  _jscbwtfg; /* -            FLAGS USED BY WTP SUPPORT               */
    char           _jscbwtsp; /* -          NUMBER OF THE LAST JOB STEP TO ISSUE WTP  */
    short int      _jscbpmg;  /* -            NUMBER OF WTP OPERATIONS ISSUED FOR THE */
    } jscbwtp;
  void * __ptr32 jscbcscb;      /* -            ADDRESS OF COMMAND SCHEDULING CONTROL     */
  struct {
    unsigned char  _jscrsv24; /* -            RESERVED                           ICB351 */
    struct {
      unsigned char  _jscbjcta[3]; /* -          SVA of JCT, use SWAREQ to convert    @P2C */
      } jscjctp;
    } jscbjct;
  void * __ptr32 jscbpscb;      /* -            ADDRESS OF TSO PROTECTED STEP CONTROL     */
  struct {
    short int      _jscbtjid; /* -            TSO TERMINAL JOB IDENTIFIER */
    } jscbasid;
  unsigned char  jscbfbyt;      /* -            FLAG BYTE  (MDC300)              @Z40RP9A */
  unsigned char  jscbrv08;      /* -            RESERVED                                  */
  int            jscbiecb;      /* -            ECB USED FOR COMMUNICATION BETWEEN        */
  unsigned char  jscbjrba[8];   /* -          JOB JOURNAL RELATIVE BYTE ADDRESS (RBA)     */
  void * __ptr32 jscbaloc;      /* -            ADDRESS OF THE ALLOCATION WORK   @ZMC1264 */
  struct {
    unsigned char  _jscbjjsb;      /* -            JOB JOURNAL STATUS INDICATORS      ICB332 */
    unsigned int   _jscbjnla : 24; /* -          INITIATOR JSCB ONLY - ADDRESS OF JSCB       */
    } jscbjnl;
  void * __ptr32 jscbjnlr;      /* -            POINTER TO JOB JOURNAL RPL         MDC023 */
  void * __ptr32 jscbsmlr;      /* -            ADDRESS OF SYSTEM MESSAGE DATA            */
  struct {
    unsigned char  _jscrsv31;      /* -            RESERVED                           ICB333 */
    unsigned int   _jscbsuba : 24; /* -          ADDRESS OF JES-SUBTL FOR THIS JOB           */
    } jscbsub;
  short int      jscbsono;      /* -            THE NUMBER OF SYSOUT DATA SETS PLUS       */
  unsigned char  jscbcrb2[2];   /* -          HIGH ORDER 2 BYTES OF THE SHR RBA USED      */
  unsigned char  jscbfrba[8];   /* -          RELATIVE BYTE ADDRESS (RBA) OF THE FIRST    */
  void * __ptr32 jscbssib;      /* -            ADDRESS OF THE SUBSYSTEM IDENTIFICATION   */
  void * __ptr32 jscdsabq;      /* -            Address of QDB for DSAB chain.            */
  int            jscgddno;      /* -            Counter used by Dynamic Allocation to     */
  struct {
    unsigned char  _jscrsv55;   /* -          RESERVED                                  */
    unsigned char  _jscsctp[3]; /* -          SVA of SCT, use SWAREQ to convert    @P2C */
    } jscsct;
  void * __ptr32 jsctmcor;      /* -            ADDRESS OF TIOT MAIN STORAGE MANAGEMENT   */
  void * __ptr32 jscbvata;      /* -            ADDRESS OF VAT USED DURING SYSTEM RESTART */
  short int      jscrsv08;      /* -            Reserved, was JSCDDNNO               @P8C */
  short int      jscbodno;      /* -            COUNTER USED BY DYNAMIC OUTPUT TO    @D1C */
  short int      jscddnum;      /* -            NUMBER OF DD ENTRIES CURRENTLY ALLOCATED  */
  unsigned char  jscrsv33;      /* -            RESERVED                           MDC019 */
  char           jscbswsp;      /* -          SWA SUBPOOL                        MDC015   */
  void * __ptr32 jscbact;       /* -            POINTER TO ACTIVE JSCB             MDC014 */
  void * __ptr32 jscrsv09;      /* -            Reserved, was JSCBUFPT               @L9C */
  void * __ptr32 jscbeacb;      /* -            Address of event log ACB.            @L8C */
  struct {
    void * __ptr32 _jscbecb1; /* -            ADDR OF CANCEL ECB WHILE         @ZMC1510 */
    void * __ptr32 _jscbecb2; /* -            ADDR OF WAIT FOR REGION ECB      @ZMC1510 */
    } jscbpgmn;
  void * __ptr32 jscdsnqp;      /* -            Pointer to the first DSENQ Table     @L5C */
  void * __ptr32 jscbcscx;      /* -            ADDRESS OF CSCX EXTENSION TO CSCB    @L2C */
  int            jscamcpl;      /* -            ALLOCATION MESSAGE CELLPOOL ID   @YC19251 */
  };
 
#define jscrsv32 jschpce._jscrsv32
#define jschpcea jschpce._jschpcea
#define jscrsv02 jscbdcb._jscrsv02
#define jscbdcba jscbdcb._jscbdcba
#define jscbwtfg jscbwtp._jscbwtfg
#define jscbwtsp jscbwtp._jscbwtsp
#define jscbpmg  jscbwtp._jscbpmg
#define jscrsv24 jscbjct._jscrsv24
#define jscbjcta jscbjct.jscjctp._jscbjcta
#define jscbtjid jscbasid._jscbtjid
#define jscbjjsb jscbjnl._jscbjjsb
#define jscbjnla jscbjnl._jscbjnla
#define jscrsv31 jscbsub._jscrsv31
#define jscbsuba jscbsub._jscbsuba
#define jscrsv55 jscsct._jscrsv55
#define jscsctp  jscsct._jscsctp
#define jscbecb1 jscbpgmn._jscbecb1
#define jscbecb2 jscbpgmn._jscbecb2
 
/* Values for field "jscbopts" */
#define jscrsv04 0x80 /* -  RESERVED                                        */
#define jscrsv05 0x40 /* -  RESERVED                                        */
#define jscblong 0x20 /* -        THE PARTITION CANNOT BE REDEFINED BECAUSE */
#define jscrsv06 0x10 /* -  RESERVED                                        */
#define jscrsv07 0x08 /* -  RESERVED                                        */
#define jscbtiod 0x04 /* -        WHEN SET BY PROGRAM, EXCLUSIVE ENQS FOR   */
#define jscsiots 0x02 /* -        CHECKPOINT MUST SCAN SIOT          MDC018 */
#define jscbauth 0x01 /* -        The step represented by this              */
 
/* Values for field "jscbswt1" */
#define jscbpass 0x80 /* -        WHEN THIS BIT IS SET TO ONE AND A         */
#define jscbunin 0x40 /* -        When ON, indicates that Allocation        */
#define jscrsv12 0x20 /* -  RESERVED                                        */
#define jscrsv13 0x10 /* -  RESERVED                                        */
#define jscrsv14 0x08 /* -  RESERVED                                        */
#define jscrsv15 0x04 /* -  RESERVED                                        */
#define jscrsv16 0x02 /* -  RESERVED                             @01C       */
#define jscbpmsg 0x01 /* -        A MESSAGE HAS BEEN ISSUED BECAUSE THE     */
 
/* Values for field "jscbwtfg" */
#define jscbiofg 0x80 /* -        THE PREVIOUS WTP I/O OPERATION HAD AN     */
#define jscbret  0x40 /* -        TEXT BREAKING INDICATOR, ADDITIONAL       */
#define jscbbmo  0x20 /* -              Buffer Messages Only flag.  Set     */
#define jscrsv19 0x10 /* -  RESERVED                                        */
#define jscrsv20 0x08 /* -  RESERVED                                        */
#define jscrsv21 0x04 /* -  RESERVED                                        */
#define jscrsv22 0x02 /* -  RESERVED                                        */
#define jscrsv23 0x01 /* -  RESERVED                                        */
 
/* Values for field "jscbcscb" */
#define jscbs1ln 0x48 /* - LENGTH OF SECTION 1                              */
 
/* Values for field "jscbfbyt" */
#define jscbrv01 0x80 /* -  RESERVED                                        */
#define jscbadsp 0x40 /* -        AUTOMATIC DATA SET PROTECTION FOR THIS    */
#define jscblgdf 0x20 /* -  IAZLGDAT invocation failed and IFA044I          */
#define jscbrv03 0x10 /* -  RESERVED                                        */
#define jscbsjfy 0x08 /* -  Used by BB131                        @P4C       */
#define jscbsjfn 0x04 /* -  Used by BB131                        @P4C       */
#define jscbrv06 0x02 /* -  RESERVED                                        */
#define jscbrv07 0x01 /* -  RESERVED                                        */
 
/* Values for field "jscbjjsb" */
#define jscbjnln 0x80 /* -        NOTHING SHOULD BE WRITTEN IN              */
#define jscbjnlf 0x40 /* -        NO JOB JOURNAL                     MDC017 */
#define jscbjnle 0x20 /* -        ERROR IN JOURNAL, DO NOT WRITE     ICB332 */
#define jscbjsbi 0x08 /* -        JOB HAS NOT ENTERED ALLOCATION FOR THE    */
#define jscbjsba 0x04 /* -        JOB HAS ENTERED ALLOCATION         ICB332 */
#define jscbjsbx 0x02 /* -        JOB HAS COMPLETED ALLOCATION       ICB332 */
#define jscbjsbt 0x01 /* -        JOB HAS ENTERED TERMINATION        ICB332 */
 
/* Values for field "jscamcpl" */
#define jscbs3ln 0x78 /* - LENGTH OF SECTION 3                ICB351        */
#define jscbdisp 0xBC /* -    DISPLACEMENT OF FIRST JSCB DATA BYTE          */
#define jscbaos1 0x48 /* - OS/VS1 JSCB LENGTH            ICB351             */
#define jscbaos2 0xC0 /* - OS/VS2 JSCB LENGTH            ICB332             */
 
#endif
 
#ifdef __open_xl__
#pragma pack()
#else
#pragma pack(reset)
#endif
