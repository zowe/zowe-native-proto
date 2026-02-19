#pragma pack(1)

#ifndef __ascb__
#define __ascb__

struct ascb {
  struct {
    unsigned char  _ascbascb[4]; /* -               ACRONYM IN EBCDIC -ASCB-             */
    void * __ptr32 _ascbfwdp;    /* -                 ADDRESS OF NEXT ASCB ON ASCB READY */
    } ascbegin;
  void * __ptr32 ascbbwdp;     /* -                 ADDRESS OF PREVIOUS ASCB ON ASCB     */
  void * __ptr32 ascbltcs;     /* -                 TCB and preemptable-class SRB   @07C */
  struct {
    struct {
      void * __ptr32 _ascbsvrb___prezos12; /* -          SVRB POOL ADDRESS.              @LLC  */
      int            _ascbsync___prezos12; /* -           COUNT USED TO SYNCHRONIZE SVRB POOL. */
      } ascbsupc___prezos12;
    } ascbdiag010;
  void * __ptr32 ascbiosp;     /* -                 POINTER TO IOS PURGE INTERFACE       */
  struct {
    unsigned char  _ascbr01c[2]; /* RESERVED, MUST BE ZERO          @L8A */
    short int      _ascbwqid;    /* LOGICAL CPU ID OF THE PROCESSOR @L8A */
    } ascbwqlk;
  struct {
    void * __ptr32 _ascbsawq___prezos11; /* -           ADDRESS OF ADDRESS SPACE SRB WEB */
    } ascb___job___step___seqnum;
  struct {
    short int      _ascbasid; /* -                 ADDRESS SPACE IDENTIFIER FOR THE */
    } ascbasn;
  unsigned char  ascbdiag026;  /* -                 IBM use only                    @0GC */
  unsigned char  ascbsrmflags; /* -                SRM flags                             */
  unsigned char  ascbll5;      /* -                 FLAGS. SERIALIZATION - LOCAL    @D2A */
  char           ascbhlhi;     /* -               INDICATION OF SUSPEND LOCKS     @L4C   */
  struct {
    char           _ascbdphi; /* -               HIGH ORDER BYTE OF HALFWORD     @L4A */
    char           _ascbdp;   /* -               DISPATCHING PRIORITY RANGE FROM      */
    } ascbdph;
  int            ascbtcbe;     /* -               Count of ready tcbs in the      @LCC   */
  void * __ptr32 ascblda;      /* -                 POINTER TO LOCAL DATA AREA PART OF   */
  unsigned char  ascbrsmf;     /* -                 RSM ADDRESS SPACE FLAGS              */
  unsigned char  ascbflg3;     /* -               Flags needing no serialization  @LDA   */
  struct {
    short int      _ascbhasi___prezos11; /* -           Local lock owning ASID.         @LJC */
    } ascbr036;
  void * __ptr32 ascbcscb;     /* -                 ADDRESS OF CSCB                      */
  void * __ptr32 ascbtsb;      /* -                 ADDRESS OF TSB                       */
  double         ascbejst;     /* -                 Accumulated job step CPU time.       */
  double         ascbewst;     /* -                 TIME OF DAY WHENEVER I-STREAM IS     */
  int            ascbjstl;     /* -                 CPU TIME LIMIT FOR THE JOB STEP      */
  int            ascbecb;      /* -                 RCT'S WORK ECB                       */
  int            ascbubet;     /* -                 TIME STAMP WHEN USER BECOMES READY   */
  void * __ptr32 ascbtlch;     /* -                 CHAIN FIELD FOR TIME LIMIT EXCEEDED  */
  void * __ptr32 ascbdump;     /* -                 SVC DUMP TASK TCB ADDRESS            */
  struct {
    short int      _ascbaffn; /* -                 CPU AFFINITY INDICATOR              */
    unsigned char  _ascbrctf; /* -                 FLAGS FOR RCT SERIALIZED BY COMPARE */
    unsigned char  _ascbflg1; /* -                 FLAG FIELD                          */
    } ascbfw1;
  int            ascbtmch;     /* -                 TERMINATION QUEUE CHAIN              */
  void * __ptr32 ascbasxb;     /* -                 POINTER TO ADDRESS SPACE EXTENSION   */
  struct {
    short int      _ascbswct; /* -                 NUMBER OF TIMES MEMORY ENTERS SHORT */
    unsigned char  _ascbdsp1; /* -                 NONDISPATCHABILITY FLAGS.           */
    unsigned char  _ascbflg2; /* -                 FLAG BYTE.                          */
    } ascbfw2;
  struct {
    short int      _filler1;  /* -                 FIRST HALFWORD OF ASCBSCNT MUST BE */
    short int      _ascbsrbs; /* -                 COUNT OF SRB'S SUSPENDED IN THIS   */
    } ascbscnt;
  void * __ptr32 ascbllwq;     /* -                 ADDRESS SPACE LOCAL LOCK        @04C */
  void * __ptr32 ascbrctp;     /* -                 POINTER TO REGION CONTROL TASK (RCT) */
  struct {
    int            _ascblock; /* -                 LOCAL LOCK.  THIS OFFSET FIXED BY    */
    void * __ptr32 _ascblswq; /* -                 ADDRESS SPACE LOCAL LOCK WEB    @L8C */
    } ascblkgp;
  int            ascbqecb;     /* -                 QUIESCE ECB                          */
  int            ascbmecb;     /* -                 MEMORY CREATE/DELETE ECB             */
  void * __ptr32 ascboucb;     /* -                 SYSTEM RESOURCES MANAGER (SRM) USER  */
  void * __ptr32 ascbouxb;     /* -                 SYSTEM RESOURCES MANAGER (SRM) USER  */
  struct {
    short int      _ascbfmct; /* -                 RESERVED. ALLOCATED PAGE FRAME  @L6C */
    unsigned char  _ascblevl; /* -                 LEVEL NUMBER OF ASCB            @D3A */
    unsigned char  _ascbfl2a; /* -                 FLAG BYTE.                      @DAA */
    } ascbfw2a;
  struct {
    int            _ascbejst___disps; /* -              Count of task dispatches. */
    } ascbhreq___prezos11;
  void * __ptr32 ascbiqea;     /* -                 POINTER TO IQE FOR ATCAM             */
  void * __ptr32 ascbrtmc;     /* -                 ANCHOR FOR SQA SDWA QUEUE            */
  unsigned char  ascbmcc[4];   /* -               USED TO HOLD A MEMORY TERMINATION      */
  void * __ptr32 ascbjbni;     /* -                 POINTER TO JOBNAME FIELD FOR         */
  void * __ptr32 ascbjbns;     /* -                 POINTER TO JOBNAME FIELD FOR         */
  struct {
    unsigned char  _ascbsrq1; /* -                 FIRST BYTE OF ASCBSRQ  */
    unsigned char  _ascbsrq2; /* -                 SECOND BYTE OF ASCBSRQ */
    unsigned char  _ascbsrq3; /* -                 THIRD BYTE OF ASCBSRQ  */
    unsigned char  _ascbsrq4; /* -                 FOURTH BYTE OF ASCBSRQ */
    } ascbsrq;
  void * __ptr32 ascbvgtt;     /* -                 ADDRESS OF VSAM GLOBAL TERMINATION   */
  void * __ptr32 ascbpctt;     /* -                 ADDRESS OF PRIVATE CATALOG           */
  short int      ascbssrb;     /* -                 COUNT OF STATUS STOP SRB'S           */
  char           ascbsmct;     /* -               NUMBER OF OUTSTANDING STEP MUST        */
  unsigned char  ascbsrbm;     /* -                 MODEL PSW BYTE 0 USED BY SRB         */
  int            ascbswtl;     /* -                 STEP WAIT TIME LIMIT          MDC029 */
  double         ascbsrbt;     /* -                 ACCUMULATED SRB TIME          MDC030 */
  void * __ptr32 ascbltcb;     /* -                 TCB and preemptable-class SRB   @07C */
  int            ascbltcn;     /* -                 Count of TCB and preemptable-   @07A */
  int            ascbtcbs;     /* -                 NUMBER OF READY TCB'S.          @L8A */
  int            ascblsqt;     /* -                 NUMBER OF TCBS ON A LOCAL LOCK  @L8A */
  void * __ptr32 ascbwprb;     /* -                 ADDRESS OF WAIT POST REQUEST BLOCK   */
  struct {
    char           _ascbndp;  /* -               NEW DISPATCHING PRIORITY             */
    char           _ascbtndp; /* -               NEW TIME SLICE DISPATCHING PRIORITY  */
    char           _ascbntsg; /* -               NEW TIME SLICE GROUP                 */
    char           _ascbiodp; /* -               I/O PRIORITY (MDC374)       @G50IP9A */
    } ascbsrdp;
  void * __ptr32 ascbloci;     /* -                 LOCK IMAGE, ADDRESS OF ASCB          */
  void * __ptr32 ascbcmlw;     /* -                 ADDRESS OF THE WEB REPRESENTING @L8C */
  struct {
    int            _ascbsrbt___disps; /* -              Count of SRB dispatches. */
    } ascbcmlc___prezos12;
  struct {
    unsigned char  _ascbsso1[3]; /* -               SPACE SWITCH EVENT OWNER    @G381P9A   */
    unsigned char  _ascbsso4;    /* -                 SPACE SWITCH EVENT OWNER    @G381P9A */
    } ascbssom;
  void * __ptr32 ascbaste;     /* -                 VIRTUAL ADDRESS OF ADDRESS  @G381P9A */
  void * __ptr32 ascbltov;     /* -                 VIRTUAL ADDRESS OF THE      @G381P9A */
  void * __ptr32 ascbatov;     /* -                 VIRTUAL ADDRESS OF          @G381P9A */
  short int      ascbetc;      /* -                 NUMBER OF ENTRY TABLES      @G381P9A */
  short int      ascbetcn;     /* -                 NUMBER OF CONNECTIONS TO    @G381P9A */
  short int      ascblxr;      /* -                 NUMBER OF LINKAGE INDEXES   @G381P9A */
  short int      ascbaxr;      /* -                 NUMBER OF AUTHORIZATION     @G381P9A */
  void * __ptr32 ascbstkh;     /* -                 ADDRESS OF LOCAL STACK POOL @G381P9A */
  struct {
    unsigned char  _ascbcsw0; /* Byte 0                          @0FA */
    unsigned char  _ascbcsw1; /* Byte 1                          @0FA */
    unsigned char  _ascbcsw2; /* Byte 2, Ser: CS                 @0FA */
    unsigned char  _ascbcsw3; /* Byte 3, Ser: CS                 @0FA */
    } ascbcswd;
  unsigned char  ascbr114[4];  /* Reserved.                       @0FC                   */
  void * __ptr32 ascbjafbaddr; /* -                Address of the JAFB             @LQA  */
  void * __ptr32 ascbxtcb;     /* -                 ADDRESS OF THE JOB STEP @G381P9A     */
  struct {
    unsigned char  _ascbcs1; /* -                 FIRST BYTE OF COMPARE AND            */
    unsigned char  _ascbcs2; /* -                 SECOND BYTE OF COMPARE AND SWAP @P5A */
    unsigned char  _ascbcs3; /* -                 3rd byte of CS flags            @0HA */
    unsigned char  _ascbcs4; /* -                 4th byte of CS flags            @0HA */
    } ascbfw3;
  void * __ptr32 ascbgxl;      /* -                 ADDRESS OF GLOBALLY LOADED MODULE    */
  double         ascbeatt;     /* -                 EXPENDED AND ACCOUNTED TASK TIME.    */
  double         ascbints;     /* -                 JOB SELECTION TIME STAMP.            */
  struct {
    unsigned char  _ascbll1; /* -                 FIRST BYTE OF FLAGS.        @G381P9A */
    unsigned char  _ascbll2; /* -                 SECOND BYTE OF FLAGS.       @G381P9A */
    unsigned char  _ascbll3; /* -                 THIRD BYTE OF FLAGS.        @G381P9A */
    unsigned char  _ascbll4; /* -                 FOURTH BYTE OF FLAGS.       @G381P9A */
    } ascbfw4;
  void * __ptr32 ascbrcms;     /* ADDRESS OF THE REQUESTED    @G381PXU                   */
  int            ascbiosc;     /* -                 I/O SERVICE MEASURE.        @G381PXU */
  short int      ascbpkml;     /* -              PKM OF LAST TASK DISPATCHED             */
  short int      ascbxcnt;     /* -                 EXCP COUNT FIELD.               @L1A */
  void * __ptr32 ascbnsqa;     /* -                 ADDRESS OF THE SQA RESIDENT          */
  void * __ptr32 ascbasm;      /* -                 ADDRESS OF THE ASM HEADER.      @L2A */
  void * __ptr32 ascbassb;     /* -                 POINTER TO ADDRESS SPACE        @D6C */
  void * __ptr32 ascbtcme;     /* -                 POINTER TO TCXTB.               @D1A */
  struct {
    unsigned char  _filler2;     /* -               BYTE 0 OF ASCBGQIR              @O3A */
    unsigned char  _filler3[2];  /* -               BYTE 1 AND 2 OF ASCBGQIR        @O3A */
    unsigned char  _ascbgqi3;    /* -               BYTE 3 OF ASCBGQIR              @O3A */
    } ascbgqir;
  int            ascblsqe;     /* -                 Number of Enclave TCBs that are on   */
  double         ascbiosx;     /* -                 I/O service measure extended.   @0AC */
  unsigned char  ascbr168;     /* -              RESERVED.                       @0IC    */
  unsigned char  ascbdiag169;  /* -           IBM use only                    @0IA       */
  unsigned char  ascbsvcn[2];  /* -              SVC Number for type-1 SVC               */
  void * __ptr32 ascbrsme;     /* -               POINTER TO RSM ADDRESS SPACE    @D5A   */
  struct {
    unsigned char  _ascbavm1; /* -                 FIRST BYTE OF ASCBAVM.          @01A */
    unsigned char  _ascbavm2; /* -                 SECOND BYTE OF ASCBAVM.         @01A */
    short int      _ascbagen; /* -                 AVM ASID REUSE GENERATION       @01A */
    } ascbavm;
  int            ascbarc;      /* -                 REASON CODE ON MEMTERM.              */
  struct {
    void * __ptr32 _ascbrsma; /* -               ADDRESS OF RSM'S CONTROL BLOCK */
    } ascbrsm;
  int            ascbdcti;     /* -              ACCUMULATED CHANNEL CONNECT TIME        */
  double         ascbend;      /* -                END OF ASCB                     @L7C  */
  };

#define ascbascb            ascbegin._ascbascb
#define ascbfwdp            ascbegin._ascbfwdp
#define ascbsvrb___prezos12 ascbdiag010.ascbsupc___prezos12._ascbsvrb___prezos12
#define ascbsync___prezos12 ascbdiag010.ascbsupc___prezos12._ascbsync___prezos12
#define ascbr01c            ascbwqlk._ascbr01c
#define ascbwqid            ascbwqlk._ascbwqid
#define ascbsawq___prezos11 ascb___job___step___seqnum._ascbsawq___prezos11
#define ascbasid            ascbasn._ascbasid
#define ascbdphi            ascbdph._ascbdphi
#define ascbdp              ascbdph._ascbdp
#define ascbhasi___prezos11 ascbr036._ascbhasi___prezos11
#define ascbaffn            ascbfw1._ascbaffn
#define ascbrctf            ascbfw1._ascbrctf
#define ascbflg1            ascbfw1._ascbflg1
#define ascbswct            ascbfw2._ascbswct
#define ascbdsp1            ascbfw2._ascbdsp1
#define ascbflg2            ascbfw2._ascbflg2
#define ascbsrbs            ascbscnt._ascbsrbs
#define ascblock            ascblkgp._ascblock
#define ascblswq            ascblkgp._ascblswq
#define ascbfmct            ascbfw2a._ascbfmct
#define ascblevl            ascbfw2a._ascblevl
#define ascbfl2a            ascbfw2a._ascbfl2a
#define ascbejst___disps    ascbhreq___prezos11._ascbejst___disps
#define ascbsrq1            ascbsrq._ascbsrq1
#define ascbsrq2            ascbsrq._ascbsrq2
#define ascbsrq3            ascbsrq._ascbsrq3
#define ascbsrq4            ascbsrq._ascbsrq4
#define ascbndp             ascbsrdp._ascbndp
#define ascbtndp            ascbsrdp._ascbtndp
#define ascbntsg            ascbsrdp._ascbntsg
#define ascbiodp            ascbsrdp._ascbiodp
#define ascbsrbt___disps    ascbcmlc___prezos12._ascbsrbt___disps
#define ascbsso1            ascbssom._ascbsso1
#define ascbsso4            ascbssom._ascbsso4
#define ascbcsw0            ascbcswd._ascbcsw0
#define ascbcsw1            ascbcswd._ascbcsw1
#define ascbcsw2            ascbcswd._ascbcsw2
#define ascbcsw3            ascbcswd._ascbcsw3
#define ascbcs1             ascbfw3._ascbcs1
#define ascbcs2             ascbfw3._ascbcs2
#define ascbcs3             ascbfw3._ascbcs3
#define ascbcs4             ascbfw3._ascbcs4
#define ascbll1             ascbfw4._ascbll1
#define ascbll2             ascbfw4._ascbll2
#define ascbll3             ascbfw4._ascbll3
#define ascbll4             ascbfw4._ascbll4
#define ascbgqi3            ascbgqir._ascbgqi3
#define ascbavm1            ascbavm._ascbavm1
#define ascbavm2            ascbavm._ascbavm2
#define ascbagen            ascbavm._ascbagen
#define ascbrsma            ascbrsm._ascbrsma

/* Values for field "ascbsawq___prezos11" */
#define ascburrq___prezos11       0x80 /* -      SYSEVENT USER READY REQUIRED    @LJC        */

/* Values for field "ascbsrmflags" */
#define ascbvcmoverride           0x80 /* -      This bit indicates that this                */
#define ascbbrokenup              0x40 /* -      This bit indicates that this                */
#define ascbvcmgivepreemption     0x20 /* -  This bit indicates that this                    */
#define ascbvcmgivesigpany        0x10 /* -  This bit indicates that this                    */
#define ascbinelighonorpriority   0x08 /* When on, specialty engine eligible                 */
#define ascbsrmflagsdiag          0x06 /* Diagnostic data for IBM use only@LSA               */

/* Values for field "ascbll5" */
#define ascbs3s                   0x20 /* -             STAGE II EXIT EFECTOR HAS       @D2M */

/* Values for field "ascbrsmf" */
#define ascb2lpu                  0x80 /* -             SECOND LEVEL PREFERRED USER.  THIS   */
#define ascb1lpu                  0x40 /* -             FIRST LEVEL PREFERRED USER           */
#define ascbn2lp                  0x20 /* -             SRM IN SYSEVENT TRANSWAP SHOULD NOT  */
#define ascbveqr                  0x10 /* -             V=R ADDRESS SPACE  (MDC372) @ZA17355 */

/* Values for field "ascbflg3" */
#define ascbcnip                  0x80 /* -             Address space created during NIP     */
#define ascbreus                  0x40 /* -             This is a reusable ASID. It may be   */
#define ascbm881                  0x20 /* This address space went through                    */
#define ascbsinglestepstartedtask 0x10 /* This address space is a single step                */

/* Values for field "ascbrctf" */
#define ascbtmno                  0x80 /* -             MEMORY IS BEING QUIESCED, IS         */
#define ascbfrs                   0x40 /* -             RESTORE REQUEST                      */
#define ascbfqu                   0x20 /* -             QUIESCE REQUEST                      */
#define ascbjste                  0x10 /* -             JOB STEP TIME EXCEEDED. NOT USED BY  */
#define ascbwait                  0x08 /* -             LONG WAIT INDICATOR                  */
#define ascbout                   0x04 /* -             ADDRESS SPACE CONSIDERED SWAPPED OUT */
#define ascbtmlw                  0x02 /* -             MEMORY IS IN A LONG WAIT             */
#define ascbtoff                  0x01 /* -             MEMORY SHOULD NOT BE CHECKED FOR JOB */

/* Values for field "ascbflg1" */
#define ascblsas                  0x80 /* -             ADDRESS SPACE IS LOGICALLY SWAPPED   */
#define ascbdstk                  0x40 /* -             SRM REQUIRES A TIME STAMP TO    @P6C */
#define ascbdstz                  0x40 /* -             Bit constant for bit position  @P7A  */
#define ascbterm                  0x10 /* -             ADDRESS SPACE TERMINATING NORMALLY   */
#define ascbabnt                  0x08 /* -             ADDRESS SPACE TERMINATING ABNORMALLY */
#define ascbmemp                  0x04 /* -             Memory Termination PURGEDQ flag @LBA */

/* Values for field "ascbdsp1" */
#define ascbssnd                  0x80 /* -             SYSTEM SET NONDISPATCHABLE AND THIS  */
#define ascbfail                  0x40 /* -             A FAILURE HAS OCCURRED WITHIN THE    */
#define ascbsnqs                  0x20 /* -             STATUS STOP NON-QUIESCABLE LEVEL     */
#define ascbssss                  0x10 /* -             STATUS STOP SRB SUMMARY              */
#define ascbstnd                  0x08 /* -             TCB'S NONDISPATCHABLE                */
#define ascbuwnd                  0x04 /* -             STATUS SET UNLOCKED WORKUNITS   @LAA */
#define ascbnoq                   0x02 /* -             ASCB NOT ON SWAPPED IN QUEUE    @L4A */

/* Values for field "ascbflg2" */
#define ascbxmpt                  0x80 /* -             ASCB EXEMPT FROM SYSTEM              */
#define ascbpxmt                  0x40 /* -             ASCB PERMANENTLY EXEMPT FROM SYSTEM  */
#define ascbcext                  0x20 /* -             CANCEL TIMER EXTENSION BECAUSE EOT   */
#define ascbs2s                   0x10 /* -             FOR LOCK MANAGER, ENTRY MADE TO      */
#define ascbncml                  0x08 /* -             ASCB NOT ELIGIBLE FOR CML LOCK       */
#define ascbnomt                  0x04 /* -             ADDRESS SPACE MUST NOT BE MEMTERMED  */
#define ascbnomd                  0x02 /* -             IF ON,ADDRESS SPACE CANNOT BE        */

/* Values for field "ascblswq" */
#define ascbs3nl                  0x80 /* -             THE LOCAL LOCK IS NEEDED BY THE @L8A */
#define ascbltcl                  0x01 /* -            THE LOCAL LOCK IS NEEDED BY SOME      */

/* Values for field "ascblevl" */
#define ascbvs00                  0x00 /* -             HBB2102 (NOT IN BASE)       @ZA68643 */
#define ascbvs01                  0x01 /* -             JBB2110                         @D3A */
#define ascbvs02                  0x02 /* -             JBB2133                         @H1A */
#define ascbvs03                  0x03 /* -             HBB4410                         @L7A */
#define ascbvers                  0x03 /* -             LEVEL OF THIS MAPPING           @L7C */

/* Values for field "ascbfl2a" */
#define ascbnopr                  0x80 /* -             NO PREEMPTION FLAG              @DAA */

/* Values for field "ascbsrq1" */
#define ascbdsg4                  0x80 /* -             SIGNAL WAITING PROCESSORS WHEN       */
#define ascbdflt                  0x40 /* -             DEFAULT LOCAL INTERSECT              */

/* Values for field "ascbsrq2" */
#define ascbdsg3                  0x80 /* -             SIGNAL WAITING PROCESSORS WHEN       */
#define ascbsrm1                  0x02 /* -             SYSTEM RESOURCE MANAGER (SRM)        */
#define ascbqver                  0x01 /* -             QUEUE VERIFICATION INTERSECTING      */

/* Values for field "ascbsrq3" */
#define ascbdsg2                  0x80 /* -             SIGNAL WAITING PROCESSORS WHEN       */
#define ascbrcti                  0x40 /* -             REGION CONTROL TASK (RCT)            */
#define ascbtcbv                  0x20 /* -             TCB VERIFICATION INTERSECTING        */
#define ascbacha                  0x10 /* -             ASCB CHAP INTERSECTING               */
#define ascbmter                  0x04 /* -             MEMORY TERMINATION INTERSECTING      */
#define ascbmini                  0x02 /* -             MEMORY INITIALIZATION INTERSECTING   */
#define ascbcbve                  0x01 /* -             CONTROL BLOCK VERIFICATION           */

/* Values for field "ascbsrq4" */
#define ascbdsg1                  0x80 /* -             SIGNAL WAITING PROCESSORS WHEN       */
#define ascbdeta                  0x40 /* -             DETACH INTERSECTING                  */
#define ascbatta                  0x20 /* -             ATTACH INTERSECTING                  */
#define ascbrtm2                  0x10 /* -             RTM2 INTERSECTING  (MDC351) @G50DP9A */
#define ascbrtm1                  0x08 /* -             RTM1 INTERSECTING  (MDC352) @G50DP9A */
#define ascbchap                  0x04 /* -             CHAP INTERSECTING  (MDC353) @G50DP9A */
#define ascbstat                  0x02 /* -             STATUS INTERSECTING                  */
#define ascbpurd                  0x01 /* -             PURGEDQ INTERSECTING                 */

/* Values for field "ascbsrbm" */
#define ascbper                   0x40 /* -             PER BIT IN ASCBSRBM - ALSO USED TO   */

/* Values for field "ascbsso4" */
#define ascbsssp                  0x02 /* -             SLIP/PER REQUESTED          @G381P9A */
#define ascbssjs                  0x01 /* -             JOB STEP TERMINATION        @G381P9A */

/* Values for field "ascbcsw2" */
#define ascbzkf                   0x80 /* @0FA                                               */

/* Values for field "ascbcsw3" */
#define ascbzu                    0x40 /* @0FA                                               */
#define ascbzf                    0x20 /* @0FA                                               */
#define ascbzn                    0x10 /* @0FA                                               */
#define ascbzm                    0x04 /* @0FA                                               */
#define ascbzs                    0x02 /* @0FA                                               */
#define ascbzc                    0x01 /* Checked.                        @0FA               */

/* Values for field "ascbcs1" */
#define ascbxmet                  0x80 /* -             IF ONE, THE ADDRESS SPACE IS    @L3C */
#define ascbxmec                  0x40 /* -             CROSS MEMORY ENTRY TABLES   @G381P9A */
#define ascbxmpa                  0x20 /* -             IF ONE, THE ADDRESS SPACE IS    @L3A */
#define ascbxmlk                  0x10 /* -             IF ONE, THE ADDRESS SPACE IS    @L3A */
#define ascbpers                  0x08 /* -             COMMUNICATION BIT FOR       @G381P9A */
#define ascbdter                  0x04 /* -             A DAT ERROR HAS OCCURRED    @G381P9A */
#define ascbpero                  0x02 /* -             PER PROCESSING NEEDS TO BE      @L5A */
#define ascbswop                  0x01 /* -             ADDRESS SPACE IS SWAPPED OUT    @L5A */

/* Values for field "ascbcs2" */
#define ascbsas                   0x80 /* -             INDICATES THAT STORAGE          @P5M */
#define ascbsmgr                  0x40 /* -             This space is or has been associated */
#define ascbdtin                  0x20 /* -             This space is or has been associated */
#define ascbxmnr                  0x10 /* -             The address space is                 */
#define ascbsdbf                  0x08 /* -             A work unit in this address space    */
#define ascbnoft                  0x04 /* -  Set this to exempt all tasks in this address    */
#define ascbpo1m                  0x02 /* -             Set this to indicate that            */
#define ascbp1m0                  0x01 /* -             Set this to indicate that            */

/* Values for field "ascbcs3" */
#define ascbxmso                  0x80 /* -             This address space has owned a       */

/* Values for field "ascbll1" */
#define ascbsspc                  0x80 /* -             STATUS STOP TASKS PENDING   @G381P9A */

/* Values for field "ascbll4" */
#define ascbnrll                  0x80 /* -             No release of Local Lock OK.    @LRA */
#define ascbtyp1                  0x02 /* -             TYPE 1 SVC HAS CONTROL.  THIS OFFSET */

/* Values for field "_filler2" */
#define ascbgqab                  0x80 /* -             ISGQSCAN INFORMATION            @O3A */

/* Values for field "ascbgqi3" */
#define ascbgqds                  0x01 /* -             ISGQSCAN INFORMATION            @O3A */

#endif

#pragma pack()
