#pragma pack(1)

#ifndef __deb__
#define __deb__

struct deb {
  struct {
    struct {
      unsigned char  _debeoeab;      /* FLAG BYTE                               ICB280 */
      unsigned int   _debeoead : 24; /* ADDRESS OF END-OF-EXTENT APPENDAGE ROUTINE     */
      } debeoea;
    struct {
      unsigned char  _debsioab;      /* FLAG BYTE                               ICB280 */
      unsigned int   _debsioad : 24; /* ADDRESS OF START I/O APPENDAGE ROUTINE  ICB280 */
      } debsioa;
    } debavt;
  struct {
    unsigned char  _debpciab;      /* FLAG BYTE                               ICB280 */
    unsigned int   _debpciad : 24; /* ADDRESS OF PROGRAM-CONTROLLED-INTERRUPTION     */
    } debpcia;
  struct {
    unsigned char  _debceab;      /* FLAG BYTE                               ICB280 */
    unsigned int   _debcead : 24; /* ADDRESS OF CHANNEL-END APPENDAGE ROUTINE       */
    } debcea;
  struct {
    unsigned char  _debxceab;      /* FLAG BYTE                               ICB280 */
    unsigned int   _debxcead : 24; /* ADDRESS OF ABNORMAL-END APPENDAGE ROUTINE      */
    } debxcea;
  unsigned char  debwkara;    /* O/C/E WORK AREA (DIRECT ACCESS)                */
  unsigned char  debdscba[7]; /* DSCB ADDRESS (BBCCHHR) USED BY O/C/E           */
  struct {
    unsigned char  _debdcbmk[4]; /* DCB MODIFICATION MASK USED BY O/C/E */
    } debxtnp;
  unsigned char  deblngth;    /* LENGTH OF DEB IN DOUBLE WORDS.  If it     @MXC */
  unsigned char  debamtyp;    /* ACCESS METHOD TYPE                      ICB380 */
  short int      debtblof;    /* Identifier in DEB table for the entry     @MXC */
  struct {
    struct {
      unsigned char  _debnmsub; /* NUMBER OF SUBROUTINES LOADED BY OPEN */
      } debamid;
    unsigned int   _debtcbb : 24; /* ADDRESS OF TCB FOR THIS DEB */
    } debtcbad;
  struct {
    unsigned char  _debamlng;     /* NUMBER OF BYTES IN THE ACCESS METHOD DEPENDENT */
    unsigned int   _debdebb : 24; /* ADDRESS OF THE NEXT DEB IN THE SAME TASK       */
    } debdebad;
  struct {
    unsigned char  _deboflgs;     /* DATA SET STATUS FLAGS                  */
    unsigned int   _debirbb : 24; /* IRB STORAGE ADDRESS USED FOR APPENDAGE */
    } debirbad;
  unsigned char  debopatb;    /* FLAGS INDICATING BOTH THE METHOD OF I/O        */
  unsigned char  debqscnt;    /* PURGE (SVC 16) - QUIESCE COUNT.  NUMBER OF     */
  unsigned char  debflgs1;    /* FLAG FIELD                                     */
  unsigned char  debflgs2;    /* FLAG FIELD TWO                            @L5C */
  struct {
    unsigned char  _debnmext;      /* NUMBER OF EXTENT DESCRIPTIONS STARTING       */
    unsigned int   _debusrpb : 24; /* ADDRESS OF FIRST IOB IN THE USER PURGE CHAIN */
    } debusrpg;
  struct {
    struct {
      unsigned char  _debprior;     /* PRIORITY OF THE TASK OWNING DEB            */
      unsigned int   _debecbb : 24; /* ADDRESS OF A PARAMETER LIST USED TO LOCATE */
      } debecbad;
    } debrrq;
  struct {
    struct {
      unsigned char  _debdebid; /* A HEX F IN LOW-ORDER 4 BITS TO IDENTIFY */
      } debprotg;
    unsigned int   _debdcbb : 24; /* ADDRESS OF DCB OR ACB ASSOCIATED WITH THIS DEB */
    } debdcbad;
  struct {
    unsigned char  _debexscl;     /* THIS FIELD IS USED TO DETERMINE THE SIZE OF */
    unsigned int   _debappb : 24; /* ADDRESS OF THE I/O APPENDAGE VECTOR TABLE   */
    } debappad;
  union {
    struct {
      unsigned char  _debsdvm;       /* DEVICE MODIFIER.  FOR MAGNETIC TAPE, MODESET */
      unsigned int   _debsucbb : 24; /* ADDRESS OF A UCB ASSOCIATED WITH A GIVEN     */
      } debsucba;
    struct {
      unsigned char  _debniee;      /* NUMBER OF EXTENTS OF INDEPENDENT INDEX AREA */
      unsigned int   _debfieb : 24; /* ADDRESS OF FIRST INDEX EXTENT               */
      } debfiead;
    } _deb_union1;
  union {
    struct {
      struct {
        unsigned char  _debsdvmx;    /* Device modifier.  Mag tape: modeset op code    */
        unsigned char  _filler1[3];  /* Reserved                                  @MXA */
        } debdeved;
      void * __ptr32 _debdvedx;    /* End of section if DEB31UCB is on.         @MXA */
      unsigned char  _filler2[8];
      } _deb_struct1;
    struct {
      struct {
        unsigned char  _debrsv06;      /* RESERVED                                ICB394 */
        unsigned int   _debrdcba : 24; /* ADDRESS OF DCB FOR THE READ ASSOCIATED DATA    */
        } debrdcb;
      struct {
        unsigned char  _debrsv07;      /* RESERVED                                ICB394 */
        unsigned int   _debpdcba : 24; /* ADDRESS OF DCB FOR THE PUNCH ASSOCIATED DATA   */
        } debpdcb;
      struct {
        unsigned char  _debrsv08;      /* RESERVED                                ICB394 */
        unsigned int   _debwdcba : 24; /* ADDRESS OF DCB FOR THE PRINT ASSOCIATED DATA   */
        } debwdcb;
      unsigned char  _filler3[4];
      } _deb_struct2;
    struct {
      unsigned char  _debvolac; /* VOLUME ACCESSIBILITY INDICATOR  (MDC327)       */
      unsigned char  _debdssql; /* DATA SET SECURITY QUALIFIER (MDC328)  @X04AA9A */
      char           _debvsequ; /* VOLUME SEQUENCE NUMBER  (MDC318)      @X04AA9A */
      unsigned char  _debeamfg; /* FLAG BYTE  (MDC319)                   @X04AA9A */
      struct {
        unsigned char  _debexdte[6]; /* EXPIRATION DATE (OUTPUT)  (MDC333)    @X04AA9A */
        unsigned char  _debwtpti;    /* WRITE PROTECT INDICATOR (OUTPUT)  (MDC334)     */
        unsigned char  _debrv008;    /* RESERVED (OUTPUT)  (MDC335)           @X04AA9A */
        } debdsid;
      struct {
        struct {
          struct {
            unsigned char  _debboerv; /* RESERVED  (MDC323)                    @X04AA9A */
            } debeodrv;
          struct {
            unsigned char  _debboett; /* BOE TRACK NUMBER  (MDC324)            @X04AA9A */
            } debeodtt;
          struct {
            unsigned char  _debboe0; /* MUST BE ZERO  (MDC325)                @X04AA9A */
            } debeod0;
          struct {
            unsigned char  _debboess; /* BOE SECTOR NUMBER  (MDC326)           @X04AA9A */
            } debeodss;
          } debboe;
        } debeod;
      } debasc09;
    struct {
      struct {
        unsigned char  _debnpee;      /* NUMBER OF EXTENTS OF PRIME DATA AREA   */
        unsigned int   _debfpeb : 24; /* ADDRESS OF THE FIRST PRIME DATA EXTENT */
        } debfpead;
      struct {
        unsigned char  _debnoee;      /* NUMBER OF EXTENTS OF INDEPENDENT OVERFLOW AREA */
        unsigned int   _debfoeb : 24; /* ADDRESS OF THE FIRST OVERFLOW EXTENT           */
        } debfoead;
      struct {
        unsigned char  _debrpsid;      /* ROTATIONAL POSITION SENSING (RPS) DEVICE       */
        unsigned int   _debexpta : 24; /* ADDRESS OF ISAM DEB EXTENSION           ICB379 */
        } debexpt;
      unsigned char  _filler4[4];
      } _deb_struct3;
    } _deb_union2;
  };

#define debeoeab debavt.debeoea._debeoeab
#define debeoead debavt.debeoea._debeoead
#define debsioab debavt.debsioa._debsioab
#define debsioad debavt.debsioa._debsioad
#define debpciab debpcia._debpciab
#define debpciad debpcia._debpciad
#define debceab  debcea._debceab
#define debcead  debcea._debcead
#define debxceab debxcea._debxceab
#define debxcead debxcea._debxcead
#define debdcbmk debxtnp._debdcbmk
#define debnmsub debtcbad.debamid._debnmsub
#define debtcbb  debtcbad._debtcbb
#define debamlng debdebad._debamlng
#define debdebb  debdebad._debdebb
#define deboflgs debirbad._deboflgs
#define debirbb  debirbad._debirbb
#define debnmext debusrpg._debnmext
#define debusrpb debusrpg._debusrpb
#define debprior debrrq.debecbad._debprior
#define debecbb  debrrq.debecbad._debecbb
#define debdebid debdcbad.debprotg._debdebid
#define debdcbb  debdcbad._debdcbb
#define debexscl debappad._debexscl
#define debappb  debappad._debappb
#define debsdvm  _deb_union1.debsucba._debsdvm
#define debsucbb _deb_union1.debsucba._debsucbb
#define debniee  _deb_union1.debfiead._debniee
#define debfieb  _deb_union1.debfiead._debfieb
#define debsdvmx _deb_union2._deb_struct1.debdeved._debsdvmx
#define debdvedx _deb_union2._deb_struct1._debdvedx
#define debrsv06 _deb_union2._deb_struct2.debrdcb._debrsv06
#define debrdcba _deb_union2._deb_struct2.debrdcb._debrdcba
#define debrsv07 _deb_union2._deb_struct2.debpdcb._debrsv07
#define debpdcba _deb_union2._deb_struct2.debpdcb._debpdcba
#define debrsv08 _deb_union2._deb_struct2.debwdcb._debrsv08
#define debwdcba _deb_union2._deb_struct2.debwdcb._debwdcba
#define debvolac _deb_union2.debasc09._debvolac
#define debdssql _deb_union2.debasc09._debdssql
#define debvsequ _deb_union2.debasc09._debvsequ
#define debeamfg _deb_union2.debasc09._debeamfg
#define debexdte _deb_union2.debasc09.debdsid._debexdte
#define debwtpti _deb_union2.debasc09.debdsid._debwtpti
#define debrv008 _deb_union2.debasc09.debdsid._debrv008
#define debboerv _deb_union2.debasc09.debeod.debboe.debeodrv._debboerv
#define debboett _deb_union2.debasc09.debeod.debboe.debeodtt._debboett
#define debboe0  _deb_union2.debasc09.debeod.debboe.debeod0._debboe0
#define debboess _deb_union2.debasc09.debeod.debboe.debeodss._debboess
#define debnpee  _deb_union2._deb_struct3.debfpead._debnpee
#define debfpeb  _deb_union2._deb_struct3.debfpead._debfpeb
#define debnoee  _deb_union2._deb_struct3.debfoead._debnoee
#define debfoeb  _deb_union2._deb_struct3.debfoead._debfoeb
#define debrpsid _deb_union2._deb_struct3.debexpt._debrpsid
#define debexpta _deb_union2._deb_struct3.debexpt._debexpta

/* Values for field "debsioab" */
#define debpgfx  0x80 /* ADDRESS IN DEBSIOAD CAN BE USED TO DETERMINE    */
#define debsiox  0x40 /* IF ZERO, DO NOT ENTER SIO APPENDAGE WHEN ERP    */
#define debiovr  0x20 /* IF ONE, EXCPVR REQUEST IS VALID.  IF ZERO,      */
#define debfix   0x10 /* INDICATION THAT DEB HAS BEEN FIXED (OS/VS2)     */
#define debsionp 0x0F /* NUMBER OF 2K PAGES TO BE FIXED FOR THE          */

/* Values for field "debpciab" */
#define debrsv24 0x80 /* RESERVED                                        */
#define debrsv25 0x40 /* RESERVED                                        */
#define debrsv26 0x20 /* RESERVED                                        */
#define debrsv27 0x10 /* RESERVED                                        */
#define debpcinp 0x0F /* NUMBER OF 2K PAGES TO BE FIXED FOR THE          */

/* Values for field "debceab" */
#define debesmvr 0x80 /* VALIDITY CHECK FOR EXCPVR CALLER    @ZA34098    */
#define debrsv29 0x40 /* RESERVED                                        */
#define debrsv30 0x20 /* RESERVED                                        */
#define debrsv31 0x10 /* RESERVED                                        */
#define debcenp  0x0F /* NUMBER OF 2K PAGES TO BE FIXED FOR THE          */

/* Values for field "debxceab" */
#define debrsv32 0x80 /* RESERVED                                        */
#define debrsv33 0x40 /* RESERVED                                        */
#define debrsv34 0x20 /* RESERVED                                        */
#define debrsv35 0x10 /* RESERVED                                        */
#define debxcenp 0x0F /* NUMBER OF 2K PAGES TO BE FIXED FOR THE          */

/* Values for field "debamtyp" */
#define debamnon 0    /* ACCESS METHOD TYPE NOT KNOWN              @L2A  */
#define debamvsm 1    /* VSAM ACCESS METHOD TYPE          @L2A           */
#define debamxcp 2    /* EXCP ACCESS METHOD TYPE          @L2A           */
#define debamtcm 4    /* TCAM ACCESS METHOD TYPE          @L2A           */
#define debamgam 8    /* GRAPHICS ACCESS METHOD TYPE      @L2A           */
#define debamtam 16   /* BTAM ACCESS METHOD TYPE          @L2A           */
#define debambpm 32   /* BPAM ACCESS METHOD TYPE          @L2A           */
#define debamsam 32   /* SEQUENTIAL ACCESS METHOD TYPE    @L2A           */
#define debambdm 64   /* DIRECT ACCESS METHOD TYPE        @L2A           */
#define debamism 128  /* ISAM ACCESS METHOD TYPE          @L2A           */
#define debamsub 129  /* SUBSYSTEM ACCESS METHOD TYPE     @L2A           */
#define debamvtm 130  /* VTAM ACCESS METHOD TYPE          @L2A           */
#define debamtap 132  /* TCAM APPLICATION ACC METHOD TYPE @L2A           */

/* Values for field "debamid" */
#define debtamid 0x00 /* TCAM DEB ID                           @ZA51573  */
#define debvamid 0x0F /* VTAM DEB ID                           @ZA51573  */

/* Values for field "deboflgs" */
#define debdisp  0xC0 /* DATA SET DISPOSITION FLAGS                      */
#define debdsold 0x40 /* OLD DATA SET                     @L2A           */
#define debdsmod 0x80 /* MOD DATA SET                     @L2A           */
#define debdsnew 0xC0 /* NEW DATA SET                     @L2A           */
#define debeof   0x20 /* END-OF-FILE (EOF) ENCOUNTERED (TAPE INPUT)      */
#define debrlse  0x10 /* RELEASE UNUSED EXTERNAL STORAGE (DASD)          */
#define debdcb   0x08 /* DCB MODIFICATION                                */
#define debsplit 0x04 /* SPLIT CYLINDER (DASD). NO LONGER SUPPORTED.     */
#define deblabel 0x02 /* NONSTANDARD LABELS                              */
#define debrerr  0x01 /* USE REDUCED ERROR RECOVERY PROCEDURE (TAPE)     */

/* Values for field "debopatb" */
#define debabend 0x80 /* SET BY ABEND INDICATING A SYSABEND OR           */
#define debzero  0x40 /* ALWAYS ZERO                                     */
#define debposit 0x30 /* DATA SET POSITIONING FLAGS                      */
#define debrered 0x10 /* REREAD                           @L2A           */
#define debleave 0x30 /* LEAVE                            @L2A           */
#define debaccs  0x0F /* TYPE OF I/O ACCESSING BEING DONE                */
#define debinput 0    /* INPUT                            @L2A           */
#define deboutpt 0x0F /* OUTPUT (OPEN PARAMETER LIST      @P8C           */
#define debxtend 0x0E /* EXTEND (OPEN PARAMETER LIST ONLY)@P8C           */
#define debinout 0x03 /* INOUT                            @L2A           */
#define deboutin 0x07 /* OUTIN (OPEN PARAMETER LIST       @P8C           */
#define debotinx 0x06 /* OUTINX (OPEN PARAMETER LIST ONLY)@P8C           */
#define debrdbck 0x01 /* RDBACK                           @L2A           */
#define debupdat 0x04 /* UPDAT                            @L2A           */

/* Values for field "debflgs1" */
#define debpwckd 0x80 /* PASSWORD WAS SUPPLIED DURING OPEN.  EOV WILL    */
#define debeofdf 0x40 /* SET BY EOV TO INFORM CLOSE THAT AN END-OF-FILE  */
#define debrsioa 0x20 /* SIO APPENDAGE RE-ENTRY AUTHORIZATION BIT        */
#define debexcpa 0x10 /* EXCP IS AUTHORIZED FOR THIS DEB       @ZA20762  */
#define debcindi 0x08 /* DCB ASSOCIATED WITH THIS DEB IS BEING           */
#define debf1cev 0x04 /* EOV PROCESSING OCCURRED DURING CLOSE            */
#define debapfin 0x02 /* IF ON, AUTHORIZED PROGRAMS CAN BE LOADED        */
#define debxtnin 0x01 /* IF ONE, DEB EXTENSION EXISTS            MDC007  */

/* Values for field "debflgs2" */
#define debiopav 0x80 /* THE I/O PREVENTION IDENTIFIER (IOPID)     @L5A  */
#define debvcr   0x40 /* RLS=CR JCL OPTION                         @19C  */
#define debvnri  0x20 /* RLS=NRI JCL OPTION                        @19C  */
#define debvcre  0x10 /* RLS=CRE JCL OPTION                        @19C  */
#define debvrls  0x08 /* VSAM RLS ACCESS(VSAM ONLY)           @P5M       */
#define debdscmp 0x08 /* TAPE DATA SET COMPACTION MODE        @H3A       */
#define debdsncp 0x04 /* TAPE DATA SET NON-COMPACTION MODE    @H3A       */
#define deb31ucb 0x02 /* UCB address field is 4 bytes. Use DEBSUCBA @MXC */
#define debtvs   0x01 /* TRANSACTIONAL VSAM                        @P6C  */

/* Values for field "debsdvm" */
#define debmtdn4 0xD3 /* 9-TRACK MODESET CCW CODE DENSITY=6250BPI  @H3A  */
#define debmtdn3 0xC3 /* 9-TRACK MODESET CCW CODE DENSITY=1600BPI  @H3A  */
#define debmtdn2 0xCB /* 9-TRACK MODESET CCW CODE DENSITY= 800BPI  @H3A  */
#define debm7dn0 0x03 /* 7-TRACK MODESET SKELETON DENSITY=200BPI   @H3A  */
#define debm7dn1 0x43 /* 7-TRACK MODESET SKELETON DENSITY=556BPI   @H3A  */
#define debm7dn2 0x83 /* 7-TRACK MODESET SKELETON DENSITY=800BPI   @H3A  */
#define debmstwi 0xC3 /* 3480 SET TAPE WRITE IMMEDIATE CCW CODE    @H3A  */
#define debmtrf0 0x80 /* TAPE RECORDING FORMAT BIT 0               @H3A  */
#define debmtrf1 0x40 /* TAPE RECORDING FORMAT BIT 1               @H3A  */
#define debmtwi  0x20 /* TAPE WRITE IMMEDIATE (NON-BUFFERED WRITE) @H3A  */
#define debminhs 0x10 /* INHIBIT SUPERVISOR COMMANDS               @H3A  */
#define debmcomp 0x08 /* COMPACTED RECORDING MODE                  @H3A  */
#define debcmpac 0x08 /* COMPACTED RECORDING MODE                  @H3A  */
#define debm3424 0x02 /* 3424 MODE SET FLAG                        @H3A  */
#define debm9348 0x02 /* RESERVED(WILL BE DELETED)                 @H3A  */
#define debminhe 0x01 /* INHIBIT CONTROL UNIT ERP                  @H3A  */
#define debm6250 0xC2 /* SET 3424 DENSITY=6250BPI @H3A                   */
#define debm1600 0x42 /* SET 3424 DENSITY=1600BPI @H3A                   */
#define debm4trk 0xC2 /* RESERVED(WILL BE DELETED)@H3A                   */
#define debm2trk 0x82 /* RESERVED(WILL BE DELETED)@H3A                   */
#define debm1trk 0x42 /* RESERVED(WILL BE DELETED)@H3A                   */

/* Values for field "debeamfg" */
#define debmulti 0x80 /* MULTI-VOLUME INDICATOR  (MDC320)      @X04AA9A  */
#define debdsopn 0x40 /* DATA SET IS OPEN  (MDC321)            @X04AA9A  */
#define debvamsg 0x20 /* VOLUME ACCESSIBILITY MESSAGE HAS BEEN ISSUED    */
#define debsecvl 0x10 /* SECURE VOLUME  (MDC332)               @X04AA9A  */
#define debrv004 0x08 /* RESERVED                          @X04AA9A      */
#define debrv005 0x04 /* RESERVED                          @X04AA9A      */
#define debrv006 0x02 /* RESERVED                          @X04AA9A      */
#define debrv007 0x01 /* RESERVED                          @X04AA9A      */

/* Values for field "debrpsid" */
#define debrpsp  0x80 /* PRIME DATA AREA IS ON RPS DEVICE                */
#define debrpsi  0x40 /* INDEPENDENT INDEX AREA IS ON RPS DEVICE         */
#define debrpso  0x20 /* INDEPENDENT OVERFLOW AREA IS ON RPS DEVICE      */
#define debrpsap 0x10 /* RPS SIO APPENDAGE HAS BEEN LOADED               */
#define debrsv09 0x08 /* RESERVED                                        */
#define debrsv10 0x04 /* RESERVED                                        */
#define debrsv11 0x02 /* RESERVED                                        */
#define debrsv12 0x01 /* RESERVED                                        */

#endif

#ifndef __debdasd__
#define __debdasd__

struct debdasd {
  struct {
    unsigned char  _debdvmod;     /* DEVICE MODIFIER - FILE MASK              */
    unsigned int   _debucba : 24; /* Address of UCB associated with this data */
    } debucbad;
  struct {
    unsigned char  _debdvmod31; /* File mask.  Valid only if DEB31UCB is on  @MXC */
    unsigned char  _debnmtrkhi; /* HIGH ORDER BYTE OF NUMBER OF TRACKS IN    @LVA */
    } debbinum;
  unsigned char  debstrcc[2]; /* CYLINDER ADDRESS FOR THE START OF AN EXTENT.   */
  unsigned char  debstrhh[2]; /* TRACK ADDRESS FOR THE START OF AN EXTENT.      */
  unsigned char  debendcc[2]; /* CYLINDER ADDRESS FOR THE END OF AN EXTENT.     */
  unsigned char  debendhh[2]; /* TRACK ADDRESS FOR THE END OF AN EXTENT.        */
  unsigned char  debnmtrk[2]; /* LOW ORDER TWO BYTES OF NUMBER OF TRACKS   @LVC */
  };

#define debdvmod   debucbad._debdvmod
#define debucba    debucbad._debucba
#define debdvmod31 debbinum._debdvmod31
#define debnmtrkhi debbinum._debnmtrkhi

#endif

#ifndef __debacsmd__
#define __debacsmd__

struct debacsmd {
  union {
    struct {
      struct {
        unsigned char  _debvolbt; /* FIRST BYTE OF DEBVOLSQ (MDC016)        YA00318 */
        char           _debvlseq; /* FOR DIRECT ACCESS, SEQUENCE NUMBER OF THE      */
        } debvolsq;
      unsigned char  _debvolnm[2]; /* TOTAL NUMBER OF VOLUMES IN A MULTIVOLUME */
      struct {
        struct {
          unsigned char  _debrsv13;      /* RESERVED                               */
          unsigned int   _debutsab : 24; /* ADDRESS OF THE USER TOTALING SAVE AREA */
          } debutsaa;
        unsigned char  _debrsv14[4]; /* RESERVED (IF USER TOTALING WAS SPECIFIED) */
        } debdsnm;
      } _debacsmd_struct1;
    struct {
      unsigned char  _debextnm;     /* FOR A PARTITIONED DATA SET OPENED FOR INPUT, */
      unsigned char  _filler1[11];
      } _debacsmd_struct2;
    struct {
      struct {
        unsigned char  _debdbpt;    /* NUMBER OF BLOCKS PER TRACK  */
        unsigned char  _debdbpe[3]; /* NUMBER OF BLOCKS PER EXTENT */
        } debdblk;
      unsigned char  _filler2[8];
      } _debacsmd_struct3;
    struct {
      struct {
        unsigned char  _debrsv15;      /* RESERVED                      */
        unsigned int   _debtbfrb : 24; /* ADDRESS OF THE BUFFER ROUTINE */
        } debtbfra;
      struct {
        unsigned char  _debrsv16;      /* RESERVED                                   */
        unsigned int   _debtccwb : 24; /* ADDRESS OF THE FIRST (OR FOLLOWING) CCW ON */
        } debtccwa;
      unsigned char  _filler3[4];
      } _debacsmd_struct4;
    struct {
      struct {
        unsigned char  _debrsv17;      /* RESERVED             */
        unsigned int   _debfucbb : 24; /* ADDRESS OF FIRST UCB */
        } debfucba;
      struct {
        unsigned char  _debrsv18;      /* RESERVED            */
        unsigned int   _deblucbb : 24; /* ADDRESS OF LAST UCB */
        } deblucba;
      unsigned char  _filler4[4];
      } _debacsmd_struct5;
    struct {
      unsigned char  _debdsnam[8]; /* FOR A PARTITIONED DATA SET OPENED FOR */
      unsigned char  _filler5[4];
      } _debacsmd_struct6;
    struct {
      int            _debdtpp;  /* NUMBER OF TRACKS PER PERIOD */
      int            _debdbpp;  /* NUMBER OF BLOCKS PER PERIOD */
      int            _debdbpef; /* NUMBER OF BLOCKS PER EXTENT */
      } _debacsmd_struct7;
    struct {
      void * __ptr32 _debdcbfa;    /* ADDRESS OF DCB FIELD AREA               MDC013 */
      void * __ptr32 _debput;      /* ADDRESS OF PUT MODULE                   ICB379 */
      unsigned char  _filler6[4];
      } _debacsmd_struct8;
    struct {
      void * __ptr32 _filler7;  /* DEBDCBFA - ADDRESS OF DCB FIELD AREA    MDC014 */
      void * __ptr32 _debget;   /* ADDRESS OF GET OR PUT MODULE - THIS FIELD      */
      void * __ptr32 _debwkpt4; /* SAME AS DCBWKPT4 - ADDRESS OF UCB       ICB379 */
      } _debacsmd_struct9;
    struct {
      void * __ptr32 _filler8;  /* DEBDCBFA - ADDRESS OF DCB FIELD AREA    MDC015 */
      void * __ptr32 _debdisad; /* ADDRESS OF PRIVILEGED MODULE ENTERED WHEN A    */
      void * __ptr32 _filler9;  /* DEBWKPT4 - SAME AS DCBWKPT4 - ADDRESS OF THE   */
      } _debacsmd_struct10;
    } _debacsmd_union1;
  union {
    struct {
      short int      _debblksi; /* MAXIMUM BLOCK SIZE  (MDC346)          @ZA03699 */
      short int      _deblrecl; /* LOGICAL RECORD LENGTH  (MDC347)       @ZA03699 */
      } _debacsmd_struct11;
    void * __ptr32 _debwkpt5;  /* SAME AS DCBWKPT5 - ADDRESS OF GET APPENDAGE  */
    void * __ptr32 _filler10;  /* DEBWKPT5 - SAME AS DCBWKPT5 - ADDRESS OF THE */
    } _debacsmd_union2;
  union {
    void * __ptr32 _debcread; /* ADDRESS OF CHANNEL-END APPENDAGE FOR           */
    void * __ptr32 _debfreed; /* ADDRESS OF DYNAMIC BUFFERING MODULE     ICB379 */
    } _debacsmd_union3;
  union {
    void * __ptr32 _debcsetl; /* ADDRESS OF CHANNEL-END APPENDAGE FOR   */
    void * __ptr32 _debrpsio; /* ADDRESS OF RPS SIO APPENDAGE MODULE IF */
    } _debacsmd_union4;
  union {
    void * __ptr32 _debcwrit; /* ADDRESS OF CHANNEL-END APPENDAGE FOR          */
    void * __ptr32 _debsioa2; /* ADDRESS OF DYNAMIC BUFFERING APPENDAGE MODULE */
    } _debacsmd_union5;
  void * __ptr32 debcchk;  /* ADDRESS OF CHANNEL-END APPENDAGE FOR         */
  void * __ptr32 debcrewt; /* ADDRESS OF CHANNEL-END APPENDAGE FOR         */
  void * __ptr32 debcreck; /* ADDRESS OF CHANNEL-END APPENDAGE FOR         */
  void * __ptr32 debaread; /* ADDRESS OF ABNORMAL-END APPENDAGE FOR        */
  void * __ptr32 debasetl; /* ADDRESS OF ABNORMAL-END APPENDAGE FOR        */
  void * __ptr32 debawrit; /* ADDRESS OF ABNORMAL-END APPENDAGE FOR        */
  void * __ptr32 debachk;  /* ADDRESS OF ABNORMAL-END APPENDAGE FOR        */
  void * __ptr32 debarewt; /* ADDRESS OF ABNORMAL-END APPENDAGE FOR        */
  void * __ptr32 debareck; /* ADDRESS OF ABNORMAL-END APPENDAGE FOR        */
  void * __ptr32 debrpsst; /* ADDRESS OF RPS SIO APPENDAGE IF ADDRSPC=REAL */
  };

#define debvolbt  _debacsmd_union1._debacsmd_struct1.debvolsq._debvolbt
#define debvlseq  _debacsmd_union1._debacsmd_struct1.debvolsq._debvlseq
#define debvolnm  _debacsmd_union1._debacsmd_struct1._debvolnm
#define debrsv13  _debacsmd_union1._debacsmd_struct1.debdsnm.debutsaa._debrsv13
#define debutsab  _debacsmd_union1._debacsmd_struct1.debdsnm.debutsaa._debutsab
#define debrsv14  _debacsmd_union1._debacsmd_struct1.debdsnm._debrsv14
#define debextnm  _debacsmd_union1._debacsmd_struct2._debextnm
#define debdbpt   _debacsmd_union1._debacsmd_struct3.debdblk._debdbpt
#define debdbpe   _debacsmd_union1._debacsmd_struct3.debdblk._debdbpe
#define debrsv15  _debacsmd_union1._debacsmd_struct4.debtbfra._debrsv15
#define debtbfrb  _debacsmd_union1._debacsmd_struct4.debtbfra._debtbfrb
#define debrsv16  _debacsmd_union1._debacsmd_struct4.debtccwa._debrsv16
#define debtccwb  _debacsmd_union1._debacsmd_struct4.debtccwa._debtccwb
#define debrsv17  _debacsmd_union1._debacsmd_struct5.debfucba._debrsv17
#define debfucbb  _debacsmd_union1._debacsmd_struct5.debfucba._debfucbb
#define debrsv18  _debacsmd_union1._debacsmd_struct5.deblucba._debrsv18
#define deblucbb  _debacsmd_union1._debacsmd_struct5.deblucba._deblucbb
#define debdsnam  _debacsmd_union1._debacsmd_struct6._debdsnam
#define debdtpp   _debacsmd_union1._debacsmd_struct7._debdtpp
#define debdbpp   _debacsmd_union1._debacsmd_struct7._debdbpp
#define debdbpef  _debacsmd_union1._debacsmd_struct7._debdbpef
#define debdcbfa  _debacsmd_union1._debacsmd_struct8._debdcbfa
#define debput    _debacsmd_union1._debacsmd_struct8._debput
#define debget    _debacsmd_union1._debacsmd_struct9._debget
#define debwkpt4  _debacsmd_union1._debacsmd_struct9._debwkpt4
#define debdisad  _debacsmd_union1._debacsmd_struct10._debdisad
#define debblksi  _debacsmd_union2._debacsmd_struct11._debblksi
#define deblrecl  _debacsmd_union2._debacsmd_struct11._deblrecl
#define debwkpt5  _debacsmd_union2._debwkpt5
#define debcread  _debacsmd_union3._debcread
#define debfreed  _debacsmd_union3._debfreed
#define debcsetl  _debacsmd_union4._debcsetl
#define debrpsio  _debacsmd_union4._debrpsio
#define debcwrit  _debacsmd_union5._debcwrit
#define debsioa2  _debacsmd_union5._debsioa2

/* Values for field "debvolbt" */
#define debexful 0x80 /* SET BY EOV WHEN REWRITING AN OLD DIRECT ACCESS */
#define debrsv36 0x40 /* RESERVED                            MDC018     */
#define debrsv37 0x20 /* RESERVED                            MDC018     */
#define debrsv38 0x10 /* RESERVED                            MDC018     */
#define debrsv39 0x08 /* RESERVED                            MDC018     */
#define debrsv40 0x04 /* RESERVED                            MDC018     */
#define debrsv41 0x02 /* RESERVED                            MDC018     */
#define debrsv42 0x01 /* RESERVED                            MDC018     */

#endif

#ifndef __debsubnm__
#define __debsubnm__

struct debsubnm {
  unsigned char  debsubid[2]; /* SUBROUTINE IDENTIFICATION.  EACH ACCESS METHOD */
  };

#endif

#ifndef __debxtn__
#define __debxtn__

struct debxtn {
  short int      debxlngh;    /* LENGTH OF DEB EXTENSION                 MDC002  */
  unsigned char  debxflg1;    /* FLAG BYTE  (MDC027)                     YM1272  */
  unsigned char  debxflg2;    /* FLAG BYTE                                 @06C  */
  void * __ptr32 debxdsab;    /* POINTER TO DSAB                         MDC004  */
  unsigned char  debxdcbm[4]; /* DCB MODIFICATION MASK USED BY O/C/EOV           */
  void * __ptr32 debxdbpr;    /* POINTER TO DEB                          MDC006  */
  unsigned char  debxdso1;    /* SAME AS DCBDSORG BYTE 1                 MDC020  */
  unsigned char  debxdso2;    /* SAME AS DCBDSORG BYTE 2                 MDC021  */
  unsigned char  debxmcf1;    /* SAME AS DCBMACRF BYTE 1                 MDC022  */
  unsigned char  debxmcf2;    /* SAME AS DCBMACRF BYTE 2                 MDC023  */
  void * __ptr32 debxxarg;    /* ADDRESS OF BDAM READ EXCLUSIVE LIST     MDC024  */
  void * __ptr32 debxopnj;    /* POINTER TO DSAB (SEPARATE FROM DEBXDSAB)        */
  void * __ptr32 debxsamb;    /* ADDRESS OF SAM BLOCK (SAMB)  (MDC351) @Z40FP9A  */
  unsigned char  debxopet[8]; /* DATASET OPEN TIME SET BY OPEN INITIAL @ZA39299  */
  struct {
    unsigned char  _debdefg1;    /* FLAG BYTE                                 @H2A */
    unsigned char  _debgattr;    /* GLOBAL ATTRIBUTES                         @H2A */
    unsigned char  _debblksz[2]; /* BLOCKSIZE IN BYTES                        @H2A */
    struct {
      unsigned char  _debnrdid[2]; /* SUBSYSTEM FUNCTION ID: NON-RETENTIVE DATA @P3C */
      unsigned char  _filler1;     /* RESERVED FOR SYSTEM USE                   @P3C */
      unsigned char  _debgattx;    /* GLOBAL ATTRIB EXTENDED 2                  @P3A */
      } debextok;
    } debxdef;
  unsigned char  debiopid[4]; /* I/O PREVENTION IDENTIFIER                 @L5A  */
  unsigned char  debblkid[4]; /* BLOCK ID VALUE USED TO CALCULATE NUMBER OF @L6A */
  unsigned char  debxcasf[4]; /* CACHE ATTRIBUTE SELECTION TOKEN            @LAA */
  unsigned char  debxflg3;    /* FLAG BYTE                                  @LDA */
  unsigned char  debxambf;    /* DEFINES THE USE OF DEBXAMB                 @LEA */
  unsigned char  debxscnt[2]; /* NUMBER OF STRIPES FOR STRIPED DATA SET     @15C */
  void * __ptr32 debxsacb;    /* POINTER TO SACB CONTROL BLOCK              @LDA */
  unsigned char  debxaflg;    /* FLAG BYTE                                  @LVC */
  unsigned char  debxrsv1;    /* RESERVED                                   @LGA */
  unsigned char  debxvlsq[2]; /* EXPECTED TAPE VOL SEQ                      @LGA */
  void * __ptr32 debxdssb;    /* POINTER TO DSSB                            @JBC */
  struct {
    struct {
      struct {
        struct {
          void * __ptr32 _debxrba; /* RBA OF VVDS CI CONTAINING NVR              @P5C */
          } debxenccb;
        } debxrdbk;
      } debxsscb;
    } debxamb;
  void * __ptr32 debxdebx;    /* POINTER TO SECOND DEB EXTENSION            @13A */
  };

#define debdefg1 debxdef._debdefg1
#define debgattr debxdef._debgattr
#define debblksz debxdef._debblksz
#define debnrdid debxdef.debextok._debnrdid
#define debgattx debxdef.debextok._debgattx
#define debxrba  debxamb.debxsscb.debxrdbk.debxenccb._debxrba

/* Values for field "debxflg1" */
#define debxcdcb     0x80 /* DEBDCBAD FIELD CONTAINS THE ADDRESS OF A        */
#define debxtskc     0x40 /* TASK CLOSE IS CLOSING THE RELATED DCB.  SET BY  */
#define debxdssi     0x20 /* DATA SET SECURITY INDICATOR.  SET BY OPEN AND   */
#define debxwind     0x10 /* MSS WINDOW PROCESSING INDICATOR.      @ZA37313  */
#define debxacis     0x08 /* ACQUIRE ISSUED WITH INHIBIT STAGE     @ZA37313  */
#define debxnfls     0x04 /* DO NOT FLUSH QSAM BUFFERS IN CLOSE BECAUSE      */
#define debxsysb     0x02 /* SYSTEM DETERMINED BLOCKSIZE               @L8C  */
#define debxrace     0x01 /* RACF EXECUTE ONLY AUTHORITY OF A PROGRAM  @L9A  */

/* Values for field "debxflg2" */
#define debxrsap     0x80 /* USED BY RESTART TO INDICATE THAT RESTART  @06A  */
#define debbyp       0x40 /* WHEN ON EXCP SCAN ROUTINE WILL SET        @LBC  */
#define debchcmp     0x20 /* WHEN ON EXCP SCAN ROUTINE WILL SET        @LBC  */
#define debxenqa     0x10 /* ENQUEUED ON PDS, DISP=SHARE, OUTPUT       @LCC  */
#define debxenqs     0x08 /* ENQUEUED ON DSCB,DISP=SHR                 @LCC  */
#define debximsc     0x04 /* IMS CLOSE IN PROGRESS                 @09C      */
#define debxcasv     0x02 /* THE CACHE ATTRIBUTE TOKEN IS VALID        @10A  */
#define debxmvlf     0x01 /* TAPE MULTIVOL & 1ST VOL READ              @LGC  */

/* Values for field "debdefg1" */
#define debnshed     0x80 /* NO SEEK HEAD PERMITTED                    @H2A  */
#define debxvdef     0x40 /* DEB DEF EXT DATA PARMS VALID. MUST BE ON  @P3C  */

/* Values for field "debgattr" */
#define debeckd      0xC0 /* EXTENT DEFINITION                         @H2A  */
#define debgaex1     0x80 /* EXTENT DEFINITION  1                      @H2A  */
#define debgaex2     0x40 /* EXTENT DEFINITION  2                      @H2A  */
#define debstrtp     0x20 /* CKD CONVERSION MODE - FOR SYSTEM USE      @P3C  */
#define debga345     0x1C /* ATTRIBUTES BITS  3,4,5                    @H2A  */
#define debga1       0x10 /* ATTRIBUTE  1                              @H2A  */
#define debga2       0x08 /* ATTRIBUTE  2                              @05C  */
#define debga3       0x04 /* ATTRIBUTE  3                              @05C  */
#define debncach     0x00 /* NORMAL CACHE ACCESS                       @L4A  */
#define debbcach     0x04 /* BYPASS CACHE LOAD                         @L7C  */
#define debicach     0x08 /* INHIBIT CACHE LOAD                        @L7C  */
#define debscach     0x0C /* SEQUENTIAL ACCESS                         @L7C  */
#define debxrflg     0x10 /* SEQUENTIAL PRESTAGE MODE                  @P3A  */
#define debrlc       0x14 /* RECORD ACCESS MODE                        @P3A  */
#define debnrd       0x02 /* NON RETENTIVE DATA ACCESS                 @08A  */
#define debinhfw     0x01 /* INHIBIT FAST WRITE                        @08A  */

/* Values for field "debgattx" */
#define debrmode     0x00 /* NO SPECIAL MODE                            @P3A */
#define debrdata     0x40 /* REGULAR DATA FORMAT                        @P3A */
#define debskey      0x80 /* SPECIAL KEY SEARCH MODE                    @P3A */
#define debdftr0     0x04 /* R0 DATA NOT REQUIRED                       @P3A */

/* Values for field "debxflg3" */
#define debxtrnc     0x80 /* QSAM TRUNC MACRO HAS BEEN ISSUED FOR A   @P3C   */
#define debxnseg     0x40 /* NULL SEGMENT ENCOUNTERED IN PDSE         @P3C   */
#define debxsmsg     0x20 /* SMS GUARANTEED SPACE DISP NEW OR MOD     @11A   */
#define debxduda     0x10 /* IEC708I DUP VOL ISSUED FOR DASD          @LOA   */
#define debxcap      0x08 /* UCB ADDRESS IN DEB CAPTURED BY OCE       @LFA   */
#define debxvtoc     0x04 /* NOT AUTHORIZED TO READ VTOC              @LSA   */
#define debxenqe     0x02 /* ENQUEUED ON PS, DISP=SHARE, OUTPUT       @LTA   */
#define debxucnt     0x01 /* DECREMENT EDI USE COUNT IN ESTAE         @P7A   */

/* Values for field "debxambf" */
#define debxabss     0x80 /* DEBXAMB DEFINES A SSCB PTR...DEBXSSCB IS @LEA   */
#define debxarba     0x40 /* DEBXAMB DEFINES AN RBA VALUE..DEBXRBA IS @15A   */
#define debxaoef     0x20 /* DEBXAMB DEFINES AN IGGSXCB PTR...        @P5C   */
#define debxaib      0x10 /* DEBXAMB DEFINES AN INTERMEDIATE BUFFER   @LKA   */
#define debxaenccb   0x08 /* DEBXAMB DEFINES AN ENCCB PTR            @L19A   */

/* Values for field "debxaflg" */
#define debxads      0x80 /* ATTRIBUTE EXTENSION DATA SET             @12A   */
#define debxainp     0x40 /* INPUT PROCESSING PERFORMED FOR           @12A   */
#define debxaout     0x20 /* OUTPUT PROCESSING PERFORMED FOR          @12A   */
#define debxaofo     0x10 /* "ATTRIBUTE EXTENSION" D/S OPENED FOR     @12A   */
#define debxlgds     0x08 /* LARGE SEQUENTIAL DATA SET BEING PROCESSED@LVA   */
#define debxdeblockx 0x04 /* DEB HAS BEEN LOCKED EXCLUSIVELY BY DEBCHK @L18A */
#define debxdeblock  0x04 /* DEB HAS BEEN LOCKED EXCLUSIVELY BY DEBCHK1@L18A */
#define debximg      0x02 /* THIS IS SYS1.IMAGELIB                      @33A */
#define debxexp      0x01 /* SET TO REQUEST SPECIAL SETTING OF IOSEXP   @24A */

#endif

#ifndef __deb2xtn__
#define __deb2xtn__

struct deb2xtn {
  unsigned char  deb2xtnn[8]; /* SECOND DEB EXTENSION EYE CATCHER          @13A */
  short int      deb2xlgh;    /* LENGTH OF SECOND DEB EXTENSION            @13A */
  struct {
    unsigned char  _deb2xsgl[2];  /* STORAGE GROUP NAME LENGTH                 @13A */
    unsigned char  _deb2xsgn[30]; /* STORAGE GROUP NAME                        @13A */
    } deb2xrsg;
  struct {
    unsigned char  _deb2xmcl[2];  /* MANAGEMENT CLASS NAME LENGTH              @13A */
    unsigned char  _deb2xmcn[30]; /* MANAGEMENT CLASS NAME                     @13A */
    } deb2xrmc;
  struct {
    unsigned char  _deb2xscl[2];  /* STORAGE CLASS LENGTH                      @13A */
    unsigned char  _deb2xscn[30]; /* STORAGE CLASS NAME                        @13A */
    } deb2xrsc;
  struct {
    unsigned char  _deb2xdcl[2];  /* DATA CLASS LENGTH                         @13A */
    unsigned char  _deb2xdcn[30]; /* DATA CLASS NAME                           @13A */
    } deb2xrdc;
  unsigned char  deb2xfg1;    /* FIRST FLAG BYTE                           @16A */
  unsigned char  deb2xuof;    /* TIOT DD OFFSET OF UNIT SELECTED BY OCE    @LFA */
  unsigned char  deb2xlsq[2]; /* TAPE VOL SEQ FROM LABEL                   @LGA */
  unsigned char  deb2xalv;    /* ANSI VERSION DURING OUTPUT                @LJA */
  unsigned char  deb2xfg2;    /* SECOND FLAG BYTE                          @LQC */
  void * __ptr32 deb2xtim;    /* PTR TO TIME STAMP                         @18A */
  unsigned char  deb2xexc;    /* DEB2XMXV EXCEED COUNT                     @20A */
  unsigned char  deb2xedi;    /* EDI SMF INDICATORS         @LTA                */
  unsigned char  deb2xfg3;    /* THIRD FLAG BYTE                           @22C */
  unsigned char  deb2xcfg;    /* ANSI V4 CCSID FLAGS(NOTE THAT ANY         @LJA */
  int            deb2xusr;    /* CCSID OF USER APPLICATION                 @LJA */
  int            deb2xtpe;    /* CCSID OF TAPE                             @LJA */
  int            deb2xlbl;    /* CCSID IN EXISTING TAPE LABEL              @LJA */
  void * __ptr32 deb2xsdc;    /* PTR TO SAM DATA CONVERSION BLOCK          @LJA */
  int            deb2xtbl;    /* ACCUMULATIVE BLOCKCNT ACROSS VOLUMES      @LLA */
  int            deb2xtcb;    /* DCB BLKCOUNT AT TCLOSE OUTPUT             @LLA */
  unsigned char  deb2xsgt;    /* STORAGE GROUP TYPE (GENERAL OR SPECIFIC)  @LMA */
  unsigned char  deb2xpar;    /* STARTING PARTITION OF TAPE FILE           @LPA */
  unsigned char  deb2xfg4;    /* FOURTH FLAG BYTE                          @LYA */
  unsigned char  deb2xfg5;    /* FIFTH FLAG BYTE                           @LZA */
  int            deb2xsbl;    /* START BLOCK NUMBER AFTER POSITIONING TP   @21A */
  void * __ptr32 deb2xstv;    /* ADDRESS OF FIRST VOLSER ENTRY             @LQA */
  void * __ptr32 deb2xnxv;    /* ADDRESS NEXT VOLSER ENTRY                 @LQA */
  int            deb2xvln;    /* NUMBER OF VOLS IN DATASET VOLUME LIST     @LQA */
  int            deb2xmxv;    /* MAXIMUM NUMBER OF VOLUME ENTRIES          @LQA */
  unsigned char  deb2xdid[8]; /* TIMESTAMP ID FOR THE DEB                  @LYA */
  unsigned char  deb2xlk1;    /* LENGTH OF KEYLABEL1                       @LYA */
  unsigned char  deb2xlk2;    /* LENGTH OF KEYLABEL2                       @LYA */
  unsigned char  deb2xfg6;    /* SIXTH FLAG BYTE                          @L13A */
  union {
    unsigned char  _deb2xfg7;     /* 7TH FLAG BYTE                            @PCC */
    unsigned char  _deb2xcmptype; /* @MDA                                          */
    } _deb2xtn_union1;
  int            deb2xket;    /* TAPE ENCRYPTION KEY EXCHANGE TIME        @25A  */
  void * __ptr32 deb2xdxp;    /* POINTER TO DEB EXTENSION                @L12A  */
  unsigned char  deb2xdsv;    /* DATA SET VERSION(0-PS(NOT EF), 1-EF VER0 @29C  */
  unsigned char  deb2xfg8;    /* 8TH FLAG BYTE                            @34A  */
  unsigned char  _filler1[2]; /* RESERVED                                 @35C  */
  double         deb2xtbx;    /* TOTAL BLOCKS ACROSS TAPE VOLUMES         @27A  */
  unsigned char  deb2xven[7]; /* VOLUME ENTRY (INCLUDES ',' AFTER VOLSER   @LQA */
  };

#define deb2xsgl     deb2xrsg._deb2xsgl
#define deb2xsgn     deb2xrsg._deb2xsgn
#define deb2xmcl     deb2xrmc._deb2xmcl
#define deb2xmcn     deb2xrmc._deb2xmcn
#define deb2xscl     deb2xrsc._deb2xscl
#define deb2xscn     deb2xrsc._deb2xscn
#define deb2xdcl     deb2xrdc._deb2xdcl
#define deb2xdcn     deb2xrdc._deb2xdcn
#define deb2xfg7     _deb2xtn_union1._deb2xfg7
#define deb2xcmptype _deb2xtn_union1._deb2xcmptype

/* Values for field "deb2xfg1" */
#define deb2xosm         0x80 /* JFCMEDIA TYPE SET BY OPEN                 @16A */
#define deb2xnlm         0x40 /* NL TAPE DATASET OPENED FOR MOD            @LLA */
#define deb2xtco         0x20 /* TCLOSE TAPE OUTPUT                        @LLA */
#define deb2xibc         0x10 /* INACCURATE BLOCK COUNT                    @LLA */
#define deb2xnl1         0x08 /* NL TAPE FILE SEQ 1                        @LLA */
#define deb2xvll         0x04 /* DEB2X INCLUDES DATASET VOLUME LIST        @LQA */
#define deb2xabc         0x02 /* ACCURATE TOTAL BLOCK COUNT                @LQA */
#define deb2xecu         0x01 /* 3490EMULATION EXT CAPACITY WRITTEN TO     @LQA */

/* Values for field "deb2xfg2" */
#define deb2x32m         0x80 /* OCE ISSUED 32BIT MODESET                  @LQA */
#define deb2xbtm         0x40 /* BUFFERED TM REQUESTED                     @LQA */
#define deb2xuss         0x20 /* USS DIRECTORY OPENED USING BPAM OR PART   @LNA */
#define deb2xexv         0x10 /* DEB2XMXV EXCEEDED                         @20A */
#define deb2xsca         0x08 /* TP MEDIA SCALED OPT PERFORMANCE           @LUA */
#define deb2btms         0x04 /* SYSTEM DEFAULTED BUFFERED TAPE MARKS      @LUA */
#define deb2xemv         0x02 /* EXTEND MULTIVOL TAPE FILE                 @21A */
#define deb2x8wn         0x01 /* RACF RC8 CHANGED TO RC0                   @LWA */

/* Values for field "deb2xedi" */
#define deb2xext         0x80 /* DSN FOUND IN EXCLUDE TABLE @LTA                */
#define deb2xopo         0x40 /* O/P AND ALREADY OPEN O/P   @LTA                */
#define deb2xino         0x20 /* I/P AND ALREADY OPEN O/P   @LTA                */
#define deb2xeps         0x10 /* DCBE, SCT OR DSAB EXCLUDED @LTA                */

/* Values for field "deb2xfg3" */
#define deb2xscs         0x80 /* SYSZEDI SCOPE SYSTEM(S)                   @22A */
#define deb2xpsc         0x40 /* PERFORMANCE SEGMENTED                     @LXA */
#define deb2xef1         0x20 /* 3592-E05 WRITING IN EFMT1 RECTECH         @LXA */
#define deb2xupf         0x10 /* USER BSAM PGFIX IN USE                    @LZA */
#define deb2xexcp        0x08 /* EAV BAM DETECTED ONE OR MORE EXCP        @L10A */
#define deb2xmsdn        0x04 /* OPEN USED MULTSDN                        @L14A */
#define deb2xtrce        0x02 /* DIAGNS=TRACE SPECIFIED                   @L16A */
#define deb2xsbs         0x01 /* START BLOCK ZERO STORED IN DEBBLKID       @23A */

/* Values for field "deb2xcfg" */
#define deb2xibm         0x80 /* IBM VERSION 4 CREATED TAPE                @LJA */
#define deb2xout         0x40 /* VERSION 4 TAPE OPENED FOR OUTPUT DISP NOT @LJA */
#define deb2xmod         0x20 /* VERSION 4 TAPE OPENED FOR OUTPUT DISP MOD @LJA */
#define deb2xudf         0x10 /* USER APPLICATION CCSID WAS DEFAULTED      @LJA */
#define deb2xtdf         0x08 /* TAPE CCSID WAS DEFAULTED                  @LJA */
#define deb2xign         0x04 /* TAPE CCSID SPECIFIED BUT NO CONVERSION    @LJA */

/* Values for field "deb2xfg4" */
#define deb2xeef         0x20 /* 3592-E05/E06 WRITING IN EEFMT2 RECTECH    @LZC */
#define deb2xe1m         0x10 /* KEKLS 1 METHOD HASH                       @LYA */
#define deb2xe1i         0x08 /* KEKLS 1 INPUT HASH                        @LYA */
#define deb2xe2m         0x04 /* KEKLS 2 METHOD HASH                       @LYA */
#define deb2xe2i         0x02 /* KEKLS 2 INPUT HASH                        @LYA */
#define deb2xee3         0x01 /* 3592-E06 WRITING IN EEFMT3 RECTECH        @LZA */

/* Values for field "deb2xfg5" */
#define deb2xef2         0x80 /* 3592-E06 WRITING IN EFMT2 RECTECH         @LZA */
#define deb2xenc         0x40 /* TAPE CARTRIDGE ENCRYPTED                  @LZA */
#define deb2xmnw         0x20 /* OPEN ENTERED WITH MOD CHANGED TO NEW      @26A */
#define deb2xdeq         0x10 /* TAPE INPUT FREE=EOV HONORED               @M4A */
#define deb2xef3         0x08 /* 3592-E07 WRITING IN EFMT3 RECTECH         @M9A */
#define deb2xee4         0x04 /* 3592-E07 WRITING IN EEFMT4 RECTECH        @M9A */
#define deb2xeeg         0x02 /* 3592-E06/E07 WRITING IN EEFMT3 RECTECH    @M9A */
#define deb2xcdt         0x01 /* FORCE EOV TO CHECK DEVICE TYPE            @28A */

/* Values for field "deb2xfg6" */
#define deb2xeex         0x80 /* DCBE INVALIDATED BECAUSE EXCP AND NO     @29C  */
#define deb2xdso         0x40 /* DCBE INVALIDATED BECAUSE DSORG IS NOT    @29C  */
#define deb2xfre         0x20 /* DCBE INVALIDATED BECAUSE DCBE STORAGE IS @29C  */
#define deb2xkey         0x10 /* DCBE INVALIDATED BECAUSE DCBE STORAGE    @29C  */
#define deb2xeid         0x08 /* DCBE INVALIDATED BECAUSE DCBEID IS NOT   @29C  */
#define deb2xmin         0x04 /* DCBE INVALIDATED BECAUSE DCBE IS NOT AT  @29C  */
#define deb2xnde         0x02 /* DCBE INVALIDATED BECAUSE THE DCBHIARCHY  @29C  */
#define deb2xrbl         0x01 /* TAPE READBACK LAST VOLUME                @32A  */

/* Values for field "deb2xfg7" */
#define deb2xvlc         0x80 /* IEC716I ISSUED                           @PCA  */
#define deb2xplr         0x40 /* TAPE INPUT PASS LEAVE OR RETAIN          @MCA  */
#define deb2xfcc         0x20 /* FREE=CLOSE IN A CONCATENATION            @30A  */
#define deb2xlvl         0x10 /* DASD LAST VOLUME DETECTED                @31A  */

/* Values for field "deb2xcmptype" */
#define deb2xcmpna       0x00 /* NOT COMPRESSED               @MDA              */
#define deb2xcmpgen      0x01 /* GENERIC COMPRESSION          @MDA              */
#define deb2xcmptlrd     0x02 /* TAILORED COMPRESSION         @MDA              */
#define deb2xcmpzedc     0x03 /* ZEDC COMPRESSION             @MEA              */

/* Values for field "deb2xfg8" */
#define deb2xdbyp        0x80 /* DCBE_BYPASS_AUTH WAS SET ON              @34A  */
#define deb2xjbyp        0x40 /* JSCBPASS WAS SET ON                      @34A  */
#define deb2xauth        0x20 /* CALLER OF OPEN WAS AUTHORIZED TO         @34A  */
#define deb2xbyp         0x10 /* OPEN BYPASSED SAF/RACF CHECKS            @34A  */
#define deb2xbpnotpds    0x08 /* BPAM USED BUT DATA SET IS NOT PDS, PDSE, @35A  */
#define deb2xextdfail    0x04 /* DADSM FAILED TO EXTEND TO NEW VOLUME DUE @36A  */
#define deb2xdsencryptok 0x02 /* THE DCBE HAS DSENCRYPT=OK              @L20A   */

#endif

#pragma pack()
