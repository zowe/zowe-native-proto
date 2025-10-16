#pragma pack(packed)

#ifndef __jproc__
#define __jproc__

struct jproc {
  unsigned char  jprcid[8];   /* I.Eye catcher                        */
  unsigned short jprclen;     /* I.Length of JPROC parameter          */
  struct {
    unsigned char  _jprcverl; /* I.SSOB version level    */
    unsigned char  _jprcverm; /* I.SSOB version modifier */
    } jprcver;
  short int      jprcvero;    /* O.Subsystem version/modifier         */
  unsigned char  _filler1[2]; /* Reserved.                            */
  void * __ptr32 jprcstrp;    /* Storage management anchor.    @Z24BK */
  unsigned char  jprcfltr;    /* IS.Indicate desired filters          */
  unsigned char  jprcflts;    /* IS.System/member selection:   @Z25DW */
  unsigned char  jprctype;    /* IS.Type of concatenation to   @Z25DW */
  unsigned char  _filler2[5]; /* Reserved                      @Z25DW */
  unsigned char  jprcpnam[8]; /* IS*.PROCLIB DD name filter           */
  unsigned char  jprcjcls[8]; /* IS.JOBCLASS whose PROCLIB you want   */
  unsigned char  jprcsysn[8]; /* IS*.System name for selection        */
  unsigned char  jprcmbrn[8]; /* IS*.JES2 MAS member name      @Z25DW */
  int            _filler3[6]; /* Reserved for future use       @Z25DW */
  void * __ptr32 jprclptr;    /* O.Pointer to first PROCLIB           */
  int            jprcnmbr;    /* O.Number of PROCLIB                  */
  void * __ptr32 jprcmptr;    /* O.Ptr to 1st system/member    @Z25DW */
  void * __ptr32 jprcmnum;    /* O.Number of system/member     @Z25DW */
  int            _filler4[9]; /* Reserved for future use       @Z25DW */
  };

#define jprcverl jprcver._jprcverl
#define jprcverm jprcver._jprcverm

/* Values for field "jprcverm" */
#define jprcv010   0x100 /* Initial version number of          */
#define jprcsvr_n_ 0x100 /* Latest version                     */
#define jprccvrl   1     /* Current version level              */
#define jprccvrm   0     /* Current version modifier           */

/* Values for field "jprcfltr" */
#define jprcfnam   0x80  /* Filter on PROCLIB DD name          */
#define jprcfjbc   0x40  /* Return PROCLIB for JOBCLASS        */
#define jprcfstc   0x20  /* Return started task PROCLIB        */
#define jprcftso   0x10  /* Return TSO logon PROCLIB           */
#define jprcfint   0x08  /* Return internal reader PROCLIB     */
#define jprcfloc   0x04  /* Return PROCLIB for address         */
#define jprcftyp   0x02  /* Filter on concatenation    @Z25DW  */

/* Values for field "jprcflts" */
#define jprcssys   0x80  /* use system selection filter        */
#define jprcsmbr   0x40  /* use member selection filter        */

/* Values for field "jprctype" */
#define jprtproc   1     /* PROCLIB type concat         @Z25DW */
#define jprtsbmt   2     /* SUBMITLIB type concat       @Z25DW */
#define jprtplcy   3     /* POLICYLIB type concat       @Z25DW */

/* Values for field "_filler4" */
#define jprcsze1   0x88  /* Fix parameter End: Ver 1           */
#define jprcsze    0x88  /* JPRC Current version               */

#endif

#ifndef __jpshdr__
#define __jpshdr__

struct jpshdr {
  unsigned char  jpsheye[8];  /* Eye-catcher                   @Z25DW */
  unsigned short jpshohdr;    /* Offset to 1st (prefix) sect   @Z25DW */
  unsigned char  _filler1[2]; /* Reserved                      @Z25DW */
  void * __ptr32 jpshnext;    /* Address of next header        @Z25DW */
  };

/* Values for field "jpshnext" */
#define jpshsize 0x10 /* Header size (internal use)    @Z25DW */

#endif

#ifndef __jprcstor__
#define __jprcstor__

struct jprcstor {
  unsigned char  jprcstid[8];   /* Eyecatcher                 */
  unsigned short jprcsthl;      /* Length of header area      */
  unsigned short _filler1;      /* Reserved for future use    */
  unsigned char  jprcstsp;      /* Subpool of area            */
  unsigned int   jprcsttl : 24; /* Total length of area (this */
  void * __ptr32 jprcstnx;      /* Pointer to next area       */
  void * __ptr32 jprcstcp;      /* Pointer to 1st available   */
  void * __ptr32 jprcstbg;      /* Start of data area         */
  };

/* Values for field "jprcstsp" */
#define jprcstpl 230  /* Recommended subpool to use */

/* Values for field "jprcstbg" */
#define jprcstsz 0x18 /* JPRCSTOR length            */

#endif

#ifndef __jprhdr__
#define __jprhdr__

struct jprhdr {
  unsigned char  jpreye[8]; /* Eye catcher                          */
  unsigned short jproprf;   /* Offset to prefix section             */
  unsigned short _filler1;  /* Reserved for future use              */
  void * __ptr32 jprnxtp;   /* Address of next PROCLIB              */
  void * __ptr32 jprjplex;  /* Address of system info        @Z25DW */
  };

/* Values for field "jprjplex" */
#define jprhdsz  0x14 /* Size of this section     */
#define jpridprf 0x00 /* PROCLIB Info - Prefix    */
#define jpridgen 0x01 /* PROCLIB Info - General   */
#define jprimgen 0x00 /* Modifier - General       */
#define jprimdsn 0x01 /* Modifier - Data set info */

#endif

#ifndef __jprpref__
#define __jprpref__

struct jprpref {
  unsigned short jprprln; /* Length of entire PROCLIB  */
  unsigned char  jprprtp; /* Type     = Prefix Section */
  unsigned char  jprprmd; /* Type Mod = General        */
  };

/* Values for field "jprprmd" */
#define jprprsz 0x04 /* Size of this section */

#endif

#ifndef __jprgeni__
#define __jprgeni__

struct jprgeni {
  unsigned short jprgln;      /* Length of this section               */
  unsigned char  jprgty;      /* Type     = General Info              */
  unsigned char  jprgmd;      /* Type Mod = General                   */
  unsigned char  jprddnam[8]; /* PROCLIB DD name                      */
  unsigned char  jprflag1;    /* Flag byte                            */
  unsigned char  jprptype;    /* Type of concatenation         @Z25DW */
  unsigned char  _filler1[2]; /* Reserved                      @Z25DW */
  short int      jprdscnt;    /* Number of data sets in concatenation */
  short int      jprdusct;    /* Concatenation use count     @Z23LSDS */
  unsigned char  jprdmvsn[8]; /* MVS system name               @Z25DW */
  unsigned char  jprdsid[8];  /* JES member name               @Z25DW */
  };

/* Values for field "jprflag1" */
#define jprp1sta 0x80 /* Static PROCLIB (from JES PROC)     */
#define jprp1stc 0x40 /* Started task PROCLIB               */
#define jprp1tso 0x20 /* TSO logon PROCLIB                  */
#define jprp1pth 0x10 /* PROCLIB includes a PATH=    @Z25DW */
#define jprp1nmv 0x08 /* A path was not allocated    @Z25DW */

/* Values for field "jprptype" */
#define jprptprc 1    /* PROCLIB type concat         @Z25DW */
#define jprptstm 2    /* SUBMITLIB type concat       @Z25DW */
#define jprptpcy 3    /* POLICYLIB type concat       @Z25DW */

/* Values for field "jprdsid" */
#define jprgensz 0x24 /* Size of this section               */

#endif

#ifndef __jprdsets__
#define __jprdsets__

struct jprdsets {
  unsigned short jprdlen;       /* Length of data set info section      */
  unsigned char  jprdtype;      /* Type of this header                  */
  unsigned char  jprdmod;       /* Modifier                             */
  unsigned short jprdoffs;      /* Offset to first info area            */
  short int      jprdnum;       /* Number of data set areas             */
  unsigned short jprdinfl;      /* Length of a data set area     @Z25DW */
  unsigned char  jprdstrt[120]; /* First data set info area             */
  };

/* Values for field "jprdstrt" */
#define jprdsize 0x0A /* Length of basic section */

#endif

#ifndef __jprdsinf__
#define __jprdsinf__

struct jprdsinf {
  unsigned char  jprddsn[44];   /* Data set name                        */
  unsigned char  jprdunit[8];   /* Data set unit                        */
  unsigned char  jprdvol[6];    /* Data set VOLSER                      */
  unsigned char  jprdflg1;      /* Data set flags              @Z23LSDS */
  unsigned char  jprdrcfm;      /* Record format (RECFM)         @Z25DW */
  unsigned char  jprdlrcl[2];   /* Record length (LRECL)         @Z25DW */
  unsigned char  jprdblks[2];   /* Block size (BLKSIZE)          @Z25DW */
  unsigned char  jprdxdsn[44];  /* Extracted data set name       @Z25DW */
  unsigned char  jprdxvol[6];   /* Extracted data set VOLSER     @Z25DW */
  unsigned char  _filler1[6];   /* Reserved                      @Z25DW */
  unsigned char  jprdpath[128]; /* Specified PATH value          @Z25DW */
  };

/* Values for field "jprdflg1" */
#define jprdalcf 0x80 /* Allocation failed         @Z23LSDS   */
#define jprdopne 0x40 /* OPEN error encountered      @Z25DW   */
#define jprdpths 0x20 /* PATH= specified             @Z25DW   */
#define jprdptht 0x10 /* PATH= value truncated       @Z25DW   */

/* Values for field "_filler1" */
#define jprdslen 0x78 /* Length of area (no PATH)      @Z25DW */

/* Values for field "jprdpath" */
#define jprdslnp 0xF8 /* Length of area (with PATH)    @Z25DW */

#endif

#pragma pack(reset)
