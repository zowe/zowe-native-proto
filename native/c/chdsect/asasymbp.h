#pragma pack(1)
 
#ifndef __symbp__
#define __symbp__
 
struct symbp {
  union {
    void * __ptr32 _symbppattern_a_;  /* Address of input pattern containing symbols */
    void * __ptr32 _symbppatternaddr; /* Same as SymbpPattern@                       */
    } _symbp_union1;
  int            symbppatternlength; /* Length of input pattern */
  union {
    void * __ptr32 _symbptarget_a_;  /* Address of output target area. The target, */
    void * __ptr32 _symbptargetaddr; /* Same as SymbpTarget@                       */
    } _symbp_union2;
  union {
    void * __ptr32 _symbptargetlength_a_;  /* Address of input output fullword field */
    void * __ptr32 _symbptargetlengthaddr; /* Same as SymbpTargetLength              */
    } _symbp_union3;
  union {
    void * __ptr32 _symbpsymboltable_a_;  /* Address of symbol table mapped by SYMBT */
    void * __ptr32 _symbpsymboltableaddr; /* Same as SymbpSymbolTable                */
    } _symbp_union4;
  union {
    void * __ptr32 _symbptimestamp_a_;  /* Address of 8-character area containing the */
    void * __ptr32 _symbptimestampaddr; /* Same as SymbpTimeStamp@                    */
    } _symbp_union5;
  union {
    int            _symbpreturncode_a_;  /* Address of fullword which is to contain the */
    void * __ptr32 _symbpreturncodeaddr; /* Same as SymbpReturnCode                     */
    } _symbp_union6;
  };
 
#define symbppattern_a_       _symbp_union1._symbppattern_a_
#define symbppatternaddr      _symbp_union1._symbppatternaddr
#define symbptarget_a_        _symbp_union2._symbptarget_a_
#define symbptargetaddr       _symbp_union2._symbptargetaddr
#define symbptargetlength_a_  _symbp_union3._symbptargetlength_a_
#define symbptargetlengthaddr _symbp_union3._symbptargetlengthaddr
#define symbpsymboltable_a_   _symbp_union4._symbpsymboltable_a_
#define symbpsymboltableaddr  _symbp_union4._symbpsymboltableaddr
#define symbptimestamp_a_     _symbp_union5._symbptimestamp_a_
#define symbptimestampaddr    _symbp_union5._symbptimestampaddr
#define symbpreturncode_a_    _symbp_union6._symbpreturncode_a_
#define symbpreturncodeaddr   _symbp_union6._symbpreturncodeaddr
 
/* Values for field "symbpreturncodeaddr" */
#define symbp___len 0x1C
 
#endif
 
#ifndef __symbfp__
#define __symbfp__
 
struct symbfp {
  union {
    void * __ptr32 _symbfppattern_a_;  /* Address of input pattern containing symbols */
    void * __ptr32 _symbfppatternaddr; /* Same as SymbfpPattern@                      */
    } _symbfp_union1;
  int            symbfppatternlength; /* Length of input pattern             */
  union {
    void * __ptr32 _symbfptarget_a_;  /* Address of output target area. The target, */
    void * __ptr32 _symbfptargetaddr; /* Same as SymbfpTarget@                      */
    } _symbfp_union2;
  union {
    void * __ptr32 _symbfptargetlength_a_;  /* Address of input output fullword field */
    void * __ptr32 _symbfptargetlengthaddr; /* Same as SymbfpTargetLength@            */
    } _symbfp_union3;
  union {
    void * __ptr32 _symbfpsymboltable_a_;  /* Address of symbol table mapped by SYMBT */
    void * __ptr32 _symbfpsymboltableaddr; /* Same as SymbfpSymbolTable               */
    } _symbfp_union4;
  union {
    void * __ptr32 _symbfptimestamp_a_;  /* Address of 8-character area containing the */
    void * __ptr32 _symbfptimestampaddr; /* Same as SymbfpTimeStamp                    */
    } _symbfp_union5;
  union {
    void * __ptr32 _symbfpreturncode_a_;  /* Address of fullword which is to contain the */
    void * __ptr32 _symbfpreturncodeaddr; /* Same as SymbfpReturnCode                    */
    } _symbfp_union6;
  void * __ptr32 symbfpworkareaaddr;  /* Address of 1024-byte work area on a */
  };
 
#define symbfppattern_a_       _symbfp_union1._symbfppattern_a_
#define symbfppatternaddr      _symbfp_union1._symbfppatternaddr
#define symbfptarget_a_        _symbfp_union2._symbfptarget_a_
#define symbfptargetaddr       _symbfp_union2._symbfptargetaddr
#define symbfptargetlength_a_  _symbfp_union3._symbfptargetlength_a_
#define symbfptargetlengthaddr _symbfp_union3._symbfptargetlengthaddr
#define symbfpsymboltable_a_   _symbfp_union4._symbfpsymboltable_a_
#define symbfpsymboltableaddr  _symbfp_union4._symbfpsymboltableaddr
#define symbfptimestamp_a_     _symbfp_union5._symbfptimestamp_a_
#define symbfptimestampaddr    _symbfp_union5._symbfptimestampaddr
#define symbfpreturncode_a_    _symbfp_union6._symbfpreturncode_a_
#define symbfpreturncodeaddr   _symbfp_union6._symbfpreturncodeaddr
 
/* Values for field "symbfpworkareaaddr" */
#define symbfp___len 0x20
 
#endif
 
#ifndef __symbt__
#define __symbt__
 
struct symbt {
  union {
    unsigned char  _symbtheader[4];
    struct {
      unsigned char  _symbtflags[2];
      unsigned char  _filler1[2];
      } _symbt_struct1;
    struct {
      unsigned char  _symbtflag0;           /* Byte 0 of SymbtFlags                         */
      unsigned char  _symbtflag1;           /* Byte 1 of SymbtFlags                         */
      short int      _symbtnumberofsymbols; /* Number of entries in symbol table. Can be 0. */
      } _symbt_struct2;
    } _symbt_union1;
  unsigned char  symbttableentries; /* Symbol table entries. One for each indicated */
  };
 
#define symbtheader          _symbt_union1._symbtheader
#define symbtflags           _symbt_union1._symbt_struct1._symbtflags
#define symbtflag0           _symbt_union1._symbt_struct2._symbtflag0
#define symbtflag1           _symbt_union1._symbt_struct2._symbtflag1
#define symbtnumberofsymbols _symbt_union1._symbt_struct2._symbtnumberofsymbols
 
/* Values for field "symbtflag0" */
#define symbtnodefaultsymbols                  0x80  /* Avoid using the default symbol set            */
#define symbtonlystaticsymbols                 0x40  /* Allow only static symbols                     */
#define symbttimestampisgmt                    0x20  /* The input timestamp is GMT-time, not          */
#define symbttimestampislocal                  0x10  /* The input timestamp is Local-Time, not        */
#define symbtwarnsubstrings                    0x08  /* When a substring problem is encountered,      */
#define symbtchecknullsubtext                  0x04  /* The presence of null sub-text will be         */
#define symbtptrsareoffsets                    0x02  /* The pointer fields within the                 */
#define symbtonlydynamicsymbols                0x01  /* Allow only dynamic symbols. This              */
 
/* Values for field "symbtflag1" */
#define symbtflag1rsv1                         0x80  /* Reserved. Must be zero. Do not use.           */
#define symbttimestampisstck                   0x40  /* The input timestamp is from the STCK          */
#define symbtwarnnosub                         0x20  /* When no substitution at all has occurred,     */
#define symbtindirectsymbolarea                0x10  /* Indicates that the symbol area is             */
#define symbtmixedcasesymbols                  0x08  /* Indicates that the input may have             */
#define symbtflag1rsv2                         0x06  /* Unused. Must be zero.                         */
#define symbtsymbt1                            0x01  /* When this bit is off, the SYMBT DSECT applies */
 
/* Values for field "symbttableentries" */
#define symbtmaxstaticsymbollengthzosv2r2      16    /* The max length of a static                    */
#define symbtmaxstaticsymbollength             8     /* The max length of a static symbol,            */
#define symbtmaxstaticentriesprezosr4          103   /* The max number of full-sized                  */
#define symbtmaxstaticentrieszosv2r2___8       1631  /* The max number of 8-byte-name                 */
#define symbtmaxstaticentrieszosv2r2___16      1119  /* The max number of                             */
#define symbtmaxstaticentrieszosv2r2___44      731   /* The max number of 16-byte-name                */
#define symbtmaxstaticentrieszosr4             928   /* The max number of full-sized                  */
#define symbtmaxstaticentrydatalengthzosv2r2   62    /* Name with "&" and "."                         */
#define symbtmaxstaticentries                  928   /* The max number of pre-z/OS2.2 full-sized      */
#define symbtmaxstaticsubtextlengthzosv2r2     17    /* The max length of                             */
#define symbtmaxstaticlongsubtextlengthzosv2r2 44    /* The max length of                             */
#define symbtmaxstaticsubtextlength            9     /* The max length of substitution text           */
#define symbtmaxstatictablesizeprezosr4        3609  /* Name with "&" and "." The max                 */
#define symbtmaxstatictablesizezosv2r2         57088 /* The max table size, taking                    */
#define symbtmaxstatictablesizeprezosv2r2      32512 /* The max table size, taking                    */
#define symbtmaxstatictablesizezosr4           32512 /* The max table size, taking into               */
#define symbtmaxstatictablesize                32512
#define symbt___len                            0x04
 
#endif
 
#ifndef __symbt1__
#define __symbt1__
 
struct symbt1 {
  union {
    unsigned char  _symbt1header[16];
    struct {
      unsigned char  _symbt1flags[2];
      unsigned char  _filler1[14];
      } _symbt1_struct1;
    struct {
      unsigned char  _symbt1flag0;           /* Byte 0 of Symbt1Flags                       */
      unsigned char  _symbt1flag1;           /* Byte 1 of Symbt1Flags                       */
      unsigned char  _symbt1flag2;
      unsigned char  _symbt1flag3;
      void * __ptr32 _symbt1nextsymbtaddr;   /* Address of next SYMBT1 or SYMBT so that the */
      unsigned char  _filler2[6];            /* Reserved, must be 0                         */
      short int      _symbt1numberofsymbols; /* Number of entries in symbol table. Can be   */
      } _symbt1_struct2;
    } _symbt1_union1;
  unsigned char  symbt1tableentries; /* Symbol table entries. One for each indicated */
  };
 
#define symbt1header          _symbt1_union1._symbt1header
#define symbt1flags           _symbt1_union1._symbt1_struct1._symbt1flags
#define symbt1flag0           _symbt1_union1._symbt1_struct2._symbt1flag0
#define symbt1flag1           _symbt1_union1._symbt1_struct2._symbt1flag1
#define symbt1flag2           _symbt1_union1._symbt1_struct2._symbt1flag2
#define symbt1flag3           _symbt1_union1._symbt1_struct2._symbt1flag3
#define symbt1nextsymbtaddr   _symbt1_union1._symbt1_struct2._symbt1nextsymbtaddr
#define symbt1numberofsymbols _symbt1_union1._symbt1_struct2._symbt1numberofsymbols
 
/* Values for field "symbt1flag0" */
#define symbt1nodefaultsymbols   0x80 /* Avoid using the default symbol set.           */
#define symbt1onlystaticsymbols  0x40 /* Allow only static symbols. If there           */
#define symbt1timestampisgmt     0x20 /* The input timestamp is GMT-time, not          */
#define symbt1timestampislocal   0x10 /* The input timestamp is Local-Time,            */
#define symbt1warnsubstrings     0x08 /* When a substring problem is                   */
#define symbt1checknullsubtext   0x04 /* The presence of null sub-text will be         */
#define symbt1ptrsareoffsets     0x02 /* The pointer fields within the                 */
#define symbt1onlydynamicsymbols 0x01 /* Allow only dynamic symbols. This              */
 
/* Values for field "symbt1flag1" */
#define symbt1flag1rsv1          0x80 /* Reserved. Must be zero. Do not use.           */
#define symbt1timestampisstck    0x40 /* The input timestamp is from the STCK          */
#define symbt1warnnosub          0x20 /* When no substitution at all has occurred,     */
#define symbt1indirectsymbolarea 0x10 /* Indicates that the symbol area is             */
#define symbt1mixedcasesymbols   0x08 /* Indicates that the input may have             */
#define symbt1flag1rsv2          0x06 /* Unused. Must be zero.                         */
#define symbt1symbt1             0x01 /* When this bit is on, the SYMBT1 DSECT applies */
 
/* Values for field "symbt1flag2" */
#define symbt1preservealignment  0x80 /* Indicates that an attempt is to be            */
#define symbt1nodoubleampersand  0x40 /* Indicates that &&symbol in the                */
#define symbt1iefsjsym           0x20 /* The symbol table area is an area returned by  */
#define symbt1continueafterfull  0x10 /* If the target buffer does not have            */
 
/* Values for field "symbt1flag3" */
#define symbt1jessymbols         0x01 /* This bit is intended for IBM use only. When   */
 
/* Values for field "symbt1tableentries" */
#define symbt1___len             0x10
 
#endif
 
#ifndef __symbte__
#define __symbte__
 
struct symbte {
  union {
    unsigned char  _symbtetableentries[16]; /* Symbol table entry. One such entry for each */
    struct {
      void * __ptr32 _symbtesymbolptr; /* Address of symbol. Do not use when bit */
      unsigned char  _filler1[12];
      } _symbte_struct1;
    struct {
      int            _symbtesymboloffset; /* Offset to symbol from start of symbol area. */
      unsigned char  _filler2[12];
      } _symbte_struct2;
    struct {
      void * __ptr32 _symbtesymbolareaaddr; /* Address of symbol area when                   */
      int            _symbtesymbollength;   /* Length of symbol (includes preceding "&" and  */
      void * __ptr32 _symbtesubtextptr;     /* Address of substitution text. Do not use when */
      unsigned char  _filler3[4];
      } _symbte_struct3;
    struct {
      unsigned char  _filler4[8];
      int            _symbtesubtextoffset; /* Offset to substitution text from start of */
      int            _symbtesubtextlength; /* Length of substitution text               */
      } _symbte_struct4;
    } _symbte_union1;
  };
 
#define symbtetableentries   _symbte_union1._symbtetableentries
#define symbtesymbolptr      _symbte_union1._symbte_struct1._symbtesymbolptr
#define symbtesymboloffset   _symbte_union1._symbte_struct2._symbtesymboloffset
#define symbtesymbolareaaddr _symbte_union1._symbte_struct3._symbtesymbolareaaddr
#define symbtesymbollength   _symbte_union1._symbte_struct3._symbtesymbollength
#define symbtesubtextptr     _symbte_union1._symbte_struct3._symbtesubtextptr
#define symbtesubtextoffset  _symbte_union1._symbte_struct4._symbtesubtextoffset
#define symbtesubtextlength  _symbte_union1._symbte_struct4._symbtesubtextlength
 
/* Values for field "symbtesubtextlength" */
#define symbte___len 0x10
 
#endif
 
#ifndef __symbth__
#define __symbth__
 
struct symbth {
  unsigned char  symbthnotinterface[4];
  int            symbthsymboltablelen;  /* The length of the symbol table (not */
  };
 
/* Values for field "symbthsymboltablelen" */
#define symbth___len 0x08
 
#endif
 
#pragma pack()
