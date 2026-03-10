#ifdef __open_xl__
#pragma pack(1)
#else
#pragma pack(packed)
#endif

#ifndef __asxb__
#define __asxb__

struct asxb {
  struct {
    unsigned char  _asxbasxb[4]; /* -               ACRONYM IN EBCDIC -ASXB-            */
    void * __ptr32 _asxbftcb;    /* -                 POINTER TO FIRST TCB ON TCB QUEUE */
    } asxbegin;
  void * __ptr32 asxbltcb;             /* -                 POINTER TO LAST TCB ON TCB QUEUE     */
  struct {
    short int      _asxbtcbs; /* -                 NUMBER TCB'S IN THE MEMORY           */
    unsigned char  _asxbflg1; /* -                 Flags                           @LDA */
    unsigned char  _asxbschd; /* -                 SCHEDULER FLAG BYTE             @LAA */
    } asxbcsw1;
  void * __ptr32 asxbmpst;             /* -                 ADDRESS OF VTAM MEMORY PROCESS       */
  void * __ptr32 asxblwa;              /* -                 ADDRESS OF LWA                MDC016 */
  void * __ptr32 asxbvfvt;             /* -                 POINTER TO INTERNAL VIRTUAL     @L2A */
  void * __ptr32 asxbsaf;              /* -                 ROUTER RRCB ADDRESS             @D1A */
  void * __ptr32 asxbihsa;             /* -                 POINTER TO INTERRUPT HANDLERS SAVE   */
  int            asxbflsa[18];         /* -               SAVE AREA FOR ANY FIRST LEVEL BRANCH   */
  void * __ptr32 asxbomcb;             /* -                 POINTER TO OBJECT ACCESS METHOD @01C */
  void * __ptr32 asxbspsa;             /* -                 POINTER TO LOCAL WORK/SAVE AREA      */
  void * __ptr32 asxbrsmd;             /* -                 POINTER TO LOCAL RSM DATA AREA       */
  void * __ptr32 asxbrctd;             /* -                 POINTER TO LOCAL RCT DATA AREA       */
  void * __ptr32 asxbdecb;             /* -                 DUMP TASK ECB                   @02C */
  void * __ptr32 asxbousb;             /* -                 POINTER TO SYSTEM RESOURCES MANAGER  */
  void * __ptr32 asxbcrwk;             /* -                 CHECKPOINT/RESTART WORKAREA POINTER. */
  unsigned char  asxbprg[16];          /* -              SVC PURGE I/O PARAMETER LIST  MDC003    */
  unsigned char  asxbpswd[8];          /* -               USER'S LOGON PASSWORD.  IF BLANK,      */
  void * __ptr32 asxbsirb;             /* -                 ADDRESS OF SIRB FOR THIS ADDRESS     */
  void * __ptr32 asxbetsk;             /* -                 ADDRESS OF ERROR TASK FOR THIS       */
  struct {
    void * __ptr32 _asxbfiqe; /* -                 POINTER TO FIRST IQE          MDC006 */
    void * __ptr32 _asxbliqe; /* -                 POINTER TO LAST IQE           MDC007 */
    void * __ptr32 _asxbfrqe; /* -                 POINTER TO FIRST RQE          MDC008 */
    void * __ptr32 _asxblrqe; /* -                 POINTER TO LAST RQE           MDC009 */
    void * __ptr32 _asxbfsrb; /* -                 ADDRESS OF FIRST SRB          MDC013 */
    void * __ptr32 _asxblsrb; /* -                 ADDRESS OF LAST SRB           MDC014 */
    } asxbaeq;
  struct {
    unsigned char  _asxbuser[7]; /* -               USER ID FOR WHICH THE JOB OR SESSION   */
    unsigned char  _filler1;     /* -                 Last byte of ASXBUSR8. ASXBSECR @04A */
    } asxbusr8;
  void * __ptr32 asxbsenv;             /* -                 ADDRESS OF ACCESS CONTROL            */
  void * __ptr32 asxbsfrs;             /* Address of SSI function request @03C                   */
  union {
    struct {
      int            _asxbr0d0;    /* Reserved as of z/OS 1.11        @08C */
      unsigned char  _filler2[4];
      } _asxb_struct1;
    struct {
      void * __ptr32 _asxbnssa___prezos11; /* NSSA POOL.                      @LGC */
      int            _asxbnsct___prezos11; /* COUNT USED TO SYNCHRONIZE THE        */
      } asxbnsdw___prezos11;
    struct {
      unsigned char  _filler3[4];
      void * __ptr32 _asxbthta;    /* Address of Task Hash Table      @08A */
      } _asxb_struct2;
    } _asxb_union1;
  struct {
    unsigned char  _asxbcrb1; /* -                 CANCEL/RCT BYTE 1  (MDC314) @ZA05360 */
    unsigned char  _asxbcrb2; /* -                 CANCEL/RCT BYTE 2  (MDC317) @ZA05360 */
    unsigned char  _asxbcrb3; /* -                 CANCEL/RCT BYTE 3  (MDC318) @ZA05360 */
    unsigned char  _asxbcrb4; /* -                 CANCEL/RCT BYTE 4  (MDC319) @ZA05360 */
    } asxbcasw;
  void * __ptr32 asxbpt0e;             /* -                 POST EXIT QUEUE HEADER               */
  void * __ptr32 asxbcapc;             /* -                 Count of task mode UCB capture  @L7A */
  void * __ptr32 asxbjsvt;             /* -                 JES COMMUNICATION AREA POINTER. @D2A */
  void * __ptr32 asxbdivw;             /* -                 ADDRESS OF THE DIV WORK/SAVE    @L4A */
  void * __ptr32 asxbcapt;             /* -                 Pointer to IOS captured UCB     @L7A */
  void * __ptr32 asxblinf;             /* -                 Latch information area          @L8A */
  void * __ptr32 asxbpirl;             /* Pointer to queue of PIRLs.                             */
  void * __ptr32 asxbitcb;             /* -                 Initial jobstep TCB address          */
  void * __ptr32 asxbrzvp;             /* -                 Address of RZV Control Table         */
  void * __ptr32 asxbgrsp;             /* -                 Address of GRS control          @P4A */
  void * __ptr32 asxbvasb;             /* Address of VASB.                                       */
  double         asxbalec;             /* AuthorizedLE Anchor             @LBA                   */
  double         asxbr110;             /* Reserved                        @0AC                   */
  void * __ptr32 asxbexta;             /* Local exits                     @LCA                   */
  void * __ptr32 asxbaxrl;             /* AXR local area                                         */
  double         asxb___mapreq___addr; /* MAPMVS tracking area address    @LHA                   */
  int            asxblcpi;             /* Loader CPOOL ID                 @07A                   */
  int            asxbtcbpmepoolid;     /* Pause Multiple CPOOL ID         @LKA                   */
  unsigned char  asxbcmtm[8];          /* Time (via STCK) when CMRO task                         */
  int            asxbcnzcpid;          /* CPoolId for CNZ                 @LMA                   */
  int            asxb___noabdump;      /* ABDUMP prevention counter.                             */
  unsigned char  asxbdiag140[16];      /* Diagnostic for IBM use only     @0AA                   */
  unsigned char  asxbr150[176];        /* Reserved                        @0AC                   */
  struct {
    void * __ptr32 _asxbnssa; /* NSSA POOL.                      @LGM */
    int            _asxbnsct; /* COUNT USED TO SYNCHRONIZE THE        */
    } asxbnsdw;
  unsigned char  asxbr208[248];        /* Reserved                        @LGA                   */
  double         asxbend;              /* -                END OF ASXB                           */
  };

#define asxbasxb            asxbegin._asxbasxb
#define asxbftcb            asxbegin._asxbftcb
#define asxbtcbs            asxbcsw1._asxbtcbs
#define asxbflg1            asxbcsw1._asxbflg1
#define asxbschd            asxbcsw1._asxbschd
#define asxbfiqe            asxbaeq._asxbfiqe
#define asxbliqe            asxbaeq._asxbliqe
#define asxbfrqe            asxbaeq._asxbfrqe
#define asxblrqe            asxbaeq._asxblrqe
#define asxbfsrb            asxbaeq._asxbfsrb
#define asxblsrb            asxbaeq._asxblsrb
#define asxbuser            asxbusr8._asxbuser
#define asxbr0d0            _asxb_union1._asxb_struct1._asxbr0d0
#define asxbnssa___prezos11 _asxb_union1.asxbnsdw___prezos11._asxbnssa___prezos11
#define asxbnsct___prezos11 _asxb_union1.asxbnsdw___prezos11._asxbnsct___prezos11
#define asxbthta            _asxb_union1._asxb_struct2._asxbthta
#define asxbcrb1            asxbcasw._asxbcrb1
#define asxbcrb2            asxbcasw._asxbcrb2
#define asxbcrb3            asxbcasw._asxbcrb3
#define asxbcrb4            asxbcasw._asxbcrb4
#define asxbnssa            asxbnsdw._asxbnssa
#define asxbnsct            asxbnsdw._asxbnsct

/* Values for field "asxbflg1" */
#define asxbhcrm             0x80   /* -             Health Check AS resmgr set      @LDA */

/* Values for field "asxbschd" */
#define asxbswup             0x80   /* -             INDICATES THAT SWA SHOULD BE WRITTEN */

/* Values for field "asxbcrb1" */
#define asxbpip              0x80   /* -             SET BY RCT TO INDICATE PURGE (SVC    */
#define asxbtfd              0x40   /* -             SET BY CANCEL TO INDICATE THAT ALL   */

/* Values for field "asxbcmtm" */
#define asxbcmtm___bit0      0x80   /* First bit, for windowing        @LLA               */

/* Values for field "asxbend" */
#define asxbtht___numentries 16     /* Number of HT entries            @08A               */
#define asxbthtl             0x40   /* Task Hash Table length          @08A               */
#define asxbtht___mask       0xF000 /* @08A                                               */
#define asxbtht___shift      12     /* Amount to shift masked value to                    */

#endif

#ifdef __open_xl__
#pragma pack()
#else
#pragma pack(reset)
#endif
