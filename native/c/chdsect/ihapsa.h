#ifdef __open_xl__
#pragma pack(1)
#else
#pragma pack(packed)
#endif

#ifndef __psa__
#define __psa__

struct psa
{
  union
  {
    struct
    {
      unsigned char _flcrnpsw[4]; /* -RESTART NEW PSW (AFTER IPL)        MDC001   */
      void *__ptr32 _filler1;     /* -  SECOND HALF OF RESTART NEW PSW     MDC128 */
    } flcippsw;
    struct
    {
      unsigned char _flceippsw[8]; /* FLCE 0x: IPL PSW */
    } flcesame;
  } _psa_union1;
  union
  {
    struct
    {
      unsigned char _flcropsw[8]; /* -      RESTART OLD PSW (AFTER IPL) */
    } flciccw1;
    unsigned char _flceiccw1[8]; /* FLCE 8x: IPL CCW1 */
  } _psa_union2;
  union
  {
    struct
    {
      void *__ptr32 _flccvt;     /* -    ADDRESS OF CVT (AFTER IPL).      @G50EP9A   */
      unsigned char _filler2[4]; /* -      RESERVED (AFTER IPL)  (MDC431)   @ZM48214 */
    } flciccw2;
    unsigned char _flceiccw2[8]; /* FLCE 10x: IPL CCW1 */
  } _psa_union3;
  union
  {
    struct
    {
      unsigned char _flceopsw[8]; /* -      EXTERNAL OLD PSW                          */
      unsigned char _flcsopsw[8]; /* -      SVC OLD PSW.  THIS OFFSET FIXED BY        */
      unsigned char _flcpopsw[8]; /* -      PROGRAM CHECK OLD PSW                     */
      unsigned char _flcmopsw[8]; /* -      MACHINE CHECK OLD PSW                     */
      unsigned char _flciopsw[8]; /* -      INPUT/OUTPUT OLD PSW                      */
      unsigned char _filler3[8];  /* -      RESERVED                         @G860PXK */
      struct
      {
        unsigned char _filler4[4]; /* -      1st 4 bytes are 0                    @H3A */
        void *__ptr32 _flccvt2;    /* -    ADDRESS OF CVT - USED BY DUMP               */
      } flccvt64;
      unsigned char _filler5[4];  /* -      RESERVED                         @G860PXH */
      unsigned char _filler6[4];  /* -      RESERVED - FLCTRACE DELETED DUE TO        */
      unsigned char _flcenpsw[4]; /* -EXTERNAL NEW PSW                                */
      void *__ptr32 _filler7;     /* -  SECOND HALF OF EXTERNAL NEW PSW               */
      unsigned char _flcsnpsw[4]; /* -SVC NEW PSW                                     */
      void *__ptr32 _filler8;     /* -  SECOND HALF OF SVC NEW PSW                    */
      unsigned char _flcpnpsw[4]; /* - PROGRAM CHECK NEW PSW, DISABLED FOR @02C       */
      void *__ptr32 _filler9;     /* -  SECOND HALF OF PROGRAM CHECK NEW PSW          */
      unsigned char _flcmnpsw[4]; /* -MACHINE CHECK NEW PSW              MDC003       */
      void *__ptr32 _filler10;    /* -  SECOND HALF OF MACHINE CHECK NEW PSW          */
      unsigned char _flcinpsw[4]; /* -INPUT/OUTPUT NEW PSW                            */
      void *__ptr32 _filler11;    /* -  SECOND HALF OF I/O NEW PSW                    */
    } _psa_struct1;
    unsigned char _flcer018[104]; /* FLCE 18x: reserved */
  } _psa_union4;
  union
  {
    int _psaeparm;               /* -         EXTERNAL INTERRUPTION PARAMETER  @G871A9A */
    unsigned char _flceeparm[4]; /* FLCE 80x: External interruption parameter           */
  } _psa_union5;
  union
  {
    struct
    {
      short int _psaspad;  /* -         ISSUING PROCESSOR'S PHYSICAL ADDRESS */
      short int _flceicod; /* -         EXTERNAL INTERRUPTION CODE           */
    } psaeepsw;
    struct
    {
      unsigned char _flcecpuad[2];  /* FLCE 84x: CPU address                */
      unsigned char _flceeicode[2]; /* FLCE 86x: External interruption code */
    } _psa_struct2;
  } _psa_union6;
  union
  {
    struct
    {
      unsigned char _filler12; /* -      RESERVED - SET TO ZERO                   */
      char _flcsvilc;          /* -       SVC INSTRUCTION LENGTH COUNTER - NUMBER */
      short int _flcsvcn;      /* -         SVC INTERRUPTION CODE - SVC NUMBER.   */
    } psaespsw;
    struct
    {
      struct
      {
        unsigned char _filler13; /* FLCE 88x: Reserved                     */
        unsigned char _flcesilc; /* FLCE 89x: SVC interruption length code */
      } flcesdatabyte0;
      unsigned char _flcesicode[2]; /* FLCE 8Ax: SVC interruption code */
    } flcesdata;
  } _psa_union7;
  union
  {
    struct
    {
      unsigned char _filler14; /* -      RESERVED - SET TO ZERO                     */
      char _flcpiilc;          /* -       PROGRAM INTERRUPT LENGTH COUNTER - NUMBER */
      struct
      {
        char _psaeecod; /* -       EXCEPTION-EXTENSION CODE.            @03C */
        char _psapicod; /* -       8-BIT INTERRUPT CODE.  THIS OFFSET FIXED  */
      } flcpicod;
      struct
      {
        unsigned char _filler15[3]; /* -                                           @L8A */
        struct
        {
          unsigned char _flcteab3; /* -      LAST BYTE OF TEA.                    @L8A */
        } flcdxc;
      } flctea;
    } psaeppsw;
    struct
    {
      struct
      {
        struct
        {
          unsigned char _filler16; /* FLCE 8Cx: Reserved                         */
          unsigned char _flcepilc; /* FLCE 8Dx: Program interruption length code */
        } flcepdatabyte0;
        struct
        {
          unsigned char _flcepicode0; /* FLCE 8Ex: Exception extension code */
          unsigned char _flcepicode1; /* FLCE 8Fx: 8-bit interruption code  */
        } flcepicode;
      } flcepdata;
      struct
      {
        unsigned char _filler17[3];
        struct
        {
          unsigned char _flcevxc; /* FLCE 93x: Vector exception code for PI 1B */
        } flcedxc;
      } flcepiinformation;
    } _psa_struct3;
  } _psa_union8;
  union
  {
    unsigned char _flcemcnum[2]; /* FLCE 94x: Monitor class number */
    struct
    {
      unsigned char _filler18;
      unsigned char _flcmcnum; /* -      MONITOR CLASS NUMBER */
    } _psa_struct4;
  } _psa_union9;
  struct
  {
    struct
    {
      unsigned char _flcpercd; /* -      PROGRAM EVENT RECORDING CODE */
    } flcepercode0;
    struct
    {
      unsigned char _flcatmid; /* -      ATM ID                               @LSA */
    } flceperatmid;
  } flcepercode;
  union
  {
    struct
    {
      unsigned char _flceperw0[4]; /* FLCE 98x: PER address word 0 */
      void *__ptr32 _flceperw1;    /* FLCE 9Cx: PER address word 1 */
    } flceper;
    struct
    {
      void *__ptr32 _flcper;      /* -         PER ADDRESS - ESA/390            @G860PXK */
      unsigned char _filler19;    /* -      RESERVED - SET TO ZERO                       */
      unsigned char _flcmtrcd[3]; /* -      MONITOR CODE (ESA/390)                       */
    } _psa_struct5;
  } _psa_union10;
  struct
  {
    unsigned char _flctearn; /* -      CONTAINS THE ACCESS REGISTER NUMBER  @L8C */
  } flceeaid;
  union
  {
    unsigned char _flcperrn;   /* -      CONTAINS THE PER STORAGE ACCESS      @L8A */
    unsigned char _flceperaid; /* FLCE A1x: PER access ID (the access register     */
  } _psa_union11;
  unsigned char flceopacid; /* FLCE A2x:                                              */
  struct
  {
    unsigned char _flcarch; /* -      Architecture information             @LSA */
  } flceamdid;
  union
  {
    unsigned char _psampl[4]; /* -      Used only prior to z/Architecture    @MFC */
    void *__ptr32 _flcempl;   /* FLCE A4x: MPL address                            */
  } _psa_union12;
  union
  {
    struct
    {
      struct
      {
        struct
        {
          unsigned char _filler20[6];
          unsigned char _flcetea6; /* FLCE AEx: Byte 6 of FlceTEA */
          unsigned char _flcetea7; /* FLCE AFx: Byte 7 of FlceTEA */
        } flcetea;
      } flceteid;
      unsigned char _filler21[8];
    } _psa_struct6;
    struct
    {
      struct
      {
        unsigned char _filler22[6];
        short int _flceteasn; /* FLCE AEx: ASN */
      } flceteasninfo;
      unsigned char _filler23[8];
    } _psa_struct7;
    struct
    {
      struct
      {
        unsigned char _filler24[4];
        int _flcepcnum; /* FLCE ACx: PC#. Bits 0-10 are 0, bit 11 is 1, */
      } flcetepcinfo;
      unsigned char _flcemonitorcode[8]; /* FLCE B0x: Monitor Code */
    } _psa_struct8;
    unsigned char _filler25[16]; /* -     RESERVED (ESA/390)               @G860PVB */
  } _psa_union13;
  union
  {
    struct
    {
      unsigned char _flcsid[4];  /* -      SUBSYSTEM ID                     @G860PVB */
      unsigned char _flciofp[4]; /* -      I/O INTERRUPTION PARAMETER       @G860PVB */
    } flciocdp;
    struct
    {
      unsigned char _flcessid[4];      /* FLCE B8x: Subsystem ID word          */
      unsigned char _flceiointparm[4]; /* FLCE BCx: I/O interruption parameter */
    } _psa_struct9;
  } _psa_union14;
  unsigned char flceiointid[4]; /* FLCE C0x: I/O interruption ID                          */
  unsigned char flcer0c4[4];    /* FLCE C4x: Reserved                                     */
  union
  {
    struct
    {
      unsigned char _flcfacl0;    /* Byte 0 of FLCFACL                    @LVA        */
      unsigned char _flcfacl1;    /* Byte 1 of FLCFACL                    @LVA        */
      unsigned char _flcfacl2;    /* Byte 2 of FLCFACL                    @LVA        */
      unsigned char _flcfacl3;    /* Byte 3 of FLCFACL                    @LVA        */
      unsigned char _flcfacl4;    /* Byte 4 of FLCFACL                    @PHA        */
      unsigned char _flcfacl5;    /* Byte 5 of FLCFACL                    @PHA        */
      unsigned char _flcfacl6;    /* Byte 6 of FLCFACL                    @PHA        */
      unsigned char _flcfacl7;    /* Byte 7 of FLCFACL                    @PHA        */
      unsigned char _flcfacl8;    /* Byte 8 of FLCFACL                    @M4A        */
      unsigned char _flcfacl9;    /* Byte 9 of FLCFACL                    @PPA        */
      unsigned char _filler26[6]; /* -      RESERVED                             @PPC */
    } flcfacl;
    struct
    {
      unsigned char _flcefacilitieslistbyte0; /* FLCE C8x            */
      unsigned char _flcefacilitieslistbyte1; /* FLCE C9x            */
      unsigned char _flcefacilitieslistbyte2; /* FLCE CAx            */
      unsigned char _flcefacilitieslistbyte3; /* FLCE CBx            */
      unsigned char _flcefacilitieslistbyte4; /* FLCE CCx            */
      unsigned char _flcefacilitieslistbyte5; /* FLCE CDx            */
      unsigned char _flcefacilitieslistbyte6; /* FLCE CEx            */
      unsigned char _flcefacilitieslistbyte7; /* FLCE CFx            */
      unsigned char _flcefacilitieslistbyte8; /* FLCE D0x bits 64-71 */
      unsigned char _flcefacilitieslistbyte9; /* FLCE D1x bits 72-79 */
      unsigned char _flcefacilitieslistbytea; /* FLCE D2x            */
      unsigned char _flcefacilitieslistbyteb; /* FLCE D3x            */
      unsigned char _flcefacilitieslistbytec; /* FLCE D4x            */
      unsigned char _flcefacilitieslistbyted; /* FLCE D5x            */
      unsigned char _flcefacilitieslistbytee; /* FLCE D6x            */
      unsigned char _flcefacilitieslistbytef; /* FLCE D7x            */
    } flcefacilitieslist;
  } _psa_union15;
  union
  {
    unsigned char _flcfacle[16];            /* -     Facilities List bytes 16-31. See     @MMA */
    unsigned char _flcefacilitieslist1[16]; /* FLCE D8x: Facilities list stored by STFLE.      */
  } _psa_union16;
  union
  {
    unsigned char _flcmcic[8];  /* -      MACHINE-CHECK INTERRUPTION CODE  @G860PVB */
    unsigned char _flcemcic[8]; /* FLCE E8x: Machine check interruption code        */
  } _psa_union17;
  unsigned char flcemcice[4];  /* FLCE F0x: Machine check interruption code              */
  unsigned char flceedcode[4]; /* FLCE F4x: External damage code                         */
  union
  {
    struct
    {
      void *__ptr32 _flcfsa;      /* -         FAILING STORAGE ADDRESS          @G860PXK */
      unsigned char _filler27[4]; /* -      RESERVED - SET TO ZERO           @G860PXK    */
    } _psa_struct10;
    unsigned char _flcefsa[8]; /* FLCE F8x: Failing storage address */
  } _psa_union18;
  union
  {
    unsigned char _flcfla[16]; /* -     FIXED LOGOUT AREA. SIZE FIXED BY     @L9C */
    struct
    {
      void *_flceemfctrarrayaddr; /* FLCE 100x: The enhanced monitor facility */
      int _flceemfctrarraysize;   /* FLCE 108x: The enhanced monitor facility */
      int _flceemfexceptioncnt;   /* FLCE 10Cx: The enhanced monitor facility */
    } _psa_struct11;
  } _psa_union19;
  union
  {
    unsigned char _flcrv110[16]; /* -     RESERVED.                            @L9A */
    struct
    {
      unsigned char _flcebea[8];  /* FLCE 110x: Breaking event address */
      unsigned char _flcer118[8]; /* FLCE 118x: Reserved               */
    } _psa_struct12;
  } _psa_union20;
  union
  {
    int _flcarsav[16]; /* -     ACCESS REGISTER SAVE AREA            @L9A */
    struct
    {
      unsigned char _flceropsw[16]; /* FLCE 120x: Restart old PSW  */
      unsigned char _flceeopsw[16]; /* FLCE 130x: External old PSW */
      unsigned char _flcesopsw[16]; /* FLCE 140x: SVC old PSW      */
      unsigned char _flcepopsw[16]; /* FLCE 150x: Program old PSW  */
    } _psa_struct13;
  } _psa_union21;
  union
  {
    unsigned char _flcfpsav[32]; /* -     FLOATING POINT REGISTER SAVE AREA */
    struct
    {
      unsigned char _flcemopsw[16]; /* FLCE 160x: Machine check old PSW */
      unsigned char _flceiopsw[16]; /* FLCE 170x: I/O old PSW           */
    } _psa_struct14;
  } _psa_union22;
  union
  {
    int _flcgrsav[16]; /* -       GENERAL REGISTER SAVE AREA */
    struct
    {
      unsigned char _flcer180[32];  /* FLCE 180x: reserved         */
      unsigned char _flcernpsw[16]; /* FLCE 1A0x: Restart new PSW  */
      unsigned char _flceenpsw[16]; /* FLCE 1B0x: External new PSW */
    } _psa_struct15;
  } _psa_union23;
  union
  {
    int _flccrsav[16]; /* -       CONTROL REGISTER SAVE AREA */
    struct
    {
      unsigned char _flcesnpsw[16]; /* FLCE 1C0x: SVC new PSW           */
      unsigned char _flcepnpsw[16]; /* FLCE 1D0x: Program new PSW       */
      unsigned char _flcemnpsw[16]; /* FLCE 1E0x: Machine check new PSW */
      unsigned char _flceinpsw[16]; /* FLCE 1F0x: I/O new PSW           */
    } _psa_struct16;
  } _psa_union24;
  struct
  {
    unsigned char _psapsa[4]; /* -    CONTROL BLOCK ACRONYM IN EBCDIC                */
    short int _psacpupa;      /* -         PHYSICAL CPU ADDRESS (CHANGED DURING ACR) */
    short int _psacpula;      /* -         LOGICAL CPU ADDRESS                       */
  } flchdend;
  void *__ptr32 psapccav; /* -         VIRTUAL ADDRESS OF PCCA                      */
  void *__ptr32 psapccar; /* -         REAL ADDRESS OF PCCA                         */
  void *__ptr32 psalccav; /* -         VIRTUAL ADDRESS OF LCCA                      */
  void *__ptr32 psalccar; /* -         REAL ADDRESS OF LCCA                         */
  void *__ptr32 psatnew;  /* -         TCB pointer. Field maintained for code       */
  void *__ptr32 psatold;  /* -         Pointer to current TCB or zero if in SRB     */
  void *__ptr32 psaanew;  /* ASCB pointer.  Field maintained for code               */
  void *__ptr32 psaaold;  /* -         Pointer to the home (current) ASCB.  @LQC    */
  struct
  {
    unsigned char _psasup1; /* -      FIRST BYTE OF PSASUPER  */
    unsigned char _psasup2; /* -      SECOND BYTE OF PSASUPER */
    unsigned char _psasup3; /* -      THIRD BYTE OF PSASUPER  */
    unsigned char _psasup4; /* -      FOURTH BYTE OF PSASUPER */
  } psasuper;
  unsigned char psarv22c[9];                      /* -     RESERVED                             @MWC        */
  unsigned char psa___workunit___cbf___atdisp[2]; /* @MHA                                                   */
  unsigned char psarv237;                         /* -     RESERVED                             @MKC        */
  union
  {
    unsigned char _psa___workunit___procclassatdisp[2]; /* -                          @MCA */
    struct
    {
      unsigned char _psa___workunit___procclassatdisp___byte0; /* @MCA */
      unsigned char _psa___workunit___procclassatdisp___byte1; /* @MCA */
    } _psa_struct17;
  } _psa_union25;
  union
  {
    unsigned char _psaprocclass[2];             /* -     PROCESSOR WUQ Offset.             */
    unsigned char _psa___bylpar___procclass[2]; /* - PROCESSOR WUQ Offset.            @H5A */
    struct
    {
      unsigned char _psaprocclass___byte0; /* @H4A */
      unsigned char _psaprocclass___byte1; /* @H4A */
    } _psa_struct18;
    struct
    {
      unsigned char _psa___bylpar___procclass___byte0; /* @H5A */
      unsigned char _psa___bylpar___procclass___byte1; /* @H5A */
    } _psa_struct19;
  } _psa_union26;
  unsigned char psaptype;     /* -      PROCESSOR TYPE INDICATOR             @H1A       */
  unsigned char psails;       /* -      INTERRUPT HANDLER LINKAGE STACK      @L9C       */
  unsigned char psalsvci[2];  /* -      LAST SVC ISSUED ON THIS PROCESSOR    @L6A       */
  unsigned char psaflags;     /* -      SYSTEM FLAGS                         @LOA       */
  unsigned char psarv241[10]; /* RESERVED FOR FUTURE USE - SC1C5.     @LOC              */
  unsigned char psascaff;     /* $$SCAFFOLD                                             */
  void *__ptr32 psalkcrf;     /* LINKAGE STACK POINTER SAVE AREA.     @D4A              */
  unsigned char psampsw[8];   /* - SETLOCK MODEL PSW                                    */
  unsigned char psaicnt[8];   /* -      Instruction count at last (re)dispatch          */
  unsigned char psaxad;       /* Must be x'AD' - ISV dependency       @MYC              */
  unsigned char psaintsm;     /* Used by IEAVEINT.                                      */
  unsigned char psarv262[14]; /* Reserved                             @MYC              */
  int psastosm;               /* -            STOSM PSASLSA,X'00' Instruction. In order */
  int psahlhis;               /* -         SAVE AREA FOR PSAHLHI              MDC050    */
  unsigned char psarecur;     /* -      RESTART FLIH RECURSION INDICATOR.  IF           */
  unsigned char psarssm;      /* -      STNSM AREA FOR IEAVERES              @L5C       */
  unsigned char psasnsm2;     /* -      STNSM AREA FOR IEAVTRT1 (MDC470) @G65RP9A       */
  unsigned char psartm1s;     /* -      Bits 0-7 of the current PSW are  @G383P9A       */
  void *__ptr32 psalwtsa;     /* -         REAL ADDRESS OF SAVE AREA USED WHEN  @LHC    */
  struct
  {
    struct
    {
      void *__ptr32 _psadispl; /* -  GLOBAL DISPATCHER LOCK  (MDC315) @G50DP9A        */
      void *__ptr32 _psaasml;  /* -         AUXILIARY STORAGE MANAGEMENT (ASM) LOCK   */
      void *__ptr32 _psasalcl; /* -  SPACE ALLOCATION LOCK  (MDC316)  @G50DP9A        */
      void *__ptr32 _psaiossl; /* -         IOS SYNCHRONIZATION LOCK           MDC010 */
      void *__ptr32 _psarsmdl; /* -         ADDRESS OF THE RSM DATA SPACE LOCK   @LBC */
      void *__ptr32 _psaiosul; /* -         IOS UNIT CONTROL BLOCK LOCK        MDC005 */
      void *__ptr32 _psarsmql; /* -         RSMQ lock                            @MIA */
      void *__ptr32 _psarv29c; /* -         RESERVED FOR LOCK EXPANSION          @LDC */
      void *__ptr32 _psarv2a0; /* -         RESERVED FOR LOCK EXPANSION          @LDC */
      void *__ptr32 _psatpacl; /* -         TCAM'S TPACBDEB LOCK               MDC009 */
      void *__ptr32 _psaoptl;  /* -   OPTIMIZER LOCK  (MDC317)         @G50DP9A       */
      void *__ptr32 _psarsmgl; /* -         RSM GLOBAL LOCK                  @G860PXH */
      void *__ptr32 _psavfixl; /* VSM FIXED SUBPOOLS LOCK          @G860PXH           */
      void *__ptr32 _psaasmgl; /* -         ASM GLOBAL LOCK                  @G860PXH */
      void *__ptr32 _psarsmsl; /* -         RSM STEAL LOCK                   @G860PXH */
      void *__ptr32 _psarsmxl; /* -         RSM CROSS MEMORY LOCK            @G860PXH */
      void *__ptr32 _psarsmal; /* -         RSM ADDRESS SPACE LOCK           @G860PXH */
      void *__ptr32 _psavpagl; /* VSM PAGEABLE SUBPOOLS LOCK       @G860PXH           */
      void *__ptr32 _psarsmcl; /* RSM COMMON LOCK                  @G860PXK           */
      void *__ptr32 _psarvlk2; /* RESERVED FOR LOCK EXPANSION      @G860PXH           */
    } psaclht1;
    struct
    {
      void *__ptr32 _psarsml;  /* RSM GLOBAL FUNCTION/RECOVERY                        */
      void *__ptr32 _psatrcel; /* TRACE BUFFER MANAGEMENT LOCK     @G860PXH           */
      void *__ptr32 _psaiosl;  /* -   IOS LOCK                             @D3C       */
      void *__ptr32 _psarvlk4; /* -         RESERVED FOR LOCK EXPANSION      @G50NP9A */
    } psaclht2;
    struct
    {
      void *__ptr32 _psacpul;  /* CPU TABLE LOCKS                  @G860PXH           */
      void *__ptr32 _psarvlk5; /* -         RESERVED FOR LOCK EXPANSION      @G50NP9A */
    } psaclht3;
    struct
    {
      void *__ptr32 _psacmsl;  /* -         CROSS MEMORY SERVICES LOCK                */
      void *__ptr32 _psalocal; /* -         LOCAL LOCK                                */
      void *__ptr32 _psarvlk6; /* -         RESERVED FOR LOCK EXPANSION      @G50NP9A */
    } psaclht4;
  } psaclht;
  void *__ptr32 psalcpua; /* -         LOGICAL CPU ADDRESS FOR LOCK INSTRUCTION.    */
  struct
  {
    struct
    {
      unsigned char _psaclhs1; /* -      FIRST BYTE OF PSACLHS. (MDC384)  @G860PXH */
      unsigned char _psaclhs2; /* -      SECOND BYTE OF PSACLHS. (MDC385) @G860PXH */
      unsigned char _psaclhs3; /* -      THIRD BYTE OF PSACLHS  (MDC386)  @G50EP9A */
      unsigned char _psaclhs4; /* -      FOURTH BYTE OF PSACLHS  (MDC392) @G50EP9A */
    } psaclhs;
  } psahlhi;
  void *__ptr32 psalita;      /* -  ADDRESS OF LOCK INTERFACE TABLE. @ZM48253           */
  unsigned char psastor8[8];  /* -      8-BYTE value for master's STO        @LSA       */
  int psacr0;                 /* -         SAVE AREA FOR CONTROL REGISTER 0             */
  unsigned char psamchfl;     /* -      MCH RECURSION FLAGS                             */
  unsigned char psasymsk;     /* -      THIS FIELD WILL BE USED IN CONJUNCTION          */
  unsigned char psaactcd;     /* -      ACTION CODE SUPPLIED BY OPERATOR     @LHC       */
  unsigned char psamchic;     /* -      MCH INITIALIZATION COMPLETE FLAGS  MDC098       */
  void *__ptr32 psawkrap;     /* -         REAL ADDRESS OF VARY CPU PARAMETER LIST      */
  void *__ptr32 psawkvap;     /* -         VIRTUAL ADDRESS OF VARY CPU PARAMETER        */
  short int psavstap;         /* -         WORK AREA FOR VARY CPU             MDC108    */
  short int psacpusa;         /* -         PHYSICAL CPU ADDRESS (STATIC)  (MDC131)      */
  int psastor;                /* -         MASTER MEMORY'S SEGMENT TABLE ORIGIN         */
  unsigned char psaidawk[90]; /* -     WORK SAVE AREA FOR private                       */
  short int psaret;           /* -            BSM 0,14 BRANCH RETURN TO CALLER     @P5A */
  short int psaretcd;         /* -            BSM 0,14 BRANCH RETURN TO CALLER     @P5A */
  struct
  {
    unsigned char _psaval___machine; /* First byte. Sample values: */
    unsigned char _filler28;
  } psaval;
  struct
  {
    struct
    {
      void *__ptr32 _psacstk;  /* -         ADDRESS OF CURRENTLY USED FUNCTIONAL      */
      void *__ptr32 _psanstk;  /* -         ADDRESS OF NORMAL FRR STACK        MDC062 */
      void *__ptr32 _psasstk;  /* -         ADDRESS OF SVC-I/O-DISPATCHER FRR STACK   */
      void *__ptr32 _psassav;  /* -         ADDRESS OF INTERRUPTED STACK SAVED BY     */
      void *__ptr32 _psamstk;  /* -         ADDRESS OF MCH FRR STACK           MDC067 */
      void *__ptr32 _psamsav;  /* -         ADDRESS OF INTERRUPTED STACK SAVED BY     */
      void *__ptr32 _psapstk;  /* -         ADDRESS OF PROGRAM CHECK FLIH FRR STACK   */
      void *__ptr32 _psapsav;  /* -         ADDRESS OF INTERRUPTED STACK SAVED BY     */
      void *__ptr32 _psaestk1; /* -         ADDRESS OF EXTERNAL FLIH FRR STACK FOR    */
      void *__ptr32 _psaesav1; /* -         ADDRESS OF INTERRUPTED STACK SAVED BY     */
      void *__ptr32 _psaestk2; /* -         ADDRESS OF EXTERNAL FLIH FRR STACK FOR    */
      void *__ptr32 _psaesav2; /* -         ADDRESS OF INTERRUPTED STACK SAVE BY      */
      void *__ptr32 _psaestk3; /* -         ADDRESS OF EXTERNAL FLIH FRR STACK FOR    */
      void *__ptr32 _psaesav3; /* -         ADDRESS OF INTERRUPTED STACK SAVED BY     */
      void *__ptr32 _psarstk;  /* -         ADDRESS OF RESTART FLIH FRR STACK  MDC077 */
      void *__ptr32 _psarsav;  /* -         ADDRESS OF INTERRUPTED STACK SAVED BY     */
    } psarsvte;
  } psarsvt;
  unsigned char psalwpsw[8];  /* -      PSW OF WORK INTERRUPTED WHEN A       @LHC       */
  double psarv3c8;            /* Reserved                             @M8C              */
  void *__ptr32 psatstk;      /* -         ADDRESS OF RTM RECOVERY STACK.               */
  void *__ptr32 psatsav;      /* -         ADDRESS OF ERROR STACK SAVED BY RTM  @L7A    */
  void *__ptr32 psaastk;      /* -         ADDRESS OF ACR FRR STACK.            @L7A    */
  void *__ptr32 psaasav;      /* -         ADDRESS OF INTERRUPT STACK SAVED BY  @L7A    */
  unsigned char psartpsw[8];  /* -      RESUME PSW FOR RTM SETRP RETRY       @L7A       */
  unsigned char psapcr0e[8];  /* -      Temp for PC FLIH/Disp for CR0E       @0MC       */
  unsigned char psasfacc[4];  /* - SETFRR ABEND COMPLETION CODE USED WHEN               */
  int psalsfcc;               /* -            L  1,PSASFACC INSTRUCTION TO LOAD    @P5A */
  short int psasvc13;         /* -            AN SVC 13 INSTRUCTION                @P5A */
  unsigned char psafpfl;      /* -      See LCCAFPFL                         @MEC       */
  unsigned char psainte;      /* -      FLAGS FOR CPU TIMER  (MDC466)    @ZM48078       */
  unsigned char psarv3fc[12]; /* -     Reserved                             @MxC        */
  void *__ptr32 psaatcvt;     /* -         ADDRESS OF VTAM ATCVT.  INITIALIZED BY       */
  void *__ptr32 psawtcod;     /* -         WAIT STATE CODE LOADED               @LHC    */
  void *__ptr32 psascwa;      /* -         ADDRESS OF SUPERVISOR CONTROL CPU            */
  void *__ptr32 psarsmsa;     /* -         ADDRESS OF RSM CPU RELATED WORK              */
  unsigned char psascpsw[4];  /* - MODEL PSW                                            */
  void *__ptr32 _filler29;    /* -         MODEL PSW SECOND HALF  (MDC325)  @G50DP9A    */
  unsigned char psasmpsw[4];  /* - SRB DISPATCH PSW  (MDC326)      @G50DP9A             */
  void *__ptr32 _filler30;    /* -         DISPATCH PSW SECOND HALF                     */
  unsigned char psapcpsw[16]; /* =     TEMPORARY OLD PSW STORAGE FOR PROGRAM            */
  unsigned char psarv438[8];  /* =     Reserved                             @M8C        */
  unsigned char psamcx16[16]; /* -     MCH exit PSW16                       @M8A        */
  unsigned char psarsp16[16]; /* -     Resume PSW field for restart interrupt           */
  union
  {
    unsigned char _psapswsv16[16]; /* -     PSW SAVE AREA FOR DISPATCHER AND ACR @M8A */
    struct
    {
      double _filler31;           /* -           Part of PSAPSWSV16                   @M8C */
      unsigned char _psapswsv[8]; /* -      PSW SAVE AREA FOR DISPATCHER AND ACR           */
    } _psa_struct20;
  } _psa_union27;
  unsigned char psacput[8]; /* -      SUPERVISOR CPU TIMER SAVE AREA                  */
  struct
  {
    unsigned char _psapcfb1; /* -      FUNCTION VALUE  (MDC484)         @G383P9A */
    unsigned char _psapcfb2; /* -      FUNCTION FLAGS  (MDC491)         @G383P9A */
    unsigned char _psapcfb3; /* -      RECURSION FLAGS  (MDC494)        @G383P9A */
    unsigned char _psapcfb4; /* -      RECURSION FLAGS                           */
  } psapcfun;
  short int psapcps2;         /* -         PASID AT TIME OF SECOND LEVEL    @G383P9A    */
  unsigned char psarv47e[2];  /* -      RESERVED                         @G860PXK       */
  unsigned char psapcwka[24]; /* -     Work area for PC FLIH. Must be                   */
  short int psapcps3;         /* -         PASID AT TIME OF THIRD LEVEL     @G383P9A    */
  short int psapcps4;         /* -         PASID AT TIME OF FOURTH LEVEL                */
  struct
  {
    unsigned char _filler32; /* -      RESERVED - FIRST BYTE OF PSAMODEW         */
    unsigned char _psamflgs; /* -      SECOND BYTE OF PSAMODEW (MDC604) @G383P9A */
    unsigned char _psamodeh; /* -      SECOND HALFWORD OF PSAMODEW.     @G383P9A */
    unsigned char _psamode;  /* -      SYSTEM MODE INDICATOR AND DISPLACEMENT    */
  } psamodew;
  unsigned char _filler33[3]; /* -      RESERVED                         @G860PXK       */
  unsigned char psastnsm;     /* -      STNSM TARGET USED BY EXIT PROLOGUE              */
  int psalkjw;                /* -         LOCAL LOCK RELEASE SRB JOURNAL   @G383P9A    */
  struct
  {
    int _psafzero; /* -         FULLWORD OF ZERO     (MDC612)    @G383P9A */
    int _filler34; /* -         FULLWORD OF ZERO     (MDC612)    @G383P9A */
  } psadzero;
  int psalkjw2;           /* -         CMS LOCK RELEASE JOURNAL WORD.   @G383P9A    */
  void *__ptr32 psalkpt;  /* -   SETLOCK TEST,TYPE=HIER                             */
  void *__ptr32 psalaa;   /* -      LE Anchor Area. Owner: LE            @LVA       */
  void *__ptr32 psalit2;  /* -  POINTER TO THE EXTENDED LOCK         @LDA           */
  void *__ptr32 psaecltp; /* -   POINTER TO THE EXTENDED CURRENT      @LDA          */
  struct
  {
    unsigned char _psalheb0; /* -      BYTE 0 OF THE CURRENT LOCK HELD      @LDA */
    unsigned char _psalheb1; /* -      BYTE 1 OF THE CURRENT LOCK HELD      @LDA */
    unsigned char _psalheb2; /* -      BYTE 2 OF THE CURRENT LOCK HELD      @LDA */
    unsigned char _psalheb3; /* -      BYTE 3 OF THE CURRENT LOCK HELD      @LDA */
  } psaclhse;
  unsigned char psarv4c8[8];    /* -    RESERVED FOR FUTURE LOCK EXPANSION.  @LDA         */
  unsigned char psarv4d0[144];  /* -    RESERVED.                            @0KC         */
  unsigned char psadiag560[36]; /* -     Diagnostic data for IBM use only     @0KA        */
  unsigned char psarv584[4];    /* -      RESERVED.                            @0KA       */
  unsigned char psahwfb;        /* -      HARDWARE FLAG BYTE.                  @L3A       */
  unsigned char psacr0cb;       /* -      CR0 CONTROL BYTE USED BY PROTPSA MACRO          */
  unsigned char psarv58a[2];    /* -      RESERVED                             @PJC       */
  int psacr0sv;                 /* -         CR0 SAVE AREA USED BY PROTPSA MACRO          */
  int psapccr0;                 /* -         PROGRAM CHECK FLIH CR0 SAVE AREA             */
  int psarcr0;                  /* -         RESTART FLIH CR0 SAVE AREA                   */
  struct
  {
    short int _psatkn; /* -         CURRENT STACK TOKEN     (MDC610) @G383P9A */
    short int _psaasd; /* -         CURRENT STACK ADDRESS SPACE               */
    int _psasel;       /* -         CURRENT STACK ELEMENTS ADDRESS            */
  } psastke;
  unsigned char psaskpsw[4];                /* PCLINK STACK/UNSTACK MODEL PSW                         */
  void *__ptr32 psaskps2;                   /* -         PCLINK PSW ADDRESS      (MDC604) @G383P9A    */
  void *__ptr32 psacpcls;                   /* -      PCLINK WORKAREA - CURRENT STACK      @L9C       */
  unsigned char psarv5ac[4];                /* -      RESERVED.                            @L9A       */
  void *__ptr32 psascfs;                    /* -      ADDRESS OF THE SUPERVISOR CONTROL    @L8C       */
  void *__ptr32 psapawa;                    /* -      ADDRESS OF PC/AUTH WORK AREA.        @L8A       */
  unsigned char psascfb;                    /* -      SUPERVISOR CONTROL FLAG BYTE.        @L1A       */
  unsigned char psarv5b9[3];                /* -      RESERVED                             @PJC       */
  unsigned char psacr0m1[4];                /* MASK OF CR0 WITH EXTERNAL MASK BITS  @0HC              */
  unsigned char psacr0m2[4];                /* MASK OF CR0 WITH ONLY EXTERNAL MASK  @0HC              */
  unsigned char psarv5c4[4];                /* -      RESERVED                             @MAA       */
  unsigned char psa___cr0emaskoffextint[8]; /* Mask of bits to turn                                   */
  unsigned char psa___cr0emaskonextint[8];  /* Mask of bits to turn                                   */
  struct
  {
    unsigned char _psa___cr0esavearea___hw[4]; /* High word save area for high word of */
    unsigned char _psa___cr0esavearea___lw[4]; /* Low word save area for low word of   */
  } psa___cr0esavearea;
  union
  {
    unsigned char _psa___windowworkarea[16]; /* WorkArea for IEAMWIN                @0IA */
    struct
    {
      unsigned char _psa___windowtoddelta[8]; /* Difference in TOD values - used in */
      unsigned char _filler35[8];
    } _psa_struct21;
    struct
    {
      unsigned char _psa___windowtoddelta___hw[4]; /* High word area for difference in TOD */
      unsigned char _psa___windowtoddelta___lw[4]; /* Low word area for difference in TOD  */
      unsigned char _filler36[8];
    } _psa_struct22;
  } _psa_union28;
  unsigned char psa___windowlastopentod[8];       /* TOD when IEAMWIN last opened a window                  */
  unsigned char psa___windowcurrenttod[8];        /* TOD when IEAMWIN last checked to open                  */
  unsigned char psarv600[80];                     /* -     RESERVED                             @0IC        */
  double psa___time___on___cp;                    /* -      Current SRB's accumulated CPU time   @0CA       */
  double psatime;                                 /* -         CURRENT SRB'S ACCUMULATED CPU TIME   @01C    */
  int psasrsav;                                   /* -        ADDRESS OF CURRENT FRR STACK     @G383P9A     */
  unsigned char psaesc8[12];                      /* -     Save area for IEAVESC8               @LPA        */
  unsigned char psadexmw[8];                      /* -         Work area for dispatcher CR3/4       @LVC    */
  unsigned char psadsars[64];                     /* -     DISPATCHER ACCESS REGISTER SAVE AREA @L9C        */
  double psa___pcflih___trace___interrupt___cput; /* - Trace interrupt CPU timer saved                      */
  union
  {
    double _psadtsav;           /* -            CPU TIMER VALUE AT LAST DISPATCH,    @01C */
    unsigned char _psaff6c0[8]; /* INITIALIZE FIELD PSADTSAV     @ZMC3284                 */
  } _psa_union29;
  struct
  {
    struct
    {
      int _psadsins; /* -        DISPATCHER Secondary ASTE Inst# S/A  @LVA */
      struct
      {
        short int _psadpkm; /* -        DISPATCHER PROGRAM KEY MASK SAVE AREA */
        short int _psadsas; /* -        DISPATCHER SECONDARY ASID SAVE AREA   */
      } psadpksa;
    } psadcr3;
    struct
    {
      int _psadpins; /* -        DISPATCHER Primary ASTE Inst# S/A    @LVA */
      struct
      {
        short int _psadax;  /* -        DISPATCHER  AUTHORIZATION        @G383P9A */
        short int _psadpas; /* -        DISPATCHER PRIMARY ASID SAVE     @G383P9A */
      } psadaxpa;
    } psadcr4;
  } psadexms;
  double psa___time___on___zcbp___normalized; /* - Current SRB's accumulated CPU                        */
  unsigned char psarv6e0[192];                /* -    RESERVED                             @MTC         */
  double psaecvt;                             /* Address of ECVT                      @M3A              */
  double psaxcvt;                             /* Address of XCVT                      @M3A              */
  unsigned char psadatlk[48];                 /* -    AREA FOR DAT-OFF ASSIST LINKAGE CODE              */
  void *__ptr32 psadatof;                     /* -         REAL STORAGE ADDRESS OF THE DAT-OFF          */
  int psadatln;                               /* -        LENGTH OF THE DAT-OFF INDEX TABLE             */
  void *__ptr32 psatbvtv;                     /* -         VIRTUAL ADDRESS CORRESPONDING TO             */
  unsigned char psatrace;                     /* -      SYSTEM TRACE FLAGS.              @G860PXK       */
  unsigned char psarv7ed[3];                  /* -      RESERVED FOR SYSTEM TRACE.           @PJC       */
  void *psatbvtr;                             /* -        REAL ADDRESS OF SYSTEM TRACE BUFFER           */
  void *__ptr32 psatrvt;                      /* -  ADDRESS OF SYSTEM TRACE VECTOR                      */
  void *__ptr32 psatot;                       /* -  ADDRESS OF SYSTEM TRACE OPERAND                     */
  union
  {
    struct
    {
      double _psaus2st; /* START SECOND SET OF ASSIGNED     @G383PXU */
      unsigned char _filler37[8];
    } _psa_struct23;
    struct
    {
      int _psacdsae; /* CALLDISP REGISTER 14 SAVE AREA   @G383PXU */
      int _psacdsaf; /* CALLDISP REGISTER 15 SAVE AREA   @G383PXU */
      int _psacdsa0; /* CALLDISP REGISTER 0  SAVE AREA   @G383PXU */
      int _psacdsa1; /* CALLDISP REGISTER 1  SAVE AREA   @G383PXU */
    } psacdsav;
  } _psa_union30;
  int psagspsw;                           /* GLOBAL SCHEDULE SYSTEM MASK SAVE @ZA63674              */
  int psagsrgs;                           /* GLOBAL SCHEDULE REGISTER SAVE    @ZA63674              */
  void *__ptr32 psa___masterasterealaddr; /* @MUC                                                   */
  int psasv01r;                           /* IEAVTRG1 register 1 save area.       @PAA              */
  int psasv14r;                           /* IEAVTRG1 register 14 save area.      @PAA              */
  int psaems2r;                           /* -        REGISTER SAVE AREA                            */
  struct
  {
    int _psatrgr0; /* -        TRACE REGISTER 0 SAVE AREA.      @G860PXH */
    int _psatrgr1; /* -        TRACE REGISTER 1 SAVE AREA.      @G860PXH */
    int _psatrgr2; /* -        TRACE REGISTER 2 SAVE AREA.      @G860PXH */
    int _psatrgr3; /* -        TRACE REGISTER 3 SAVE AREA.      @G860PXH */
    int _psatrgr4; /* -        TRACE REGISTER 4 SAVE AREA.      @G860PXH */
    int _psatrgr5; /* -        TRACE REGISTER 5 SAVE AREA.      @G860PXH */
    int _psatrgr6; /* -        TRACE REGISTER 6 SAVE AREA.      @G860PXH */
    int _psatrgr7; /* -        TRACE REGISTER 7 SAVE AREA.      @G860PXH */
    int _psatrgr8; /* -        TRACE REGISTER 8 SAVE AREA.      @G860PXH */
    int _psatrgr9; /* -        TRACE REGISTER 9 SAVE AREA.      @G860PXH */
    int _psatrgra; /* -        TRACE REGISTER 10 SAVE AREA.     @G860PXH */
    int _psatrgrb; /* -        TRACE REGISTER 11 SAVE AREA.     @G860PXH */
    int _psatrgrc; /* -        TRACE REGISTER 12 SAVE AREA.     @G860PXH */
    int _psatrgrd; /* -        TRACE REGISTER 13 SAVE AREA.     @G860PXH */
    int _psatrgre; /* -        TRACE REGISTER 14 SAVE AREA.     @G860PXH */
    int _psatrgrf; /* -        TRACE REGISTER 15 SAVE AREA.     @G860PXH */
  } psatrsav;
  unsigned char psatrsv1[4];  /* -     Trace Save 1                         @M8A        */
  unsigned char psatrsvs[4];  /* -     Trace Save for SLIP/PER              @M8A        */
  unsigned char psatrsv2[8];  /* -     Trace Save 2                         @M8A        */
  unsigned char psarv878[40]; /* -     RESERVED.                            @M8A        */
  unsigned char psagsavh[8];  /* -     Register save area used by           @09C        */
  union
  {
    unsigned char _psagsav[64];  /* -         REGISTER SAVE AREA USED BY   */
    unsigned char _psaff8a8[64]; /* INITIALIZE FIELD PSAGSAV      @ZMC3284 */
  } _psa_union31;
  int psascrg1;    /* -        GLOBAL SCHEDULE REGISTER SAVE AREA            */
  int psascrg2;    /* -        GLOBAL SCHEDULE REGISTER SAVE AREA            */
  int psagpreg[3]; /* -       REGISTER SAVE AREA FOR SVC FLIH                */
  int psarsreg;    /* -        RESTART FLIH REGISTER SAVE       @G860PXK     */
  int psapcgr8;    /* -        PROGRAM FLIH REGISTER 8 SAVE AREA             */
  int psapcgr9;    /* -        PROGRAM FLIH REGISTER 9 SAVE AREA             */
  struct
  {
    int _psapcgra; /* -        PROGRAM FLIH REGISTER 10 SAVE AREA */
    int _psapcgrb; /* -        PROGRAM FLIH REGISTER 11 SAVE AREA */
  } psapcgab;
  struct
  {
    int _psalkr0;  /* -        IEAVELK REGISTER 0 SAVE AREA     @G860PXK */
    int _psalkr1;  /* -        IEAVELK REGISTER 1 SAVE AREA     @G860PXK */
    int _psalkr2;  /* -        IEAVELK REGISTER 2 SAVE AREA     @G860PXK */
    int _psalkr3;  /* -        IEAVELK REGISTER 3 SAVE AREA     @G860PXK */
    int _psalkr4;  /* -        IEAVELK REGISTER 4 SAVE AREA     @G860PXK */
    int _psalkr5;  /* -        IEAVELK REGISTER 5 SAVE AREA     @G860PXK */
    int _psalkr6;  /* -        IEAVELK REGISTER 6 SAVE AREA     @G860PXK */
    int _psalkr7;  /* -        IEAVELK REGISTER 7 SAVE AREA     @G860PXK */
    int _psalkr8;  /* -        IEAVELK REGISTER 8 SAVE AREA     @G860PXK */
    int _psalkr9;  /* -        IEAVELK REGISTER 9 SAVE AREA     @G860PXK */
    int _psalkr10; /* -        IEAVELK REGISTER 10 SAVE AREA    @G860PXK */
    int _psalkr11; /* -        IEAVELK REGISTER 11 SAVE AREA    @G860PXK */
    int _psalkr12; /* -        IEAVELK REGISTER 12 SAVE AREA    @G860PXK */
    int _psalkr13; /* -        IEAVELK REGISTER 13 SAVE AREA    @G860PXK */
    int _psalkr14; /* -        IEAVELK REGISTER 14 SAVE AREA    @G860PXK */
    int _psalkr15; /* -        IEAVELK REGISTER 15 SAVE AREA    @G860PXK */
  } psalksa;
  union
  {
    unsigned char _psaslsa[72];  /* -         SINGLE LEVEL SAVE AREA USED BY DISABLED */
    unsigned char _psaff950[72]; /* INITIALIZE FIELD PSASLSA       @ZMC3284           */
  } _psa_union32;
  union
  {
    unsigned char _psajstsa[64]; /* -     SAVE AREA FOR JOB STEP TIMING        @H1A */
    unsigned char _psaff998[64]; /* INITIALIZE FIELD PSAJSTSA         @H1A          */
  } _psa_union33;
  union
  {
    struct
    {
      double _psaus2nd; /* END SECOND SET OF ASSIGNED           @H1M */
      unsigned char _filler38[56];
    } _psa_struct24;
    struct
    {
      int _psaslkr0; /* -        IEAVESLK REGISTER 0 SAVE AREA        @P4A */
      int _psaslkr1; /* -        IEAVESLK REGISTER 1 SAVE AREA        @P4A */
      int _psaslkr2; /* -        IEAVESLK REGISTER 2 SAVE AREA        @P4A */
      int _psaslkr3; /* -        IEAVESLK REGISTER 3 SAVE AREA        @P4A */
      int _psaslkr4; /* -        IEAVESLK REGISTER 4 SAVE AREA        @P4A */
      int _psaslkr5; /* -        IEAVESLK REGISTER 5 SAVE AREA        @P4A */
      int _psaslkr6; /* -        IEAVESLK REGISTER 6 SAVE AREA        @P4A */
      int _psaslkr7; /* -        IEAVESLK REGISTER 7 SAVE AREA        @P4A */
      int _psaslkr8; /* -        IEAVESLK REGISTER 8 SAVE AREA        @P4A */
      int _psaslkr9; /* -        IEAVESLK REGISTER 9 SAVE AREA        @P4A */
      int _psaslkra; /* -        IEAVESLK REGISTER 10 SAVE AREA       @P4A */
      int _psaslkrb; /* -        IEAVESLK REGISTER 11 SAVE AREA       @P4A */
      int _psaslkrc; /* -        IEAVESLK REGISTER 12 SAVE AREA       @P4A */
      int _psaslkrd; /* -        IEAVESLK REGISTER 13 SAVE AREA       @P4A */
      int _psaslkre; /* -        IEAVESLK REGISTER 14 SAVE AREA       @P4A */
      int _psaslkrf; /* -        IEAVESLK REGISTER 15 SAVE AREA       @P4A */
    } psaslksa;
  } _psa_union34;
  unsigned char psa___setlocki___savearea[8]; /* SETLOCKI Register save area       @MSA                 */
  int psa___lastlogcpuheldlock;               /* When waiting to obtain a spin lock, the                */
  unsigned char psarva24[24];                 /* -     RESERVED                             @MSC        */
  unsigned char psascsav[64];                 /* IEAVESC0 save area                   @P7A              */
  unsigned char psasflgs;                     /* Schedule flags                       @P8A              */
  unsigned char psamiscf;                     /* Miscellaneous flags                  @LVA              */
  unsigned char psarva7e[2];                  /* Reserved for future use - SC1C5      @LVC              */
  unsigned char psarva80[188];                /* -    RESERVED                             @P8C         */
  void *__ptr32 psagsch7;                     /* -  ENABLED GLOBAL SCHEDULE ENTRY                       */
  void *__ptr32 psagsch8;                     /* -  DISABLED GLOBAL SCHEDULE ENTRY                      */
  void *__ptr32 psalsch1;                     /* -  ENABLED SCHEDULE ENTRY POINT                        */
  void *__ptr32 psalsch2;                     /* -  DISABLED SCHEDULE ENTRY POINT                       */
  void *__ptr32 psasvt;                       /* -  ADDRESS OF SUPERVISOR VECTOR TABLE                  */
  void *__ptr32 psasvtx;                      /* Address of Supervisor Vector Table   @LNC              */
  struct
  {
    void *__ptr32 _psaffrr;  /* Fast FRR address.  This field is     @PSC */
    void *__ptr32 _psaffrrs; /* Fast FRR stack.  This field is       @PSA */
  } psafafrr;
  unsigned char psarvb5c[36]; /* -     Reserved                             @PSC        */
  union
  {
    unsigned char _psarvb80[1112]; /* -  Reserved                             @0LC */
    unsigned char _psastak[1112];  /* -  Do not use.                          @0LC */
  } _psa_union35;
  unsigned char psarvfd8[40]; /* -    Reserved                             @PJC         */
  double psaend;              /* -           END OF PSA            (MDC612)   @G383P9A  */
};

#define flcrnpsw _psa_union1.flcippsw._flcrnpsw
#define flceippsw _psa_union1.flcesame._flceippsw
#define flcropsw _psa_union2.flciccw1._flcropsw
#define flceiccw1 _psa_union2._flceiccw1
#define flccvt _psa_union3.flciccw2._flccvt
#define flceiccw2 _psa_union3._flceiccw2
#define flceopsw _psa_union4._psa_struct1._flceopsw
#define flcsopsw _psa_union4._psa_struct1._flcsopsw
#define flcpopsw _psa_union4._psa_struct1._flcpopsw
#define flcmopsw _psa_union4._psa_struct1._flcmopsw
#define flciopsw _psa_union4._psa_struct1._flciopsw
#define flccvt2 _psa_union4._psa_struct1.flccvt64._flccvt2
#define flcenpsw _psa_union4._psa_struct1._flcenpsw
#define flcsnpsw _psa_union4._psa_struct1._flcsnpsw
#define flcpnpsw _psa_union4._psa_struct1._flcpnpsw
#define flcmnpsw _psa_union4._psa_struct1._flcmnpsw
#define flcinpsw _psa_union4._psa_struct1._flcinpsw
#define flcer018 _psa_union4._flcer018
#define psaeparm _psa_union5._psaeparm
#define flceeparm _psa_union5._flceeparm
#define psaspad _psa_union6.psaeepsw._psaspad
#define flceicod _psa_union6.psaeepsw._flceicod
#define flcecpuad _psa_union6._psa_struct2._flcecpuad
#define flceeicode _psa_union6._psa_struct2._flceeicode
#define flcsvilc _psa_union7.psaespsw._flcsvilc
#define flcsvcn _psa_union7.psaespsw._flcsvcn
#define flcesilc _psa_union7.flcesdata.flcesdatabyte0._flcesilc
#define flcesicode _psa_union7.flcesdata._flcesicode
#define flcpiilc _psa_union8.psaeppsw._flcpiilc
#define psaeecod _psa_union8.psaeppsw.flcpicod._psaeecod
#define psapicod _psa_union8.psaeppsw.flcpicod._psapicod
#define flcteab3 _psa_union8.psaeppsw.flctea.flcdxc._flcteab3
#define flcepilc _psa_union8._psa_struct3.flcepdata.flcepdatabyte0._flcepilc
#define flcepicode0 _psa_union8._psa_struct3.flcepdata.flcepicode._flcepicode0
#define flcepicode1 _psa_union8._psa_struct3.flcepdata.flcepicode._flcepicode1
#define flcevxc _psa_union8._psa_struct3.flcepiinformation.flcedxc._flcevxc
#define flcemcnum _psa_union9._flcemcnum
#define flcmcnum _psa_union9._psa_struct4._flcmcnum
#define flcpercd flcepercode.flcepercode0._flcpercd
#define flcatmid flcepercode.flceperatmid._flcatmid
#define flceperw0 _psa_union10.flceper._flceperw0
#define flceperw1 _psa_union10.flceper._flceperw1
#define flcper _psa_union10._psa_struct5._flcper
#define flcmtrcd _psa_union10._psa_struct5._flcmtrcd
#define flctearn flceeaid._flctearn
#define flcperrn _psa_union11._flcperrn
#define flceperaid _psa_union11._flceperaid
#define flcarch flceamdid._flcarch
#define psampl _psa_union12._psampl
#define flcempl _psa_union12._flcempl
#define flcetea6 _psa_union13._psa_struct6.flceteid.flcetea._flcetea6
#define flcetea7 _psa_union13._psa_struct6.flceteid.flcetea._flcetea7
#define flceteasn _psa_union13._psa_struct7.flceteasninfo._flceteasn
#define flcepcnum _psa_union13._psa_struct8.flcetepcinfo._flcepcnum
#define flcemonitorcode _psa_union13._psa_struct8._flcemonitorcode
#define flcsid _psa_union14.flciocdp._flcsid
#define flciofp _psa_union14.flciocdp._flciofp
#define flcessid _psa_union14._psa_struct9._flcessid
#define flceiointparm _psa_union14._psa_struct9._flceiointparm
#define flcfacl0 _psa_union15.flcfacl._flcfacl0
#define flcfacl1 _psa_union15.flcfacl._flcfacl1
#define flcfacl2 _psa_union15.flcfacl._flcfacl2
#define flcfacl3 _psa_union15.flcfacl._flcfacl3
#define flcfacl4 _psa_union15.flcfacl._flcfacl4
#define flcfacl5 _psa_union15.flcfacl._flcfacl5
#define flcfacl6 _psa_union15.flcfacl._flcfacl6
#define flcfacl7 _psa_union15.flcfacl._flcfacl7
#define flcfacl8 _psa_union15.flcfacl._flcfacl8
#define flcfacl9 _psa_union15.flcfacl._flcfacl9
#define flcefacilitieslistbyte0 _psa_union15.flcefacilitieslist._flcefacilitieslistbyte0
#define flcefacilitieslistbyte1 _psa_union15.flcefacilitieslist._flcefacilitieslistbyte1
#define flcefacilitieslistbyte2 _psa_union15.flcefacilitieslist._flcefacilitieslistbyte2
#define flcefacilitieslistbyte3 _psa_union15.flcefacilitieslist._flcefacilitieslistbyte3
#define flcefacilitieslistbyte4 _psa_union15.flcefacilitieslist._flcefacilitieslistbyte4
#define flcefacilitieslistbyte5 _psa_union15.flcefacilitieslist._flcefacilitieslistbyte5
#define flcefacilitieslistbyte6 _psa_union15.flcefacilitieslist._flcefacilitieslistbyte6
#define flcefacilitieslistbyte7 _psa_union15.flcefacilitieslist._flcefacilitieslistbyte7
#define flcefacilitieslistbyte8 _psa_union15.flcefacilitieslist._flcefacilitieslistbyte8
#define flcefacilitieslistbyte9 _psa_union15.flcefacilitieslist._flcefacilitieslistbyte9
#define flcefacilitieslistbytea _psa_union15.flcefacilitieslist._flcefacilitieslistbytea
#define flcefacilitieslistbyteb _psa_union15.flcefacilitieslist._flcefacilitieslistbyteb
#define flcefacilitieslistbytec _psa_union15.flcefacilitieslist._flcefacilitieslistbytec
#define flcefacilitieslistbyted _psa_union15.flcefacilitieslist._flcefacilitieslistbyted
#define flcefacilitieslistbytee _psa_union15.flcefacilitieslist._flcefacilitieslistbytee
#define flcefacilitieslistbytef _psa_union15.flcefacilitieslist._flcefacilitieslistbytef
#define flcfacle _psa_union16._flcfacle
#define flcefacilitieslist1 _psa_union16._flcefacilitieslist1
#define flcmcic _psa_union17._flcmcic
#define flcemcic _psa_union17._flcemcic
#define flcfsa _psa_union18._psa_struct10._flcfsa
#define flcefsa _psa_union18._flcefsa
#define flcfla _psa_union19._flcfla
#define flceemfctrarrayaddr _psa_union19._psa_struct11._flceemfctrarrayaddr
#define flceemfctrarraysize _psa_union19._psa_struct11._flceemfctrarraysize
#define flceemfexceptioncnt _psa_union19._psa_struct11._flceemfexceptioncnt
#define flcrv110 _psa_union20._flcrv110
#define flcebea _psa_union20._psa_struct12._flcebea
#define flcer118 _psa_union20._psa_struct12._flcer118
#define flcarsav _psa_union21._flcarsav
#define flceropsw _psa_union21._psa_struct13._flceropsw
#define flceeopsw _psa_union21._psa_struct13._flceeopsw
#define flcesopsw _psa_union21._psa_struct13._flcesopsw
#define flcepopsw _psa_union21._psa_struct13._flcepopsw
#define flcfpsav _psa_union22._flcfpsav
#define flcemopsw _psa_union22._psa_struct14._flcemopsw
#define flceiopsw _psa_union22._psa_struct14._flceiopsw
#define flcgrsav _psa_union23._flcgrsav
#define flcer180 _psa_union23._psa_struct15._flcer180
#define flcernpsw _psa_union23._psa_struct15._flcernpsw
#define flceenpsw _psa_union23._psa_struct15._flceenpsw
#define flccrsav _psa_union24._flccrsav
#define flcesnpsw _psa_union24._psa_struct16._flcesnpsw
#define flcepnpsw _psa_union24._psa_struct16._flcepnpsw
#define flcemnpsw _psa_union24._psa_struct16._flcemnpsw
#define flceinpsw _psa_union24._psa_struct16._flceinpsw
#define psapsa flchdend._psapsa
#define psacpupa flchdend._psacpupa
#define psacpula flchdend._psacpula
#define psasup1 psasuper._psasup1
#define psasup2 psasuper._psasup2
#define psasup3 psasuper._psasup3
#define psasup4 psasuper._psasup4
#define psa___workunit___procclassatdisp _psa_union25._psa___workunit___procclassatdisp
#define psa___workunit___procclassatdisp___byte0 _psa_union25._psa_struct17._psa___workunit___procclassatdisp___byte0
#define psa___workunit___procclassatdisp___byte1 _psa_union25._psa_struct17._psa___workunit___procclassatdisp___byte1
#define psaprocclass _psa_union26._psaprocclass
#define psa___bylpar___procclass _psa_union26._psa___bylpar___procclass
#define psaprocclass___byte0 _psa_union26._psa_struct18._psaprocclass___byte0
#define psaprocclass___byte1 _psa_union26._psa_struct18._psaprocclass___byte1
#define psa___bylpar___procclass___byte0 _psa_union26._psa_struct19._psa___bylpar___procclass___byte0
#define psa___bylpar___procclass___byte1 _psa_union26._psa_struct19._psa___bylpar___procclass___byte1
#define psadispl psaclht.psaclht1._psadispl
#define psaasml psaclht.psaclht1._psaasml
#define psasalcl psaclht.psaclht1._psasalcl
#define psaiossl psaclht.psaclht1._psaiossl
#define psarsmdl psaclht.psaclht1._psarsmdl
#define psaiosul psaclht.psaclht1._psaiosul
#define psarsmql psaclht.psaclht1._psarsmql
#define psarv29c psaclht.psaclht1._psarv29c
#define psarv2a0 psaclht.psaclht1._psarv2a0
#define psatpacl psaclht.psaclht1._psatpacl
#define psaoptl psaclht.psaclht1._psaoptl
#define psarsmgl psaclht.psaclht1._psarsmgl
#define psavfixl psaclht.psaclht1._psavfixl
#define psaasmgl psaclht.psaclht1._psaasmgl
#define psarsmsl psaclht.psaclht1._psarsmsl
#define psarsmxl psaclht.psaclht1._psarsmxl
#define psarsmal psaclht.psaclht1._psarsmal
#define psavpagl psaclht.psaclht1._psavpagl
#define psarsmcl psaclht.psaclht1._psarsmcl
#define psarvlk2 psaclht.psaclht1._psarvlk2
#define psarsml psaclht.psaclht2._psarsml
#define psatrcel psaclht.psaclht2._psatrcel
#define psaiosl psaclht.psaclht2._psaiosl
#define psarvlk4 psaclht.psaclht2._psarvlk4
#define psacpul psaclht.psaclht3._psacpul
#define psarvlk5 psaclht.psaclht3._psarvlk5
#define psacmsl psaclht.psaclht4._psacmsl
#define psalocal psaclht.psaclht4._psalocal
#define psarvlk6 psaclht.psaclht4._psarvlk6
#define psaclhs1 psahlhi.psaclhs._psaclhs1
#define psaclhs2 psahlhi.psaclhs._psaclhs2
#define psaclhs3 psahlhi.psaclhs._psaclhs3
#define psaclhs4 psahlhi.psaclhs._psaclhs4
#define psaval___machine psaval._psaval___machine
#define psacstk psarsvt.psarsvte._psacstk
#define psanstk psarsvt.psarsvte._psanstk
#define psasstk psarsvt.psarsvte._psasstk
#define psassav psarsvt.psarsvte._psassav
#define psamstk psarsvt.psarsvte._psamstk
#define psamsav psarsvt.psarsvte._psamsav
#define psapstk psarsvt.psarsvte._psapstk
#define psapsav psarsvt.psarsvte._psapsav
#define psaestk1 psarsvt.psarsvte._psaestk1
#define psaesav1 psarsvt.psarsvte._psaesav1
#define psaestk2 psarsvt.psarsvte._psaestk2
#define psaesav2 psarsvt.psarsvte._psaesav2
#define psaestk3 psarsvt.psarsvte._psaestk3
#define psaesav3 psarsvt.psarsvte._psaesav3
#define psarstk psarsvt.psarsvte._psarstk
#define psarsav psarsvt.psarsvte._psarsav
#define psapswsv16 _psa_union27._psapswsv16
#define psapswsv _psa_union27._psa_struct20._psapswsv
#define psapcfb1 psapcfun._psapcfb1
#define psapcfb2 psapcfun._psapcfb2
#define psapcfb3 psapcfun._psapcfb3
#define psapcfb4 psapcfun._psapcfb4
#define psamflgs psamodew._psamflgs
#define psamodeh psamodew._psamodeh
#define psamode psamodew._psamode
#define psafzero psadzero._psafzero
#define psalheb0 psaclhse._psalheb0
#define psalheb1 psaclhse._psalheb1
#define psalheb2 psaclhse._psalheb2
#define psalheb3 psaclhse._psalheb3
#define psatkn psastke._psatkn
#define psaasd psastke._psaasd
#define psasel psastke._psasel
#define psa___cr0esavearea___hw psa___cr0esavearea._psa___cr0esavearea___hw
#define psa___cr0esavearea___lw psa___cr0esavearea._psa___cr0esavearea___lw
#define psa___windowworkarea _psa_union28._psa___windowworkarea
#define psa___windowtoddelta _psa_union28._psa_struct21._psa___windowtoddelta
#define psa___windowtoddelta___hw _psa_union28._psa_struct22._psa___windowtoddelta___hw
#define psa___windowtoddelta___lw _psa_union28._psa_struct22._psa___windowtoddelta___lw
#define psadtsav _psa_union29._psadtsav
#define psaff6c0 _psa_union29._psaff6c0
#define psadsins psadexms.psadcr3._psadsins
#define psadpkm psadexms.psadcr3.psadpksa._psadpkm
#define psadsas psadexms.psadcr3.psadpksa._psadsas
#define psadpins psadexms.psadcr4._psadpins
#define psadax psadexms.psadcr4.psadaxpa._psadax
#define psadpas psadexms.psadcr4.psadaxpa._psadpas
#define psaus2st _psa_union30._psa_struct23._psaus2st
#define psacdsae _psa_union30.psacdsav._psacdsae
#define psacdsaf _psa_union30.psacdsav._psacdsaf
#define psacdsa0 _psa_union30.psacdsav._psacdsa0
#define psacdsa1 _psa_union30.psacdsav._psacdsa1
#define psatrgr0 psatrsav._psatrgr0
#define psatrgr1 psatrsav._psatrgr1
#define psatrgr2 psatrsav._psatrgr2
#define psatrgr3 psatrsav._psatrgr3
#define psatrgr4 psatrsav._psatrgr4
#define psatrgr5 psatrsav._psatrgr5
#define psatrgr6 psatrsav._psatrgr6
#define psatrgr7 psatrsav._psatrgr7
#define psatrgr8 psatrsav._psatrgr8
#define psatrgr9 psatrsav._psatrgr9
#define psatrgra psatrsav._psatrgra
#define psatrgrb psatrsav._psatrgrb
#define psatrgrc psatrsav._psatrgrc
#define psatrgrd psatrsav._psatrgrd
#define psatrgre psatrsav._psatrgre
#define psatrgrf psatrsav._psatrgrf
#define psagsav _psa_union31._psagsav
#define psaff8a8 _psa_union31._psaff8a8
#define psapcgra psapcgab._psapcgra
#define psapcgrb psapcgab._psapcgrb
#define psalkr0 psalksa._psalkr0
#define psalkr1 psalksa._psalkr1
#define psalkr2 psalksa._psalkr2
#define psalkr3 psalksa._psalkr3
#define psalkr4 psalksa._psalkr4
#define psalkr5 psalksa._psalkr5
#define psalkr6 psalksa._psalkr6
#define psalkr7 psalksa._psalkr7
#define psalkr8 psalksa._psalkr8
#define psalkr9 psalksa._psalkr9
#define psalkr10 psalksa._psalkr10
#define psalkr11 psalksa._psalkr11
#define psalkr12 psalksa._psalkr12
#define psalkr13 psalksa._psalkr13
#define psalkr14 psalksa._psalkr14
#define psalkr15 psalksa._psalkr15
#define psaslsa _psa_union32._psaslsa
#define psaff950 _psa_union32._psaff950
#define psajstsa _psa_union33._psajstsa
#define psaff998 _psa_union33._psaff998
#define psaus2nd _psa_union34._psa_struct24._psaus2nd
#define psaslkr0 _psa_union34.psaslksa._psaslkr0
#define psaslkr1 _psa_union34.psaslksa._psaslkr1
#define psaslkr2 _psa_union34.psaslksa._psaslkr2
#define psaslkr3 _psa_union34.psaslksa._psaslkr3
#define psaslkr4 _psa_union34.psaslksa._psaslkr4
#define psaslkr5 _psa_union34.psaslksa._psaslkr5
#define psaslkr6 _psa_union34.psaslksa._psaslkr6
#define psaslkr7 _psa_union34.psaslksa._psaslkr7
#define psaslkr8 _psa_union34.psaslksa._psaslkr8
#define psaslkr9 _psa_union34.psaslksa._psaslkr9
#define psaslkra _psa_union34.psaslksa._psaslkra
#define psaslkrb _psa_union34.psaslksa._psaslkrb
#define psaslkrc _psa_union34.psaslksa._psaslkrc
#define psaslkrd _psa_union34.psaslksa._psaslkrd
#define psaslkre _psa_union34.psaslksa._psaslkre
#define psaslkrf _psa_union34.psaslksa._psaslkrf
#define psaffrr psafafrr._psaffrr
#define psaffrrs psafafrr._psaffrrs
#define psarvb80 _psa_union35._psarvb80
#define psastak _psa_union35._psastak

/* Values for field "flcsvilc" */
#define flcsilcb 0x07 /* -        SIGNIFICANT BITS IN ILC FIELD - LAST      */

/* Values for field "flcesilc" */
#define flcesilcb 0x07 /* FLCE 89x: Significant bits in ILC. Last bit        */

/* Values for field "flcpiilc" */
#define flcpilcb 0x07 /* -        SIGNIFICANT BITS IN ILC FIELD - LAST      */

/* Values for field "psapicod" */
#define psapiper 0x80 /* -        PER INTERRUPT OCCURRED             MDC089 */
#define psapimc 0x40  /* -        MONITOR CALL INTERRUPT OCCURRED    MDC090 */
#define psapipc 0x3F  /* -        AN UNSOLICITED PROGRAM CHECK HAS          */

/* Values for field "_filler15" */
#define flcteaxm 0x80 /* -      IF 0 FLCTEA IS RELATIVE TO THE PRIMARY      */

/* Values for field "flcteab3" */
#define flcsopi 0x04        /* -      Suppression on protection flag       @LQA   */
#define flctstdp 0x00       /* -      IF 1, THE PRIMARY STD WAS USED.      @L8A   */
#define flctstda 0x01       /* -      IF 1, THE STD WAS AR QUALIFIED.      @L8A   */
#define flctstds 0x02       /* -      IF 1, THE SECONDARY STD WAS USED.    @L8A   */
#define flctstdh 0x03       /* -      IF 1, THE HOME STD WAS USED.         @L8A   */
#define flcteacl 0x7FFFF000 /* Mask to leave only TEA address       @LSA          */

/* Values for field "flcepilc" */
#define flcepilcb 0x07 /* FLCE 8Dx: Significant bits in ILC. Last bit        */

/* Values for field "flcepicode1" */
#define flcepiper 0x80 /* FLCE 8Fx: PER interruption code                    */
#define flcepimc 0x40  /* FLCE 8Fx: Monitor Call interruption code           */
#define flcepipc 0x3F  /* FLCE 8Fx: An unsolicited program interruption      */

/* Values for field "flcepercode0" */
#define flcepersb 0x80  /* FLCE 96x: PER successful branch event              */
#define flceperif 0x40  /* FLCE 96x: PER instruction fetch event              */
#define flcepersa 0x20  /* FLCE 96x: PER storage alteration event             */
#define flcepersk 0x10  /* FLCE 96x: PER storage key alteration event         */
#define flcepersar 0x08 /* FLCE 96x: PER storage alteration using real        */
#define flceperzad 0x04 /* FLCE 96x: PER zero address detection               */
#define flcepertransactionend 0x02

/* Values for field "flcatmid" */
#define flcpswb4 0x80 /* PSW.4 part of ATMID                  @LSA          */

/* Values for field "flceperatmid" */
#define flceperpsw4 0x80       /* FLCE 97x: PER PSW bit 4                            */
#define flceperatmidvalid 0x40 /* FLCE 97x: When 1, the ATMID bits are valid         */
#define flceperpsw32 0x20      /* FLCE 97x: PER PSW bit 32                           */
#define flceperpsw5 0x10       /* FLCE 97x: PER PSW bit 5                            */
#define flceperpsw16 0x08      /* FLCE 97x: PER PSW bit 16                           */
#define flceperpsw17 0x04      /* FLCE 97x: PER PSW bit 17                           */
#define flceperasceid 0x03     /* FLCE 97x: PER ASCE identification. If a            */

/* Values for field "flceeaid" */
#define flceeaid0 0x80        /* Bit 0 of EAID. Zero                                */
#define flceeaid1 0x40        /* Bit 1 of EAID. Zero                                */
#define flceeaid2 0x20        /* Bit 2 of EAID. Set only when PIC 2C for PTI        */
#define flceeaid3 0x10        /* Bit 3 of EAID. Set only when PIC 2C for SSAIR      */
#define flceeaid___arnum 0x0F /* AR number. Zero when Bit 1 or Bit 2 is set         */

/* Values for field "flcarch" */
#define psazarch 0x01 /* -      z/Architecture                       @LSA   */
#define psaesame 0x01 /* -      z/Architecture                       @LSA   */

/* Values for field "flceamdid" */
#define flceloeme 0x01 /* Logout is Z/Architecture                           */

/* Values for field "flcetea6" */
#define flceaefsi 0x0C /* Access-exception Fetch/Store indicator: 00 --      */

/* Values for field "flcetea7" */
#define flcepealc 0x08  /* FLCE AFx: Protection exception due to              */
#define flcesopi 0x04   /* FLCE AFx: Suppress on protection indication        */
#define flceteastd 0x03 /* FLCE AFx: Segment table designation for TEA:       */

/* Values for field "flcfacl0" */
#define flcfn3 0x80   /* -     N3 installed                         @LVA    */
#define flcfzari 0x40 /* -     z/Architecture installed             @LVA    */
#define flcfzara 0x20 /* -     z/Architecture active                @LVA    */
#define flcfaslx 0x02 /* -     ASN & LX reuse facility installed    @LVA    */

/* Values for field "flcfacl1" */
#define flcfedat 0x80 /* DAT features                         @0BA          */
#define flcfsrs 0x40  /* Sense-running-status                 @LZA          */
#define flcfsske 0x20 /* Cond. SSKE instruction installed     @0AA          */
#define flcfctop 0x10 /* STSI-enhancement                     @LYA          */

/* Values for field "flcfacl2" */
#define flcfetf2 0x80 /* Extended Translation facility 2      @LVA          */
#define flcfcrya 0x40 /* Cryptographic assist                 @LVA          */
#define flcfld 0x20   /* Long Displacement facility           @LVA          */
#define flcfldhp 0x10 /* Long Displacement High Performance   @LVA          */
#define flcfhmas 0x08 /* HFP Multiply Add/Subtract            @LVA          */
#define flcfeimm 0x04 /* Extended immediate when z/Arch       @LVA          */
#define flcfetf3 0x02 /* Extended Translation Facility 3 when @LVA          */
#define flcfhun 0x01  /* HFP unnormalized extension           @LVA          */

/* Values for field "flcfacl3" */
#define flcfet2e 0x80 /* ETF2-enhancement                   031215          */
#define flcfstkf 0x40 /* STCKF-enhancement                    @PIA          */
#define flcfet3e 0x02 /* ETF3-enhancement                   040512          */
#define flcfect 0x01  /* ECT-facility                         @LXA          */

/* Values for field "flcfacl4" */
#define flcfcssf 0x80 /* Compare-and-swap-and-store           @LXA          */
#define flcfcsf2 0x40 /* Compare-and-swap-and-store 2         @LXA          */
#define flcfgief 0x20 /* General-Instructions-Extension       @M0A          */
#define flcfocm 0x01  /* Obsolete CPU-measurement facility. Use             */

/* Values for field "flcfacl5" */
#define flcffpse 0x40 /* Floating-point-support enhancement   @PMA          */
#define flcfdfp 0x20  /* Decimal-floating-point               @PMA          */
#define flcfdfph 0x10 /* Decimal-floating-point high performance            */
#define flcfpfpo 0x08 /* PFPO instruction                   070424          */

/* Values for field "flcfacl8" */
#define flcfcaai 0x40 /* Crypto AP-Queue adapter interruption @M5A          */
#define flcfcmc 0x10  /* CPU-measurement counter facility     @M4A          */
#define flcfcms 0x08  /* CPU-measurement sampling facility    @M4A          */
#define flcfsclp 0x04 /* Possible future enhancement          @M7A          */
#define flcfaisi 0x02 /* AISI facility                        @PPA          */
#define flcfaen 0x01  /* AEN  facility                        @PPA          */

/* Values for field "flcfacl9" */
#define flcfais 0x80 /* AIS  facility                        @PPA          */

/* Values for field "flcefacilitieslistbyte0" */
#define flcezarchn3 0x80                     /* Instructions marked "N3" in the instruction        */
#define flceesamen3 0x80                     /* Instructions marked "N3" in the instruction        */
#define flcezarchinstalled 0x40              /* The z/Architecture mode is installed on            */
#define flceesameinstalled 0x40              /* The z/Architecture mode is installed on            */
#define flcezarch 0x20                       /* The z/Architecture mode is active on the CPU       */
#define flceesame 0x20                       /* The z/Architecture mode is active on the CPU       */
#define flceidteinstalled 0x10               /* IDTE is installed                                  */
#define flceidteclearingcombinedsegment 0x08 /* IDTE does clearing of                              */
#define flceidteclearingcombinedregion 0x04  /* IDTE does clearing of                              */
#define flceasnandlxreuseinstalled 0x02      /* The ASN and LX reuse facility is                   */
#define flcestfle 0x01                       /* STFLE instruction is available                     */

/* Values for field "flcefacilitieslistbyte1" */
#define flceedatfeat 0x80              /* DAT features                                       */
#define flcesenserunningstatus 0x40    /* sense-running-status facility                      */
#define flcecondsskeinstalled 0x20     /* The conditional SSKE instruction is                */
#define flceconfigurationtopology 0x10 /* STSI-enhancement for configuration                 */
#define flcecqcif 0x08                 /* 110524                                             */
#define flceipterange 0x04             /* IPTE-range facility is installed                   */
#define flcenonqkeysetting 0x02        /* Nonquiescing key-setting facility is               */
#define flceapft 0x01                  /* The APFT facility is installed / 091111            */

/* Values for field "flcefacilitieslistbyte2" */
#define flceetf2 0x80                  /* Extended translation facility 2 is present         */
#define flcecryptoassist 0x40          /* The cryptographic assist is present                */
#define flcemessagesecurityassist 0x40 /* The message security assist is                     */
#define flcelongdisplacement 0x20      /* The long displacement facility is                  */
#define flcelongdisplacementhp 0x10    /* The long displacement facility has                 */
#define flcehfpmas 0x08                /* The HFP Multiply add/subtract facility is          */
#define flceextendedimmediate 0x04     /* The extended immediate facility is                 */
#define flceetf3 0x02                  /* The extended translaction facility 3 is            */
#define flcehfpunnormextension 0x01    /* The HFP unnormalized extension                     */

/* Values for field "flcefacilitieslistbyte3" */
#define flceetf2e 0x80 /* ETF2 enhancement is present 031215                 */
#define flcestckf 0x40 /* STCKF enhancement is present                       */
#define flceparse 0x20 /* Parsing enhancement facility is present            */
#define flcetcsf 0x08  /* TOD clock steering facility                        */
#define flceetf3e 0x02 /* ETF3 enhancement is present 040512                 */
#define flceectf 0x01  /* Extract Cpu Time facility                          */

/* Values for field "flcefacilitieslistbyte4" */
#define flcecssf 0x80                   /* Compare-and-swap-and-store facility                */
#define flcecssf2 0x40                  /* Compare-and-swap-and-store facility 2              */
#define flcegeneralinstextension 0x20   /* General-Instructions- Extension                    */
#define flceenhancedmonitor 0x08        /* The Enhanced Monitor facility is                   */
#define flceobsoletecpumeasurement 0x01 /* Obsolete. Meant CPU-measurement                    */

/* Values for field "flcefacilitieslistbyte5" */
#define flcesetprogramparm 0x80   /* Set-Program-Parameter facility is                  */
#define flcefpsef 0x40            /* Floating-point-support enhancement facility        */
#define flcedfpf 0x20             /* Decimal-floating-point facility                    */
#define flcedfpfhp 0x10           /* Decimal-floating-point facility high               */
#define flcepfpo 0x08             /* PFPO instruction 070424                            */
#define flcedistinctoperands 0x04 /* z196 is the first machine with this                */
#define flcehighword 0x04
#define flceloadstoreoncondition 0x04
#define flcepopulationcount 0x04
#define flcecmpef 0x01 /* Possible future enhancement                        */

/* Values for field "flcefacilitieslistbyte6" */
#define flcemiscinstext 0x40      /* Bit 49 - Miscellaneous instruction                 */
#define flceexecutionhint 0x40    /* Bit 49 - Execution hint facility.                  */
#define flceloadandtrap 0x40      /* Bit 49 - Load and trap facility.                   */
#define flceconstrainedtx 0x20    /* Bit 50 - Constrained Transactional                 */
#define flceloadstoreoncond2 0x04 /* Bit 53 -                                           */

/* Values for field "flcefacilitieslistbyte7" */
#define flcemie2 0x20 /* Bit 58                                             */

/* Values for field "flcefacilitieslistbyte8" */
#define flceri 0x80                     /* FlceRI                                             */
#define flcecryptoapqai 0x40            /* Crypto AP-Queue adapter interruption               */
#define flcecpumeasurementcounter 0x10  /* CPU-measurement counter facility                   */
#define flcecpumeasurementsampling 0x08 /* CPU-measurement sampling facility                  */
#define flcesclp 0x04                   /* Possible future enhancement                        */
#define flceaisi 0x02                   /* AISI facility, bit 70                              */
#define flceaen 0x01                    /* AEN facility, bit 71                               */

/* Values for field "flcefacilitieslistbyte9" */
#define flceais 0x80                    /* AIS facility, bit 72                               */
#define flcetransactionalexecution 0x40 /* Bit 73 - Transactional Execution                   */
#define flcemsa4 0x04                   /* MSA4 facility, bit 77                              */
#define flceedat2 0x02                  /* Bit 78 - Enhanced Dat-2                            */

/* Values for field "flceinpsw" */
#define flcesame___len 0x200

/* Values for field "psasup1" */
#define psaio 0x80    /* -        I/O FLIH                                  */
#define psasvc 0x40   /* -        SVC FLIH                                  */
#define psaext 0x20   /* -        EXTERNAL FLIH                             */
#define psapi 0x10    /* -        PROGRAM CHECK FLIH                        */
#define psalock 0x08  /* -        LOCK ROUTINE                              */
#define psadisp 0x04  /* -        DISPATCHER                                */
#define psatctl 0x02  /* -        TCTL RECOVERY FLAG  (MDC310)     @Z40FP9A */
#define psatype6 0x01 /* -        TYPE 6 SVC IN CONTROL  (MDC311)  @Z40FP9A */

/* Values for field "psasup2" */
#define psaipcri 0x80 /* -        REMOTE IMMEDIATE SIGNAL SERVICE ROUTINE   */
#define psasvcr 0x40  /* -        SUPER FRR USES FOR SVC FLIH      @ZMC3227 */
#define psasvcrr 0x20 /* -        SVC RECOVERY RECURSION INDICATOR.         */
#define psaacr 0x04   /* -        AUTOMATIC CPU RECONFIGURATION (ACR) IN    */
#define psartm 0x02   /* -        RECOVERY TERMINATION MONITOR (RTM) IN     */
#define psalcr 0x01   /* -        USED BY RTM TO SERIALIZE CALLS OF    @L5C */

/* Values for field "psasup3" */
#define psaiosup 0x80 /* -        IF ON, A MAINLINE IOS COMPONENT SUCH AS   */
#define psaspr 0x10   /* -        SUPER FRR IS ACTIVE  (MDC305)    @ZA02995 */
#define psaesta 0x08  /* -        SVC 60 RECOVERY ROUTINE ACTIVE            */
#define psarsm 0x04   /* -        REAL STORAGE MANAGER (RSM) ENTERED FOR    */
#define psaulcms 0x02 /* -        LOCK MANAGER UNCONDITIONAL LOCAL OR       */
#define psaslip 0x01  /* -        IEAVTSLP RECURSION CONTROL BIT            */

/* Values for field "psasup4" */
#define psaldwt 0x80 /* -        BLWLDWT IS IN CONTROL TO LOAD A      @LHC */
#define psasmf 0x40  /* -        SMF SUSPEND/RESET     (MDC599)   @G743PBB */
#define psaesar 0x20 /* -        SUPERVISOR ANALYSIS ROUTER IS ACTIVE @L5C */
#define psamch 0x10  /* -        Machine Check Handler is active.     @PKA */

/* Values for field "psaprocclass___byte1" */
#define psaprocclass___cp 0x00     /* Standard CP. 0 is offset to SWUQ     @H4A          */
#define psaprocclass___zcbp 0x02   /* zCBP.                                @MTA          */
#define psaprocclass___zaap 0x02   /* zAAP.                                @H4A          */
#define psaprocclass___ziip 0x04   /* zIIP.                                @H5A          */
#define psaprocclass___sup 0x04    /* zIIP.                                @H4A          */
#define psaprocclassindex___cp 0   /* CP ProcClass index                   @0JA          */
#define psaprocclassindex___zcbp 1 /* zCBP ProcClass index                 @MTA          */
#define psaprocclassindex___zaap 1 /* zAAP ProcClass index                 @0JA          */
#define psaprocclassindex___ziip 2 /* zIIP ProcClass index                 @0JA          */
#define psaprocclassindex___max 2  /* Max ProcClass index                  @0JA          */
#define psaprocclassconverter 2    /* Procclass conversion factor          @0EA          */
#define psamaxprocclass 4          /* PSA Max procclass                    @0EA          */
#define psamaxprocclassindex 0x02

/* Values for field "psaptype" */
#define psaifa 0x40              /* Indicates Special Processor          @H3C          */
#define psa___bylpar___zcbp 0x40 /* @MTA                                               */
#define psa___bylpar___zaap 0x40 /* @H5A                                               */
#define psa___bylpar___ifa 0x40  /* @H5A                                               */
#define psazcbpds 0x20           /* zCBP that is different speed than CP @MTA          */
#define psaifads 0x20            /* zAAP (IFA) that is different                       */
#define psadscrp 0x10            /* Discretionary Processor              @LYA          */
#define psaziip 0x08             /* zIIP                                 @H4A          */
#define psa___bylpar___ziip 0x08 /* @H5A                                               */
#define psasup 0x08              /* zIIP                                 @H4A          */
#define psa___bylpar___sup 0x08  /* @H5A                                               */
#define psaziipds 0x04           /* zIIP that is different speed than CP @H4A          */
#define psasupds 0x04            /* zIIP that is different speed than CP @H4A          */

/* Values for field "psails" */
#define psailsio 0x80 /* -      THE I/O FLIH IS USING THE            @L9A   */
#define psailsex 0x40 /* -      THE EXTERNAL FLIH IS USING THE       @L9A   */
#define psailspc 0x20 /* -      THE PROGRAM FLIH IS USING THE        @L9A   */
#define psailsds 0x10 /* -      THE DISPATCHER IS USING THE          @L9A   */
#define psailsrs 0x08 /* -      THE RESTART FLIH IS USING THE        @L9A   */
#define psailsor 0x04 /* -      EXIT IS USING THE INTERRUPT HANDLER  @LAA   */
#define psailst6 0x02 /* -      TYPE 6 SVC IS USING THE INTERRUPT    @D2A   */
#define psailslk 0x01 /* -      THE INTERRUPT HANDLER LINKAGE STACK  @D4A   */

/* Values for field "psaflags" */
#define psaaeit 0x80 /* -      ADDRESSING ENVIRONMENT IS IN         @LOA   */
#define psatx 0x08   /* Equivalent to CVTTX                  @MBA          */
#define psatxc 0x04  /* Equivalent to CVTTXC                 @MBA          */

/* Values for field "psascaff" */
#define psaemema 0x80 /* $$SCAFFOLD: z/Architecture                         */

/* Values for field "psampsw" */
#define psapiom 0x02 /* INPUT/OUTPUT INTERRUPT MASK      @G860PXK          */
#define psapexm 0x01 /* EXTERNAL INTERRUPT MASK          @G860PXK          */

/* Values for field "psarsmcl" */
#define psalks1 0x13 /* COUNT OF LOCKS IN CLHT1 (19)         @MJC          */

/* Values for field "psarsml" */
#define psarsmex 0x80 /* -        BIT 0 OF PSARSML. IF ON, THE RSM          */

/* Values for field "psatrcel" */
#define psatrcex 0x80 /* -        BIT 0 OF PSATRCEL. IF ON THE TRACE        */

/* Values for field "psaiosl" */
#define psaiosex 0x80 /* -        BIT 0 OF PSAIOSL. IF ON THE IOS      @D3A */
#define psalks2 3     /* COUNT OF LOCKS IN CLHT2              @D3C          */

/* Values for field "psacpul" */
#define psalks3 1 /* COUNT OF LOCKS IN CLHT3          @G860PXH          */

/* Values for field "psalocal" */
#define psalks4 2 /* COUNT OF LOCKS IN CLHT4          @G860PXH          */

/* Values for field "psaclhs1" */
#define psacpuli 0x80 /* -        CPU LOCK INDICATOR               @G860PXH */
#define psasum 0x10   /* -        SUMMARY BIT. IF ON, AT LEAST ONE     @LDA */
#define psarsmli 0x08 /* -        RSM LOCK INDICATOR               @G860PXH */
#define psatrcei 0x04 /* -        TRACE LOCK INDICATOR             @G860PXH */
#define psaiosi 0x02  /* -        IOS LOCK INDICATOR                   @D3A */

/* Values for field "psaclhs2" */
#define psarsmci 0x10 /* -        RSM COMMON LOCK INDICATOR        @G860PXK */
#define psarsmgi 0x08 /* -        RSM GLOBAL LOCK INDICATOR        @G860PXH */
#define psavfixi 0x04 /* -        VSM FIX LOCK INDICATOR           @G860PXH */
#define psaasmgi 0x02 /* -        ASM GLOBAL LOCK INDICATOR        @G860PXH */
#define psarsmsi 0x01 /* -        RSM STEAL LOCK INDICATOR         @G860PXH */

/* Values for field "psaclhs3" */
#define psarsmxi 0x80 /* -        RSM CROSS MEMORY LOCK INDICATOR  @G860PXH */
#define psarsmai 0x40 /* -        RSM ADDRESS SPACE LOCK INDICATOR @G860PXH */
#define psavpagi 0x20 /* -        VSM PAGE LOCK INDICATOR          @G860PXH */
#define psadspli 0x10 /* -        DISPATCHER LOCK INDICATOR                 */
#define psaasmli 0x08 /* -        ASM LOCK INDICATOR  (MDC388)     @G50EP9A */
#define psasalli 0x04 /* -        SPACE ALLOCATION LOCK INDICATOR           */
#define psaiosli 0x02 /* -        IOS SYNCHRONIZATION LOCK INDICATOR        */
#define psarsmdi 0x01 /* -        RSM DATA SPACE LOCK INDICATOR        @LBA */

/* Values for field "psaclhs4" */
#define psaiouli 0x80 /* -        IOS UCB LOCK INDICATOR  (MDC393) @G50EP9A */
#define psarsmqi 0x40 /* -        RSMQ lock indicator                  @MIA */
#define psatpali 0x08 /* -        TPACBDEB LOCK INDICATOR (MDC397) @G50EP9A */
#define psasrmli 0x04 /* -        SYSTEM RESOURCE MANAGER (SRM) LOCK        */
#define psacmsli 0x02 /* -        CROSS MEMORY SERVICES LOCK INDICATOR      */
#define psalclli 0x01 /* -        LOCAL LOCK INDICATOR  (MDC400)   @G50EP9A */

/* Values for field "psafpfl" */
#define psabfp 0x10 /* Additional FP status is being saved  @MEC          */
#define psavss 0x08 /* VRs are being saved                  @MEC          */
#define psagsf 0x04 /* GSF controls are being saved         @MQA          */

/* Values for field "psainte" */
#define psanuin 0x80 /* -        CPU TIMER CANNOT BE USED                  */

/* Values for field "psapcfb1" */
#define psapcmc 0x01  /* -        MC INTERRUPT      (MDC605)       @G383P9A */
#define psapcpf 0x02  /* -        PAGE FAULT                       @G383P9A */
#define psapcps 0x03  /* -        PER/SPACE SWITCH INTERRUPT       @G383PXU */
#define psapcad 0x04  /* -        ADDRESSING EXCEPTION  (MDC488)   @G383P9A */
#define psapctr 0x05  /* -        TRANSLATION EXCEPTION  (MDC489)  @G383P9A */
#define psapcpc 0x06  /* -        PROGRAM CHECK  (MDC490)          @G383P9A */
#define psapctrc 0x07 /* -        TRACE INTERRUPT                  @G860PXK */
#define psapcaf 0x08  /* -        NEW VALUE FOR PROGRAM INTERRUPT      @03A */
#define psapcls 0x09  /* -        LINKAGE STACK INTERRUPT FUNCTION     @L8A */
#define psapcart 0x0A /* -        ACCESS REGISTER TRANSLATION          @L8A */
#define psapcdpf 0x0B /* -        DISABLED PAGE/SEGMENT FAULT          @LCA */
#define psapcdar 0x0C /* -        DISABLED ART PIC X'2B' FUNCTION      @LCA */
#define psapcprt 0x0D /* -        Protection exception function value  @LQA */
#define psapcmax 0x0D /* -        MAXIMUM VALID FUNCTION VALUE         @LQC */

/* Values for field "psapcfb2" */
#define psapctrr 0x80 /* -        TRACE INTERRUPT RECURSION        @YA01102 */
#define psapcmt 0x40  /* -        TRACE RECURSION FLAG  (MDC493)   @G383P9A */

/* Values for field "psapcfb3" */
#define psapcp1 0x80  /* -        FIRST LEVEL PROGRAM CHECK        @G383P9A */
#define psapcp2 0x40  /* -        SECOND LEVEL PROGRAM CHECK       @G383P9A */
#define psapcde 0x20  /* -        DAT ERROR CONDITION  (MDC497)    @G383P9A */
#define psapclv 0x10  /* -        0=REGISTERS IN LCCA, 1=REGISTERS @G383P9A */
#define psapcp3 0x08  /* -        THIRD LEVEL PROGRAM CHECK        @G383P9A */
#define psapcp4 0x04  /* -        FOURTH LEVEL PROGRAM CHECK       @G383P9A */
#define psapcpfr 0x02 /* -        RECURSIVE PAGE FAULT INDICATOR       @LAA */
#define psapcavr 0x01 /* -        RECURSIVE ASTE VALIDITY INDICATOR    @LCA */

/* Values for field "psapcfb4" */
#define psapcdnv 0x80 /* -        DUCT validity indicator              @PBA */
#define psapclsr 0x40 /* -        IEAVLSIH has invoked IARPTEPR and    @PEA */

/* Values for field "psamflgs" */
#define psanss 0x80   /* -        ENABLED UNLOCKED TASK WITH FRR   @G383P9A */
#define psaprsrb 0x40 /* -        Preemptable-class SRB                @LPA */

/* Values for field "psamode" */
#define psataskm 0x00 /* -        TASK MODE VALUE  (MDC338)        @G50DP9A */
#define psasrbm 0x04  /* -        SRB MODE VALUE  (MDC339)         @G50DP9A */
#define psawaitm 0x08 /* -        WAIT MODE VALUE  (MDC340)        @G50DP9A */
#define psadispm 0x10 /* -        DISPATCHER MODE VALUE  (MDC342)  @G50DP9A */
#define psapsrbm 0x20 /* -        PSEUDO SRB MODE FLAG BIT.  THIS BIT MAY   */

/* Values for field "psalheb0" */
#define psablsdi 0x80 /* -        BMFLSD LOCK INDICATOR.               @LGA */
#define psaxdsi 0x40  /* -        XCFDS LOCK INDICATOR.                @LEA */
#define psaxresi 0x20 /* -        XCFRES LOCK INDICATOR.               @LEA */
#define psaxqi 0x10   /* -        XCFQ LOCK INDICATOR.                 @LEA */
#define psaeseti 0x08 /* -        ETRSET LOCK INDICATOR.               @LFA */
#define psaixsci 0x04 /* -        IXLSCH LOCK INDICATOR.               @LMC */
#define psaixshi 0x02 /* -        IXLSHR LOCK INDICATOR.               @LMC */
#define psaixdsi 0x01 /* -        IXLDS LOCK INDICATOR.                @LLA */

/* Values for field "psalheb1" */
#define psaixlli 0x80 /* -        IXLSHELL LOCK INDICATOR.             @LMC */
#define psauluti 0x40 /* -        IOSULUT LOCK INDICATOR.              @LJA */
#define psaixlri 0x20 /* -        IXLREQST LOCK INDICATOR.             @05A */
#define psawlmri 0x10 /* -        WLMRES LOCK INDICATOR                @LRA */
#define psawlmqi 0x08 /* -        WLMQ LOCK INDICATOR.                 @LRA */
#define psacntxi 0x04 /* -        CONTEXT LOCK INDICATOR               @LRA */
#define psaregsi 0x02 /* -        REGSRV LOCK INDICATOR.               @LRA */
#define psassdli 0x01 /* -        SSD LOCK INDICATOR.                  @LTA */

/* Values for field "psalheb2" */
#define psagrsli 0x80 /* -        GRSINT lock indicator                @M1A */
#define psamisli 0x40 /* -        MISC lock indicator                  @MGA */
#define psapslk1 0x40 /* -        n/a                                  @MGC */
#define psadnu2 0x20  /* -        n/a                                  @MGA */
#define psapnlk1 0x20 /* -        n/a                                  @MGC */
#define psadnu3 0x10  /* -        n/a                                  @MGA */
#define psaiolk1 0x10 /* -        n/a                                  @MGC */
#define psadnu4 0x08  /* -        n/a                                  @MGA */
#define psapxlk1 0x08 /* -        n/a                                  @MGC */
#define psadnu5 0x04  /* -        n/a                                  @MGA */
#define psadrlk3 0x04 /* -        n/a                                  @MGC */
#define psadrlk2 0x02 /* -        HCWDRLK2 lock indicator              @M6A */
#define psadrlk1 0x01 /* -        HCWDRLK1 lock indicator              @M6A */

/* Values for field "psalheb3" */
#define psasrmei 0x80 /* -        SRMENQ lock indicator                @M9A */
#define psassdgi 0x40 /* -        SSDGROUP lock indicator              @MLA */

/* Values for field "psacr0cb" */
#define psaenabl 0x10 /* -        TO ENABLE PSA PROTECTION                  */
#define psadsabl 0x00 /* -        TO DISABLE PSA PROTECTION                 */

/* Values for field "psacr0sv" */
#define psacr0en 0x10 /* -        IF 0, PSA PROTECT DISABLED.  IF 1, PSA    */
#define psacr0ed 0x80 /* DAT features. Bit is in PSACR0SV+1   @0BA          */
#define psacr0al 0x08 /* -        IF 1, ASN & LX Reuse facility is          */
#define psacr0fp 0x04 /* -        IF 1, extended floating point is          */
#define psacr0vi 0x02 /* -        IF 1, vector instructions are             */

/* Values for field "psarcr0" */
#define psarpen 0x10 /* -        IF 0, PSA PROTECT DISABLED.  IF 1, PSA    */

/* Values for field "psascfb" */
#define psaiopr 0x80               /* -        INDICATES IF INTERRUPTED TASK SHOULD @L1A */
#define psaiorty 0x40              /* -        I/O FLIH RECOVERY FLAG. IF 1,        @L5A */
#define psa___lockspinentered 0x20 /* -   Set whenever supervisor spins for a            */

/* Values for field "psatrace" */
#define psatroff 0x80 /* -        IF ON, SYSTEM TRACE SUSPENDED ON THIS     */

/* Values for field "psasflgs" */
#define psaschda 0x80      /* Schedule is active                   @LPA          */
#define psamcha 0x40       /* Machine Check is active              @06A          */
#define psarsta 0x20       /* Restart is active                    @06A          */
#define psaegra 0x10       /* Global Recovery is active            @06A          */
#define psartma 0x08       /* Selected RTM functions are active    @06A          */
#define psadontgetweb 0x04 /* A WEB or WEBQLOCK is held. IEAVESC0                */

/* Values for field "psamiscf" */
#define psaalr 0x80 /* Equivalent to CVTALR                 @LVA          */

#endif

#ifdef __open_xl__
#pragma pack()
#else
#pragma pack(reset)
#endif
