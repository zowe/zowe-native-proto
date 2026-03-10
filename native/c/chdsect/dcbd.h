#ifdef __open_xl__
#pragma pack(1)
#else
#pragma pack(packed)
#endif
 
#ifndef __ihadcb__
#define __ihadcb__
 
struct ihadcb {
  union {
    struct {
      struct {
        unsigned char  _filler1;       /* RESERVED                                */
        unsigned int   _dcbwtoia : 24; /* A BINARY IDENTIFICATION NUMBER ASSIGNED */
        } dcbwtoid;
      struct {
        unsigned char  _filler2;       /* RESERVED                                */
        unsigned int   _dcberrca : 24; /* ADDRESS OF 32 BYTES OF DECLARED STORAGE */
        } dcberrcn;
      unsigned char  _filler3[8];
      } _ihadcb_struct1;
    struct {
      void * __ptr32 _dcbdcbe;      /* DCBE ADDRESS ASSOCIATED WITH THIS    @L5A */
      unsigned char  _filler4[12];
      } _ihadcb_struct2;
    struct {
      unsigned char  _dcbrelad[4]; /* --- PARTITIONED ORGANIZATION DATA SET -   */
      char           _dcbkeycn;    /* KEYED BLOCK OVERHEAD CONSTANT             */
      unsigned char  _dcbfdad[8];  /* DISK ADDRESS OF THE BLOCK THAT WAS   @13C */
      unsigned char  _filler5[3];
      } _ihadcb_struct3;
    struct {
      void * __ptr32 _filler6;     /* DCBE ADDRESS ASSOCIATED WITH THIS    @L5A */
      unsigned char  _filler7[8];  /* RESERVED                             @L5C */
      int            _dcbblkct;    /* BLOCK COUNT FOR EACH VOLUME               */
      } _ihadcb_struct4;
    struct {
      unsigned char  _dcbssid[8];  /* BEFORE DCB IS OPENED - NAME OF USER'S */
      unsigned char  _filler8[8];
      } _ihadcb_struct5;
    struct {
      void * __ptr32 _dcbqsmex; /* ADDRESS OF QSAM EXIT ROUTINE */
      struct {
        struct {
          unsigned char  _filler9;       /* RESERVED                                 */
          unsigned int   _dcbssada : 24; /* ADDRESS OF USER'S STACKER SELECT ROUTINE */
          } dcbimg;
        } dcbssad;
      struct {
        unsigned char  _dcbmrfg;       /* BUFFER INDICATOR                  */
        unsigned int   _dcbimaga : 24; /* ADDRESS OF PARAMETER LIST USED TO */
        } dcbimage;
      struct {
        struct {
          unsigned char  _dcbmrind;      /* INDICATOR AND COUNTER BYTE               */
          unsigned int   _dcbecbla : 24; /* ADDRESS OF ECB LIST PASSED TO WAIT MACRO */
          } dcbhdr;
        } dcbecblt;
      } _ihadcb_struct6;
    struct {
      unsigned char  _filler10[4];
      unsigned char  _dcblnnum;     /* 3886  DOCUMENT LINE NUMBER */
      unsigned char  _dcblfmat;     /* 3886  LINE FORMAT NUMBER   */
      unsigned char  _dcborflg;     /* 3886  FLAGS                */
      unsigned char  _filler11;     /* RESERVED                   */
      struct {
        struct {
          unsigned char  _filler12;      /* RESERVED                                 */
          unsigned int   _dcbdspla : 24; /* ADDRESS OF DSPLY (BSAM) ROUTINE USED FOR */
          } dcbfrid;
        } dcbdsply;
      struct {
        struct {
          struct {
            unsigned char  _filler13;      /* RESERVED                                */
            unsigned int   _dcbrdlna : 24; /* ADDRESS OF RDLNE (QSAM) ROUTINE USED TO */
            } dcbfrtba;
          } dcbrdlne;
        } dcbrescn;
      } _ihadcb_struct7;
    struct {
      unsigned char  _filler14[8];
      void * __ptr32 _dcblctbl;     /* ADDRESS OF TRANSLATE TABLE */
      unsigned char  _filler15[4];  /* RESERVED                   */
      } _ihadcb_struct8;
    struct {
      unsigned char  _filler16[12];
      struct {
        unsigned char  _filler17;      /* LAST BYTE OF DCBFDAD           */
        unsigned int   _dcbdvtba : 24; /* ADDRESS OF ENTRY IN I/O DEVICE */
        } dcbdvtbl;
      } _ihadcb_struct9;
    } _ihadcb_union1;
  union {
    struct {
      struct {
        unsigned char  _dcbstack; /* STACKER SELECTION (BITS 4-7) */
        } dcbmode;
      unsigned char  _filler18;  /* DCBDEVT - DEVICE TYPE           */
      unsigned char  _filler19;  /* RESERVED                        */
      unsigned char  _dcbfunc;   /* FUNCTION INDICATOR FOR THE 3525 */
      } _ihadcb_struct10;
    struct {
      char           _dcbkeyle;     /* KEY LENGTH OF DATA SET */
      unsigned char  _dcbdevt;      /* DEVICE TYPE            */
      unsigned char  _filler20[2];
      } dcbrelb;
    struct {
      char           _filler21;  /* DCBKEYLE - KEY LENGTH OF DATA SET     */
      unsigned char  _filler22;  /* DCBDEVT - DEVICE TYPE                 */
      short int      _dcbtrbal;  /* CODE INDICATING CAPACITY REMAINING ON */
      } _ihadcb_struct11;
    struct {
      unsigned char  _dcbtrtch;  /* TAPE RECORDING TECHNIQUE FOR 7-TRACK TAPE */
      unsigned char  _filler23;  /* DCBDEVT - DEVICE TYPE                     */
      unsigned char  _dcbden;    /* TAPE DENSITY - 2400 SERIES MAGNETIC TAPE  */
      unsigned char  _filler24;  /* RESERVED                                  */
      } _ihadcb_struct12;
    struct {
      unsigned char  _dcbcode;   /* PAPER TAPE CODE BEING USED.  THE */
      unsigned char  _filler25;  /* DCBDEVT - DEVICE TYPE            */
      unsigned char  _filler26;  /* RESERVED                         */
      unsigned char  _dcbptflg;  /* PAPER TAPE FLAGS                 */
      } _ihadcb_struct13;
    struct {
      unsigned char  _dcbprtsp;  /* NUMBER INDICATING NORMAL PRINTER SPACING   */
      unsigned char  _filler27;  /* DCBDEVT - DEVICE TYPE                      */
      unsigned char  _dcbprtov;  /* TEST-FOR-PRINTER-OVERFLOW MASK             */
      unsigned char  _dcbprbyt;  /* PRINTER BYTE                      @Z40MSRZ */
      } _ihadcb_struct14;
    struct {
      unsigned char  _dcborbyt;  /* OPTICAL READER BYTE USED BY BSAM/QSAM */
      unsigned char  _filler28;  /* DCBDEVT - DEVICE TYPE                 */
      unsigned char  _dcbeib;    /* ERROR INDICATOR BYTE                  */
      unsigned char  _filler29;  /* RESERVED                              */
      } _ihadcb_struct15;
    struct {
      unsigned char  _dcbmrflg;  /* FLAG BYTE                              */
      unsigned char  _filler30;  /* DCBDEVT - DEVICE TYPE                  */
      unsigned char  _dcbappin;  /* AN INDICATOR USED BY THE APPENDAGES TO */
      unsigned char  _filler31;  /* RESERVED                               */
      } _ihadcb_struct16;
    struct {
      unsigned char  _filler32;
      int            _dcbrel : 24;  /* NUMBER OF RELATIVE TRACKS OR BLOCKS IN */
      } _ihadcb_struct17;
    } _ihadcb_union2;
  union {
    struct {
      char           _dcbbufno;      /* NUMBER OF BUFFERS REQUIRED FOR THIS DATA */
      unsigned int   _dcbbufca : 24; /* ADDRESS OF BUFFER POOL CONTROL BLOCK     */
      } dcbbufcb;
    struct {
      unsigned char  _filler33;     /* RESERVED */
      unsigned char  _filler34[3];
      } _ihadcb_struct18;
    } _ihadcb_union3;
  short int      dcbbufl;      /* LENGTH OF BUFFER.  MAY RANGE FROM 0 TO    */
  struct {
    unsigned char  _dcbdsrg1; /* FIRST BYTE OF DCBDSORG  */
    unsigned char  _dcbdsrg2; /* SECOND BYTE OF DCBDSORG */
    } dcbdsorg;
  union {
    struct {
      struct {
        struct {
          struct {
            unsigned char  _dcbqslm; /* QSAM LOCATE MODE LOGICAL RECORD INTERFACE */
            } dcblnp;
          unsigned int   _dcbodeba : 24; /* ADDRESS OF OLD DEB */
          } dcbodeb;
        } dcbicqe;
      } dcbiobad;
    struct {
      unsigned char  _filler35;      /* RESERVED                    */
      unsigned int   _dcbsvcxa : 24; /* POINTER TO EXIT LIST OF JES */
      } dcbsvcxl;
    } _ihadcb_union4;
  struct {
    struct {
      struct {
        unsigned char  _dcbbfaln; /* BUFFER ALIGNMENT BITS */
        } dcbbftek;
      } dcbhiarc;
    unsigned int   _dcbeoda : 24; /* ADDRESS OF A USER-PROVIDED ROUTINE TO */
    } dcbeodad;
  struct {
    unsigned char  _dcbrecfm;      /* RECORD FORMAT                          */
    unsigned int   _dcbexlsa : 24; /* ADDRESS OF USER-PROVIDED LIST OF EXITS */
    } dcbexlst;
  union {
    unsigned char  _dcbddnam[8]; /* NAME ON THE DD STATEMENT WHICH DEFINES */
    struct {
      unsigned short _dcbtiot; /* OFFSET FROM TIOT ORIGIN TO TIOELNGH FIELD */
      struct {
        unsigned char  _dcbmacf1; /* FIRST BYTE OF DCBMACRF  */
        unsigned char  _dcbmacf2; /* SECOND BYTE OF DCBMACRF */
        } dcbmacrf;
      struct {
        unsigned char  _dcbiflgs;     /* SAME AS DCBIFLG BEFORE OPEN */
        unsigned int   _dcbdeba : 24; /* ADDRESS OF ASSOCIATED DEB   */
        } dcbdebad;
      } _ihadcb_struct19;
    } _ihadcb_union5;
  union {
    struct {
      struct {
        unsigned char  _dcboflg;       /* SAME AS DCBOFLGS BEFORE OPEN     @ZA11086 */
        unsigned int   _dcbwrita : 24; /* ADDRESS OF WRITE MODULE          @ZA11086 */
        } dcbwrite;
      } dcbread;
    struct {
      struct {
        unsigned char  _dcboflg1;     /* SAME AS DCBOFLGS BEFORE OPEN     @ZA14562 */
        unsigned int   _dcbputa : 24; /* ADDRESS OF PUT MODULE            @ZA11086 */
        } dcbput;
      } dcbget;
    struct {
      unsigned char  _dcboflgs; /* FLAGS USED BY OPEN ROUTINE               */
      unsigned char  _dcbiflg;  /* FLAGS USED BY IOS IN COMMUNICATING ERROR */
      struct {
        unsigned char  _dcbmacr1; /* FIRST BYTE OF DCBMACR  */
        unsigned char  _dcbmacr2; /* SECOND BYTE OF DCBMACR */
        } dcbmacr;
      } _ihadcb_struct20;
    } _ihadcb_union6;
  struct {
    struct {
      struct {
        unsigned char  _dcboptcd;      /* OPTION CODES            */
        unsigned int   _dcbchcka : 24; /* ADDRESS OF CHECK MODULE */
        } dcbcheck;
      } dcbperr;
    } dcbgerr;
  struct {
    char           _dcbiobl;      /* IOB LENGTH IN DOUBLE WORDS             */
    unsigned int   _dcbsyna : 24; /* ADDRESS OF USER-PROVIDED SYNAD ROUTINE */
    } dcbsynad;
  struct {
    unsigned char  _dcbcind1; /* CONDITION INDICATORS */
    } dcbflag1;
  unsigned char  dcbcind2;     /* CONDITION INDICATORS                      */
  short int      dcbblksi;     /* MAXIMUM BLOCK SIZE                        */
  unsigned char  dcbwcpo;      /* OFFSET OF WRITE CHANNEL PROGRAM FROM THE  */
  char           dcbwcpl;      /* LENGTH OF WRITE CHANNEL PROGRAM           */
  unsigned char  dcboffsr;     /* OFFSET OF READ CCW FROM BSAM/BPAM PREFIX  */
  unsigned char  dcboffsw;     /* OFFSET OF WRITE CCW FROM BSAM/BPAM PREFIX */
  union {
    struct {
      unsigned char  _filler36;      /* INTERNAL ACCESS METHOD USE */
      unsigned int   _dcbcicba : 24; /* POINTER TO JES C.I.        */
      } dcbcicb;
    void * __ptr32 _dcbioba; /* FOR NORMAL SCHEDULING, ADDRESS OF QSAM OR */
    struct {
      unsigned char  _filler37;     /* @L4A                                      */
      unsigned int   _dcbiobb : 24; /* SAME AS DCBIOBA ABOVE                @L4A */
      } _ihadcb_struct21;
    } _ihadcb_union7;
  union {
    struct {
      char           _dcbncp;        /* NUMBER OF CHANNEL PROGRAMS.             */
      unsigned int   _dcbeobra : 24; /* ADDRESS OF END-OF-BLOCK MODULE FOR READ */
      } dcbeobr;
    struct {
      void * __ptr32 _dcbeobad; /* FOR SIMPLE BUFFERING, ADDRESS OF LAST */
      } dcblccw;
    } _ihadcb_union8;
  union {
    struct {
      struct {
        unsigned char  _filler38;     /* -          DCBRECBT DELETED                     @L7C */
        unsigned int   _dcbreca : 24; /* ADDRESS OF CURRENT OR NEXT LOGICAL RECORD            */
        } dcbrecad;
      } dcbcccw;
    void * __ptr32 _dcbeobw; /* ADDRESS OF END-OF-BLOCK MODULE FOR WRITE. */
    } _ihadcb_union9;
  union {
    struct {
      struct {
        unsigned char  _dcbusasi; /* FLAG BYTE FOR ASCII TAPES */
        } dcbqsws;
      struct {
        char           _dcbdircq; /* NUMBER OF BYTES USED IN LAST DIRECTORY */
        } dcbbufof;
      } dcbdirct;
    short int      _filler39;  /* DCBDIRCT - NUMBER OF BYTES USED IN LAST */
    struct {
      unsigned char  _filler40;  /* DCBQSWS - FLAG BYTE                     */
      char           _filler41;  /* DCBDIRCQ - NUMBER OF BYTES USED IN LAST */
      } _ihadcb_struct22;
    } _ihadcb_union10;
  union {
    short int      _dcblrecl;  /* LOGICAL RECORD LENGTH            */
    short int      _filler42;  /* DCBLRECL - LOGICAL RECORD LENGTH */
    } _ihadcb_union11;
  union {
    struct {
      struct {
        void * __ptr32 _dcbpoint; /* ADDRESS OF NOTE/POINT MODULE */
        } dcbnote;
      } dcbcntrl;
    struct {
      unsigned char  _dcberopt;       /* ERROR OPTION                              */
      unsigned int   _filler43 : 24;  /* DCBCNTRA  - ADDRESS OF CNTRL MODULE  @L2A */
      } _ihadcb_struct23;
    } _ihadcb_union12;
  unsigned char  _filler44[2]; /* RESERVED                                  */
  short int      dcbprecl;     /* FORMAT F RECORDS: BLOCK LENGTH            */
  void * __ptr32 dcbeob;       /* ADDRESS OF END OF BLOCK MODULE            */
  };
 
#define dcbwtoia  _ihadcb_union1._ihadcb_struct1.dcbwtoid._dcbwtoia
#define dcberrca  _ihadcb_union1._ihadcb_struct1.dcberrcn._dcberrca
#define dcbdcbe   _ihadcb_union1._ihadcb_struct2._dcbdcbe
#define dcbrelad  _ihadcb_union1._ihadcb_struct3._dcbrelad
#define dcbkeycn  _ihadcb_union1._ihadcb_struct3._dcbkeycn
#define dcbfdad   _ihadcb_union1._ihadcb_struct3._dcbfdad
#define dcbblkct  _ihadcb_union1._ihadcb_struct4._dcbblkct
#define dcbssid   _ihadcb_union1._ihadcb_struct5._dcbssid
#define dcbqsmex  _ihadcb_union1._ihadcb_struct6._dcbqsmex
#define dcbssada  _ihadcb_union1._ihadcb_struct6.dcbssad.dcbimg._dcbssada
#define dcbmrfg   _ihadcb_union1._ihadcb_struct6.dcbimage._dcbmrfg
#define dcbimaga  _ihadcb_union1._ihadcb_struct6.dcbimage._dcbimaga
#define dcbmrind  _ihadcb_union1._ihadcb_struct6.dcbecblt.dcbhdr._dcbmrind
#define dcbecbla  _ihadcb_union1._ihadcb_struct6.dcbecblt.dcbhdr._dcbecbla
#define dcblnnum  _ihadcb_union1._ihadcb_struct7._dcblnnum
#define dcblfmat  _ihadcb_union1._ihadcb_struct7._dcblfmat
#define dcborflg  _ihadcb_union1._ihadcb_struct7._dcborflg
#define dcbdspla  _ihadcb_union1._ihadcb_struct7.dcbdsply.dcbfrid._dcbdspla
#define dcbrdlna  _ihadcb_union1._ihadcb_struct7.dcbrescn.dcbrdlne.dcbfrtba._dcbrdlna
#define dcblctbl  _ihadcb_union1._ihadcb_struct8._dcblctbl
#define dcbdvtba  _ihadcb_union1._ihadcb_struct9.dcbdvtbl._dcbdvtba
#define dcbstack  _ihadcb_union2._ihadcb_struct10.dcbmode._dcbstack
#define dcbfunc   _ihadcb_union2._ihadcb_struct10._dcbfunc
#define dcbkeyle  _ihadcb_union2.dcbrelb._dcbkeyle
#define dcbdevt   _ihadcb_union2.dcbrelb._dcbdevt
#define dcbtrbal  _ihadcb_union2._ihadcb_struct11._dcbtrbal
#define dcbtrtch  _ihadcb_union2._ihadcb_struct12._dcbtrtch
#define dcbden    _ihadcb_union2._ihadcb_struct12._dcbden
#define dcbcode   _ihadcb_union2._ihadcb_struct13._dcbcode
#define dcbptflg  _ihadcb_union2._ihadcb_struct13._dcbptflg
#define dcbprtsp  _ihadcb_union2._ihadcb_struct14._dcbprtsp
#define dcbprtov  _ihadcb_union2._ihadcb_struct14._dcbprtov
#define dcbprbyt  _ihadcb_union2._ihadcb_struct14._dcbprbyt
#define dcborbyt  _ihadcb_union2._ihadcb_struct15._dcborbyt
#define dcbeib    _ihadcb_union2._ihadcb_struct15._dcbeib
#define dcbmrflg  _ihadcb_union2._ihadcb_struct16._dcbmrflg
#define dcbappin  _ihadcb_union2._ihadcb_struct16._dcbappin
#define dcbrel    _ihadcb_union2._ihadcb_struct17._dcbrel
#define dcbbufno  _ihadcb_union3.dcbbufcb._dcbbufno
#define dcbbufca  _ihadcb_union3.dcbbufcb._dcbbufca
#define dcbdsrg1  dcbdsorg._dcbdsrg1
#define dcbdsrg2  dcbdsorg._dcbdsrg2
#define dcbqslm   _ihadcb_union4.dcbiobad.dcbicqe.dcbodeb.dcblnp._dcbqslm
#define dcbodeba  _ihadcb_union4.dcbiobad.dcbicqe.dcbodeb._dcbodeba
#define dcbsvcxa  _ihadcb_union4.dcbsvcxl._dcbsvcxa
#define dcbbfaln  dcbeodad.dcbhiarc.dcbbftek._dcbbfaln
#define dcbeoda   dcbeodad._dcbeoda
#define dcbrecfm  dcbexlst._dcbrecfm
#define dcbexlsa  dcbexlst._dcbexlsa
#define dcbddnam  _ihadcb_union5._dcbddnam
#define dcbtiot   _ihadcb_union5._ihadcb_struct19._dcbtiot
#define dcbmacf1  _ihadcb_union5._ihadcb_struct19.dcbmacrf._dcbmacf1
#define dcbmacf2  _ihadcb_union5._ihadcb_struct19.dcbmacrf._dcbmacf2
#define dcbiflgs  _ihadcb_union5._ihadcb_struct19.dcbdebad._dcbiflgs
#define dcbdeba   _ihadcb_union5._ihadcb_struct19.dcbdebad._dcbdeba
#define dcboflg   _ihadcb_union6.dcbread.dcbwrite._dcboflg
#define dcbwrita  _ihadcb_union6.dcbread.dcbwrite._dcbwrita
#define dcboflg1  _ihadcb_union6.dcbget.dcbput._dcboflg1
#define dcbputa   _ihadcb_union6.dcbget.dcbput._dcbputa
#define dcboflgs  _ihadcb_union6._ihadcb_struct20._dcboflgs
#define dcbiflg   _ihadcb_union6._ihadcb_struct20._dcbiflg
#define dcbmacr1  _ihadcb_union6._ihadcb_struct20.dcbmacr._dcbmacr1
#define dcbmacr2  _ihadcb_union6._ihadcb_struct20.dcbmacr._dcbmacr2
#define dcboptcd  dcbgerr.dcbperr.dcbcheck._dcboptcd
#define dcbchcka  dcbgerr.dcbperr.dcbcheck._dcbchcka
#define dcbiobl   dcbsynad._dcbiobl
#define dcbsyna   dcbsynad._dcbsyna
#define dcbcind1  dcbflag1._dcbcind1
#define dcbcicba  _ihadcb_union7.dcbcicb._dcbcicba
#define dcbioba   _ihadcb_union7._dcbioba
#define dcbiobb   _ihadcb_union7._ihadcb_struct21._dcbiobb
#define dcbncp    _ihadcb_union8.dcbeobr._dcbncp
#define dcbeobra  _ihadcb_union8.dcbeobr._dcbeobra
#define dcbeobad  _ihadcb_union8.dcblccw._dcbeobad
#define dcbreca   _ihadcb_union9.dcbcccw.dcbrecad._dcbreca
#define dcbeobw   _ihadcb_union9._dcbeobw
#define dcbusasi  _ihadcb_union10.dcbdirct.dcbqsws._dcbusasi
#define dcbdircq  _ihadcb_union10.dcbdirct.dcbbufof._dcbdircq
#define dcblrecl  _ihadcb_union11._dcblrecl
#define dcbpoint  _ihadcb_union12.dcbcntrl.dcbnote._dcbpoint
#define dcberopt  _ihadcb_union12._ihadcb_struct23._dcberopt
 
/* Values for field "dcbmrfg" */
#define dcbmrbct 0xC0 /* TWO-BIT BINARY COUNTER WHICH INDICATES             */
 
/* Values for field "dcbmrind" */
#define dcbmrdct 0xE0 /* THREE-BIT BINARY COUNTER OF                        */
#define dcbmrscu 0x10 /* DCB WAS ALTERED WHEN SYNAD ROUTINE WAS             */
#define dcbmrplo 0x08 /* POCKET LIGHT HAS BEEN TURNED ON                    */
#define dcbmrpls 0x04 /* POCKET LIGHT 0-6 IS BEING SET ON                   */
#define dcbmrerp 0x02 /* ERROR RECOVERY PROCEDURE IS EXECUTING FOR          */
#define dcbmrers 0x01 /* ERROR RECOVERY PROCEDURE IS EXECUTING FOR          */
 
/* Values for field "dcborflg" */
#define dcbeopfg 0x80 /* END OF PAGE                                        */
 
/* Values for field "dcbstack" */
#define dcbmodec 0x80 /* COLUMN BINARY MODE                                 */
#define dcbmodee 0x40 /* EBCDIC MODE                                        */
#define dcbmodeo 0x20 /* OPTICAL MARK READ MODE                             */
#define dcbmoder 0x10 /* READ COLUMN ELIMINATE MODE                         */
#define dcbstck2 0x02 /* STACKER 2                                          */
#define dcbstck1 0x01 /* STACKER 1                                          */
 
/* Values for field "_filler18" */
#define dcbdvcr0 0x41 /* 2540 CARD READER                                   */
#define dcbdvcp0 0x42 /* 2540 CARD PUNCH                                    */
#define dcbdvcrp 0x43 /* 1442 CARD READ PUNCH                               */
#define dcbdvcr1 0x44 /* 2501 CARD READER                                   */
#define dcbdvcpr 0x45 /* 2520 CARD READ PUNCH                               */
#define dcbdvcr2 0x46 /* 3505 CARD READER                                   */
#define dcbdvcp1 0x4C /* 3525 CARD PUNCH                                    */
 
/* Values for field "dcbfunc" */
#define dcbfncbi 0x80 /* INTERPRET (PUNCH AND PRINT TWO LINES)              */
#define dcbfncbr 0x40 /* READ                                               */
#define dcbfncbp 0x20 /* PUNCH                                              */
#define dcbfncbw 0x10 /* PRINT                                              */
#define dcbfncbd 0x08 /* DATA PROTECTION                                    */
#define dcbfncbx 0x04 /* THIS DATA SET IS TO BE PRINTED                     */
#define dcbfncbt 0x02 /* TWO-LINE PRINT SUPPORT REQUEST                     */
 
/* Values for field "dcbdevt" */
#define dcbdvtrm 0x4F /* TERMINAL.  (DD CONTAINS TERM=TS)                   */
 
/* Values for field "_filler22" */
#define dcbdv311 0x21 /* 2311 DISK STORAGE                @ZA46311          */
#define dcbdv301 0x22 /* 2301 PARALLEL DRUM                                 */
#define dcbdv303 0x23 /* 2303 SERIAL DRUM                                   */
#define dcbdv345 0x24 /* 9345 DISK STORAGE FACILITY(NO LONGER @11C          */
#define dcbdv321 0x25 /* 2321 DATA CELL STORAGE           @ZA46311          */
#define dcbd1305 0x26 /* 2305 DRUM MODEL-1                @ZA46311          */
#define dcbdv305 0x27 /* 2305 DRUM MODEL-2                @ZA46311          */
#define dcbdv314 0x28 /* 2314/2319 DISK STORAGE FACILITY  @ZA46311          */
#define dcbdv330 0x29 /* 3330 DISK STORAGE FACILITY       @ZA46311          */
#define dcbdv340 0x2A /* 3340/3344 DISK STORAGE FACILITY  @ZA46311          */
#define dcbdv350 0x2B /* 3350 DISK STORAGE FACILITY       @ZA46311          */
#define dcbdv375 0x2C /* 3375 DISK STORAGE FACILITY           @01A          */
#define dcbdv331 0x2D /* 3330 MODEL-11 OR 3333 MODEL-11   @ZA46311          */
#define dcbdv380 0x2E /* 3380 DISK STORAGE FACILITY           @01A          */
#define dcbdv390 0x2F /* 3390 DISK STORAGE FACILITY           @08A          */
 
/* Values for field "dcbtrtch" */
#define dcbmte   0x23 /* E  - EVEN PARITY                                   */
#define dcbmtt   0x3B /* T  - BCD/EBCDIC TRANSLATION                        */
#define dcbmtc   0x13 /* C  - DATA CONVERSION                               */
#define dcbmtet  0x2B /* ET - EVEN PARITY AND TRANSLATION                   */
#define dcbcmpac 0x08 /* COMPACTION                     @H2A                */
#define dcbnopac 0x04 /* NO COMPACTION                  @H2A                */
 
/* Values for field "_filler23" */
#define dcbdvmt  0x81 /* 3490 MAGNETIC TAPE UNIT (NO LONGER   @10C          */
#define dcbdvmt2 0x82 /* -        3423 DEVICE                          @L6A */
#define dcbdvmt3 0x83 /* 3400 SERIES MAGNETIC TAPE UNIT                     */
#define dcbdvmt4 0x80 /* 3480 MAGNETIC TAPE UNIT              @03A          */
#define dcbdvmt5 0x83 /* 3590 MAGNETIC TAPE UNIT              @P1A          */
 
/* Values for field "dcbden" */
#define dcbmtdn0 0x03 /* 0       200 BPI       -                            */
#define dcbmtdn1 0x43 /* 1       556 BPI       -                            */
#define dcbmtdn2 0x83 /* 2       800 BPI     800 BPI                        */
#define dcbmtdn3 0xC3 /* 3         -        1600 BPI                        */
#define dcbmtdn4 0xD3 /* 4         -        6250 BPI                        */
 
/* Values for field "dcbcode" */
#define dcbptcdn 0x80 /* N - NO CONVERSION                                  */
#define dcbptcdi 0x40 /* I - IBM BCD                                        */
#define dcbptcdf 0x20 /* F - FRIDEN                                         */
#define dcbptcdb 0x10 /* B - BURROUGHS                                      */
#define dcbptcdc 0x08 /* C - NATIONAL CASH REGISTER                         */
#define dcbptcda 0x04 /* A - ASCII (8-TRACK)                                */
#define dcbptcdt 0x02 /* T - TELETYPE                                       */
 
/* Values for field "_filler25" */
#define dcbdvptp 0x50 /* 2671 PAPER TAPE READER                             */
 
/* Values for field "dcbptflg" */
#define dcbptic  0x10 /* INVALID CHARACTER IN LAST RECORD READ              */
#define dcbptect 0x08 /* END OF RECORD CHARACTER REACHED IN                 */
#define dcbptecr 0x04 /* END OF RECORD CHARACTER DETECTED DURING            */
#define dcbptuct 0x02 /* IF ONE, UPPER CASE TRANSLATE.                      */
#define dcbpterr 0x01 /* ERROR DETECTED ON READ                             */
 
/* Values for field "dcbprtsp" */
#define dcbprsp0 0x01 /* 0 - NO SPACING                                     */
#define dcbprsp1 0x09 /* 1 - SPACE ONE LINE                                 */
#define dcbprsp2 0x11 /* 2 - SPACE TWO LINES                                */
#define dcbprsp3 0x19 /* 3 - SPACE THREE LINES                              */
 
/* Values for field "_filler27" */
#define dcbdvpr1 0x48 /* 1403 PRINTER AND 1404 PRINTER (CONTINUOUS          */
#define dcbdvpr2 0x4A /* 1443 PRINTER                                       */
#define dcbdvpr3 0x49 /* 3211 PRINTER                                       */
#define dcbdvpr4 0x4B /* 3203-4 PRINTER                   @32034AP          */
#define dcbdvprt 0x4D /* ANY OTHER PRINTER DEVICE TYPE.   @42450LP          */
#define dcbdvpr5 0x4E /* 3800 PRINTER                     @Z40MSRZ          */
 
/* Values for field "dcbprtov" */
#define dcbprc9  0x20 /* 9  - TEST FOR CHANNEL 9 OVERFLOW                   */
#define dcbprc12 0x10 /* 12 - TEST FOR CHANNEL 12 OVERFLOW                  */
 
/* Values for field "dcbprbyt" */
#define dcbtrcid 0x03 /* 2-BIT ID OF 3800 TRANSLATE    @Z40MSRZ             */
 
/* Values for field "dcborbyt" */
#define dcborsyn 0x80 /* SYNAD IN CONTROL                                   */
#define dcboreof 0x40 /* END OF FILE (EOF)                                  */
#define dcborbfp 0x20 /* BUFFERS PRIMED (QSAM)                              */
 
/* Values for field "_filler28" */
#define dcbdvor5 0x5A /* 1285 OPTICAL READER                                */
#define dcbdvor7 0x5B /* 1287 OPTICAL READER                                */
#define dcbdvor8 0x5C /* 1288 OPTICAL READER                                */
#define dcbdvor9 0x57 /* 3886 OPTICAL READER                                */
 
/* Values for field "dcbeib" */
#define dcbornrm 0x40 /* THE 1287 OR 1288 SCANNER WAS UNABLE TO             */
#define dcborrej 0x20 /* FOR 1287, A STACKER SELECT COMMAND WAS             */
#define dcborerr 0x10 /* A NONRECOVERABLE ERROR HAS OCCURRED.               */
#define dcboreck 0x08 /* AN EQUIPMENT CHECK RESULTED IN AN                  */
#define dcborwlr 0x04 /* A WRONG-LENGTH RECORD CONDITION HAS                */
#define dcborhpr 0x02 /* FOR QSAM - OPERATOR ENTERED ONE OR MORE            */
#define dcbordck 0x01 /* A DATA CHECK HAS OCCURRED                          */
 
/* Values for field "dcbmrflg" */
#define dcbmrscc 0x80 /* FIRST OR SECOND SECONDARY CONTROL UNIT             */
#define dcbmrdbg 0x40 /* DEBUGGING MODE IN USE                              */
#define dcbmrdru 0x20 /* DISENGAGE REQUESTED BY USER                        */
#define dcbmrdr  0x10 /* DISENGAGE REQUESTED                                */
#define dcbmrpcc 0x0C /* TWO-BIT BINARY COUNTER INDICATING                  */
#define dcbmrdwt 0x02 /* WTO MESSAGE MUST BE DELETED                        */
#define dcbmrue  0x01 /* UNIT EXCEPTION                                     */
 
/* Values for field "_filler30" */
#define dcbdvmr  0x5D /* 1419 MAGNETIC CHARACTER READER                     */
#define dcbdvors 0x5F /* 1275 OPTICAL READER SORTER                         */
#define dcbdvmrs 0x56 /* 3890 MAGNETIC CHARACTER READER SORTER              */
#define dcbdvdri 0x59 /* 3895 DOCUMENT READER/INSCRIBER   @G30HSJK          */
 
/* Values for field "dcbdsrg1" */
#define dcbdsgis 0x80 /* IS - INDEXED SEQUENTIAL ORGANIZATION               */
#define dcbdsgps 0x40 /* PS - PHYSICAL SEQUENTIAL ORGANIZATION              */
#define dcbdsgda 0x20 /* DA - DIRECT ORGANIZATION                           */
#define dcbdsgcx 0x10 /* CX - BTAM OR QTAM LINE GROUP                       */
#define dcbdsgpo 0x02 /* PO - PARTITIONED ORGANIZATION                      */
#define dcbdsgu  0x01 /* U  - UNMOVABLE, THE DATA CONTAINS                  */
 
/* Values for field "dcbdsrg2" */
#define dcbdsggs 0x80 /* GS - GRAPHICS ORGANIZATION                         */
#define dcbdsgtx 0x40 /* TX - TCAM LINE GROUP                               */
#define dcbdsgtq 0x20 /* TQ - TCAM MESSAGE QUEUE                            */
#define dcbacbm  0x08 /* ALWAYS 0 IN DCB. ALWAYS 1 IN ACB     @L5C          */
#define dcbdsgtr 0x04 /* TR - TCAM 3705                                     */
 
/* Values for field "dcbqslm" */
#define dcb1dvds 0x80 /* ONLY ONE DEVICE IS ALLOCATED TO THIS               */
#define dcbupdcm 0x40 /* UPDATE COMPLETE, FREE OLD DEB                      */
#define dcbupdbt 0x30 /* UPDATE BITS                                        */
#define dcbupdt  0x20 /* UPDATE TO TAKE PLACE                               */
#define dcbnupd  0x30 /* NO UPDATE TO TAKE PLACE                            */
#define dcbsvdeb 0x10 /* OLD DEB ADDRESS MUST BE SAVED                      */
 
/* Values for field "dcbbfaln" */
#define dcbh1    0x80 /* HIERARCHY 1 MAIN STORAGE IF BIT5 IS ZERO.          */
#define dcbbft   0x70 /* BUFFERING TECHNIQUE                                */
#define dcbbfta  0x60 /* QSAM LOCATE MODE PROCESSING OF SPANNED             */
#define dcbbftr  0x20 /* FOR BSAM CREATE BDAM PROCESSING OF                 */
#define dcbbfts  0x40 /* SIMPLE BUFFERING - BIT 3 IS ZERO                   */
#define dcbbftkr 0x20 /* UNBLOCKED SPANNED RECORDS - SOFTWARE               */
#define dcbbfte  0x10 /* EXCHANGE BUFFERING - BIT 1 IS ZERO                 */
#define dcbbftkd 0x08 /* DYNAMIC BUFFERING (BTAM)                           */
#define dcbbftk  0x08 /* LRECL  IN 'K' UNITS FOR XLRI         @L2A          */
#define dcbbxlri 0x68 /* EXTENDED LOGICAL RECORD   @L2A                     */
#define dcbh0    0x04 /* HIERARCHY 0 MAIN STORAGE IF BIT0 IS ZERO           */
#define dcbbfa   0x03 /* BUFFER ALIGNMENT                                   */
#define dcbbfad  0x02 /* DOUBLEWORD BOUNDARY                                */
#define dcbbfaf1 0x01 /* FULLWORD NOT A DOUBLEWORD BOUNDARY,                */
#define dcbbfaf2 0x03 /* FULLWORD NOT A DOUBLEWORD BOUNDARY,                */
 
/* Values for field "dcbrecfm" */
#define dcbrecla 0xE0 /* RECORD LENGTH INDICATOR - ASCII                    */
#define dcbrecd  0x20 /* ASCII VARIABLE RECORD LENGTH                       */
#define dcbrecl  0xC0 /* RECORD LENGTH INDICATOR                            */
#define dcbrecf  0x80 /* FIXED RECORD LENGTH                                */
#define dcbrecv  0x40 /* VARIABLE RECORD LENGTH                             */
#define dcbrecu  0xC0 /* UNDEFINED RECORD LENGTH                            */
#define dcbrecto 0x20 /* TRACK OVERFLOW                                     */
#define dcbrecbr 0x10 /* BLOCKED RECORDS                                    */
#define dcbrecsb 0x08 /* FOR FIXED LENGTH RECORD FORMAT - STANDARD          */
#define dcbreccc 0x06 /* CONTROL CHARACTER INDICATOR                        */
#define dcbrecca 0x04 /* ASA CONTROL CHARACTER                              */
#define dcbreccm 0x02 /* MACHINE CONTROL CHARACTER                          */
#define dcbrecc  0x00 /* NO CONTROL CHARACTER                               */
#define dcbreckl 0x01 /* KEY LENGTH (KEYLEN) WAS SPECIFIED IN DCB           */
 
/* Values for field "dcbiflgs" */
#define dcbifec  0xC0 /* ERROR CORRECTION INDICATOR                         */
#define dcbifpct 0x30 /* PRINTER CARRIAGE TAPE PUNCH INDICATOR              */
#define dcbifioe 0x0C /* IOS ERROR ROUTINE USE INDICATOR                    */
#define dcbifldt 0x02 /* POSSIBLE LOST DATA CONDITION     @42480LP          */
 
/* Values for field "dcboflgs" */
#define dcboflwr 0x80 /* IF ZERO, LAST I/O OPERATION WAS READ OR            */
#define dcbofiod 0x80 /* DATA SET IS BEING OPENED FOR INPUT OR              */
#define dcboflrb 0x40 /* LAST I/O OPERATION WAS IN READ BACKWARD            */
#define dcbofeov 0x20 /* SET TO 1 BY EOV WHEN IT CALLS CLOSE                */
#define dcbofopn 0x10 /* AN OPEN HAS BEEN SUCCESSFULLY COMPLETED            */
#define dcbofppc 0x08 /* SET TO 1 BY PROBLEM PROGRAM TO INDICATE A          */
#define dcboftm  0x04 /* TAPE MARK HAS BEEN READ                            */
#define dcbofuex 0x02 /* SET TO 0 BY AN I/O SUPPORT FUNCTION WHEN           */
#define dcbolock 0x02 /* SAME USE AS DCBOFUEX                 @LAA          */
#define dcbofiof 0x01 /* SET TO 1 BY AN I/O SUPPORT FUNCTION IF             */
#define dcbobusy 0x01 /* SAME USE AS DCBOFIOF                 @LAA          */
 
/* Values for field "dcbiflg" */
#define dcbibec  0xC0 /* ERROR CORRECTION INDICATOR                         */
#define dcbifnep 0x00 /* NOT IN ERROR PROCEDURE                             */
#define dcbex    0x40 /* ERROR CORRECTION OR IOS PAGE FIX IN                */
#define dcbifpec 0xC0 /* PERMANENT ERROR CORRECTION                         */
#define dcbibpct 0x30 /* PRINTER CARRIAGE TAPE PUNCH INDICATOR              */
#define dcbifc9  0x20 /* CHANNEL 9 PRINTER CARRIAGE TAPE PUNCH              */
#define dcbifc12 0x10 /* CHANNEL 12 PRINTER CARRIAGE TAPE PUNCH             */
#define dcbibioe 0x0C /* IOS ERROR ROUTINE USE INDICATOR                    */
#define dcbifer  0x00 /* ALWAYS USE I/O SUPERVISOR ERROR ROUTINE            */
#define dcbifne1 0x04 /* NEVER USE I/O SUPERVISOR ERROR ROUTINE             */
#define dcbiftim 0x04 /* TEST IOS MASK (IMSK) FOR ERROR PROCEDURE           */
#define dcbifne2 0x08 /* NEVER USE I/O SUPERVISOR ERROR ROUTINE             */
#define dcbifne3 0x0C /* NEVER USE I/O SUPERVISOR ERROR ROUTINE             */
 
/* Values for field "dcbmacr1" */
#define dcbmrecp 0x80 /* EXECUTE CHANNEL PROGRAM (EXCP) ---                 */
#define dcbmrfe  0x40 /* FOUNDATION EXTENSION IS PRESENT (EXCP)             */
#define dcbmrget 0x40 /* GET (QSAM, QISAM, TCAM)                            */
#define dcbmrptq 0x40 /* PUT FOR MESSAGE GROUP (QTAM) ---                   */
#define dcbmrapg 0x20 /* APPENDAGES ARE REQUIRED (EXCP)                     */
#define dcbmrrd  0x20 /* READ (BSAM, BPAM, BISAM, BDAM, BTAM)               */
#define dcbmrwrq 0x20 /* WRITE FOR LINE GROUP (QTAM) ---                    */
#define dcbmrci  0x10 /* COMMON INTERFACE (EXCP)                            */
#define dcbmrmvg 0x10 /* MOVE MODE OF GET (QSAM, QISAM)                     */
#define dcbmrrdk 0x10 /* KEY SEGMENT WITH READ (BDAM) ---                   */
#define dcbmrlcg 0x08 /* LOCATE MODE OF GET (QSAM, QISAM)                   */
#define dcbmrrdi 0x08 /* ID ARGUMENT WITH READ (BDAM) ---                   */
#define dcbmrabc 0x04 /* USER'S PROGRAM MAINTAINS ACCURATE BLOCK            */
#define dcbmrpt1 0x04 /* POINT (WHICH IMPLIES NOTE) (BSAM, BPAM)            */
#define dcbmrsbg 0x04 /* SUBSTITUTE MODE OF GET (QSAM)                      */
#define dcbmrdbf 0x04 /* DYNAMIC BUFFERING (BISAM, BDAM) ---                */
#define dcbpgfxa 0x02 /* PAGE FIX APPENDAGE IS SPECIFIED (EXCP)             */
#define dcbmrcrl 0x02 /* CNTRL (BSAM, QSAM)                                 */
#define dcbmrchk 0x02 /* CHECK (BISAM)                                      */
#define dcbmrrdx 0x02 /* READ EXCLUSIVE (BDAM) ---                          */
#define dcbmrdmg 0x01 /* DATA MODE OF GET (QSAM)                            */
#define dcbmrck  0x01 /* CHECK (BDAM) --- RESERVED (EXCP, BSAM,             */
 
/* Values for field "dcbmacr2" */
#define dcbmrstl 0x80 /* SETL (QISAM) --- ALWAYS ZERO (BSAM, QSAM,          */
#define dcbmrput 0x40 /* PUT (QSAM, TCAM) - PUT OR PUTX (QISAM)             */
#define dcbmrgtq 0x40 /* GET FOR MESSAGE GROUP (QTAM) ---                   */
#define dcbmrwrt 0x20 /* WRITE (BSAM, BPAM, BISAM, BDAM, BTAM)              */
#define dcbmrrdq 0x20 /* READ FOR LINE GROUP (QTAM) ---                     */
#define dcbmrmvp 0x10 /* MOVE MODE OF PUT (QSAM, QISAM)                     */
#define dcbmrwrk 0x10 /* KEY SEGMENT WITH WRITE (BDAM) ---                  */
#define dcbmr5wd 0x08 /* FIVE-WORD DEVICE INTERFACE (EXCP)                  */
#define dcbmrldm 0x08 /* LOAD MODE BSAM (CREATE BDAM DATA SET)              */
#define dcbmrlcp 0x08 /* LOCATE MODE OF PUT (QSAM, QISAM)                   */
#define dcbmridw 0x08 /* ID ARGUMENT WITH WRITE (BDAM) ---                  */
#define dcbmr4wd 0x04 /* FOUR-WORD DEVICE INTERFACE (EXCP)                  */
#define dcbmrpt2 0x04 /* POINT (WHICH IMPLIES NOTE) (BSAM, BPAM)            */
#define dcbmrtmd 0x04 /* SUBSTITUTE MODE (QSAM)                             */
#define dcbmruip 0x04 /* UPDATE IN PLACE (PUTX) (QISAM) ---                 */
#define dcbmr3wd 0x02 /* THREE-WORD DEVICE INTERFACE (EXCP)                 */
#define dcbmrctl 0x02 /* CNTRL (BSAM, QSAM)                                 */
#define dcbmrstk 0x02 /* SETL BY KEY (QISAM)                                */
#define dcbmrawr 0x02 /* ADD TYPE OF WRITE (BDAM) ---                       */
#define dcbmr1wd 0x01 /* ONE-WORD DEVICE INTERFACE (EXCP)                   */
#define dcbmrswa 0x01 /* USER'S PROGRAM HAS PROVIDED A SEGMENT              */
#define dcbmrdmd 0x01 /* DATA MODE (QSAM)                                   */
#define dcbmrsti 0x01 /* SETL BY ID (QISAM) ---                             */
#define dcblngxe 0x34 /* LENGTH OF DCB FOR EXCP WITH          @L1A          */
 
/* Values for field "dcboptcd" */
#define dcboptw  0x80 /* WRITE VALIDITY CHECK (DASD)                        */
#define dcboptu  0x40 /* ALLOW DATA CHECK CAUSED BY INVALID                 */
#define dcboptc  0x20 /* CHAINED SCHEDULING                                 */
#define dcbopth  0x10 /* 1287/1288 OPTICAL READER - HOPPER EMPTY            */
#define dcbopto  0x10 /* 1285/1287 OPTICAL READER - ON-LINE                 */
#define dcbbckpt 0x10 /* CHANNEL-END APPENDAGE IS TO BYPASS DOS             */
#define dcboptq  0x08 /* TRANSLATION TO OR FROM ASCII                       */
#define dcboptz  0x04 /* MAGNETIC TAPE DEVICES - USE REDUCED ERROR          */
#define dcbsrchd 0x04 /* USE SEARCH DIRECT, INSTEAD OF SEARCH               */
#define dcboptt  0x02 /* USER TOTALING (BSAM, QSAM)                         */
#define dcboptj  0x01 /* 3800 PRINTER, OPTCD=J; (DYNAMIC  @Z40MSRZ          */
 
/* Values for field "dcbcind1" */
#define dcbcntov 0x80 /* DIRECT ACCESS - TRACK OVERFLOW IN USE              */
#define dcbstqck 0x80 /* STOP EQUAL QUICK WAS SPECIFIED FOR                 */
#define dcbstfls 0x40 /* STOP EQUAL FLUSH WAS SPECIFIED FOR                 */
#define dcbcnsrd 0x40 /* SEARCH DIRECT (BSAM, BPAM, QSAM)                   */
#define dcbcnevb 0x20 /* END OF VOLUME - USED BY EOB ROUTINES               */
#define dcbcneva 0x10 /* END OF VOLUME - USED BY CHANNEL-END                */
#define dcbcnci  0x08 /* SAM-SI COMPATABILITY INTERFACE (CI)                */
#define dcbcnbrm 0x04 /* BLOCKED RECORD BIT MODIFIED (BSAM,BPAM,            */
#define dcbcbndf 0x02 /* -      OPEN DEFAULTED BUFNO (QSAM)          @L7A   */
#define dcbcnexb 0x01 /* EXCHANGE BUFFERING SUPPORTED (QSAM)                */
#define dcbcdwdo 0x01 /* INTERNAL USE ONLY, DCBOFFSW IS       @LBA          */
 
/* Values for field "dcbcind2" */
#define dcbcnsto 0x80 /* PARTITIONED DATA SET - STOW HAS BEEN               */
#define dcbcnwr0 0x40 /* DIRECT ORGANIZATION DATA SET - LAST I/O            */
#define dcbcnclo 0x20 /* CLOSE IN PROCESS (QSAM)                            */
#define dcbcnioe 0x10 /* PERMANENT I/O ERROR (BSAM, BPAM, QSAM)             */
#define dcbcnbfp 0x08 /* OPEN ACQUIRED BUFFER POOL                          */
#define dcbcnchs 0x04 /* CHAINED SCHEDULING BEING SUPPORTED                 */
#define dcbcnfeo 0x02 /* FEOV BIT (BSAM, BPAM, QSAM)                        */
#define dcbcnqsm 0x01 /* ALWAYS ZERO (BSAM, BPAM)                           */
 
/* Values for field "dcbusasi" */
#define dcbblbp  0x40 /* BLOCK PREFIX IS FOUR BYTE FIELD                    */
#define dcbqadfs 0x38 /* USED TO PERFORM SEQUENCE                           */
#define dcbqadf1 0x20 /* FIRST BIT OF DCBQADFS                              */
#define dcbqadf2 0x10 /* SECOND BIT OF DCBQADFS                             */
#define dcbqadf3 0x08 /* THIRD BIT OF DCBQADFS                              */
#define dcb3525a 0x02 /* DCB IS 3525 - ASSOCIATED DATA                      */
#define dcbqstru 0x01 /* TRUNC ENTRY POINT ENTERED (QSAM)                   */
 
/* Values for field "dcbqsws" */
#define dcbpopen 0x04 /* QSAM PARALLEL INPUT PROCESSING                     */
 
/* Values for field "dcbpoint" */
#define dcblngbs 0x58 /* LENGTH OF DCB FOR BSAM INTERFACE     @L1A          */
#define dcblngpo 0x58 /* LENGTH OF DCB FOR BPAM INTERFACE     @L1A          */
 
/* Values for field "dcberopt" */
#define dcberacc 0x80 /* ACCEPT PERMANENT ERROR                             */
#define dcberskp 0x40 /* SKIP PERMANENT ERROR                               */
#define dcberabe 0x20 /* ABNORMAL END OF TASK                               */
#define dcberbam 0x01 /* OPEN TURNED OFF BFTEK=A              @05A          */
 
/* Values for field "dcbeob" */
#define dcblngqs 0x60 /* LENGTH OF DCB WITH QSAM INTERFACE    @L1A          */
#define dcblngps 0x60 /* LENGTH OF DCB SUFFICIENT FOR QSAM OR @P2C          */
 
#endif
 
#ifdef __open_xl__
#pragma pack()
#else
#pragma pack(reset)
#endif
