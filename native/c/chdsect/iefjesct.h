#pragma pack(packed)

#ifndef __jesct__
#define __jesct__

struct jesct {
  unsigned char  jesctid[4];  /* ACRONYM: JEST               @G383P2J */
  void * __ptr32 jesunits;    /* POINTER TO SYSRES UCB       @G389P2N */
  void * __ptr32 jeswaa;      /* ADDRESS OF THE SWA            Y02668 */
  void * __ptr32 jesqmgr;     /* ADDRESS OF SWA MANAGER        Y02668 */
  void * __ptr32 jesresqm;    /* ENTRY POINT USED TO INTERFACE        */
  void * __ptr32 jesssreq;    /* ADDRESS OF THE IEFSSREQ       Y02668 */
  void * __ptr32 jesssct;     /* ADDRESS OF THE FIRST          Y02668 */
  unsigned char  jespjesn[4]; /* NAME OF PRIMARY JOB ENTRY     Y02668 */
  void * __ptr32 jesalloc;    /* DEVICE ALLOCATION ENTRY     @Z40FPPJ */
  void * __ptr32 jesunalc;    /* DEVICE UNALLOCATION ENTRY   @Z40FPPJ */
  void * __ptr32 jescatl;     /* DEVICE ALLOCATION PRIVATE   @Z40FPPJ */
  int            jesnucbs;    /* NUMBER OF TAPE AND DA UCB'S @G383P2J */
  void * __ptr32 jessasta;    /* ADDRESS OF SUBSYSTEM        @G29AN2F */
  void * __ptr32 jesedt;      /* Address of Allocation Eligible  @P3C */
  void * __ptr32 jesrecm;     /* ADDRESS OF IEFJRECM         @G38AP2N */
  void * __ptr32 jesrecf;     /* ADDRESS OF IEFJRECF         @G38AP2N */
  void * __ptr32 jeshash;     /* ADDRESS OF SUBSYSTEM        @G385P2N */
  short int      jesnrss;     /* TOTAL NUMBER OF SUBSYSTEMS  @G385P2N */
  unsigned char  jesflg;      /* FLAG BYTE                   @G385P2N */
  unsigned char  jesjesfg;    /* PRIMARY SUBSYSTEM FLAGS        @L1C  */
  void * __ptr32 jesallop;    /* POINTER TO ALLOCATION DESCRIPTOR     */
  short int      jesalloa;    /* ASID OF ALLOCATION ADDRESS SPACE     */
  unsigned char  jesallof;    /* ALLOCATION FUNCTION FLAGS   @G860P2M */
  unsigned char  jesrsv08;    /* RESERVED                    @G383P2J */
  void * __ptr32 jespcdp;     /* POINTER IN CSA FOR PCDPARMS @G382P2P */
  int            jesaucbs;    /* NUMBER OF ALL UCBS IN THE   @G383P2J */
  int            jesduecb;    /* DISPLAY ALLOCATION SDUMP    @G383P2J */
  void * __ptr32 jesuplp;     /* UCB POINTER LIST ADDRESS    @G860P2P */
  void * __ptr32 jesmntp;     /* POINTER TO ARRAY OF MOUNT-  @G860PPK */
  void * __ptr32 jesctext;    /* POINTER TO THE PAGEABLE JESCT        */
  void * __ptr32 jesppt;      /* POINTER TO THE PROGRAM PROPERTIES    */
  void * __ptr32 jesrstrt;    /* POINTER TO RESTART CODE TABLE   @L3C */
  void * __ptr32 jesparse;    /* POINTER TO THE PARSER ROUTINE   @D2A */
  void * __ptr32 jesxb603;    /* POINTER TO RESTART COMPONENT    @R2C */
  void * __ptr32 jesdaca;     /* POINTER TO THE DEVICE ALLOCATION     */
  void * __ptr32 jesrsv28;    /* RESERVED FIELD                  @L6A */
  };

/* Values for field "jesflg" */
#define jesjssnt 0x80 /* IEFJSSNT EXISTS             @G385P2N */
#define jesfsit  0x40 /* FSI Trace installed.            @T3C */
#define jesfrqex 0x20 /* SSI function request exit            */
#define jesrsv15 0x10 /* RESERVED                    @G385P2N */
#define jesrsv16 0x08 /* RESERVED                    @G385P2N */
#define jesrsv17 0x04 /* RESERVED                    @G385P2N */
#define jesrsv18 0x02 /* RESERVED                    @G385P2N */
#define jesrsv19 0x01 /* RESERVED                    @G385P2N */

/* Values for field "jesjesfg" */
#define jespsuba 0x80 /* PRIMARY SUBSYSTEM ACTIVE INDICATOR   */
#define jespsubi 0x40 /* IF JESPSUBA=1 AND THIS BIT =0 THEN   */
#define jes3actv 0x20 /* JES3 SUBSYSTEM ACTIVE           @T4C */
#define jes3outd 0x10 /* JES3 support of OUTADD/OUTDEL        */
#define jesrsv24 0x08 /* RESERVED                        @L1A */
#define jesrsv25 0x04 /* RESERVED                        @L1A */
#define jesrsv26 0x02 /* RESERVED                        @L1A */
#define jesrsv27 0x01 /* RESERVED                        @L1A */

/* Values for field "jesallof" */
#define jesuasr  0x80 /* UNIT ALLOCATION STATUS RECORDING IS  */
#define jesuasf  0x40 /* UNIT ALLOCATION STATUS RECORDING     */
#define jesupler 0x20 /* UPL DOES NOT MATCH THE UCBS @G860P2M */
#define jesalrdy 0x10 /* ALLOCATION READY                @L3C */
#define jesv2edt 0x08 /* EDT VERSION 2 OR LATER INDICATOR     */
#define jesdefst 0x04 /* SET BY IEFAB4I0. WHEN ON, THIS BIT   */
#define jesibsav 0x02 /* Allocation Insulated DD support      */
#define jesrsv07 0x01 /* RESERVED                    @G383P2J */

#endif

#ifndef __jespext__
#define __jespext__

struct jespext {
  unsigned char  jessid[7];                         /* IDENTIFIER 'JESPEXT'            @H1A */
  unsigned char  jessvers;                          /* CONTROL BLOCK VERSION NUMBER    @H1A */
  void * __ptr32 jessjcnl;                          /* ADDRESS OF SCHEDULER JCL        @H1A */
  void * __ptr32 jessjdvt;                          /* ADDRESS OF JCL DEFINITION       @H1A */
  void * __ptr32 jessjrnl;                          /* ADDRESS OF JOURNAL WRITE RTNE   @H1A */
  void * __ptr32 jesdb401;                          /* Unused except for formatter use @P5C */
  void * __ptr32 jesxvnsl;                          /* IEFXVNSL ENTRY POINT            @L2A */
  void * __ptr32 jesgb4dc;                          /* IEFGB4DC ENTRY POINT            @L2A */
  void * __ptr32 jesgb4uv;                          /* IEFGB4UV ENTRY POINT            @L2A */
  void * __ptr32 jesab445;                          /* Address of the Devcie           @LEC */
  void * __ptr32 jesgb400;                          /* ALLOCATION PUT INTERFACE RTNE.  @L2A */
  void * __ptr32 jesqb551;                          /* IEFQB551 ENTRY POINT            @L2A */
  void * __ptr32 jesqb556;                          /* IEFQB556 ENTRY POINT            @L2A */
  void * __ptr32 jesxbput;                          /* JOURNAL PUT/GET INTERFACE RTN   @L2A */
  void * __ptr32 jesib650;                          /* IEFIB650 ENTRY POINT (MSG MOD)  @T1C */
  void * __ptr32 jessjf;                            /* ADDRESS OF SCHEDULER JCL        @D1A */
  int            jestiots;                          /* SIZE OF THE TASK I/O TABLE TIOT @L4A */
  int            jesmaxdd;                          /* MAXIMUM NUMBER OF SINGLE UNIT   @L4A */
  void * __ptr32 jespqmst;                          /* ADDRESS OF THE SWA MANAGER      @L5A */
  void * __ptr32 jespqdir;                          /* ADDRESS OF THE SWA MANAGER      @L5A */
  void * __ptr32 jesgdtok;                          /* ADDRESS OF THE ALLOCATION GET   @D4A */
  void * __ptr32 jessmsib;                          /* POINTER TO THE STORAGE MANAGEMENT    */
  void * __ptr32 jesqbsva;                          /* ADDRESS OF SWA MANAGER ROUTINE  @D5C */
  void * __ptr32 jesmechk;                          /* ADDRESS OF THE MUTUAL EXCLUSIVITY    */
  void * __ptr32 jesxbchk;                          /* Address of the scheduler checkpoint  */
  void * __ptr32 jesfsicb;                          /* Address of FSI trace Control         */
  void * __ptr32 jessjtcl;                          /* Address of the SWBTU processor  @L8A */
  int            jespptus;                          /* PPT table concurrent use count  @LBC */
  void * __ptr32 jespptsc;                          /* PPT scan routine IEFPPTSC:      @LBC */
  int            jesdsnno;                          /* Counter for final qualifier of  @P3C */
  unsigned char  jesdsnid[2];                       /* ID for temporary data sets on   @P6A */
  short int      jesrsvea;                          /* Reserved for future use         @P6C */
  int            jesssivt;                          /* Token for SSI vector table      @LDA */
  unsigned char  jesssipc[4];                       /* PC number for IEFSSI macro      @LDA */
  unsigned char  jesvtpc[4];                        /* PC number for IEFSSVT macro     @LDA */
  void * __ptr32 jesmsgt_a_;                        /* SSI message table address       @LDA */
  struct {
    unsigned char  _jespalf1; /* 1ST BYTE OF ALLOCXX SYSTEM FLAGS@02A */
    unsigned char  _jespalf2; /* 2ND BYTE OF ALLOCXX SYSTEM FLAGS@02A */
    unsigned char  _jespalf3; /* 3rd byte of ALLOCxx SYSTEM flags@LJC */
    unsigned char  _jespalf4; /* Reserved for future use         @02A */
    } jespalf;
  int            jesj201d;                          /* DOM ID for IEFJ201A             @05C */
  int            jesrsved;                          /* Reserved for future use         @P3A */
  int            jessch___moduletable[4];           /* Module table for Scheduler, each     */
  unsigned char  jessmf___limits___policy___ptr[8]; /* LIMIT table policy address      @LIA */
  int            jessmf___limits___policy___cnt;    /* LIMIT table use count           @LIA */
  void * __ptr32 jesjwtom;                          /* Address of IEFJWTOM             @LKA */
  void * __ptr32 jesjactl;                          /* Address of IEFJACTL             @LKA */
  void * __ptr32 jesjwtog;                          /* Address of IEFJWTOM glue        @LKA */
  unsigned char  jes_d_4rsv[16];                    /* Reserved and available          @LKC */
  };

#define jespalf1 jespalf._jespalf1
#define jespalf2 jespalf._jespalf2
#define jespalf3 jespalf._jespalf3
#define jespalf4 jespalf._jespalf4

/* Values for field "jespalf1" */
#define jesbr14l 0x80 /* On when SYSTEM IEFBR14_DELMIGDS      */
#define jesbr14n 0x40 /* On when SYSTEM IEFBR14_DELMIGDS      */
#define jestlibe 0x20 /* On when SYSTEM TAPELIB_PREF is       */
#define jestlibd 0x10 /* On when SYSTEM TAPELIB_PREF is       */
#define jesvufl  0x08 /* On when SYSTEM VERIFY_UNCAT is       */
#define jesvutrk 0x04 /* On when SYSTEM VERIFY_UNCAT is       */
#define jesvumtr 0x02 /* On when SYSTEM VERIFY_UNCAT is       */
#define jestdsfu 0x01 /* On when SYSTEM TEMPDSFORMAT is  @02A */

/* Values for field "jespalf2" */
#define jestdsfi 0x80 /* On when SYSTEM TEMPDSFORMAT is  @02A */
#define jesdseme 0x40 /* On when SYSTEM MEMDSENQMGMT is       */
#define jesdsemd 0x20 /* On when SYSTEM MEMDSENQMGMT is       */
#define jesvultr 0x10 /* On when SYSTEM VERIFY_UNCAT is       */
#define jesbchrs 0x08 /* On when SYSTEM BATCH_RCLMIGDS set    */
#define jesbchrp 0x04 /* On when SYSTEM BATCH_RCLMIGDS set    */
#define jesopexp 0x02 /* On when SYSTEM OPTCDB_SPLIT is       */
#define jesopcat 0x01 /* On when SYSTEM OPTCDB_SPLIT is       */

/* Values for field "jespalf3" */
#define jesssswa 0x80 /* On when SYSTEM SWBSTORAGE is set to  */
#define jesssatb 0x40 /* On when SYSTEM SWBSTORAGE is set to  */

/* Values for field "jes_d_4rsv" */
#define jesscver 8    /* CURRENT VERSION LEVEL           @LDC */

#endif

#pragma pack(reset)
