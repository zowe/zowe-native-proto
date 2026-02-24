#ifdef __open_xl__
#pragma pack(1)
#else
#pragma pack(packed)
#endif

#ifndef __mdb__
#define __mdb__

struct mdb
{
  short int mdblen;         /* MDB length     */
  unsigned char mdbtype[2]; /* MDB type       */
  unsigned char mdbmid[4];  /* Acronym 'MDB ' */
  unsigned char mdbver[4];  /* Revision code  */
};

/* Values for field "mdbtype" */
#define mdbtyp1 0x01 /* Type for MDB Type 1               */

/* Values for field "mdbver" */
#define mdbver1 0x01 /* Revision code 1              @L4C */
#define mdbvid 0x01  /* Current revision code             */
#define mdbhlen 0x0C /* Length of MDB Header section @P7C */

#endif

#ifndef __mdbg__
#define __mdbg__

struct mdbg
{
  short int mdbglen;         /* General object length      */
  unsigned char mdbgtype[2]; /* General object type        */
  struct
  {
    unsigned char _mdbgsyid;   /* System ID                     @L1A */
    unsigned char _mdbgseq[3]; /* Sequence Number               @L1A */
  } mdbgmid;
  unsigned char mdbgtimh[8]; /* Time stamp HH.MM.SS format */
  unsigned char mdbgtimt[3]; /* Time stamp .TH format      */
  unsigned char mdbgrsv1;    /* Reserved                   */
  unsigned char mdbgdstp[7]; /* Date stamp                 */
  unsigned char mdbgrsv2;    /* Reserved                   */
  unsigned char mdbgmflg[2]; /* Message flags              */
  unsigned char mdbgrsv3[2]; /* Reserved                   */
  struct
  {
    unsigned char _mdbgfcon; /* Foreground control field      @P3C */
    unsigned char _mdbgfcol; /* Foreground color field        @P3C */
    unsigned char _mdbgfhil; /* Foreground highlighting       @P3C */
    unsigned char _mdbgfint; /* Foreground intensity          @P3C */
  } mdbgfgpa;
  struct
  {
    unsigned char _mdbgbcon; /* Background control field      @P3C */
    unsigned char _mdbgbcol; /* Background color field        @P3C */
    unsigned char _mdbgbhil; /* Background highlighting       @P3C */
    unsigned char _mdbgbint; /* Background intensity          @P3C */
  } mdbgbgpa;
  unsigned char mdbgosnm[8]; /* Originating system name    */
  unsigned char mdbgjbnm[8]; /* Job name                   */
};

#define mdbgsyid mdbgmid._mdbgsyid
#define mdbgseq mdbgmid._mdbgseq
#define mdbgfcon mdbgfgpa._mdbgfcon
#define mdbgfcol mdbgfgpa._mdbgfcol
#define mdbgfhil mdbgfgpa._mdbgfhil
#define mdbgfint mdbgfgpa._mdbgfint
#define mdbgbcon mdbgbgpa._mdbgbcon
#define mdbgbcol mdbgbgpa._mdbgbcol
#define mdbgbhil mdbgbgpa._mdbgbhil
#define mdbgbint mdbgbgpa._mdbgbint

/* Values for field "mdbgtype" */
#define mdbgobj 0x01 /* Type for general object            */

/* Values for field "mdbgmflg" */
#define mdbgdom 0x8000  /* DOM bit.  If this bit is on it     */
#define mdbgalrm 0x4000 /* Sound warning alarm (processor     */
#define mdbghold 0x2000 /* Hold bit, Hold message until DOMed */

#endif

#ifndef __mdbscp__
#define __mdbscp__

struct mdbscp
{
  short int mdbclen;         /* Control program object length @P7C */
  unsigned char mdbctype[2]; /* Object type                        */
  struct
  {
    unsigned char _mdbcver[4];  /* MVS CP object version level   @P5A */
    unsigned char _mdbcpnam[4]; /* Control Program name ("MVS")  @P5A */
    unsigned char _mdbcfmid[8]; /* FMID of originating system    @P5A */
  } mdbcprod;
  unsigned char mdbcerc[16]; /* Routing codes                 @P7C */
  struct
  {
    unsigned char _mdbdesc1; /* Descriptor codes byte 1       @L1A */
    unsigned char _mdbdesc2; /* Descriptor codes byte 2       @L1A */
  } mdbcdesc;
  struct
  {
    unsigned char _mdbmlvl1; /* Message level byte 1          @L1A */
    unsigned char _mdbmlvl2; /* Message level byte 2          @L1A */
  } mdbcmlvl;
  struct
  {
    unsigned char _mdbcatt1; /* First byte of attributes  */
    unsigned char _mdbcatt2; /* Second byte of attributes */
  } mdbcattr;
  short int mdbcrsv7;        /* Reserved                      @LCC */
  short int mdbcrsv5;        /* Reserved                      @D3C */
  short int mdbcasid;        /* ASID of issuer                     */
  void *__ptr32 mdbctcb;     /* Job Step TCB for issuer       @P6C */
  unsigned char mdbctokn[4]; /* Token (for DOM)               @P3C */
  unsigned char mdbcsyid;    /* System ID (for DOM)                */
  unsigned char mdbdomfl;    /* DOM flags                          */
  unsigned char mdbcmisc;    /* Miscellaneous Routing Info    @P4C */
  unsigned char mdbcmsc2;    /* Miscellaneous OPERLOG info    @L8C */
  unsigned char mdbcojid[8]; /* Originating Job ID                 */
  unsigned char mdbckey[8];  /* Retrieval key (Source: WTO)   @P7C */
  unsigned char mdbcauto[8]; /* Automation token                   */
  unsigned char mdbccart[8]; /* Command and Response Token         */
  unsigned char mdbccnid[4]; /* 4-Byte Console ID                  */
  struct
  {
    unsigned char _mdbcmgt1; /* First byte of message type  */
    unsigned char _mdbcmgt2; /* Second byte of message type */
  } mdbcmsgt;
  short int mdbcrpyl;             /* Reply ID Length                    */
  unsigned char mdbcrpyi[8];      /* Reply ID (EBCDIC representation)   */
  unsigned char mdbctoff2[2];     /* Like MDBCTOFF but allows for       */
  unsigned char mdbctoff[2];      /* Offset in the message text field   */
  unsigned char mdbcrpyb[4];      /* Reply ID (Binary representation)   */
  unsigned char mdbcarea;         /* Area ID                       @P4C */
  char mdb___autor___reply___len; /* Reply length for auto-reply   @LMC */
  unsigned char mdbclcnt[4];      /* Number of lines in message    @D3C */
  unsigned char mdbcojbn[8];      /* Originating job name          @D3A */
  unsigned char mdbcsplx[8];      /* Sysplex name                  @D4A */
  struct
  {
    struct
    {
      unsigned char _mdbcrfb1; /* Request flags byte one        @L8A */
      unsigned char _mdbcrfb2; /* Request flags byte two        @L8A */
      unsigned char _mdbcrfb3; /* Request flags byte three      @L8A */
    } mdbcrflg;
    unsigned char _mdbcsupb; /* Suppression byte              @L8A */
  } mdbcxmod;
  unsigned char mdbccnnm[8]; /* Console name                  @D5A */
  struct
  {
    unsigned char _mdbmcsf1; /* First byte of MCS flags       @D5A */
    unsigned char _mdbmcsf2; /* Second byte of MCS flags      @D5A */
  } mdbcmcsf;
  short int mdb___autor___delay;         /* Auto-reply delay time         @LMC */
  unsigned char mdbcetod[16];            /* Time stamp of when message was     */
  unsigned char mdb___misc___flags;      /* Misc. Flags                   @LMA */
  unsigned char mdb___autor___reply[64]; /* Auto-reply reply              @LMA */
  unsigned char _filler1[23];            /* Reserved                      @LMA */
};

#define mdbcver mdbcprod._mdbcver
#define mdbcpnam mdbcprod._mdbcpnam
#define mdbcfmid mdbcprod._mdbcfmid
#define mdbdesc1 mdbcdesc._mdbdesc1
#define mdbdesc2 mdbcdesc._mdbdesc2
#define mdbmlvl1 mdbcmlvl._mdbmlvl1
#define mdbmlvl2 mdbcmlvl._mdbmlvl2
#define mdbcatt1 mdbcattr._mdbcatt1
#define mdbcatt2 mdbcattr._mdbcatt2
#define mdbcmgt1 mdbcmsgt._mdbcmgt1
#define mdbcmgt2 mdbcmsgt._mdbcmgt2
#define mdbcrfb1 mdbcxmod.mdbcrflg._mdbcrfb1
#define mdbcrfb2 mdbcxmod.mdbcrflg._mdbcrfb2
#define mdbcrfb3 mdbcxmod.mdbcrflg._mdbcrfb3
#define mdbcsupb mdbcxmod._mdbcsupb
#define mdbmcsf1 mdbcmcsf._mdbmcsf1
#define mdbmcsf2 mdbcmcsf._mdbmcsf2

/* Values for field "mdbctype" */
#define mdbcobj 0x02 /* Type for control prog object  @P7C */

/* Values for field "mdbcfmid" */
#define mdbcver1 0x01      /* MVS CP object version 1       @P5A */
#define mdbcver2 0x02      /* JBB4422 object version 2      @L5A */
#define mdbcver3 0x03      /* OY65627 object version 3      @P9C */
#define mdbcver4 0x04      /* HBB5510 object version 4      @P9A */
#define mdbcver5 0x05      /* HBB5520 object version 5      @L8A */
#define mdbcv10 0x10       /* Structurally equivalent ot HBB5520 */
#define mdbcv20 0x20       /* Structurally equivalent ot HBB5520 */
#define mdbcv30 0x30       /* HBB5520 object with OW20064   @03A */
#define mdbcv50 0x50       /* HBB7750 level                 @LKA */
#define mdbcv70 0x70       /* HBB7770 level                 @LMA */
#define mdbcvid 0x70       /* Current MVS CP object version @LMC */
#define mdbcmvs 0xD4E5E240 /* Control Program name          @P5A */

/* Values for field "mdbdesc1" */
#define mdbdesca 0x80 /* System failure                @L1A */
#define mdbdescb 0x40 /* Immediate action required     @L1A */
#define mdbdescc 0x20 /* Eventual action required      @L1A */
#define mdbdescd 0x10 /* System status                 @L1A */
#define mdbdesce 0x08 /* Immediate command response    @L1A */
#define mdbdescf 0x04 /* Job status                    @L1A */
#define mdbdescg 0x02 /* Application program/processor @L1A */
#define mdbdesch 0x01 /* Out-of-line                   @L1A */

/* Values for field "mdbdesc2" */
#define mdbdesci 0x80 /* Operator's request            @L1A */
#define mdbdescj 0x40 /* Reserved                      @LEC */
#define mdbdesck 0x20 /* Critical eventual action      @L1A */
#define mdbdescl 0x10 /* Important Information         @P7C */
#define mdbdescm 0x08 /* Previously automated          @L5C */
#define mdbdescn 0x04 /* Reserved                      @L1A */
#define mdbdesco 0x02 /* Reserved                      @L1A */
#define mdbdescp 0x01 /* Reserved                      @L1A */

/* Values for field "mdbmlvl1" */
#define mdbmlr 0x80   /* WTOR                          @L1A */
#define mdbmlia 0x40  /* Immediate action              @L1A */
#define mdbmlce 0x20  /* Critical eventual action      @L1A */
#define mdbmle 0x10   /* Eventual action               @L1A */
#define mdbmli 0x08   /* Informational                 @L1A */
#define mdbmlbc 0x04  /* Broadcast                     @L1A */
#define mdbmlrsg 0x02 /* Reserved                      @L1A */
#define mdbmlrsh 0x01 /* Reserved                      @L1A */

/* Values for field "mdbmlvl2" */
#define mdbmlrsi 0x80 /* Reserved                      @L1A */
#define mdbmlrsj 0x40 /* Reserved                      @L1A */
#define mdbmlrsk 0x20 /* Reserved                      @L1A */
#define mdbmlrsl 0x10 /* Reserved                      @L1A */
#define mdbmlrsm 0x08 /* Reserved                      @L1A */
#define mdbmlrsn 0x04 /* Reserved                      @L1A */
#define mdbmlrso 0x02 /* Reserved                      @P4C */
#define mdbmlrsp 0x01 /* Reserved                      @P4C */

/* Values for field "mdbcatt1" */
#define mdbcsupp 0x80 /* Message is suppressed         @P9A */
#define mdbcmcsc 0x40 /* Message is command response        */
#define mdbcauth 0x20 /* Message issued by authorized       */
#define mdbcretn 0x10 /* Message is retained by AMRF   @P7C */
#define mdbcspvd 0x08 /* WQE Backlog Message           @01A */
#define mdbcqnly 0x04 /* Console only                  @02A */

/* Values for field "mdbdomfl" */
#define mdbdmsgi 0x80 /* DOM by message id (can be found    */
#define mdbdsysi 0x40 /* DOM by system ID                   */
#define mdbdasid 0x20 /* DOM by ASID                        */
#define mdbdjtcb 0x10 /* DOM by job step TCB                */
#define mdbdtokn 0x08 /* DOM by token                       */
#define mdbdnorm 0x04 /* This is a Normal DOM          @LAA */

/* Values for field "mdbcmisc" */
#define mdbcrsv2 0x80 /* Reserved. Was MDBCUD          @LDC */
#define mdbcrsv3 0x40 /* Reserved. Was MDBCFUDO        @LDC */
#define mdbrsv18 0x20 /* Reserved - was MDBCFIDO (Queue by  */
#define mdbcaut 0x10  /* Queue by automation           @L5A */
#define mdbchc 0x08   /* Queue by hardcopy             @L6A */
#define mdbcintc 0x04 /* Receiving INTIDS (Console ID       */
#define mdbcunkc 0x02 /* Receiving UNKNIDS (Unknown         */

/* Values for field "mdbcmsc2" */
#define mdbcocmd 0x80 /* Echo operator command         @L8A */
#define mdbcicmd 0x40 /* Echo internal command         @L8A */
#define mdbcwtl 0x20  /* Result of WTL macro           @L8A */
#define mdbcopon 0x10 /* MDB has been sent from USS    @LIA */

/* Values for field "mdbcmgt1" */
#define mdbmsgta 0x80 /* Display jobnames              @D2A */
#define mdbmsgtb 0x40 /* Display status                @D2A */
#define mdbmsgtc 0x20 /* Monitor active                @D2A */
#define mdbmsgtd 0x10 /* Indicates existence of QID field   */
#define mdbrsv13 0x08 /* Reserved                      @D2A */
#define mdbmsgtf 0x04 /* Monitor SESS                  @D2A */
#define mdbrsv14 0x02 /* Reserved                      @D2A */
#define mdbrsv15 0x01 /* Reserved                      @D2A */

/* Values for field "mdbcrfb1" */
#define mdbcrcmt 0x80 /* Message text was changed      @L8A */
#define mdbcrcrc 0x40 /* Routing code(s) were changed  @L8A */
#define mdbcrcdc 0x20 /* Descriptor code(s) were changed    */
#define mdbcrqpc 0x10 /* Queued to a particular active      */
#define mdbrsv17 0x08 /* Reserved - was MDBCRQUN (Queue     */
#define mdbcrqrc 0x04 /* Queued by routing codes only  @L8A */
#define mdbrsv16 0x02 /* Reserved - was MDBCRCCN (1-byte    */
#define mdbcrpml 0x01 /* Minor lines were processed    @L8A */

/* Values for field "mdbcrfb2" */
#define mdbcrdtm 0x80 /* Message was deleted           @L8A */
#define mdbcroms 0x40 /* MPF suppression Overrided     @L8A */
#define mdbcrfhc 0x20 /* Hardcopy forced               @L8A */
#define mdbcrnhc 0x10 /* No hardcopy forced            @L8A */
#define mdbcrhco 0x08 /* Only hardcopy forced          @L8A */
#define mdbcrbca 0x04 /* Broadcasted message to active      */
#define mdbcrbcn 0x02 /* Did not broadcast message          */
#define mdbcrnrt 0x01 /* AMRF is not to retain this         */

/* Values for field "mdbcrfb3" */
#define mdbcrret 0x80 /* AMRF is to retain this message@PCC */
#define mdbcrcky 0x40 /* Changed the retrieval key     @L8A */
#define mdbcrcfc 0x20 /* Changed the 4-byte console id @L8A */
#define mdbcrcmf 0x10 /* Changed the message type flags     */
#define mdbcrano 0x08 /* Automation was not required   @L8A */
#define mdbcrays 0x04 /* Automation was required and/or     */
#define mdbcqhco 0x02 /* Message issued hardcopy only  @L8A */
#define mdbcrsv8 0x01 /* Reserved. Was MDBCHUD         @LDC */

/* Values for field "mdbcsupb" */
#define mdbcsnsv 0x80                    /* Not serviced by any WTO user exit  */
#define mdbcseer 0x40                    /* A WTO user exit ABENDed while      */
#define mdbcsnsi 0x20                    /* Not serviced because of an         */
#define mdbcsaut 0x10                    /* Indicate automation specified @L8A */
#define mdbc___processed___by___mfa 0x08 /* Message Flood Automation           */
#define mdbcsssi 0x04                    /* Suppressed by a subsystem     @L8A */
#define mdbcswto 0x02                    /* Suppressed by a WTO user exit      */
#define mdbcsmpf 0x01                    /* Suppressed by MPF or Message       */

/* Values for field "mdbmcsf1" */
#define mdbmcsa 0x80  /* Route/Descriptor code fields       */
#define mdbmcsb 0x40  /* Message queued to console id       */
#define mdbmcsc 0x20  /* MCSFLAG=RESP was specified    @03A */
#define mdbmcsd 0x10  /* Message type field exists     @03A */
#define mdbmcse 0x08  /* MCSFLAG=REPLY was specified   @03A */
#define mdbmcsf 0x04  /* MCSFLAG=BRDCST was specified  @03A */
#define mdbmcsg 0x02  /* MCSFLAG=HRDCPY was specified  @03A */
#define mdbmcshx 0x01 /* Reserved - meant MCSFLAG=QREG0     */

/* Values for field "mdbmcsf2" */
#define mdbmcsi 0x80 /* MCSFLAG=NOTIME was specified  @D5A */
#define mdbmcsj 0x40 /* MLWTO indicator               @03A */
#define mdbmcsk 0x20 /* Primary subsystem use         @03A */
#define mdbmcsl 0x10 /* Extended WPL used             @03A */
#define mdbmcsm 0x08 /* MCSFLAG= CMD was specified    @03A */
#define mdbmcsn 0x04 /* MCSFLAG=NOCPY was specified   @03A */
#define mdbmcso 0x02 /* WQEBLK used                   @03A */

/* Values for field "mdb___misc___flags" */
#define mdb___autor___data___valid 0x80     /* MDB contains valid auto-reply      */
#define mdb___autor___delay___in___sec 0x40 /* Auto-reply delay time is in        */
#define mdb___no___syslog 0x20              /* Copy of WQENSYL (for JES3).   @05A */
#define mdb___wqej3b1 0x10                  /* Copy of WQEJ3B1 (for JES3).   @05A */

#endif

#ifndef __mdbt__
#define __mdbt__

struct mdbt
{
  short int mdbtlen;         /* Text object length */
  unsigned char mdbttype[2]; /* Text object type   */
  struct
  {
    unsigned char _mdbtlnt1; /* Line type flags byte 1 */
    unsigned char _mdbtlnt2; /* Line type flags byte 2 */
  } mdbtlnty;
  struct
  {
    unsigned char _mdbtpcon; /* Presentation control field    @P3C */
    unsigned char _mdbtpcol; /* Presentation color field      @P3C */
    unsigned char _mdbtphil; /* Presentation highlighting     @P3C */
    unsigned char _mdbtpint; /* Presentation intensity        @P3C */
  } mdbtmtpa;
};

#define mdbtlnt1 mdbtlnty._mdbtlnt1
#define mdbtlnt2 mdbtlnty._mdbtlnt2
#define mdbtpcon mdbtmtpa._mdbtpcon
#define mdbtpcol mdbtmtpa._mdbtpcol
#define mdbtphil mdbtmtpa._mdbtphil
#define mdbtpint mdbtmtpa._mdbtpint

/* Values for field "mdbttype" */
#define mdbtobj 0x04 /* Type for message text object       */

/* Values for field "mdbtlnt1" */
#define mdbtcont 0x80 /* Control text                       */
#define mdbtlabt 0x40 /* Label text                         */
#define mdbtdatt 0x20 /* Data text                          */
#define mdbtendt 0x10 /* End text                           */
#define mdbtprot 0x08 /* Prompt text                        */
#define mdbtoptt 0x04 /* Reserved for IBM use          @04C */
#define mdbtrsv2 0x02 /* Reserved                      @D3C */

/* Values for field "mdbtlnt2" */
#define mdbtfpaf 0x01 /* Text object presentation attribute */

#endif

#ifdef __open_xl__
#pragma pack()
#else
#pragma pack(reset)
#endif
