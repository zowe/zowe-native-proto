#pragma pack(1)
 
#ifndef __sdwa__
#define __sdwa__
 
struct sdwa {
  void * __ptr32 sdwaparm;      /* -     PARAMETER LIST ADDRESS IF (E)STAE MACRO    */
  struct {
    struct {
      unsigned char  _sdwacmpf;    /* -     FLAG BITS IN COMPLETION CODE.            */
      unsigned char  _sdwacmpc[3]; /* -   SYSTEM COMPLETION CODE (FIRST 12 BITS) AND */
      } sdwaabcc;
    } sdwafiob;
  struct {
    unsigned char  _sdwacmka;      /* -     CHANNEL INTERRUPT MASKS.                  */
    unsigned char  _sdwamwpa;      /* -     PSW KEY AND 'M-W-P'.                      */
    unsigned char  _sdwainta[2];   /* -   INTERRUPT CODE (LAST 2 BYTES OF INTERRUPT   */
    unsigned char  _sdwapmka;      /* -     INSTRUCTION LENGTH CODE, CONDITION CODE,  */
    unsigned int   _sdwanxta : 24; /* -   ADDRESS OF NEXT INSTRUCTION TO BE EXECUTED. */
    } sdwactl1;
  struct {
    unsigned char  _sdwacmkp;      /* -     CHANNEL INTERRUPT MASKS.                  */
    unsigned char  _sdwamwpp;      /* -     PSW KEY AND 'M-W-P'.                      */
    unsigned char  _sdwaintp[2];   /* -   INTERRUPT CODE (LAST 2 BYTES OF INTERRUPT   */
    unsigned char  _sdwapmkp;      /* -     INSTRUCTION LENGTH CODE, CONDITION CODE,  */
    unsigned int   _sdwanxtp : 24; /* -   ADDRESS OF NEXT INSTRUCTION TO BE EXECUTED. */
    } sdwactl2;
  struct {
    struct {
      int            _sdwagr00; /* -     GPR 0.  */
      int            _sdwagr01; /* -     GPR 1.  */
      int            _sdwagr02; /* -     GPR 2.  */
      int            _sdwagr03; /* -     GPR 3.  */
      int            _sdwagr04; /* -     GPR 4.  */
      int            _sdwagr05; /* -     GPR 5.  */
      int            _sdwagr06; /* -     GPR 6.  */
      int            _sdwagr07; /* -     GPR 7.  */
      int            _sdwagr08; /* -     GPR 8.  */
      int            _sdwagr09; /* -     GPR 9.  */
      int            _sdwagr10; /* -     GPR 10. */
      int            _sdwagr11; /* -     GPR 11. */
      int            _sdwagr12; /* -     GPR 12. */
      int            _sdwagr13; /* -     GPR 13. */
      int            _sdwagr14; /* -     GPR 14. */
      int            _sdwagr15; /* -     GPR 15. */
      } sdwatx___pitdb___grsv;
    } sdwagrsv;
  struct {
    void * __ptr32 _sdwarbad;    /* -     RB ADDRESS OF ABENDING PROGRAM (IF SUPERVISOR */
    unsigned char  _filler1[4];  /* -   CONTAINS ZEROS IF SUPERVISOR MODE PROGRAM       */
    } sdwaname;
  void * __ptr32 sdwaepa;       /* -     ENTRY POINT ADDRESS OF ABENDING PROGRAM.   */
  void * __ptr32 sdwaiobr;      /* -     POINTER TO SDWAFIOB FIELD,                 */
  struct {
    struct {
      unsigned char  _sdwaemk1; /* INTERRUPT INFORMATION MASKS     */
      unsigned char  _sdwamwp1; /* PSW KEY AND 'M-W-P'             */
      unsigned char  _sdwaint1; /* CONDITION CODE AND PROGRAM MASK */
      unsigned char  _filler2;  /* RESERVED                        */
      struct {
        unsigned char  _sdwaamf1;    /* ADDRESSING MODE FLAG                    @G860P1S */
        unsigned char  _sdwaadd1[3]; /* INSTRUCTION ADDRESS                              */
        } sdwanxt1;
      } sdwatx___pitdb___ec1;
    } sdwaec1;
  union {
    struct {
      unsigned char  _filler3;  /* RESERVED                                */
      unsigned char  _sdwailc1; /* INSTRUCTION LENGTH CODE FOR PSW DEFINED */
      struct {
        unsigned char  _sdwaic1h; /* High byte of PI code                        @LUA */
        unsigned char  _sdwaicd1; /* 8 BIT INTERRUPT CODE                             */
        } sdwainc1;
      void * __ptr32 _sdwatran; /* VIRTUAL ADDRESS CAUSING TRANSLATION     */
      } sdwaaec1;
    struct {
      unsigned char  _filler4[7];
      struct {
        unsigned char  _sdwavxc; /* Vector exception code when program interrupt */
        } sdwadxc;
      } _sdwa_struct1;
    } _sdwa_union1;
  struct {
    unsigned char  _sdwaemk2; /* INTERRUPT INFORMATION MASKS     */
    unsigned char  _sdwamwp2; /* PSW KEY AND 'M-W-P'             */
    unsigned char  _sdwaint2; /* CONDITION CODE AND PROGRAM MASK */
    unsigned char  _filler5;  /* RESERVED                        */
    struct {
      unsigned char  _sdwaamf2;    /* ADDRESSING MODE FLAG                    @G860P1S */
      unsigned char  _sdwaadd2[3]; /* INSTRUCTION ADDRESS                              */
      } sdwanxt2;
    } sdwaec2;
  struct {
    unsigned char  _filler6;  /* RESERVED                                */
    unsigned char  _sdwailc2; /* INSTRUCTION LENGTH CODE FOR PSW DEFINED */
    struct {
      unsigned char  _sdwaic2h; /* High byte of PI code                        @LUA */
      unsigned char  _sdwaicd2; /* 8 BIT INTERRUPT CODE                             */
      } sdwainc2;
    void * __ptr32 _sdwatrn2; /* VIRTUAL ADDRESS CAUSING TRANSLATION     */
    } sdwaaec2;
  struct {
    int            _sdwasr00; /* GPR 0.  */
    int            _sdwasr01; /* GPR 1.  */
    int            _sdwasr02; /* GPR 2.  */
    int            _sdwasr03; /* GPR 3.  */
    int            _sdwasr04; /* GPR 4.  */
    int            _sdwasr05; /* GPR 5.  */
    int            _sdwasr06; /* GPR 6.  */
    int            _sdwasr07; /* GPR 7.  */
    int            _sdwasr08; /* GPR 8.  */
    int            _sdwasr09; /* GPR 9.  */
    int            _sdwasr10; /* GPR 10. */
    int            _sdwasr11; /* GPR 11. */
    int            _sdwasr12; /* GPR 12. */
    int            _sdwasr13; /* GPR 13. */
    int            _sdwasr14; /* GPR 14. */
    int            _sdwasr15; /* GPR 15. */
    } sdwasrsv;
  struct {
    unsigned char  _sdwaspid;    /* SUBPOOL ID OF STORAGE CONTAINING THIS SDWA */
    unsigned char  _sdwalnth[3]; /* LENGTH OF THIS SDWA IN BYTES               */
    } sdwaidnt;
  struct {
    struct {
      void * __ptr32 _sdwasckb; /* BEGINNING VIRTUAL ADDRESS OF STORAGE CHECK */
      void * __ptr32 _sdwascke; /* ENDING VIRTUAL ADDRESS OF STORAGE CHECK    */
      } sdwastck;
    struct {
      unsigned char  _sdwamchs; /* MCH FLAG BYTE                                    */
      unsigned char  _sdwamchd; /* INPUT INFORMATION TO RECOVERY ROUTINE CONCERNING */
      } sdwamchi;
    unsigned char  _sdwacpid[2]; /* ID OF OF FAILING CPU CAUSING ACR                 */
    unsigned char  _sdwarsr1;    /* ADDITIONAL STORAGE FRAME ERROR INDICATORS        */
    unsigned char  _sdwarsr2;    /* ADDITIONAL STORAGE ERROR INDICATORS.             */
    unsigned char  _sdwamcho;    /* OTHER MACHINE CHECK FLAGS                   @04A */
    unsigned char  _filler7;     /* RESERVED                                    @04C */
    void * __ptr32 _sdwarfsa;    /* REAL STORAGE FAILING ADDRESS  ( VALID ONLY IF    */
    unsigned char  _sdwatime[8]; /* TIME STAMP OF ASSOCIATED MACHINE CHECK RECORD    */
    } sdwamch;
  struct {
    unsigned char  _sdwaerra; /* ERROR TYPE CAUSING ENTRY TO RECOVERY EXIT */
    unsigned char  _sdwaerrb; /* ADDITIONAL ERROR ENTRY INFORMATION        */
    unsigned char  _sdwaerrc; /* ADDITIONAL ERROR ENTRY INFORMATION        */
    unsigned char  _sdwaerrd; /* ADDITIONAL ERROR ENTRY INFORMATION        */
    } sdwaflgs;
  unsigned char  sdwafmid[2];   /* ASID OF MEMORY IN WHICH ERROR OCCURRED.          */
  unsigned char  sdwaiofs;      /* THIS IS THE CURRENT I/O STATUS (THE I/O          */
  unsigned char  sdwacpui;      /* Low order byte of the error logical CPU id.      */
  struct {
    unsigned char  _sdwartyf;      /* ADDRESSING MODE INDICATOR BYTE          @G860P1S */
    unsigned int   _filler8 : 24;  /* LOW THREE ORDER BYTES OF RETRY ADDRESS  @G860P1S */
    } sdwartya;
  void * __ptr32 sdwareca;      /* ADDRESS OF VARIABLE RECORDING AREA WITHIN SDWA   */
  struct {
    unsigned char  _filler9[2];  /* RESERVED                                */
    short int      _sdwalcpu;    /* LOGICAL ADDRESS OF CPU HOLDING RESOURCE */
    } sdwacpua;
  struct {
    unsigned char  _sdwarcde; /* RETURN CODE FROM RECOVERY ROUTINE TO INDICATE   */
    unsigned char  _sdwaacf2; /* FLAGS TO INDICATE ADDITIONAL PROCESSING         */
    unsigned char  _sdwaacf3; /* FLAGS INDICATING SOME GLOBAL LOCKS TO BE FREED. */
    unsigned char  _sdwaacf4; /* ADDITIONAL LOCKS TO BE FREED, OR ADDITIONAL     */
    } sdwaparq;
  struct {
    struct {
      void * __ptr32 _sdwalrsd;  /* LOCKWORD FOR THE RSM DATA SPACE LOCK        @L6C */
      void * __ptr32 _sdwaiulw;  /* LOCKWORD FOR THE IOSUCB LOCK                     */
      void * __ptr32 _filler10;  /* LOCKWORD - RESERVED                         @P3C */
      void * __ptr32 _sdwaiplw;  /* LOCKWORD FOR THE IOSYNCH LOCK                    */
      void * __ptr32 _sdwaaplw;  /* LOCKWORD FOR THE ASM CLASS LOCK         Z40WPXH  */
      void * __ptr32 _filler11;  /* RESERVED                                   @L9C  */
      void * __ptr32 _filler12;  /* RESERVED                                   @L9C  */
      void * __ptr32 _sdwatalw;  /* LOCKWORD - RESERVED                     Z40WPXH  */
      } sdwalkws;
    } sdwalkwa;
  unsigned char  sdwaasid[2];   /* ASID FOR LOGREC DEBUGGING (HOME ASID)   @ZA05516 */
  unsigned char  sdwaseq_n_[2]; /* ERRORID SEQUENCE NUMBER                 @G17EP5W */
  struct {
    unsigned char  _sdwamodn[8]; /* THE LOAD MODULE NAME INVOLVED IN THE ERROR  */
    unsigned char  _sdwacsct[8]; /* THE CSECT (MICROFICHE) NAME INVOLVED IN THE */
    unsigned char  _sdwarexn[8]; /* THE RECOVERY ROUTINE (MICROFICHE) NAME      */
    } sdwarecp;
  void * __ptr32 sdwadpla;      /* POINTER TO DUMP PARAMETER LIST                   */
  struct {
    struct {
      unsigned char  _sdwadpid;  /* ID OF DUMP REQUESTED                           */
      unsigned char  _sdwadpfs;  /* DUMP FLAGS                                     */
      unsigned char  _sdwadpf2;  /* DUMP FLAGS 2                          @G382P2F */
      unsigned char  _filler13;  /* RESERVED                                       */
      } sdwadump;
    struct {
      struct {
        unsigned char  _sdwasda0; /* SDATA OPTIONS FLAG ONE                 @G33SPHW */
        unsigned char  _sdwasda1; /* SDATA OPTIONS                         @G33SPHW  */
        } sdwasdat;
      unsigned char  _sdwapdat;  /* PDATA OPTIONS */
      unsigned char  _filler14;  /* RESERVED      */
      } sdwaddat;
    } sdwasnpa;
  struct {
    struct {
      void * __ptr32 _sdwafrm1; /* BEGINNING ADDRESS FOR STORAGE RANGE 1 */
      void * __ptr32 _sdwato1;  /* ENDING ADDRESS FOR STORAGE RANGE 1    */
      void * __ptr32 _sdwafrm2; /* BEGINNING ADDRESS FOR STORAGE RANGE2  */
      void * __ptr32 _sdwato2;  /* ENDING ADDRESS FOR STORAGE RANGE 2    */
      void * __ptr32 _sdwafrm3; /* BEGINNING ADDRESS FOR STORAGE RANGE 3 */
      void * __ptr32 _sdwato3;  /* ENDING ADDRESS FOR STORAGE RANGE 3    */
      void * __ptr32 _sdwafrm4; /* BEGINNING ADDRESS FOR STORAGE RANGE 4 */
      void * __ptr32 _sdwato4;  /* ENDING ADDRESS FOR STORAGE RANGE 4    */
      } sdwadpsl;
    short int      _sdwa2cid; /* 2-byte ERRORID logical CPU id.  See SDWADPSA     */
    unsigned char  _sdwaopic; /* Low byte of original PIC, without PER bit.  @0BA */
    struct {
      unsigned char  _sdwaiflags; /* Flag bits                                   @0CC */
      } sdwaiflg;
    } sdwadpsa;
  struct {
    unsigned char  _sdwaverf[2]; /* FFFF INDICATES VID FIELD IS VALID       @G38FP2F */
    unsigned char  _sdwavid[2];  /* VERSION INDICATOR, EXPLAINED AS FOLLOWS:    @L1A */
    } sdwaveri;
  void * __ptr32 sdwaxpad;      /* ADDR OF THE EXTENSION POINTERS (SDWAPTRS)   @L1C */
  struct {
    struct {
      struct {
        unsigned char  _sdwakm[2];   /* KEY MASK                                @G381P2F */
        unsigned char  _sdwascnd[2]; /* ASID OF THE SECONDARY ADDR SPACE -SASID @G381P2F */
        } sdwacr3;
      struct {
        unsigned char  _sdwaax[2];   /* AUTHORIZATION INDEX                     @G381P2F */
        unsigned char  _sdwaprim[2]; /* ASID OF THE PRIMARY ADDR SPACE -PASID   @G381P2F */
        } sdwacr4;
      } sdwacrgs;
    void * __ptr32 _sdwacmla; /* ADDRESS OF ASCB OF CML TO BE FREED      @G381P2F */
    } sdwaxm;
  unsigned char  sdwacomu[8];   /* FRR to ESTAE communication buffer       @G382P2F */
  void * __ptr32 sdwacomp;      /* THIS WORD IS PROVIDED FOR COMMUNICATION OF       */
  unsigned char  sdwaertm[4];   /* ERRORID TIME STAMP                     @G17EP5W  */
  struct {
    unsigned char  _sdwavral[2];  /* LENGTH OF VARIABLE RECORDING AREA                */
    unsigned char  _sdwadpva;     /* BITS THAT DEFINE DATA IN VARIABLE AREA           */
    unsigned char  _sdwaural;     /* LENGTH OF USER SUPPLIED INFORMATION IN           */
    unsigned char  _sdwavra[255]; /* VARIABLE RECORDING AREA                 @G388P2F */
    unsigned char  _sdwaid[5];    /* CONTAINS 'SDWA ' AS ID                  @G860P1F */
    } sdwara;
  double         sdwaend;       /* END OF NON-EXTENDED SDWA                @G381P2F */
  };
 
#define sdwacmpf   sdwafiob.sdwaabcc._sdwacmpf
#define sdwacmpc   sdwafiob.sdwaabcc._sdwacmpc
#define sdwacmka   sdwactl1._sdwacmka
#define sdwamwpa   sdwactl1._sdwamwpa
#define sdwainta   sdwactl1._sdwainta
#define sdwapmka   sdwactl1._sdwapmka
#define sdwanxta   sdwactl1._sdwanxta
#define sdwacmkp   sdwactl2._sdwacmkp
#define sdwamwpp   sdwactl2._sdwamwpp
#define sdwaintp   sdwactl2._sdwaintp
#define sdwapmkp   sdwactl2._sdwapmkp
#define sdwanxtp   sdwactl2._sdwanxtp
#define sdwagr00   sdwagrsv.sdwatx___pitdb___grsv._sdwagr00
#define sdwagr01   sdwagrsv.sdwatx___pitdb___grsv._sdwagr01
#define sdwagr02   sdwagrsv.sdwatx___pitdb___grsv._sdwagr02
#define sdwagr03   sdwagrsv.sdwatx___pitdb___grsv._sdwagr03
#define sdwagr04   sdwagrsv.sdwatx___pitdb___grsv._sdwagr04
#define sdwagr05   sdwagrsv.sdwatx___pitdb___grsv._sdwagr05
#define sdwagr06   sdwagrsv.sdwatx___pitdb___grsv._sdwagr06
#define sdwagr07   sdwagrsv.sdwatx___pitdb___grsv._sdwagr07
#define sdwagr08   sdwagrsv.sdwatx___pitdb___grsv._sdwagr08
#define sdwagr09   sdwagrsv.sdwatx___pitdb___grsv._sdwagr09
#define sdwagr10   sdwagrsv.sdwatx___pitdb___grsv._sdwagr10
#define sdwagr11   sdwagrsv.sdwatx___pitdb___grsv._sdwagr11
#define sdwagr12   sdwagrsv.sdwatx___pitdb___grsv._sdwagr12
#define sdwagr13   sdwagrsv.sdwatx___pitdb___grsv._sdwagr13
#define sdwagr14   sdwagrsv.sdwatx___pitdb___grsv._sdwagr14
#define sdwagr15   sdwagrsv.sdwatx___pitdb___grsv._sdwagr15
#define sdwarbad   sdwaname._sdwarbad
#define sdwaemk1   sdwaec1.sdwatx___pitdb___ec1._sdwaemk1
#define sdwamwp1   sdwaec1.sdwatx___pitdb___ec1._sdwamwp1
#define sdwaint1   sdwaec1.sdwatx___pitdb___ec1._sdwaint1
#define sdwaamf1   sdwaec1.sdwatx___pitdb___ec1.sdwanxt1._sdwaamf1
#define sdwaadd1   sdwaec1.sdwatx___pitdb___ec1.sdwanxt1._sdwaadd1
#define sdwailc1   _sdwa_union1.sdwaaec1._sdwailc1
#define sdwaic1h   _sdwa_union1.sdwaaec1.sdwainc1._sdwaic1h
#define sdwaicd1   _sdwa_union1.sdwaaec1.sdwainc1._sdwaicd1
#define sdwatran   _sdwa_union1.sdwaaec1._sdwatran
#define sdwavxc    _sdwa_union1._sdwa_struct1.sdwadxc._sdwavxc
#define sdwaemk2   sdwaec2._sdwaemk2
#define sdwamwp2   sdwaec2._sdwamwp2
#define sdwaint2   sdwaec2._sdwaint2
#define sdwaamf2   sdwaec2.sdwanxt2._sdwaamf2
#define sdwaadd2   sdwaec2.sdwanxt2._sdwaadd2
#define sdwailc2   sdwaaec2._sdwailc2
#define sdwaic2h   sdwaaec2.sdwainc2._sdwaic2h
#define sdwaicd2   sdwaaec2.sdwainc2._sdwaicd2
#define sdwatrn2   sdwaaec2._sdwatrn2
#define sdwasr00   sdwasrsv._sdwasr00
#define sdwasr01   sdwasrsv._sdwasr01
#define sdwasr02   sdwasrsv._sdwasr02
#define sdwasr03   sdwasrsv._sdwasr03
#define sdwasr04   sdwasrsv._sdwasr04
#define sdwasr05   sdwasrsv._sdwasr05
#define sdwasr06   sdwasrsv._sdwasr06
#define sdwasr07   sdwasrsv._sdwasr07
#define sdwasr08   sdwasrsv._sdwasr08
#define sdwasr09   sdwasrsv._sdwasr09
#define sdwasr10   sdwasrsv._sdwasr10
#define sdwasr11   sdwasrsv._sdwasr11
#define sdwasr12   sdwasrsv._sdwasr12
#define sdwasr13   sdwasrsv._sdwasr13
#define sdwasr14   sdwasrsv._sdwasr14
#define sdwasr15   sdwasrsv._sdwasr15
#define sdwaspid   sdwaidnt._sdwaspid
#define sdwalnth   sdwaidnt._sdwalnth
#define sdwasckb   sdwamch.sdwastck._sdwasckb
#define sdwascke   sdwamch.sdwastck._sdwascke
#define sdwamchs   sdwamch.sdwamchi._sdwamchs
#define sdwamchd   sdwamch.sdwamchi._sdwamchd
#define sdwacpid   sdwamch._sdwacpid
#define sdwarsr1   sdwamch._sdwarsr1
#define sdwarsr2   sdwamch._sdwarsr2
#define sdwamcho   sdwamch._sdwamcho
#define sdwarfsa   sdwamch._sdwarfsa
#define sdwatime   sdwamch._sdwatime
#define sdwaerra   sdwaflgs._sdwaerra
#define sdwaerrb   sdwaflgs._sdwaerrb
#define sdwaerrc   sdwaflgs._sdwaerrc
#define sdwaerrd   sdwaflgs._sdwaerrd
#define sdwartyf   sdwartya._sdwartyf
#define sdwalcpu   sdwacpua._sdwalcpu
#define sdwarcde   sdwaparq._sdwarcde
#define sdwaacf2   sdwaparq._sdwaacf2
#define sdwaacf3   sdwaparq._sdwaacf3
#define sdwaacf4   sdwaparq._sdwaacf4
#define sdwalrsd   sdwalkwa.sdwalkws._sdwalrsd
#define sdwaiulw   sdwalkwa.sdwalkws._sdwaiulw
#define sdwaiplw   sdwalkwa.sdwalkws._sdwaiplw
#define sdwaaplw   sdwalkwa.sdwalkws._sdwaaplw
#define sdwatalw   sdwalkwa.sdwalkws._sdwatalw
#define sdwamodn   sdwarecp._sdwamodn
#define sdwacsct   sdwarecp._sdwacsct
#define sdwarexn   sdwarecp._sdwarexn
#define sdwadpid   sdwasnpa.sdwadump._sdwadpid
#define sdwadpfs   sdwasnpa.sdwadump._sdwadpfs
#define sdwadpf2   sdwasnpa.sdwadump._sdwadpf2
#define sdwasda0   sdwasnpa.sdwaddat.sdwasdat._sdwasda0
#define sdwasda1   sdwasnpa.sdwaddat.sdwasdat._sdwasda1
#define sdwapdat   sdwasnpa.sdwaddat._sdwapdat
#define sdwafrm1   sdwadpsa.sdwadpsl._sdwafrm1
#define sdwato1    sdwadpsa.sdwadpsl._sdwato1
#define sdwafrm2   sdwadpsa.sdwadpsl._sdwafrm2
#define sdwato2    sdwadpsa.sdwadpsl._sdwato2
#define sdwafrm3   sdwadpsa.sdwadpsl._sdwafrm3
#define sdwato3    sdwadpsa.sdwadpsl._sdwato3
#define sdwafrm4   sdwadpsa.sdwadpsl._sdwafrm4
#define sdwato4    sdwadpsa.sdwadpsl._sdwato4
#define sdwa2cid   sdwadpsa._sdwa2cid
#define sdwaopic   sdwadpsa._sdwaopic
#define sdwaiflags sdwadpsa.sdwaiflg._sdwaiflags
#define sdwaverf   sdwaveri._sdwaverf
#define sdwavid    sdwaveri._sdwavid
#define sdwakm     sdwaxm.sdwacrgs.sdwacr3._sdwakm
#define sdwascnd   sdwaxm.sdwacrgs.sdwacr3._sdwascnd
#define sdwaax     sdwaxm.sdwacrgs.sdwacr4._sdwaax
#define sdwaprim   sdwaxm.sdwacrgs.sdwacr4._sdwaprim
#define sdwacmla   sdwaxm._sdwacmla
#define sdwavral   sdwara._sdwavral
#define sdwadpva   sdwara._sdwadpva
#define sdwaural   sdwara._sdwaural
#define sdwavra    sdwara._sdwavra
#define sdwaid     sdwara._sdwaid
 
/* Values for field "sdwacmpf" */
#define sdwareq         0x80 /* - ON, SYSABEND/SYSMDUMP/SYSUDUMP DUMP TO BE        */
#define sdwastep        0x40 /* - ON, JOBSTEP TO BE TERMINATED.                    */
#define sdwastcc        0x10 /* - ON, DON'T STORE COMPLETION CODE.                 */
#define sdwarcf         0x04 /* - ON, REASON CODE IN SDWACRC IS VALID         @PBC */
 
/* Values for field "sdwacmka" */
#define sdwaioa         0xFE /* - I/O INTERRUPTS (ALL ZEROS OR ALL ONES).          */
#define sdwaexta        0x01 /* - EXTERNAL INTERRUPT.                              */
 
/* Values for field "sdwamwpa" */
#define sdwakeya        0xF0 /* - PSW KEY.                                         */
#define sdwamcka        0x04 /* - MACHINE CHECK INTERRUPT.                         */
#define sdwawata        0x02 /* - WAIT STATE.                                      */
#define sdwaspva        0x01 /* - SUPERVISOR/PROBLEM-PROGRAM MODE.                 */
 
/* Values for field "sdwapmka" */
#define sdwaila         0xC0 /* - INSTRUCTION LENGTH CODE.                         */
#define sdwacca         0x30 /* - LAST CONDITION CODE.                             */
#define sdwafpa         0x08 /* - FIXED-POINT OVERFLOW.                            */
#define sdwadoa         0x04 /* - DECIMAL OVERFLOW.                                */
#define sdwaeua         0x02 /* - EXPONENT UNDERFLOW.                              */
#define sdwasga         0x01 /* - SIGNIFICANCE.                                    */
 
/* Values for field "sdwacmkp" */
#define sdwaiop         0xFE /* - I/O INTERRUPTS (ALL ZEROS OR ALL ONES).          */
#define sdwaextp        0x01 /* - EXTERNAL INTERRUPT.                              */
 
/* Values for field "sdwamwpp" */
#define sdwakeyp        0xF0 /* - PSW KEY.                                         */
#define sdwamckp        0x04 /* - MACHINE CHECK INTERRUPT.                         */
#define sdwawatp        0x02 /* - WAIT STATE.                                      */
#define sdwaspvp        0x01 /* - SUPERVISOR/PROBLEM-PROGRAM MODE.                 */
 
/* Values for field "sdwapmkp" */
#define sdwailp         0xC0 /* - INSTRUCTION LENGTH CODE.                         */
#define sdwaccp         0x30 /* - LAST CONDITION CODE.                             */
#define sdwafpp         0x08 /* - FIXED-POINT OVERFLOW.                            */
#define sdwadop         0x04 /* - DECIMAL OVERFLOW.                                */
#define sdwaeup         0x02 /* - EXPONENT UNDERFLOW.                              */
#define sdwasgp         0x01 /* - SIGNIFICANCE.                                    */
 
/* Values for field "sdwaemk1" */
#define sdwaper1        0x40 /* ON,PROGRAM EVENT RECORDING                         */
#define sdwatrm1        0x04 /* ON,ADDRESS TRANSLATION ACTIVE                      */
#define sdwaio1         0x02 /* OFF,I/0 INTERRUPTION CAN NOT OCCUR                 */
#define sdwaext1        0x01 /* OFF,EXTERNAL INTERRUPTION CANNOT OCCUR             */
 
/* Values for field "sdwamwp1" */
#define sdwakey1        0xF0 /* PSW KEY                                            */
#define sdwaect1        0x08 /* EXTENDED CONTROL MODE BIT                          */
#define sdwamck1        0x04 /* OFF,MACHINE CHECK CANNOT OCCUR                     */
#define sdwawat1        0x02 /* ON,CPU IN WAIT STATE                               */
#define sdwapgm1        0x01 /* ON,PROBLEM STATE                                   */
 
/* Values for field "sdwaint1" */
#define sdwaascm        0xC0 /* ADDRESS SPACE CONTROL MODE BITS                    */
#define sdwas1          0x80 /* ADDRESS SPACE SELECTION BIT             @G381P2F   */
#define sdwacc1         0x30 /* CONDITION CODE                                     */
#define sdwafpo1        0x08 /* FIXED POINT OVERFLOW                               */
#define sdwadec1        0x04 /* DECIMAL OVERFLOW                                   */
#define sdwaexp1        0x02 /* EXPONENT UNDERFLOW                                 */
#define sdwasgn1        0x01 /* SIGNIFICANCE                                       */
 
/* Values for field "sdwaamf1" */
#define sdwamod1        0x80 /* ADDRESSING MODE OF THE NEXT INSTRUCTION @G860P1S   */
 
/* Values for field "sdwailc1" */
#define sdwail1         0x06 /* ILC                                                */
 
/* Values for field "sdwaic1h" */
#define sdwaptx1        0x02 /* Program interrupt during transactional             */
 
/* Values for field "sdwaicd1" */
#define sdwaipr1        0x80 /* PER INTERRUPT OCCURRED                             */
#define sdwaimc1        0x40 /* MONITOR CALL INTERRUPT OCCURRED                    */
#define sdwaipc1        0x3F /* AN UNSOLICITED PROGRAM CHECK                       */
 
/* Values for field "sdwaemk2" */
#define sdwaper2        0x40 /* ON,PROGRAM EVENT RECORDING                         */
#define sdwatrm2        0x04 /* ON,ADDRESS TRANSLATION ACTIVE                      */
#define sdwaio2         0x02 /* OFF,I/0 INTERRUPTION CANNOT OCCUR                  */
#define sdwaext2        0x01 /* OFF,EXTERNAL INTERRUPTION CANNOT OCCUR             */
 
/* Values for field "sdwamwp2" */
#define sdwakey2        0xF0 /* PSW KEY                                            */
#define sdwaect2        0x08 /* EXTENDED CONTROL MODE BIT                          */
#define sdwamck2        0x04 /* OFF,MACHINE CHECK CANNOT OCCUR                     */
#define sdwawat2        0x02 /* ON,CPU IN WAIT STATE                               */
#define sdwapgm2        0x01 /* ON,PROBLEM STATE                                   */
 
/* Values for field "sdwaint2" */
#define sdwas2          0x80 /* ADDRESS SPACE SELECTION BIT             @G381P2F   */
#define sdwacc2         0x30 /* CONDITION CODE                                     */
#define sdwafpo2        0x08 /* FIXED POINT OVERFLOW                               */
#define sdwadec2        0x04 /* DECIMAL OVERFLOW                                   */
#define sdwaexp2        0x02 /* EXPONENT UNDERFLOW                                 */
#define sdwasgn2        0x01 /* SIGNIFICANCE                                       */
 
/* Values for field "sdwaamf2" */
#define sdwamod2        0x80 /* ADDRESSING MODE OF THE NEXT INSTRUCTION @G860P1S   */
 
/* Values for field "sdwailc2" */
#define sdwail2         0x06 /* ILC                                                */
 
/* Values for field "sdwaic2h" */
#define sdwaptx2        0x02 /* Program interrupt during transactional             */
 
/* Values for field "sdwaicd2" */
#define sdwaipr2        0x80 /* PER INTERRUPT OCCURRED                             */
#define sdwaimc2        0x40 /* MONITOR CALL INTERRRUPT OCCURRED                   */
#define sdwaipc2        0x3F /* AN UNSOLICITED PROGRAM CHECK                       */
 
/* Values for field "sdwamchs" */
#define sdwasrvl        0x80 /* ON,STORAGE ADDRESSES SUPPLIED                      */
#define sdwarcdf        0x40 /* ON,MACHINE CHECK RECORD NOT RECORDED               */
#define sdwatsvl        0x20 /* ON,TIME STAMP IS VALID                             */
#define sdwainvp        0x10 /* ON,STORAGE IS RECONFIGURED, PAGE IS INVALIDATED    */
#define sdwarsrc        0x08 /* ON,STORAGE RECONFIGURATION (SDWARSR1,SDWARSR2)     */
#define sdwarsrf        0x04 /* ON,STORAGE RECONFIGURATION NOT ATTEMPTED.          */
#define sdwavriv        0x02 /* ON, INDICATES VECTOR REGISTERS ARE UNPREDICTABLE   */
#define sdwaargu        0x01 /* ON, INDICATES ACCESS REGISTERS ARE UNPREDICTABLE   */
 
/* Values for field "sdwamchd" */
#define sdwaskyf        0x80 /* ON,STORAGE KEY FAILURE                             */
#define sdwaregu        0x40 /* ON,GENERAL PURPOSE REGISTER CONTENTS AT TIME OF    */
#define sdwapswu        0x20 /* ON,PSW AND/OR CONTROL REGISTERS AT TIME OF         */
#define sdwasck         0x10 /* ON,INDICATES STORAGE DATA CHECK                    */
#define sdwaacr         0x08 /* ON,INDICATES ACR REQUEST                           */
#define sdwainsf        0x04 /* ON,INSTRUCTION FAILURE                             */
#define sdwafprx        0x02 /* ON,CONTENTS OF FLOATING POINT REGISTERS AT TIME    */
#define sdwaterr        0x01 /* ON,TIMER ERROR - CAUSES ENTRY TO                   */
 
/* Values for field "sdwarsr1" */
#define sdwapref        0x20 /* PREFERRED FRAME                         @G860P1F   */
#define sdwavrcn        0x10 /* V = R CANDIDATE - CAN GO OFFLINE        @G860P1F   */
#define sdwanswp        0x08 /* LONG-TERM NON-SWAPPABLE ADDRESS SPACE   @G860P1F   */
#define sdwanswa        0x04 /* NON-SWAPPABLE ADDRESS SPACE             @G860P1F   */
#define sdwamser        0x02 /* STORAGE ERROR ALREADY SET IN FRAME.                */
#define sdwachng        0x01 /* CHANGE INDICATOR WAS ON IN FRAME.                  */
 
/* Values for field "sdwarsr2" */
#define sdwaofln        0x80 /* FRAME OFFLINE OR SCHEDULED TO GO OFFLINE           */
#define sdwaintc        0x40 /* INTERCEPT                                          */
#define sdwasper        0x20 /* STORAGE ERROR PERMANENT ON FRAME.                  */
#define sdwanucl        0x10 /* FRAME CONTAINS PERMANENT RESIDENT STORAGE,         */
#define sdwafsqa        0x08 /* FRAME IN SQA                                       */
#define sdwaflsq        0x04 /* FRAME IN LSQA                                      */
#define sdwapgfx        0x02 /* FRAME IS PAGE FIXED                                */
#define sdwaveqr        0x01 /* FRAME IS VIRTUAL = REAL                 @G860P1F   */
 
/* Values for field "sdwamcho" */
#define sdwaskpr        0x80 /* SKIP RECORDING REQUESTED BY MACHINE CHECK   @04A   */
 
/* Values for field "sdwaerra" */
#define sdwamchk        0x80 /* ON INDICATES MACHINE CHECK                         */
#define sdwapchk        0x40 /* ON INDICATES PROGRAM CHECK                         */
#define sdwarkey        0x20 /* ON INDICATES CONSOLE RESTART KEY WAS DEPRESSED     */
#define sdwasvcd        0x10 /* ON INDICATES TASK ISSUED SVC 13                    */
#define sdwaabtm        0x08 /* ON INDICATES SYSTEM FORCED SVC 13(I.E.ABTERM)      */
#define sdwasvce        0x04 /* ON,INDICATES AN SVC WAS ISSUED BY                  */
#define sdwatexc        0x02 /* ON,INDICATES AN UNRECOVERABLE TRANSLATION          */
#define sdwapgio        0x01 /* ON,INDICATES A PAGE I/O ERROR                      */
#define sdwastrm        0x01 /* ON,INDICATES AN RTM1 SERVICE ROUTINE    @G38AP1F   */
 
/* Values for field "sdwaerrb" */
#define sdwapdip        0x80 /* ON INDICATES THAT THIS TASK WAS PARALLEL           */
#define sdwanmfs        0x40 /* Not My Fault Summary -- indicates that this        */
#define sdwasrbt        0x20 /* On, indicates that this abend was issued           */
#define sdwasrbs        0x10 /* On - this SDWA was allocated for an SRB            */
#define sdwatyp1        0x08 /* ON TYPE 1 SVC IN CONTROL AT TIME OF ERROR          */
#define sdwaenrb        0x04 /* ON ENABLED RB IN CONTROL AT TIME OF ERROR          */
#define sdwaldis        0x02 /* ON A LOGICALLY OR PHYSICALLY DISABLED ROUTINE      */
#define sdwasrbm        0x01 /* ON SYSTEM IN SRB MODE AT TIME OF ERROR             */
 
/* Values for field "sdwaerrc" */
#define sdwastaf        0x80 /* ON INDICATES A PREVIOUS (E)STA  OR FRR             */
#define sdwastai        0x40 /* ON A (E)STAI EXIT PREVIOUSLY RECEIVED CONTROL      */
#define sdwairb         0x20 /* ON AN IRB PRECEDED THE RB THAT IS                  */
#define sdwaperc        0x10 /* ON THIS RECOVERY ROUTINE IS BEING PERCOLATED TO    */
#define sdwaeas         0x08 /* ON INDICATES A LOWER LEVEL EXIT HAS RECOGNIZED     */
#define sdwaskip        0x04 /* ON INDICATES FRRS WERE SKIPPED          @G381P2F   */
#define sdwalcl         0x02 /* ON IND ENTRY AS A LOCAL RESOURCE MGR    @G382P2F   */
#define sdwaglbl        0x01 /* ON IND ENTRY AS A GLOBAL RESOURCE MGR   @G382P2F   */
 
/* Values for field "sdwaerrd" */
#define sdwaclup        0x80 /* ON INDICATES RECOVERY EXIT ONLY TO CLEANUP AND     */
#define sdwanrbe        0x40 /* ON RB ASSOCIATED WITH THIS ESTA EXIT WAS NOT       */
#define sdwastae        0x20 /* ON THIS ESTA EXIT HAS BEEN ENTERED FOR A           */
#define sdwacts         0x10 /* ON,THIS TASK WAS NOT IN CONTROL AT TIME OF         */
#define sdwamabd        0x08 /* ON,THIS TASK WAS NOT IN CONTROL AT TIME OF         */
#define sdwarpiv        0x04 /* ON, THE REGISTERS, PSW AND CONTROL REGISTERS       */
#define sdwamciv        0x02 /* ON,MACHINE CHECK ERROR INFORMATION                 */
#define sdwaerfl        0x01 /* ON,ERRORID INFORMATION AVAILABLE       @G17EP5W    */
 
/* Values for field "sdwaiofs" */
#define sdwaioqr        0x80 /* ON,I/O FOR FAILING PROGRAM HAS BEEN QUIESCED AND   */
#define sdwaioht        0x40 /* ON,I/O FOR FAILING PROGRAM HAS BEEN HALTED AND     */
#define sdwanoio        0x20 /* ON,FAILING PROGRAM HAS NO I/O OUTSTANDING          */
#define sdwaniop        0x10 /* ON,USER REQUESTED NO I/O PROCESSING                */
 
/* Values for field "sdwartyf" */
#define sdwaamod        0x80 /* This bit is never looked at. The AMODE of the      */
 
/* Values for field "sdwarcde" */
#define sdwacwt         0    /* 0 ,CONTINUE WITH TERMINATION. THIS INDICATION      */
#define sdwarety        4    /* 4 ,RETRY USING RETRY ADDRESS IN SDWARTYA FIELD     */
#define sdwapsti        16   /* 16,PREVENT FURTHER (E)STAI PROCESSING              */
 
/* Values for field "sdwaacf2" */
#define sdwarcrd        0x80 /* ON,RECORDING REQUESTED                             */
#define sdwarfxm        0x40 /* ON,RETRY TO FULLXM AT TIME OF FRR SET.             */
#define sdwaspin        0x20 /* ON,PROGRAM INTERRUPTED VIA  THE RESTART KEY WAS    */
#define sdwarerr        0x10 /* ON,RETRY USING THE CROSS MEMORY ADDRESSING         */
#define sdwauprg        0x08 /* ON,UPDATED REGISTERS STARTING WITH SDWASR00 ARE    */
#define sdwafree        0x04 /* ON, SDWA (RTCA) TO BE FREED PRIOR TO RETRY.        */
#define sdwaserp        0x02 /* ON,SERIALIZE PERCOLATION (USED WHEN AN SRB MODE    */
#define sdwacml         0x01 /* ON,FREE THE CROSS MEMORY LOCAL LOCK     @G381P2F   */
 
/* Values for field "sdwaacf3" */
#define sdwafrsx        0x80 /* ON, THE RSM CROSS MEMORY CLASS LOCK     @G860P1S   */
#define sdwafrsa        0x40 /* ON, THE RSM ADDRESS SPACE CLASS LOCK    @G860P1S   */
#define sdwafvsp        0x20 /* ON, THE VSM PAGE LOCK                   @G860P1S   */
#define sdwadisp        0x10 /* ON,THE DISPATCHER LOCK                             */
#define sdwaasmp        0x08 /* ON,THE ASM CLASS LOCK                   Z40WPXH    */
#define sdwasall        0x04 /* ON, THE SALLOC LOCK                                */
#define sdwaiprg        0x02 /* ON, THE IOSYNCH LOCK                               */
#define sdwafrsd        0x01 /* ON, THE RSM DATA SPACE LOCK                 @L6C   */
 
/* Values for field "sdwaacf4" */
#define sdwaiucb        0x80 /* ON, FREE IOSUCB LOCK                               */
#define sdwarsmq        0x40 /* ON, FREE RSMQ LOCK                          @LXA   */
#define sdwatadb        0x08 /* RESERVED LOCK                           Z40WPXH    */
#define sdwaoptm        0x04 /* ON, FREE SYSTEM RESOURCES MGR(SRM) LOCK            */
#define sdwacms         0x02 /* ON, FREE CMS LOCK                                  */
#define sdwafllk        0x01 /* ON, FREE LOCAL LOCK                                */
 
/* Values for field "sdwadpfs" */
#define sdwadpt         0x80 /* ALWAYS OFF, INDICATES SNAP DUMP REQUEST            */
#define sdwadlst        0x40 /* ALWAYS ON, INDICATES  OS/VS2 REL. 2 DUMP           */
#define sdwaensn        0x20 /* ON,ENHANCED DUMP OPTIONS              @G33SPHW     */
#define sdwaslst        0x02 /* ON,STORAGE LISTS SUPPLIED FOR DUMP                 */
 
/* Values for field "sdwadpf2" */
#define sdwadvs3        0x80 /* ON, STORAGE RANGES IN SDWADSR, OFF, STORAGE        */
#define sdwaxlst        0x08 /* ON,DATA SPACE STORAGE LISTS SUPPLIED FOR DUMP      */
#define sdwalvl2        0x04 /* ON, MVS/SP2.1 VERSION OF SNAP PARMLIST  @G860P1F   */
#define sdwasubl        0x02 /* ON, SUBPOOL LIST SUPPLIED               @G860P1C   */
 
/* Values for field "sdwasda0" */
#define sdwanuc         0x80 /* DISPLAY NUCLEUS                                    */
#define sdwasqa         0x40 /* DISPLAY SQA                                        */
#define sdwalsqa        0x20 /* DISPLAY LSQA                                       */
#define sdwaswa         0x10 /* DISPLAY SWA                                        */
#define sdwagtf         0x08 /* DISPLAY GTF INCORE TRACE TABLE                     */
#define sdwacbs         0x04 /* FORMAT AND DISPLAY CONTROL BLOCKS                  */
#define sdwaqqs         0x02 /* FORMAT AND DISPLAY QCBS/QELS                       */
#define sdwadm          0x01 /* FORMAT DATA MGT CONTROL BLOCKS        @G33SPHW     */
 
/* Values for field "sdwasda1" */
#define sdwaio          0x80 /* FORMAT I/O SUPERVISOR CONTROL BLOCKS  @G33SPHW     */
#define sdwaerr         0x40 /* FORMAT ERROR CONTROL BLOCKS           @G33SPHW     */
#define sdwasum         0x10 /* PROVIDE SUMMARY DUMP                    @G860P1S   */
#define sdwaalln        0x08 /* DISPLAY ENTIRE VIRTUAL NUCLEUS          @G860P1S   */
 
/* Values for field "sdwapdat" */
#define sdwadsas        0x80 /* DISPLAY SAVE AREAS                                 */
#define sdwadsah        0x40 /* DISPLAY SAVE AREA HEADER                           */
#define sdwadreg        0x20 /* DISPLAY REGISTERS                                  */
#define sdwatlpa        0x10 /* DISPLAY LPA MODULES OF TASK                        */
#define sdwatjpa        0x08 /* DISPLAY JPA MODULES OF TASK                        */
#define sdwadpsw        0x04 /* DISPLAY PSW                                        */
#define sdwauspl        0x02 /* DISPLAY USER SUBPOOLS                              */
 
/* Values for field "sdwaiflags" */
#define sdwa___valid1   0x80 /* When on, the next 3 bits have valid values  @0CA   */
#define sdwa___integmon 0x40 /* Integrity monitor event, when SDWA_Valid1          */
#define sdwa___jscbauth 0x20 /* For an error event that occurred in task mode,     */
#define sdwa___sdip     0x10 /* SVC Dump is in progress,                           */
 
/* Values for field "sdwavid" */
#define sdwavs3         1    /* 1, INDICATES THE SDWA IS AT AN MVS/SYSTEM          */
#define sdwavs4         2    /* 2, INDICATES THE SDWA IS AT AN MVS/SYSTEM          */
#define sdwavs5         3    /* 3, INDICATES THE SDWA IS AT AN MVS/SYSTEM   @L1A   */
#define sdwavs6         4    /* 4, INDICATES THE SDWA IS AT AN MVS/SYSTEM   @L4A   */
#define sdwavs7         5    /* 5, INDICATES THE SDWA IS AT AN MVS/SYSTEM   @L9A   */
#define sdwavs8         6    /* 6, INDICATES THE SDWA IS AT AN MVS/SYSTEM   @L9A   */
#define sdwavs9         7    /* 7, indicates the SDWA is at an OS/390 R10   @L9A   */
#define sdwavs10        8    /* 8, indicates the SDWA is at a z/OS R7       @LNA   */
#define sdwavs11        9    /* 9, indicates the SDWA is at a z/OS V2R1     @LUA   */
#define sdwavsn         9    /* 9, indicates the SDWA is at a z/OS V2R1     @LUC   */
 
/* Values for field "sdwadpva" */
#define sdwahex         0x80 /* SDWAVRA DATA TO BE PRINTED BY EREP IN HEX          */
#define sdwaebc         0x40 /* SDWAVRA DATA TO BE PRINTED BY EREP IN EBCDIC       */
#define sdwavram        0x20 /* SDWAVRA DATA IS IN THE FORMAT MAPPED BY THE        */
 
#endif
 
#ifndef __sdwarc1__
#define __sdwarc1__
 
struct sdwarc1 {
  struct {
    struct {
      unsigned char  _sdwacid[5];   /* COMPONENT ID OF THE COMPONENT INVOLVED           */
      unsigned char  _sdwasc[23];   /* NAME OF THE SUBCOMPONENT AND THE MODULE          */
      struct {
        unsigned char  _sdwamdat[8]; /* ASSEMBLY DATE OF THE MODULE INVOLVED IN */
        unsigned char  _sdwamvrs[8]; /* VERSION OF THE MODULE - PTF OR PRODUCT  */
        } sdwamlvl;
      struct {
        int            _sdwahrc; /* HEXADECIMAL DECLARE FOR SDWACRC             @PBC */
        } sdwacrc;
      unsigned char  _sdwarrl[8];   /* ENTRY POINT LABEL OF THE RECOVERY ROUTINE THAT   */
      unsigned char  _sdwacidb[4];  /* THE COMPONENT ID BASE (PREFIX) NUMBER,  @G38PP1F */
      char           _sdwasdrc;     /* SVCDUMP STATUS INDICATOR (FOR USE BY SDUMP)      */
      unsigned char  _sdwaccrc;     /* FLAGS FOR COMPCODE AND REASON CODE      @G860P1C */
      unsigned char  _sdwaretf;     /* SDWA RETRY FLAGS                            @L3A */
      unsigned char  _sdwatype;     /* TYPE OF RECOVERY ROUTINE THAT RTM GAVE CONTROL   */
      unsigned char  _sdwahlhi[4];  /* Copy of PSAHLHI (Highest Lock Held Indicator)    */
      unsigned char  _sdwasupr[4];  /* Copy of PSASUPER (Supervisor Control Word) at    */
      unsigned char  _sdwaspn[4];   /* Copy of LCCASPIN (Processor Spinning Indicators) */
      unsigned char  _sdwaeadr[4];  /* FRR OR ESTAE RECOVERY ROUTINE ADDRESS.           */
      unsigned char  _sdwafrre[24]; /* IF FRR EXISTS: COPY OF FRR PARAMETER AREA FROM   */
      unsigned char  _sdwasdrn[4];  /* SDUMP REASON FLAGS FOR TAKING PARTIAL   @ZMC2916 */
      struct {
        unsigned char  _sdwadaet[8]; /* DAE STATUS FLAGS MAPPED BY ADYDSTAT @P1C */
        unsigned char  _sdwaocur[2]; /* NUMBER OF OCCURRENCES OF THIS PROBLEM.   */
        } sdwadaew;
      } sdwarc1z;
    struct {
      struct {
        unsigned char  _sdwaasi1[2]; /* ADDRESS SPACE ID OF TASK FOR PURGEDQ     */
        int            _sdwatcb;     /* ADDRESS OF TCB FOR PURGEDQ          @L1A */
        } sdwapgta;
      struct {
        unsigned char  _sdwafain[12]; /* 12 BYTES OF INSTRUCTION STREAM AS    */
        int            _sdwaascb;     /* ADDRESS OF ASCB FOR FAILING ADDRESS  */
        int            _sdwaasst;     /* ADDRESS OF ADDRESS SPACE SEGMENT     */
        struct {
          unsigned char  _sdwaoabf;    /* FLAGS IN COMPLETION CODE REGISTER.  @L1A */
          unsigned char  _sdwaocmp[3]; /* COMPLETION CODE.                    @L1A */
          } sdwasabc;
        int            _sdwaocrc;     /* ORIGINAL REASON CODE FROM SDWACRC AT */
        } sdwart12;
      } sdwarc1p;
    struct {
      int            _sdwacre0; /* CONTROL REGISTER 0                  @L4A */
      int            _sdwacre1; /* CONTROL REGISTER 1                  @L4A */
      int            _sdwacre2; /* CONTROL REGISTER 2                  @L4A */
      int            _sdwacre3; /* CONTROL REGISTER 3                  @L4A */
      int            _sdwacre4; /* CONTROL REGISTER 4                  @L4A */
      int            _sdwacre5; /* CONTROL REGISTER 5                  @L4A */
      int            _sdwacre6; /* CONTROL REGISTER 6                  @L4A */
      int            _sdwacre7; /* CONTROL REGISTER 7                  @L4A */
      int            _sdwacre8; /* CONTROL REGISTER 8                  @L4A */
      int            _sdwacre9; /* CONTROL REGISTER 9                  @L4A */
      int            _sdwacrea; /* CONTROL REGISTER 10                 @L4A */
      int            _sdwacreb; /* CONTROL REGISTER 11                 @L4A */
      int            _sdwacrec; /* CONTROL REGISTER 12                 @L4A */
      int            _sdwacred; /* CONTROL REGISTER 13                 @L4A */
      int            _sdwacree; /* CONTROL REGISTER 14                 @L4A */
      int            _sdwacref; /* CONTROL REGISTER 15                 @L4A */
      } sdwacrer;
    struct {
      int            _sdwaare0; /* ACCESS REGISTER 0                   @L4A */
      int            _sdwaare1; /* ACCESS REGISTER 1                   @L4A */
      int            _sdwaare2; /* ACCESS REGISTER 2                   @L4A */
      int            _sdwaare3; /* ACCESS REGISTER 3                   @L4A */
      int            _sdwaare4; /* ACCESS REGISTER 4                   @L4A */
      int            _sdwaare5; /* ACCESS REGISTER 5                   @L4A */
      int            _sdwaare6; /* ACCESS REGISTER 6                   @L4A */
      int            _sdwaare7; /* ACCESS REGISTER 7                   @L4A */
      int            _sdwaare8; /* ACCESS REGISTER 8                   @L4A */
      int            _sdwaare9; /* ACCESS REGISTER 9                   @L4A */
      int            _sdwaarea; /* ACCESS REGISTER 10                  @L4A */
      int            _sdwaareb; /* ACCESS REGISTER 11                  @L4A */
      int            _sdwaarec; /* ACCESS REGISTER 12                  @L4A */
      int            _sdwaared; /* ACCESS REGISTER 13                  @L4A */
      int            _sdwaaree; /* ACCESS REGISTER 14                  @L4A */
      int            _sdwaaref; /* ACCESS REGISTER 15                  @L4A */
      } sdwaarer;
    struct {
      int            _sdwaars0; /* ACCESS REGISTER 0                   @L4A */
      int            _sdwaars1; /* ACCESS REGISTER 1                   @L4A */
      int            _sdwaars2; /* ACCESS REGISTER 2                   @L4A */
      int            _sdwaars3; /* ACCESS REGISTER 3                   @L4A */
      int            _sdwaars4; /* ACCESS REGISTER 4                   @L4A */
      int            _sdwaars5; /* ACCESS REGISTER 5                   @L4A */
      int            _sdwaars6; /* ACCESS REGISTER 6                   @L4A */
      int            _sdwaars7; /* ACCESS REGISTER 7                   @L4A */
      int            _sdwaars8; /* ACCESS REGISTER 8                   @L4A */
      int            _sdwaars9; /* ACCESS REGISTER 9                   @L4A */
      int            _sdwaarsa; /* ACCESS REGISTER 10                  @L4A */
      int            _sdwaarsb; /* ACCESS REGISTER 11                  @L4A */
      int            _sdwaarsc; /* ACCESS REGISTER 12                  @L4A */
      int            _sdwaarsd; /* ACCESS REGISTER 13                  @L4A */
      int            _sdwaarse; /* ACCESS REGISTER 14                  @L4A */
      int            _sdwaarsf; /* ACCESS REGISTER 15                  @L4A */
      } sdwaarsv;
    unsigned char  _sdwaduct[64]; /* DISPATCHABLE UNIT CONTROL TABLE     @L4A         */
    unsigned char  _sdwatear;     /* TRANSLATION EXCEPTION ACCESS REGISTER            */
    unsigned char  _sdwaxflg;     /* EXTENDED FLAG AREA                  @L4A         */
    unsigned char  _sdwasflg;     /* SUBSPACE FLAG AREA                          @LGA */
    unsigned char  _sdwaarch;     /* Copy of FLCARCH                     @PHC         */
    struct {
      struct {
        unsigned char  _sdwapcep[4]; /* PC ESTAE PARAM VALUE                @L4A */
        } sdwamst1;
      struct {
        unsigned char  _sdwapcea[4]; /* PC ESTAE PARAM ALET VALUE           @L4A */
        } sdwamst2;
      } sdwaprm2;
    int            _sdwalsed;     /* PTR TO LINKAGE STK ENTRY (CR15)     @D2C         */
    unsigned char  _sdwaclse[4];  /* Copy of PSACLHSE (Locks Held String Extension)   */
    unsigned char  _sdwalslv[2];  /* FOR RETRY: NUMBER OF BAKR ENTRIES PAST           */
    unsigned char  _sdwartam;     /* Retry Amode: 0 = "normal", 1 = AMODE 24,         */
    unsigned char  _filler1;      /* RESERVED                            @LxC         */
    unsigned char  _sdwastkn[8];  /* STOKEN OF THE SUBSPACE AT TIME OF ERROR - VALID  */
    unsigned char  _sdwasnm[8];   /* NAME OF THE SUBSPACE AT TIME OF ERROR - VALID    */
    unsigned char  _sdwasnam[8];  /* Name of the SYSTEM that this record was created  */
    } sdwaserv;
  double         sdwasend; /* END OF SERV EXTENSION OF SDWA           @G388P2F */
  };
 
#define sdwacid  sdwaserv.sdwarc1z._sdwacid
#define sdwasc   sdwaserv.sdwarc1z._sdwasc
#define sdwamdat sdwaserv.sdwarc1z.sdwamlvl._sdwamdat
#define sdwamvrs sdwaserv.sdwarc1z.sdwamlvl._sdwamvrs
#define sdwahrc  sdwaserv.sdwarc1z.sdwacrc._sdwahrc
#define sdwarrl  sdwaserv.sdwarc1z._sdwarrl
#define sdwacidb sdwaserv.sdwarc1z._sdwacidb
#define sdwasdrc sdwaserv.sdwarc1z._sdwasdrc
#define sdwaccrc sdwaserv.sdwarc1z._sdwaccrc
#define sdwaretf sdwaserv.sdwarc1z._sdwaretf
#define sdwatype sdwaserv.sdwarc1z._sdwatype
#define sdwahlhi sdwaserv.sdwarc1z._sdwahlhi
#define sdwasupr sdwaserv.sdwarc1z._sdwasupr
#define sdwaspn  sdwaserv.sdwarc1z._sdwaspn
#define sdwaeadr sdwaserv.sdwarc1z._sdwaeadr
#define sdwafrre sdwaserv.sdwarc1z._sdwafrre
#define sdwasdrn sdwaserv.sdwarc1z._sdwasdrn
#define sdwadaet sdwaserv.sdwarc1z.sdwadaew._sdwadaet
#define sdwaocur sdwaserv.sdwarc1z.sdwadaew._sdwaocur
#define sdwaasi1 sdwaserv.sdwarc1p.sdwapgta._sdwaasi1
#define sdwatcb  sdwaserv.sdwarc1p.sdwapgta._sdwatcb
#define sdwafain sdwaserv.sdwarc1p.sdwart12._sdwafain
#define sdwaascb sdwaserv.sdwarc1p.sdwart12._sdwaascb
#define sdwaasst sdwaserv.sdwarc1p.sdwart12._sdwaasst
#define sdwaoabf sdwaserv.sdwarc1p.sdwart12.sdwasabc._sdwaoabf
#define sdwaocmp sdwaserv.sdwarc1p.sdwart12.sdwasabc._sdwaocmp
#define sdwaocrc sdwaserv.sdwarc1p.sdwart12._sdwaocrc
#define sdwacre0 sdwaserv.sdwacrer._sdwacre0
#define sdwacre1 sdwaserv.sdwacrer._sdwacre1
#define sdwacre2 sdwaserv.sdwacrer._sdwacre2
#define sdwacre3 sdwaserv.sdwacrer._sdwacre3
#define sdwacre4 sdwaserv.sdwacrer._sdwacre4
#define sdwacre5 sdwaserv.sdwacrer._sdwacre5
#define sdwacre6 sdwaserv.sdwacrer._sdwacre6
#define sdwacre7 sdwaserv.sdwacrer._sdwacre7
#define sdwacre8 sdwaserv.sdwacrer._sdwacre8
#define sdwacre9 sdwaserv.sdwacrer._sdwacre9
#define sdwacrea sdwaserv.sdwacrer._sdwacrea
#define sdwacreb sdwaserv.sdwacrer._sdwacreb
#define sdwacrec sdwaserv.sdwacrer._sdwacrec
#define sdwacred sdwaserv.sdwacrer._sdwacred
#define sdwacree sdwaserv.sdwacrer._sdwacree
#define sdwacref sdwaserv.sdwacrer._sdwacref
#define sdwaare0 sdwaserv.sdwaarer._sdwaare0
#define sdwaare1 sdwaserv.sdwaarer._sdwaare1
#define sdwaare2 sdwaserv.sdwaarer._sdwaare2
#define sdwaare3 sdwaserv.sdwaarer._sdwaare3
#define sdwaare4 sdwaserv.sdwaarer._sdwaare4
#define sdwaare5 sdwaserv.sdwaarer._sdwaare5
#define sdwaare6 sdwaserv.sdwaarer._sdwaare6
#define sdwaare7 sdwaserv.sdwaarer._sdwaare7
#define sdwaare8 sdwaserv.sdwaarer._sdwaare8
#define sdwaare9 sdwaserv.sdwaarer._sdwaare9
#define sdwaarea sdwaserv.sdwaarer._sdwaarea
#define sdwaareb sdwaserv.sdwaarer._sdwaareb
#define sdwaarec sdwaserv.sdwaarer._sdwaarec
#define sdwaared sdwaserv.sdwaarer._sdwaared
#define sdwaaree sdwaserv.sdwaarer._sdwaaree
#define sdwaaref sdwaserv.sdwaarer._sdwaaref
#define sdwaars0 sdwaserv.sdwaarsv._sdwaars0
#define sdwaars1 sdwaserv.sdwaarsv._sdwaars1
#define sdwaars2 sdwaserv.sdwaarsv._sdwaars2
#define sdwaars3 sdwaserv.sdwaarsv._sdwaars3
#define sdwaars4 sdwaserv.sdwaarsv._sdwaars4
#define sdwaars5 sdwaserv.sdwaarsv._sdwaars5
#define sdwaars6 sdwaserv.sdwaarsv._sdwaars6
#define sdwaars7 sdwaserv.sdwaarsv._sdwaars7
#define sdwaars8 sdwaserv.sdwaarsv._sdwaars8
#define sdwaars9 sdwaserv.sdwaarsv._sdwaars9
#define sdwaarsa sdwaserv.sdwaarsv._sdwaarsa
#define sdwaarsb sdwaserv.sdwaarsv._sdwaarsb
#define sdwaarsc sdwaserv.sdwaarsv._sdwaarsc
#define sdwaarsd sdwaserv.sdwaarsv._sdwaarsd
#define sdwaarse sdwaserv.sdwaarsv._sdwaarse
#define sdwaarsf sdwaserv.sdwaarsv._sdwaarsf
#define sdwaduct sdwaserv._sdwaduct
#define sdwatear sdwaserv._sdwatear
#define sdwaxflg sdwaserv._sdwaxflg
#define sdwasflg sdwaserv._sdwasflg
#define sdwaarch sdwaserv._sdwaarch
#define sdwapcep sdwaserv.sdwaprm2.sdwamst1._sdwapcep
#define sdwapcea sdwaserv.sdwaprm2.sdwamst2._sdwapcea
#define sdwalsed sdwaserv._sdwalsed
#define sdwaclse sdwaserv._sdwaclse
#define sdwalslv sdwaserv._sdwalslv
#define sdwartam sdwaserv._sdwartam
#define sdwastkn sdwaserv._sdwastkn
#define sdwasnm  sdwaserv._sdwasnm
#define sdwasnam sdwaserv._sdwasnam
 
/* Values for field "sdwaccrc" */
#define sdwaccf              0x80 /* =1, IF RECOVERY EXIT ALTERED COMPCODE    @G860P1C */
#define sdwareaf             0x40 /* =1, IF RECOVERY EXIT ALTERED REASON CODE @G860P1C */
 
/* Values for field "sdwaretf" */
#define sdwart15             0x80 /* ON, SET REGISTER 15 ON RETRY TO THE VALUE IN      */
#define sdwaremr             0x40 /* ON, REMOVE RECOVERY ROUTINE ON RETRY              */
#define sdwafrlk             0x20 /* ON, FREE LOCKS ON A RETRY WHOSE BIT SETTINGS      */
#define sdwaup64             0x10 /* If on, use the 64-bit GPRs for                    */
#define sdwakeax             0x08 /* ON, when retrying keep the current EAX rather     */
#define sdwag64r             0x04 /* If on in a logrec record or SDWA in a dump,       */
 
/* Values for field "sdwatype" */
#define sdwanrec             0    /* NO RECOVERY WAS SET UP                      @L4A  */
#define sdwatfrr             1    /* FRR WAS GIVEN CONTROL                       @L4A  */
#define sdwatest             2    /* ESTAE/I/X WAS GIVEN CONTROL                 @L4A  */
#define sdwatarr             3    /* ARR WAS GIVEN CONTROL                       @L4A  */
 
/* Values for field "sdwaoabf" */
#define sdwaoreq             0x80 /* ORIGINAL VALUE OF SDWAREQ           @L1A          */
#define sdwaostp             0x40 /* ORIGINAL VALUE OF SDWASTEP          @L1A          */
#define sdwaostc             0x10 /* ORIGINAL VALUE OF SDWASTCC          @L1A          */
#define sdwaorcf             0x04 /* VALID REASON CODE IN SDWAOCRC.      @L1A          */
 
/* Values for field "sdwatear" */
#define sdwatean             0x0F /* Actual bits for AR number           @LMA          */
 
/* Values for field "sdwaxflg" */
#define sdwaintf             0x80 /* ON, SDWAEC2, SDWASRSV, AND SDWAARSV ARE           */
#define sdwateav             0x40 /* ON, SDWATRAN CONTAINS A VALID ADDRESS       @L8A  */
#define sdwateiv             0x20 /* ON, SDWATRAN CONTAINS A VALID ASID          @L8A  */
#define sdwaestx             0x10 /* ON, IF SDWATYPE = SDWATEST, THE RECOVERY    @LEA  */
#define sdwatepc             0x08 /* ON, SDWATRAN CONTAINS A VALID PC number     @LIA  */
#define sdwatirr             0x04 /* On, if SDWATYPE = SDWATARR, the recovery          */
#define sdwasval             0x02 /* On, the state of SDWASRBS is valid          @PNA  */
#define sdwareleasecodevalid 0x01 /* On, indicates that the abended RB                 */
 
/* Values for field "sdwasflg" */
#define sdwasvld             0x80 /* ON IF SUBSPACE INFORMATION AT TIME OF ERROR       */
#define sdwassa              0x40 /* ON IF A SUBSPACE WAS ACTIVE AT TIME OF ERROR      */
#define sdwabsa              0x02 /* Indicates that Reduced Authority (set via the     */
#define sdwassrs             0x01 /* TURNED ON BY AN ESTAE-TYPE RECOVERY ROUTINE       */
 
/* Values for field "sdwaarch" */
#define sdwazarc             0x01 /* Copy of PSAZARCH                    @PHA          */
#define sdwaesam             0x01 /* Copy of PSAZARCH                    @PHA          */
 
/* Values for field "sdwartam" */
#define sdwarasr             0    /* Retry using default AMODE system rules      @06A  */
#define sdwara24             1    /* Retry to AMODE 24 specifically              @06A  */
#define sdwara31             2    /* Retry to AMODE 31 specifically              @06A  */
#define sdwara64             3    /* Retry to AMODE 64 specifically              @06A  */
 
#endif
 
#ifndef __sdwarc2__
#define __sdwarc2__
 
struct sdwarc2 {
  struct {
    struct {
      unsigned char  _sdwarfsh[4]; /* High half of FSA (zero pre-z/Architecture)  @LIA */
      unsigned char  _sdwarfsl[4]; /* Low half of FSA                             @LIA */
      } sdwarfse;
    unsigned char  _sdwamcic[8]; /* MACHINE CHECK INTERRUPT CODE            @G860P1C */
    } sdwaioma;
  double         sdwaiend; /* END OF SDWAIOMA EXTENSION OF SDWA       @G860P1C */
  };
 
#define sdwarfsh sdwaioma.sdwarfse._sdwarfsh
#define sdwarfsl sdwaioma.sdwarfse._sdwarfsl
#define sdwamcic sdwaioma._sdwamcic
 
#endif
 
#ifndef __sdwarc3__
#define __sdwarc3__
 
struct sdwarc3 {
  struct {
    unsigned char  _sdwaflk1;    /* FLAGS INDICATING WHAT LOCKS ARE TO      @G860P1C */
    unsigned char  _sdwaflk2;    /* FLAGS INDICATING WHAT LOCKS ARE TO      @G860P1C */
    struct {
      unsigned char  _sdwafle1; /* FLAGS FOR LOCKS TO BE FREED IN FIRST BYTE OF  */
      unsigned char  _sdwafle2; /* FLAGS FOR LOCKS TO BE FREED IN SECOND BYTE OF */
      unsigned char  _sdwafle3; /* FLAGS FOR LOCKS TO BE FREED IN THIRD BYTE OF  */
      unsigned char  _sdwafle4; /* FLAGS FOR LOCKS TO BE FREED IN FOURTH BYTE OF */
      } sdwaflke;
    unsigned char  _filler1[2];  /* RESERVED                                    @L9A */
    int            _sdwalrsg;    /* LOCKWORD ADDR FOR THE RSMGL  LOCK       @G860P1C */
    int            _sdwalasg;    /* LOCKWORD ADDR FOR THE ASMGL  LOCK       @G860P1C */
    int            _sdwalrss;    /* LOCKWORD ADDR FOR THE RSMST  LOCK       @G860P1C */
    int            _sdwalrsx;    /* LOCKWORD ADDR FOR THE RSMXM  LOCK       @G860P1C */
    int            _sdwalrsa;    /* LOCKWORD ADDR FOR THE RSMAD  LOCK       @G860P1C */
    int            _sdwalrsc;    /* LOCKWORD ADDR FOR THE RSMCM  LOCK       @G860P1S */
    } sdwaflck;
  double         sdwalend; /* END OF SDWAFLCK EXTENSION OF SDWA       @G860P1C */
  };
 
#define sdwaflk1 sdwaflck._sdwaflk1
#define sdwaflk2 sdwaflck._sdwaflk2
#define sdwafle1 sdwaflck.sdwaflke._sdwafle1
#define sdwafle2 sdwaflck.sdwaflke._sdwafle2
#define sdwafle3 sdwaflck.sdwaflke._sdwafle3
#define sdwafle4 sdwaflck.sdwaflke._sdwafle4
#define sdwalrsg sdwaflck._sdwalrsg
#define sdwalasg sdwaflck._sdwalasg
#define sdwalrss sdwaflck._sdwalrss
#define sdwalrsx sdwaflck._sdwalrsx
#define sdwalrsa sdwaflck._sdwalrsa
#define sdwalrsc sdwaflck._sdwalrsc
 
/* Values for field "sdwaflk1" */
#define sdwafcpu 0x80 /* ON, FREE THE CPU LOCK                   @G860P1S */
#define sdwafrsm 0x08 /* ON, FREE THE RSM LOCK                   @G860P1S */
#define sdwaftrc 0x04 /* ON, FREE THE TRACE LOCK                 @G860P1S */
#define sdwaiocb 0x02 /* ON, THE IOS LOCK                            @P4M */
 
/* Values for field "sdwaflk2" */
#define sdwafrsc 0x10 /* ON, FREE THE RSM COMMON CLASS LOCK      @G860P1S */
#define sdwafrsg 0x08 /* ON, FREE THE RSM GLOBAL CLASS LOCK      @G860P1S */
#define sdwafvsf 0x04 /* ON, FREE THE VSM FIX LOCK               @G860P1S */
#define sdwafasg 0x02 /* ON, FREE THE ASM GLOBAL CLASS LOCK      @G860P1S */
#define sdwafrss 0x01 /* ON, FREE THE RSM STEAL CLASS LOCK       @G860P1S */
 
/* Values for field "sdwafle1" */
#define sdwablsd 0x80 /* ON, FREE THE BMFLSD LOCK                    @LCA */
#define sdwaxds  0x40 /* ON, FREE THE XCFDS LOCK                     @LBA */
#define sdwaxres 0x20 /* ON, FREE THE XCFRES LOCK                    @LBA */
#define sdwaxq   0x10 /* ON, FREE THE XCFQ LOCK                      @LBA */
#define sdwaeset 0x08 /* ON, FREE THE ETRSET LOCK                    @LAA */
#define sdwaixsc 0x04 /* ON, FREE THE IXLSCH  LOCK                   @LFA */
#define sdwaixsr 0x02 /* ON, FREE THE IXLSHR  LOCK                   @LFA */
#define sdwaixds 0x01 /* ON, FREE THE IXLDS   LOCK                   @LFA */
 
/* Values for field "sdwafle2" */
#define sdwaixsh 0x80 /* ON, FREE THE IXLSHELL LOCK                  @LFA */
#define sdwaulut 0x40 /* ON, FREE THE IOSULUT LOCK                   @LDA */
#define sdwaixre 0x20 /* ON, FREE THE IXLREQST LOCK                  @03A */
#define sdwawlmr 0x10 /* On, free the WLMRES lock                    @PFC */
#define sdwawlmq 0x08 /* On, free the WLMQ lock                      @PEC */
#define sdwacntx 0x04 /* On, free the CONTEXT lock                   @PFC */
#define sdwargsv 0x02 /* On, free the REGSRV lock                    @LJA */
#define sdwassd  0x01 /* On, free the SSD lock                       @LLA */
 
/* Values for field "sdwafle3" */
#define sdwagrsi 0x80 /* On, free the GRSINT lock                    @LOA */
#define sdwamisl 0x40 /* On, free the MISC lock                      @LWA */
#define sdwaslk1 0x40 /* N/A                                         @LWC */
#define sdwadnu2 0x20 /* N/A                                         @LWA */
#define sdwanlk1 0x20 /* N/A                                         @LWC */
#define sdwadnu3 0x10 /* N/A                                         @LWA */
#define sdwaolk1 0x10 /* N/A                                         @LWC */
#define sdwadnu4 0x08 /* N/A                                         @LWA */
#define sdwaxlk1 0x08 /* N/A                                         @LWC */
#define sdwadnu5 0x04 /* N/A                                         @LWA */
#define sdwarlk3 0x04 /* N/A                                         @LWC */
#define sdwarlk2 0x02 /* On, free the HCWDRLK2 lock                  @LPA */
#define sdwarlk1 0x01 /* On, free the HCWDRLK1 lock                  @LPA */
 
/* Values for field "sdwafle4" */
#define sdwasrme 0x80 /* On, free the SRMENQ lock                    @LTA */
#define sdwassdg 0x40 /* On, free the SSDGROUP lock                  @LYA */
 
#endif
 
#ifndef __sdwarc4__
#define __sdwarc4__
 
struct sdwarc4 {
  struct {
    struct {
      double         _sdwag6400; /* Register 0                                  @LIA */
      double         _sdwag6401; /* Register 1                                  @LIA */
      double         _sdwag6402; /* Register 2                                  @LIA */
      double         _sdwag6403; /* Register 3                                  @LIA */
      double         _sdwag6404; /* Register 4                                  @LIA */
      double         _sdwag6405; /* Register 5                                  @LIA */
      double         _sdwag6406; /* Register 6                                  @LIA */
      double         _sdwag6407; /* Register 7                                  @LIA */
      double         _sdwag6408; /* Register 8                                  @LIA */
      double         _sdwag6409; /* Register 9                                  @LIA */
      double         _sdwag6410; /* Register 10                                 @LIA */
      double         _sdwag6411; /* Register 11                                 @LIA */
      double         _sdwag6412; /* Register 12                                 @LIA */
      double         _sdwag6413; /* Register 13                                 @LIA */
      double         _sdwag6414; /* Register 14                                 @LIA */
      double         _sdwag6415; /* Register 15                                 @LIA */
      } sdwatx___pitdb___g64;
    } sdwag64;
  unsigned char  _filler1;           /* Reserved                                    @07A */
  unsigned char  sdwareleasecode[3]; /* Release code when the abended RB level was       */
  unsigned char  _filler2[4];        /* Reserved                                    @07C */
  struct {
    unsigned char  _sdwatx___pitdb___g64h[64]; /* Same as SDWAG64H                         @LZA */
    } sdwag64h;
  struct {
    unsigned char  _sdwac640[8]; /* z/Architecture CR0 at time of error         @LMA */
    unsigned char  _sdwac641[8]; /* z/Architecture CR1 at time of error         @LMA */
    unsigned char  _sdwac642[8]; /* z/Architecture CR2 at time of error         @LMA */
    struct {
      struct {
        unsigned char  _filler3[4];          /* @LMA                                             */
        unsigned char  _sdwac643___km[2];    /* Key Mask                                    @LMA */
        unsigned char  _sdwac643___sasid[2]; /* Secondary ASID                              @LMA */
        } sdwac643;
      struct {
        unsigned char  _filler4[4];          /* @LMA                                             */
        unsigned char  _sdwac644___ax[2];    /* Authorization index                         @LMA */
        unsigned char  _sdwac644___pasid[2]; /* Primary ASID                                @LMA */
        } sdwac644;
      } sdwac64___xm;
    unsigned char  _sdwac645[8]; /* z/Architecture CR5 at time of error         @LMA */
    unsigned char  _sdwac646[8]; /* z/Architecture CR6 at time of error         @LMA */
    unsigned char  _sdwac647[8]; /* z/Architecture CR7 at time of error         @LMA */
    unsigned char  _sdwac648[8]; /* z/Architecture CR8 at time of error         @LMA */
    unsigned char  _sdwac649[8]; /* z/Architecture CR9 at time of error         @LMA */
    unsigned char  _sdwac64a[8]; /* z/Architecture CRA at time of error         @LMA */
    unsigned char  _sdwac64b[8]; /* z/Architecture CRB at time of error         @LMA */
    unsigned char  _sdwac64c[8]; /* z/Architecture CRC at time of error         @LMA */
    unsigned char  _sdwac64d[8]; /* z/Architecture CRD at time of error         @LMA */
    unsigned char  _sdwac64e[8]; /* z/Architecture CRE at time of error         @LMA */
    unsigned char  _sdwac64f[8]; /* z/Architecture CRF at time of error         @LMA */
    } sdwac64s;
  struct {
    unsigned char  _sdwatrnehigh[4]; /* 8-byte TEA upper half                       @M0A */
    unsigned char  _sdwatrnelow[4];  /* 8-byte TEA lower half                       @M0A */
    } sdwatrne;
  unsigned char  sdwabea[8];         /* Breaking Event Address                      @LNA */
  struct {
    unsigned char  _sdwatx___pitdb___psw16[16]; /* Same as SDWAPSW16                       @LZA */
    } sdwapsw16;
  double         sdwaeend;           /* End of 64-bit extension of the SDWA         @LIA */
  };
 
#define sdwag6400              sdwag64.sdwatx___pitdb___g64._sdwag6400
#define sdwag6401              sdwag64.sdwatx___pitdb___g64._sdwag6401
#define sdwag6402              sdwag64.sdwatx___pitdb___g64._sdwag6402
#define sdwag6403              sdwag64.sdwatx___pitdb___g64._sdwag6403
#define sdwag6404              sdwag64.sdwatx___pitdb___g64._sdwag6404
#define sdwag6405              sdwag64.sdwatx___pitdb___g64._sdwag6405
#define sdwag6406              sdwag64.sdwatx___pitdb___g64._sdwag6406
#define sdwag6407              sdwag64.sdwatx___pitdb___g64._sdwag6407
#define sdwag6408              sdwag64.sdwatx___pitdb___g64._sdwag6408
#define sdwag6409              sdwag64.sdwatx___pitdb___g64._sdwag6409
#define sdwag6410              sdwag64.sdwatx___pitdb___g64._sdwag6410
#define sdwag6411              sdwag64.sdwatx___pitdb___g64._sdwag6411
#define sdwag6412              sdwag64.sdwatx___pitdb___g64._sdwag6412
#define sdwag6413              sdwag64.sdwatx___pitdb___g64._sdwag6413
#define sdwag6414              sdwag64.sdwatx___pitdb___g64._sdwag6414
#define sdwag6415              sdwag64.sdwatx___pitdb___g64._sdwag6415
#define sdwatx___pitdb___g64h  sdwag64h._sdwatx___pitdb___g64h
#define sdwac640               sdwac64s._sdwac640
#define sdwac641               sdwac64s._sdwac641
#define sdwac642               sdwac64s._sdwac642
#define sdwac643___km          sdwac64s.sdwac64___xm.sdwac643._sdwac643___km
#define sdwac643___sasid       sdwac64s.sdwac64___xm.sdwac643._sdwac643___sasid
#define sdwac644___ax          sdwac64s.sdwac64___xm.sdwac644._sdwac644___ax
#define sdwac644___pasid       sdwac64s.sdwac64___xm.sdwac644._sdwac644___pasid
#define sdwac645               sdwac64s._sdwac645
#define sdwac646               sdwac64s._sdwac646
#define sdwac647               sdwac64s._sdwac647
#define sdwac648               sdwac64s._sdwac648
#define sdwac649               sdwac64s._sdwac649
#define sdwac64a               sdwac64s._sdwac64a
#define sdwac64b               sdwac64s._sdwac64b
#define sdwac64c               sdwac64s._sdwac64c
#define sdwac64d               sdwac64s._sdwac64d
#define sdwac64e               sdwac64s._sdwac64e
#define sdwac64f               sdwac64s._sdwac64f
#define sdwatrnehigh           sdwatrne._sdwatrnehigh
#define sdwatrnelow            sdwatrne._sdwatrnelow
#define sdwatx___pitdb___psw16 sdwapsw16._sdwatx___pitdb___psw16
 
#endif
 
#ifndef __sdwarc5__
#define __sdwarc5__
 
struct sdwarc5 {
  struct {
    unsigned char  _sdwatxg64[128]; /* Same as SDWATX_ABORT_G64 */
    } sdwatx___abort___g64;
  struct {
    unsigned char  _sdwatxpsw16[16]; /* Same as SDWATX_ABORT_PSW16 */
    } sdwatx___abort___psw16;
  double         sdwa5end; /* End of SDWARC5                              @LUA */
  };
 
#define sdwatxg64   sdwatx___abort___g64._sdwatxg64
#define sdwatxpsw16 sdwatx___abort___psw16._sdwatxpsw16
 
#endif
 
#ifndef __sdwaptrs__
#define __sdwaptrs__
 
struct sdwaptrs {
  void * __ptr32 sdwadsrp; /* ADDR DUMP STORAGE RANGES PTR. - SDWANRC1    @L1C */
  void * __ptr32 sdwasrvp; /* ADDR ADDITIONAL COMP SERV DATA - SDWARC1    @L1C */
  void * __ptr32 sdwaxiom; /* ADDR OF I/O MACHINE CHECK AREA - SDWARC2    @L1C */
  void * __ptr32 sdwaxspl; /* ADDR OF STORAGE SUBPOOLS AREA - SDWANRC2    @L1C */
  void * __ptr32 sdwaxlck; /* ADDR ADDITIONAL FRELOCK DATA - SDWARC3      @01M */
  void * __ptr32 sdwadspp; /* DATA SPACE STORAGE RANGES POINTER - SDWANRC3     */
  void * __ptr32 sdwaxeme; /* Addr 64-bit information - SDWARC4           @LIA */
  void * __ptr32 sdwaxrc5; /* Addr SDWARC5                                @LUA */
  double         sdwapend; /* END OF PTRS EXTENSION OF SDWA           @G38FP2F */
  };
 
#endif
 
#ifndef __sdwanrc1__
#define __sdwanrc1__
 
struct sdwanrc1 {
  unsigned char  sdwadsr[240]; /* DUMP STORAGE RANGES                     @G382P2F */
  double         sdwarend;     /* END OF DSR EXTENSION OF SDWA            @G382P2F */
  };
 
#endif
 
#ifndef __sdwanrc2__
#define __sdwanrc2__
 
struct sdwanrc2 {
  struct {
    short int      _sdwaspln;    /* NUMBER OF SUBPOOLS TO BE DUMPED         @G860P1C */
    short int      _sdwaspls[7]; /* IDS OF SUBPOOLS TO BE DUMPED            @G860P1C */
    } sdwasple;
  double         sdwasen; /* END OF SDWASPLS EXTENSION OF SDWA       @G860P1C */
  };
 
#define sdwaspln sdwasple._sdwaspln
#define sdwaspls sdwasple._sdwaspls
 
#endif
 
#ifndef __sdwanrc3__
#define __sdwanrc3__
 
struct sdwanrc3 {
  struct {
    unsigned char  _sdwadxsr[15][16]; /* DUMPOPX RANGE (UP TO 15)       @L7A */
    } sdwadxsl;
  double         sdwadend; /* @L7A */
  };
 
#define sdwadxsr sdwadxsl._sdwadxsr
 
/* Values for field "sdwadend" */
#define sdwadxmx 15    /* UP TO 15 DATA SPACE STORAGE RANGES MAY BE */
#define sdwalen  0x298 /* LENGTH OF SDWA                            */
#define sdwaplen 0x20  /* LENGTH OF PTRS EXTENSION      @G381P2F    */
#define sdwarlen 0xF0  /* LENGTH OF DSR EXTENSION       @G382P2F    */
#define sdwaclen 0x1C8 /* LENGTH OF SERV EXTENSION      @G388P2F    */
#define sdwailen 0x10  /* LENGTH OF IOMA EXTENSION      @G860P1C    */
#define sdwallen 0x20  /* LENGTH OF FRELOCK EXTENSION   @G860P1C    */
#define sdwaspl  0x10  /* LENGTH OF SUBPOOL EXTENSION   @G860P1C    */
#define sdwadlen 0xF0  /* LENGTH OF EXTENSION FOR DATA SPACE        */
#define sdwanlns 0x1F0 /* Non-recordable extensions  @LIA           */
#define sdwaelen 0x168 /* Length of z/Architecture extension @LIA   */
#define sdwarc5l 0x90  /* Length of SDWARC5                  @LUA   */
#define sdwarlns 0x3F0 /* @LUC                                      */
#define sdwamlnp 0x688 /* @LUC                                      */
#define sdwamlen 0x6A8 /* @LIC                                      */
#define sdwatlen 0x898 /* @LIC                                      */
#define sdwaolen 0x6A0 /* @LUC                                      */
#define sdwaslen 0x7B8 /* @LIC                                      */
#define sdwaflen 0x9A8 /* @LIC                                      */
#define sdwanopr 0     /* THIS FIELD IS ONLY DEFINED IN             */
 
#endif
 
#ifndef __vramap__
#define __vramap__
 
struct vramap {
  struct {
    char           _vrakey; /* KEY TO IDENTIFY THE DATA THAT FOLLOWS. THE */
    char           _vralen; /* LENGTH OF THE DATA THAT FOLLOWS.  THE      */
    } vrakl;
  char           vradat; /* VARIABLE LENGTH DATA. THIS DATA IS FOLLOWED */
  };
 
#define vrakey vrakl._vrakey
#define vralen vrakl._vralen
 
/* Values for field "vradat" */
#define vralenkl 0x02 /* LENGTH OF THE VRAKL FIELD (VRAKEY AND            */
#define vracom   1    /* THE VRADAT DATA IS THE 5-BYTE EBCDIC COMPONENT   */
#define vrasc    2    /* THE DATA IS EBCDIC TEXT TO IDENTIFY THE          */
#define vralvl   3    /* THE DATA IS THE EBCDIC LEVEL FOR THE FAILING     */
#define vradt    4    /* THE DATA IS THE EBCDIC ASSEMBLY DATE FOR THE     */
#define vraptf   5    /* THE DATA IS THE 7-BYTE EBCDIC PTF, SU, OR        */
#define vrarc    6    /* THE DATA IS A HEXADECIMAL RETURN OR REASON       */
#define vraqvod  7    /* THE DATA IS THE REGISTER 15 AND ERROR PORTIONS   */
#define vraqerr  8    /* THIS KEY INDICATES A QUEUE ERROR FOR THE         */
#define vralvls  9    /* THE DATA IS THE EBCDIC SYSTEM RELEASE OR         */
#define vrarrp   16   /* ('10'X) THE DATA IS THE HEXADECIMAL RECOVERY     */
#define vracbm   17   /* ('11'X) THE DATA IS THE MAPPING MACRO NAME       */
#define vracb    18   /* ('12'X) THE DATA IS THE HEXADECIMAL CONTENTS     */
#define vracbf   19   /* ('13'X) THE DATA IS THE NAME OF A CONTROL        */
#define vracba   20   /* ('14'X) THE DATA IS THE 4 BYTE ADDRESS OF        */
#define vracbo   21   /* ('15'X) THE DATA IS THE OFFSET OF A CONTROL      */
#define vracbl   22   /* ('16'X) THE DATA IS THE LENGTH OF THE CONTROL    */
#define vracbi   24   /* ('18'X) THE DATA IS A ONE BYTE CONTROL BLOCK     */
#define vracbia  25   /* ('19'X) THE DATA IS A ONE BYTE C.B. ID NUMBER    */
#define vracbi2  26   /* ('1A'X) THE DATA IS A ONE BYTE CONTROL BLOCK     */
#define vrapli   32   /* ('20'X) THE DATA IS EBCDIC TEXT TO IDENTIFY      */
#define vrapl    33   /* ('21'X) THE DATA IS THE HEXADECIMAL CONTENTS     */
#define vrafpi   34   /* ('22'X) THE DATA IS EBCDIC TEXT TO IDENTIFY      */
#define vrafp    35   /* ('23'X) THE DATA IS THE HEXADECIMAL CONTENTS     */
#define vrapa    36   /* ('24'X) THE DATA DESCRIBES THE EXECUTION PATH    */
#define vrap2    37   /* ('25'X) THE DATA DESCRIBES THE EXECUTION         */
#define vralk    38   /* ('26'X) THE DATA IS THE EBCDIC NAME OF A         */
#define vrawai   39   /* ('27'X) THE DATA IS EBCDIC TEXT TO IDENTIFY      */
#define vrawa    40   /* ('28'X) THE DATA IS THE HEXADECIMAL CONTENTS     */
#define vrawap   41   /* ('29'X) THE DATA IS THE ADDRESS OF A WORK AREA   */
#define vralbl   48   /* ('30'X) THE DATA IS AN EBCDIC LABEL              */
#define vrarrl   49   /* ('31'X) THE DATA IS THE LABEL OF THE             */
#define vramid   51   /* ('33'X) THE DATA IS AN EBCDIC MESSAGE ID FOR     */
#define vramsg   52   /* ('34'X) THE DATA IS EBCDIC MESSAGE TEXT FOR      */
#define vraerr   53   /* ('35'X) THE DATA IS EBCDIC INFORMATION ABOUT     */
#define vraehx   54   /* ('36'X) THE DATA IS HEXADECIMAL INFORMATION      */
#define vrahid   55   /* ('37'X) THE DATA IS AN EBCDIC HEADER TO          */
#define vrahex   56   /* ('38'X) THE DATA IS HEXADECIMAL INFORMATION      */
#define vraebc   57   /* ('39'X) THE DATA IS EBCDIC INFORMATION           */
#define vraaid   58   /* ('3A'X) THE DATA IS THE 2-BYTE HEXADECIMAL       */
#define vratcb   59   /* ('3B'X) THE DATA IS THE ADDRESS OF THE TCB       */
#define vraca    60   /* ('3C'X) THE DATA IS THE ADDRESS OF THE CALLER    */
#define vracan   61   /* ('3D'X) THE DATA IS THE NAME OF THE MODULE       */
#define vraoa    64   /* ('40'X) THE DATA IS THE ORIGINAL HEXADECIMAL     */
#define vrapsw   65   /* ('41'X) THE DATA IS THE PSW FROM THE ORIGINAL    */
#define vrains   66   /* ('42'X) THE DATA IS THE FAILING INSTRUCTION      */
#define vraregs  67   /* ('43'X) THE DATA IS THE GENERAL PURPOSE          */
#define vrarega  68   /* ('44'X) THE DATA IS THE ADDRESS OF AN AREA       */
#define vraor15  69   /* ('45'X) THE DATA IS REGISTER 15 AT THE TIME      */
#define vradsn   70   /* ('46'X) THE DATA IS THE EBCDIC NAME OF A         */
#define vradev   71   /* ('47'X) THE DATA IS THE EBCDIC NAME OF A DEVICE  */
#define vrasn    72   /* ('48'X) THE DATA IS HEXADECIMAL I/O SENSE DATA   */
#define vrast    73   /* ('49'X) THE DATA IS HEXADECIMAL I/O STATUS DATA  */
#define vrau     74   /* ('4A'X) THE DATA IS AN EBCDIC UNIT ADDRESS OR    */
#define vraccw   75   /* ('4B'X) THE DATA IS THE HEXADECIMAL CCW FOR      */
#define vracsw   76   /* ('4C'X) THE DATA IS THE HEXADECIMAL CSW FOR      */
#define vradvt   77   /* ('4D'X) THE DATA IS HEXADECIMAL DEVICE TYPE      */
#define vravol   78   /* ('4E'X) THE DATA IS AN EBCDIC VOLUME SERIAL      */
#define vrareq   80   /* ('50'X) THE DATA IS ONE OR MORE KEYS WHICH ARE   */
#define vraopt   81   /* ('51'X) THE DATA IS ONE OR MORE KEYS WHICH, IF   */
#define vraminsc 82   /* ('52'X) THE DATA IS A 2 BYTE MINIMUM COUNT OF    */
#define vradae   83   /* ('53'X) NO DATA IS ASSOCIATED WITH THIS KEY.     */
#define vraminsl 84   /* ('54'X) THE DATA IS A 2 BYTE MINIMUM LENGTH      */
#define vrafreg  96   /* ('60'X) THE DATA IS A 1 BYTE REGISTER NUMBER     */
#define vracscb  99   /* ('63'X) THE DATA IS THE CSCB CONTROL BLOCK WITH  */
#define vracscba 100  /* ('64'X) THE DATA IS THE ADDRESS OF THE CSCB      */
#define vrajob   101  /* ('65'X) THE DATA IS THE JOBNAME THAT FAILED.     */
#define vrastp   102  /* ('66'X) THE DATA IS THE STEPNAME THAT FAILED     */
#define vracmd   103  /* ('67'X) THE DATA IS AN EBCDIC TSO COMMAND OR     */
#define vrajcl   104  /* ('68'X) THE DATA IS A JCL STATEMENT              */
#define vranodae 105  /* ('69'X) NO DATA IS ASSOCIATED WITH THIS KEY.     */
#define vraepn   115  /* ('73'X) THE DATA IS THE NAME OF THE ENTRY        */
#define vraetf   119  /* ('77'X) THE DATA IS THE ADDRESS OF THE ENTRY     */
#define vractf   120  /* ('78'X) THE DATA IS THE ADDRESS OF THE CSECT     */
#define vraltf   121  /* ('79'X) THE DATA IS THE ADDRESS OF THE LOAD      */
#define vramo    122  /* ('7A'X) THE DATA IS THE HEXADECIMAL OFFSET       */
#define vrailo   123  /* ('7B'X) THE DATA IS THE HEXADECIMAL OFFSET OF    */
#define vraimo   124  /* ('7C'X) THE DATA IS THE HEXADECIMAL OFFSET OF    */
#define vrafid   125  /* ('7D'X) THE DATA IS THE EBCDIC FEATURE ID FOR    */
#define vrapid   126  /* ('7E'X) THE DATA IS THE EBCDIC PRODUCT ID FOR    */
#define vraiap   160  /* ('A0'X) THE DATA IS THE NAME OF AN ANALYTIC      */
#define vraial   161  /* ('A1'X) THE DATA IS A PARAMETER LIST FOR USE BY  */
#define vraicl   162  /* ('A2'X) THE DATA IS A PARAMETER LIST FOR USE IN  */
#define vraidp   163  /* ('A3'X) THE DATA IS THE NAME OF THE DUMP (OR     */
#define vralkwa  164  /* ('A4'X) THE DATA IS THE ADDRESS OF THE LOCKWORD  */
#define vrarrk   200  /* ('C8'X) THIS KEY AND KEYS 201 THRU 239 ('EF'X)   */
#define vrarrk1  201  /* ('C9'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk2  202  /* ('CA'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk3  203  /* ('CB'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk4  204  /* ('CC'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk5  205  /* ('CD'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk6  206  /* ('CE'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk7  207  /* ('CF'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk8  208  /* ('D0'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk9  209  /* ('D1'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk10 210  /* ('D2'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk11 211  /* ('D3'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk12 212  /* ('D4'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk13 213  /* ('D5'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk14 214  /* ('D6'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk15 215  /* ('D7'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk16 216  /* ('D8'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk17 217  /* ('D9'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk18 218  /* ('DA'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk19 219  /* ('DB'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk20 220  /* ('DC'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk21 221  /* ('DD'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk22 222  /* ('DE'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk23 223  /* ('DF'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk24 224  /* ('E0'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk25 225  /* ('E1'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk26 226  /* ('E2'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk27 227  /* ('E3'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk28 228  /* ('E4'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk29 229  /* ('E5'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk30 230  /* ('E6'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk31 231  /* ('E7'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk32 232  /* ('E8'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk33 233  /* ('E9'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk34 234  /* ('EA'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk35 235  /* ('EB'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk36 236  /* ('EC'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk37 237  /* ('ED'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk38 238  /* ('EE'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk39 239  /* ('EF'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vraskp   250  /* ('FA'X) THIS KEY CAN BE USED TO SKIP TO A        */
#define vraend   255  /* ('FF'X) THE DATA FROM THIS KEY FIELD TO THE      */
#define efabs    1001 /* ('3E9'X) THE DATA IS THE SYSTEM ABEND CODE. @L1A */
#define efabu    1002 /* ('3EA'X) THE DATA IS THE USER ABEND CODE.   @L1A */
#define efldmd   1003 /* ('3EB'X) THE DATA IS THE FAILING LOAD MODULE     */
#define efcsct   1004 /* ('3EC'X) THE DATA IS THE FAILING CSECT NAME.@L1A */
#define efrexn   1005 /* ('3E9'X) THE DATA IS THE RECOVERY ROUTINE        */
#define efpsw    1011 /* ('3E9'X) THE DATA IS THE PSW REGISTER            */
#define e1c1d1c  1101 /* THIS KEY SHOULD NOT BE USED. IT IS RETAINED FOR  */
#define e1cid1c  1101 /* ('44D'X) THE DATA IS THE COMPONENT ID.      @01A */
#define e1sub1c  1102 /* ('44E'X) THE DATA IS THE SUBFUNCTION.       @L1A */
#define e1amd1c  1105 /* ('451'X) THE DATA IS THE ASSEMBLY DATE OF        */
#define e1vrs1c  1106 /* ('452'X) THE DATA IS THE VERSION OF THE          */
#define e1hrc1c  1108 /* ('454'X) THE DATA IS THE REASON OR RETURN        */
#define e1rrl1c  1110 /* ('456'X) THE DATA IS THE LABEL OF THE            */
#define e1cdb1c  1114 /* ('45A'X) THE DATA IS THE COMPONENT ID BASE       */
#define e1ccr1c  1116 /* ('45C'X) THE DATA ARE PROGRAM STATUS             */
#define e1hlh1c  1118 /* ('45E'X) COPY OF PSAHLHI-HIGHEST LOCK HELD       */
#define e1sup1c  1120 /* ('460'X) COPY OF PSASUPER (SUPERVISOR CONTROL    */
#define e1spn1c  1124 /* ('464'X) COPY OF LCCASPIN (PROCESSOR IS          */
#define e1fi1c   1126 /* ('466'X) THE DATA ARE THE 12 BYTES OF THE        */
#define e1frr1c  1128 /* ('468'X) THE DATA IS A COPY OF THE FRR PARAMETER */
#define e1asid1c 1130 /* ('46A'X) THE DATA IS THE ASID OF THE FAILING     */
#define e1orcc1c 1132 /* ('46C'X) THE DATA IS THE ORIGINAL COMPLETION     */
#define e1orrc1c 1134 /* ('46E'X) THE DATA IS THE ORIGINAL REASON         */
#define e1pids1c 1136 /* ('470'X) THE DATA IS THE PRODUCT/COMPONENT ID.   */
#define e2mcic   1203 /* ('4B3'X) THE DATA IS THE MACHINE CHECK           */
#define rinvld   0    /* ('0'X)   INVALID SYMPTOM.                   @L1A */
#define rabndsr  1    /* ('01'X) THE DATA IS THE SYSTEM ABEND CODE.  @L1A */
#define rabndur  2    /* ('02'X) THE DATA IS THE USER ABEND CODE.    @L1A */
#define rfldsr   3    /* ('03'X) THE DATA IS A FIELD NAME OR LABEL.  @L1A */
#define rlvlsr   4    /* ('04'X) THE DATA IS THE COMPONENT, SU, PP,       */
#define rmsgidr  5    /* ('05'X) MESSAGE IDENTIFIER.                 @L1A */
#define radrsr   6    /* ('06'X) ADDRESS OR OFFSET.                  @L1A */
#define rpcssr   7    /* ('07'X) THE DATA IS JCL, AN OPERATOR COMMAND     */
#define rpidsr   8    /* ('08'X) THE DATA IS A COMPONENT IDENTIFIER AS    */
#define rprcsr   9    /* ('09'X) THE DATA IS THE RETURN OR REASON         */
#define rptfsrr  10   /* ('0A'X) THE DATA IS A PTF IDENTIFIER.       @L1A */
#define rpzfsr   11   /* ('0B'X) THE DATA IS A SUPERZAP IDENTIFIER.  @L1A */
#define rregsr   12   /* ('0C'X) THE DATA ARE THE CONTENTS OF THE         */
#define rridsr   13   /* ('0D'X) MODULE, CSECT, ROUTINE NAME,             */
#define rstatr   14   /* ('0E'X) CSW, DSW STATUS.                    @L1A */
#define rvaluhr  15   /* ('0F'X) THE DATA IS HEXADECIMAL IN THE SOURCE    */
#define rvalucr  16   /* ('10'X) THE DATA IS CHARACTER IN THE SOURCE      */
#define rvalubr  17   /* ('11'X) THE DATA IS A FLAG FIELD IN THE SOURCE   */
 
#endif
 
#pragma pack()
