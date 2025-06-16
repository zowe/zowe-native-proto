#pragma pack(packed)
 
#ifndef __dcbe__
#define __dcbe__
 
struct dcbe {
  unsigned char  dcbeid[4];      /* DCBE eyecatcher 'DCBE'.                   */
  short int      dcbelen;        /* Length of DCBE.                           */
  unsigned char  dcbersv1[2];    /* Reserved.                                 */
  void * __ptr32 dcbedcb;        /* DCB address. Set by system.               */
  unsigned char  dcberela[4];    /* Partitioned data set - Address            */
  unsigned char  dcbeflg1;       /* Flags set by system.                      */
  unsigned char  dcbeflg2;       /* Flags set by user.                        */
  short int      dcbenstr;       /* The number of stripes for a               */
  unsigned char  dcbeflg3;       /* Flags set by user                    @L3A */
  unsigned char  dcbeflg5;       /* Flags                                @L9C */
  unsigned char  _filler1[2];    /* Reserved.                            @L9C */
  struct {
    int            _filler2;   /* Hi-order half of Block size          @L2A */
    int            _dcbeblksi; /* Block size, valid only if            @L2A */
    } dcbeblksi8;
  struct {
    int            _dcbesiz0; /* High order word of DCBEXSIZ.         @L1C */
    int            _dcbesize; /* Number of blocks in current               */
    } dcbexsiz;
  void * __ptr32 dcbeeoda;       /* Address of user provided end-             */
  void * __ptr32 dcbesyna;       /* Address of user provided SYNAD            */
  void * __ptr32 dcbe___pdseext; /* Address of PDSE DCBE extension       @PUA */
  short int      dcbenmfl;       /* Number tape files before synchronize @MAA */
  unsigned char  dcbemacc;       /* Accumulation number multiplier.           */
  unsigned char  dcbemsdn;       /* Multiplier of System Determined NCP.      */
  };
 
#define dcbeblksi dcbeblksi8._dcbeblksi
#define dcbesiz0  dcbexsiz._dcbesiz0
#define dcbesize  dcbexsiz._dcbesize
 
/* Values for field "dcbeflg1" */
#define dcbeopen                            0x80 /* DCBE has been successfully                */
#define dcbemd31                            0x40 /* User may call access method in            */
#define dcbeslbi                            0x20 /* Before DCB OPEN exit: SAM supports   @L2A */
#define dcbe___32bit___inuse                0x10 /* Device using 32-bit block numbers,   @L3A */
#define dcbebenefix                         0x08 /* Performance would be benefited if    @L6A */
#define dcbe___ds___encryption              0x04 /* Data set is encrypted (DS1ENCRP=ON). @LAA */
#define dcbe___hyperwrite___fb              0x02 /* Set by system during OPEN after the  @04A */
#define dcbe___bypass___extent___check___fb 0x01 /* Set by system during OPEN    @06A         */
 
/* Values for field "dcbeflg2" */
#define dcbebu31                            0x80 /* RMODE31=BUFF.                             */
#define dcbeneod                            0x40 /* PASTEOD=YES.                              */
#define dcbe___concurrentrw                 0x20 /* CONCURRENTRW=YES                     @03A */
#define dcbenver                            0x10 /* NOVER=YES.                                */
#define dcbegsiz                            0x08 /* GETSIZE=YES.                              */
#define dcbeulbi                            0x04 /* BLKSIZE specified on DCBE.  User     @L2A */
#define dcbe___reqst___xcap                 0x02 /* CAPACITYMODE=XCAP.  Extended         @L3A */
#define dcbeexps                            0x01 /* Bypass extended data integrity       @L4A */
 
/* Values for field "dcbeflg3" */
#define dcbelarge                           0x80 /* BLOCKTOKENSIZE=LARGE.  BSAM/BPAM:    @L5A */
#define dcbebfxu                            0x40 /* FIXED=USER                                */
#define dcbeeadscbok                        0x20 /* User supports 28-bit cylinder             */
#define dcbelocany                          0x10 /* User allows XTIOT and allows DSABs and    */
#define dcbe___pdseext___valid              0x08 /* DCBE_PDSEEXT is valid                @PUA */
#define dcbesync___none                     0x07 /* SYNC=NONE if all three bits are 1    @L3A */
#define dcbesync___system                   0x01 /* SYNC=SYSTEM if low 3 bits are 001.   @L3A */
 
/* Values for field "dcbeflg5" */
#define dcbe___bypass___extent___check      0x10 /* BYPASS_EXTENT_CHECK=YES         @06A      */
#define dcbedsencnp                         0x08 /* 1=No block in the data set has a     @LCA */
#define dcbedsencryptok                     0x04 /* DSENCRYPT=OK                         @05A */
#define dcbe___hyperwrite                   0x02 /* HYPERWRITE=YES                       @04A */
#define dcbetracklock                       0x01 /* CONCURRENTRW=(YES,TRKLOCK)           @L9A */
 
/* Values for field "dcbemsdn" */
#define dcbeminl                            0x38 /* Minimum length of DCBE.  This label       */
 
#endif
 
#pragma pack(reset)
