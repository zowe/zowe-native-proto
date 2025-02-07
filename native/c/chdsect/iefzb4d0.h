#pragma pack(packed)

#ifndef __s99rbp__
#define __s99rbp__

struct s99rbp
{
  int s99rbptr; /* REQUEST BLOCK POINTER */
};

/* Values for field "s99rbptr" */
#define s99rbpnd 0x80 /* LAST POINTER INDICATOR */

#endif

#ifndef __s99rb__
#define __s99rb__

struct s99rb
{
  unsigned char s99rbln; /* LENGTH OF REQUEST BLOCK         */
  unsigned char s99verb; /* VERB CODE                       */
  struct
  {
    unsigned char _s99flg11; /* FIRST FLAGS BYTE     */
    unsigned char _s99flg12; /* SECOND BYTE OF FLAGS */
  } s99flag1;
  struct
  {
    unsigned char _s99error[2]; /* ERROR REASON CODE       */
    unsigned char _s99info[2];  /* INFORMATION REASON CODE */
  } s99rsc;
  int s99txtpp; /* ADDR OF LIST OF TEXT UNIT PTRS  */
  int s99s99x;  /* ADDR OF REQ BLK EXTENSION  @L1C */
  struct
  {
    unsigned char _s99flg21; /* FIRST BYTE OF FLAGS  */
    unsigned char _s99flg22; /* SECOND BYTE OF FLAGS */
    unsigned char _s99flg23; /* THIRD BYTE OF FLAGS  */
    unsigned char _s99flg24; /* FOURTH BYTE OF FLAGS */
  } s99flag2;
};

#define s99flg11 s99flag1._s99flg11
#define s99flg12 s99flag1._s99flg12
#define s99error s99rsc._s99error
#define s99info s99rsc._s99info
#define s99flg21 s99flag2._s99flg21
#define s99flg22 s99flag2._s99flg22
#define s99flg23 s99flag2._s99flg23
#define s99flg24 s99flag2._s99flg24

/* Values for field "s99verb" */
#define s99vrbal 0x01 /* ALLOCATION                       */
#define s99vrbun 0x02 /* UNALLOCATION                     */
#define s99vrbcc 0x03 /* CONCATENATION                    */
#define s99vrbdc 0x04 /* DECONCATENATION                  */
#define s99vrbri 0x05 /* REMOVE IN-USE                    */
#define s99vrbdn 0x06 /* DDNAME ALLOCATION                */
#define s99vrbin 0x07 /* INFORMATION RETRIEVAL            */

/* Values for field "s99flg11" */
#define s99oncnv 0x80 /* ALLOC FUNCTION-DO NOT USE AN     */
#define s99nocnv 0x40 /* ALLOC FUNCTION-DO NOT USE AN     */
#define s99nomnt 0x20 /* ALLOC FUNCTION-DO NOT MOUNT      */
#define s99jbsys 0x10 /* ALLOC FUNC-JOB RELATED SYSOUT    */
#define s99cnenq 0x08 /* ALL FUNCTIONS-ISSUE A   @ZA32641 */
#define s99gdgnt 0x04 /* ALLOC FUNCTION - IGNORE @YA10531 */
#define s99msgl0 0x02 /* All functions - ignore the  @01A */
#define s99nomig 0x01 /* ALLOC function - do not     @03A */

/* Values for field "s99flg12" */
#define s99nosym 0x80 /* Allocate, unallocate, info       */
#define s99acucb 0x40 /* Alloc function-use Actual        */
#define s99dsaba 0x20 /* Request that the DSAB for        */
#define s99dxacu 0x10 /* Request above-the-line DSABs,    */

/* Values for field "s99flg21" */
#define s99wtvol 0x80 /* ALLOC FUNCTION-WAIT FOR          */
#define s99wtdsn 0x40 /* ALLOC FUNCTION-WAIT FOR DSNAME   */
#define s99nores 0x20 /* ALLOC FUNCTION-DO NOT DO         */
#define s99wtunt 0x10 /* ALLOC FUNCTION-WAIT FOR UNITS    */
#define s99offln 0x08 /* ALLOC FUNCTION-CONSIDER OFFLINE  */
#define s99tionq 0x04 /* ALL FUNCTIONS-TIOT ENQ ALREADY   */
#define s99catlg 0x02 /* ALLOC FUNCTION-SET SPECIAL       */
#define s99mount 0x01 /* ALLOC FUNCTION-MAY MOUNT VOLUME  */

/* Values for field "s99flg22" */
#define s99udevt 0x80 /* ALLOCATION FUNCTION-UNIT NAME    */
#define s99pcint 0x40 /* ALLOC FUNCTION-ALLOC    @Y30QPPB */
#define s99dyndi 0x20 /* ALLOC FUNCTION-NO JES3  @ZA63125 */
#define s99tioex 0x10 /* ALLOC FUNCTION - XTIOT           */
#define s99aserr 0x08 /* Unit Allocation / Unallocation   */
#define s99igncl 0x04 /* Alloc function - ignore          */
#define s99dasup 0x02 /* Alloc function - suppress        */

#endif

#ifndef __s99tupl__
#define __s99tupl__

struct s99tupl
{
  // int            s99tuptr; /* TEXT UNIT POINTER */
  void *__ptr32 s99tuptr; /* TEXT UNIT POINTER */ // TODO(Kelosky): need to make this dynamic
};

/* Values for field "s99tuptr" */
#define s99tupln 0x80 /* LAST TEXT UNIT POINTER IN LIST */

#endif

#ifndef __s99tunit__
#define __s99tunit__

struct s99tunit
{
  unsigned char s99tukey[2]; /* KEY                              */
  unsigned char s99tunum[2]; /* N0. OF LENGTH+PARAMETER ENTRIES  */
  unsigned char s99tulng[2]; /* LENGH OF 1ST (OR ONLY) PARAMETER */
  unsigned char s99tupar;    /* 1ST (OR ONLY) PARAMETER          */
};

#endif

#ifndef __s99tufld__
#define __s99tufld__

struct s99tufld
{
  unsigned char s99tulen[2]; /* LENGTH OF PARAMETER */
  unsigned char s99tuprm;    /* PARAMETER           */
};

#endif

#ifndef __s99rbx__
#define __s99rbx__

struct s99rbx
{
  unsigned char s99eid[6]; /* CONTROL BLOCK ID ='S99RBX'  @L1A */
  unsigned char s99ever;   /* VERSION NUMBER              @L1A */
  unsigned char s99eopts;  /* PROCESSING OPTIONS          @L1A */
  unsigned char s99esubp;  /* SUBPOOL FOR MESSAGE BLOCKS  @L1A */
  unsigned char s99ekey;   /* STORAGE KEY FOR MESSAGE     @L1A */
  unsigned char s99emgsv;  /* SEVERITY LEVEL FOR MESSAGES @L1A */
  unsigned char s99enmsg;  /* NUMBER OF MESSAGE BLOCKS         */
  int s99ecppl;            /* ADDRESS OF CPPL             @L1A */
  struct
  {
    unsigned char _s99ercr; /* RESERVED                    @L1A */
    unsigned char _s99ercm; /* RESERVED                    @D1C */
    unsigned char _s99erco; /* RETURN CODE DEALING WITH    @L1A */
    unsigned char _s99ercf; /* RETURN CODE DEALING WITH    @L1A */
  } s99emrc;
  int s99ewrc;  /* PUTLINE/WTO RETURN CODE     @L1A */
  int s99emsgp; /* MESSAGE BLOCK POINTER       @L1A */
  struct
  {
    unsigned char _s99eerr[2];  /* ERROR REASON CODE       */
    unsigned char _s99einfo[2]; /* INFORMATION REASON CODE */
  } s99esirc;
  unsigned char s99ersn[4]; /* SMS REASON CODE             @02C */
};

#define s99ercr s99emrc._s99ercr
#define s99ercm s99emrc._s99ercm
#define s99erco s99emrc._s99erco
#define s99ercf s99emrc._s99ercf
#define s99eerr s99esirc._s99eerr
#define s99einfo s99esirc._s99einfo

/* Values for field "s99ever" */
#define s99rbxvr 0x01 /* CURRENT VERSION NUMBER      @L2A */

/* Values for field "s99eopts" */
#define s99eimsg 0x80 /* ISSUE MSG BEFORE RETURNING  @L1A */
#define s99ermsg 0x40 /* RETURN MSG TO CALLER        @L1A */
#define s99elsto 0x20 /* USER STORAGE SHOULD BE      @L1A */
#define s99emkey 0x10 /* USER SPECIFIED STORAGE KEY  @L1A */
#define s99emsub 0x08 /* USER SPECIFIED SUBPOOL FOR  @L1A */
#define s99ewtp 0x04  /* USE WTO FOR MESSAGE OUTPUT  @L2A */

/* Values for field "s99emgsv" */
#define s99xinfo 0x00 /* INFORMATIONAL MSG SEVERITY  @L2A */
#define s99xwarn 0x04 /* WARNING MESSAGE SEVERITY    @L2A */
#define s99xseve 0x08 /* SEVERE MESSAGE SEVERITY     @L2A */

/* Values for field "s99ersn" */
#define s99rbxln 0x24 /* LENGTH OF DECLARED S99RBX   @P3A */

#endif

#pragma pack(reset)
