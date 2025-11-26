#pragma pack(packed)

#ifndef __ucb__
#define __ucb__

struct ucb {
  int            _filler1[118]; /* Reserved                        @LFC */
  char           ucbeti;        /* A binary number used by the exit     */
  char           ucbsti;        /* Increment which, when multiplied by  */
  unsigned char  ucbfl6;        /* Device features byte                 */
  char           ucbati;        /* Index to the attention table (ANTAB) */
  char           ucbsnsct;      /* Count of sense bytes presented       */
  unsigned char  ucbflp1;       /* Flag byte                            */
  unsigned char  ucbstli;       /* Statistics table lookup index        */
  unsigned char  ucbfl7;        /* Miscellaneous usage flags            */
  void * __ptr32 ucbiext;       /* Pointer to IOS UCB extension    @01C */
  unsigned char  ucbchprm;      /* Channel path recovery mask           */
  char           ucbsati;       /* Attention table index                */
  short int      ucbasid;       /* ASID of the memory to which this     */
  struct {
    unsigned char  _filler2;     /* Reserved               */
    unsigned char  _ucbwtoid[3]; /* WTO message identifier */
    } ucbwtowd;
  struct {
    short int      _ucbddti;  /* Contains DDT name list index during */
    short int      _filler3;  /* Remainder of DDT address            */
    } ucbddt;
  void * __ptr32 ucbclext;      /* Pointer to device class              */
  short int      ucbdctof;      /* Device connect time                  */
  unsigned char  ucbcsflg;      /* Miscellaneous flags which should     */
  unsigned char  ucbfl8;        /* Miscellaneous usage flags       @A6C */
  int            ucblock;       /* Device lock word                     */
  void * __ptr32 ucbioq;        /* Address of last queuing element      */
  struct {
    unsigned char  _ucbjbnr; /* Flag byte               */
    unsigned char  _ucbfl5;  /* Flags                   */
    unsigned char  _ucbid;   /* UCB identification (FF) */
    unsigned char  _ucbstat; /* Device status           */
    } ucbob;
  short int      ucbchan;       /* Binary device number                 */
  struct {
    unsigned char  _ucbfla; /* I/O supervisor flag byte A */
    unsigned char  _ucbflb; /* I/O supervisor flag byte B */
    } ucbsfls;
  void * __ptr32 ucbnxucb;      /* Address of the next UCB on the UCB   */
  unsigned char  ucbwgt;        /* Flags                                */
  unsigned char  ucbdummy[3];   /* Dummy UCBs use this field to    @M5C */
  struct {
    unsigned char  _ucbtbyt1; /* Model bits   */
    unsigned char  _ucbtbyt2; /* Option flags */
    struct {
      unsigned char  _ucbtbyt3; /* Class bits */
      } ucbdvcls;
    struct {
      unsigned char  _ucbtbyt4; /* Device code */
      } ucbuntyp;
    } ucbtyp;
  unsigned char  ucbflc;        /* I/O supervisor flag byte C           */
  unsigned char  _filler4[3];   /* Reserved                        @L7A */
  union {
    struct {
      struct {
        unsigned char  _ucbaof1; /* First byte of UCBAOF  */
        unsigned char  _ucbaof2; /* Second byte of UCBAOF */
        } ucbaof;
      char           _ucbatnct; /* Attention count.  The number of  */
      unsigned char  _filler5;  /* UCBGCB - control byte.  Used for */
      } _ucb_struct1;
    struct {
      void * __ptr32 _ucbctcal; /* Address of JES3 routine for */
      } ucbctcad;
    unsigned char  _ucbvtoc[4]; /* Relative address of VTOC for this  */
    struct {
      short int      _ucbfsct; /* Data set sequence count  */
      short int      _ucbfseq; /* Data set sequence number */
      } _ucb_struct2;
    void * __ptr32 _ucbxtadr;   /* Address of UCS UCB extension (1403 */
    void * __ptr32 _ucbiosba;   /* Address of IOSB.  Set by IOS for   */
    struct {
      short int      _ucbstart; /* Last start address                 */
      char           _ucbopen;  /* Number of DCB's that are currently */
      unsigned char  _ucbgcb;   /* Graphic control byte used for      */
      } _ucb_struct3;
    void * __ptr32 _ucbrv040;   /* Reserved for use as teleprocessing */
    } _ucb_union1;
  union {
    struct {
      struct {
        void * __ptr32 _ucbapub; /* 3838 VPSS APUB address */
        } ucbrv066;
      unsigned char  _filler6[4];
      } _ucb_struct4;
    struct {
      struct {
        unsigned char  _ucbgraf;      /* Graphics status flags (BTAM) */
        unsigned int   _ucbirba : 24; /* Address of the IRB used for  */
        } ucbirb;
      struct {
        struct {
          struct {
            char           _ucbirln; /* Initialized RLN.  The relative line */
            } ucbinrln;
          unsigned int   _ucbrdyqa : 24; /* Asynchronous ready notification IRB */
          } ucbrdyq;
        } ucbldnca;
      } _ucb_struct5;
    struct {
      unsigned char  _ucbvoli[6]; /* Volume serial number */
      unsigned char  _ucbstab;    /* Volume status        */
      unsigned char  _ucbdmct;    /* Volume use byte      */
      } _ucb_struct6;
    unsigned char  _filler7[8];  /* UCBVOLI, UCBSTAB and UCBDMCT as in */
    struct {
      void * __ptr32 _ucbteb;    /* Address of Task Entry (TE) block */
      unsigned char  _ucbsns[4]; /* Sense information                */
      } _ucb_struct7;
    struct {
      void * __ptr32 _ucbicncb;    /* Pointer to VTAM's ICNCB */
      unsigned char  _filler8[4];
      } _ucb_struct8;
    struct {
      unsigned char  _ucbctcf1;    /* Channel-to-channel (CTC) device flag */
      unsigned char  _ucbrv042[3]; /* Reserved for CTC owner               */
      void * __ptr32 _ucbctcwa;    /* IECTCATN work area address           */
      } _ucb_struct9;
    } _ucb_union2;
  union {
    struct {
      struct {
        char           _ucbdi;       /* Device or devices on a control unit */
        unsigned int   _ucbbtb : 24; /* Address of buffer table             */
        } ucbbta;
      unsigned char  _filler9[4];
      } _ucb_struct10;
    struct {
      struct {
        char           _ucbrln;        /* Device index.  Index to the DEB UCB */
        unsigned int   _ucbctlna : 24; /* Control block link.  If the device  */
        } ucbctlnk;
      unsigned char  _filler10[4];
      } _ucb_struct11;
    struct {
      struct {
        unsigned char  _ucbcf2b0; /* First byte of IOS CTC flags     @L5A */
        unsigned char  _ucbcf2b1; /* Second byte of IOS CTC flags    @L5A */
        unsigned char  _ucbcf2b2; /* Third byte of IOS CTC flags     @L5A */
        unsigned char  _ucbcdcmd; /* FCTC Debug cmd code (flag byte3)@L5A */
        } ucbctcf2;
      unsigned char  _filler11[4];
      } _ucb_struct12;
    struct {
      char           _ucbsqc;       /* Number of reserve macro instructions */
      unsigned char  _ucbfl4;       /* Direct access flag byte              */
      short int      _ucbuser;      /* Number of current users              */
      unsigned char  _ucbobs1x;     /* Device dependent seg extension @01A  */
      unsigned char  _filler12[3];
      } _ucb_struct13;
    struct {
      unsigned char  _ucbfser[6]; /* Before open, message IDs.  See       */
      unsigned char  _ucbtfl2;    /* Flag byte                       @H2C */
      unsigned char  _ucbtfl1;    /* Flag byte                            */
      } _ucb_struct14;
    struct {
      unsigned char  _filler13[4];
      void * __ptr32 _ucbbase;      /* Address of base exposure UCB */
      } _ucb_struct15;
    } _ucb_union3;
  union {
    struct {
      unsigned char  _ucbvopt;      /* -               VOLUME STATISTICS OPTION BITS */
      unsigned int   _ucbxtnb : 24; /* Address of the Segment Extension              */
      } ucbxtn;
    void * __ptr32 _ucbnexp; /* Base - address of first exposure */
    } _ucb_union4;
  union {
    struct {
      struct {
        short int      _ucbctd; /* Serial number in binary of tape */
        } ucbmt;
      char           _ucbobrid;     /* Outboard recorder ID            @H1A */
      char           _ucbmdrid;     /* Miscellaneous data record ID    @H1A */
      char           _ucbtr;        /* The number (binary) of temporary     */
      unsigned char  _filler14[3];
      } _ucb_struct16;
    unsigned char  _ucbpavbi[8]; /* Reserved for IOS use           @01A */
    struct {
      unsigned char  _filler15[4];
      unsigned char  _ucbmtfl1;     /* MSGDISP dismount request         */
      char           _ucbtw;        /* The number (binary) of temporary */
      short int      _ucbsio;       /* The number (binary) of start I/O */
      } _ucb_struct17;
    } _ucb_union5;
  char           ucbpr;         /* The number (binary) of permanent     */
  char           ucbpw;         /* The number (binary) of permanent     */
  struct {
    char           _ucbnb;  /* The number (binary) of noise blocks */
    unsigned char  _ucbms;  /* Mode set operation code for data    */
    short int      _ucberg; /* The number (binary) of erase gaps   */
    short int      _ucbcln; /* The number (binary) of cleaner      */
    } ucbser;
  };

#define ucbwtoid  ucbwtowd._ucbwtoid
#define ucbddti   ucbddt._ucbddti
#define ucbjbnr   ucbob._ucbjbnr
#define ucbfl5    ucbob._ucbfl5
#define ucbid     ucbob._ucbid
#define ucbstat   ucbob._ucbstat
#define ucbfla    ucbsfls._ucbfla
#define ucbflb    ucbsfls._ucbflb
#define ucbtbyt1  ucbtyp._ucbtbyt1
#define ucbtbyt2  ucbtyp._ucbtbyt2
#define ucbtbyt3  ucbtyp.ucbdvcls._ucbtbyt3
#define ucbtbyt4  ucbtyp.ucbuntyp._ucbtbyt4
#define ucbaof1   _ucb_union1._ucb_struct1.ucbaof._ucbaof1
#define ucbaof2   _ucb_union1._ucb_struct1.ucbaof._ucbaof2
#define ucbatnct  _ucb_union1._ucb_struct1._ucbatnct
#define ucbctcal  _ucb_union1.ucbctcad._ucbctcal
#define ucbvtoc   _ucb_union1._ucbvtoc
#define ucbfsct   _ucb_union1._ucb_struct2._ucbfsct
#define ucbfseq   _ucb_union1._ucb_struct2._ucbfseq
#define ucbxtadr  _ucb_union1._ucbxtadr
#define ucbiosba  _ucb_union1._ucbiosba
#define ucbstart  _ucb_union1._ucb_struct3._ucbstart
#define ucbopen   _ucb_union1._ucb_struct3._ucbopen
#define ucbgcb    _ucb_union1._ucb_struct3._ucbgcb
#define ucbrv040  _ucb_union1._ucbrv040
#define ucbapub   _ucb_union2._ucb_struct4.ucbrv066._ucbapub
#define ucbgraf   _ucb_union2._ucb_struct5.ucbirb._ucbgraf
#define ucbirba   _ucb_union2._ucb_struct5.ucbirb._ucbirba
#define ucbirln   _ucb_union2._ucb_struct5.ucbldnca.ucbrdyq.ucbinrln._ucbirln
#define ucbrdyqa  _ucb_union2._ucb_struct5.ucbldnca.ucbrdyq._ucbrdyqa
#define ucbvoli   _ucb_union2._ucb_struct6._ucbvoli
#define ucbstab   _ucb_union2._ucb_struct6._ucbstab
#define ucbdmct   _ucb_union2._ucb_struct6._ucbdmct
#define ucbteb    _ucb_union2._ucb_struct7._ucbteb
#define ucbsns    _ucb_union2._ucb_struct7._ucbsns
#define ucbicncb  _ucb_union2._ucb_struct8._ucbicncb
#define ucbctcf1  _ucb_union2._ucb_struct9._ucbctcf1
#define ucbrv042  _ucb_union2._ucb_struct9._ucbrv042
#define ucbctcwa  _ucb_union2._ucb_struct9._ucbctcwa
#define ucbdi     _ucb_union3._ucb_struct10.ucbbta._ucbdi
#define ucbbtb    _ucb_union3._ucb_struct10.ucbbta._ucbbtb
#define ucbrln    _ucb_union3._ucb_struct11.ucbctlnk._ucbrln
#define ucbctlna  _ucb_union3._ucb_struct11.ucbctlnk._ucbctlna
#define ucbcf2b0  _ucb_union3._ucb_struct12.ucbctcf2._ucbcf2b0
#define ucbcf2b1  _ucb_union3._ucb_struct12.ucbctcf2._ucbcf2b1
#define ucbcf2b2  _ucb_union3._ucb_struct12.ucbctcf2._ucbcf2b2
#define ucbcdcmd  _ucb_union3._ucb_struct12.ucbctcf2._ucbcdcmd
#define ucbsqc    _ucb_union3._ucb_struct13._ucbsqc
#define ucbfl4    _ucb_union3._ucb_struct13._ucbfl4
#define ucbuser   _ucb_union3._ucb_struct13._ucbuser
#define ucbobs1x  _ucb_union3._ucb_struct13._ucbobs1x
#define ucbfser   _ucb_union3._ucb_struct14._ucbfser
#define ucbtfl2   _ucb_union3._ucb_struct14._ucbtfl2
#define ucbtfl1   _ucb_union3._ucb_struct14._ucbtfl1
#define ucbbase   _ucb_union3._ucb_struct15._ucbbase
#define ucbvopt   _ucb_union4.ucbxtn._ucbvopt
#define ucbxtnb   _ucb_union4.ucbxtn._ucbxtnb
#define ucbnexp   _ucb_union4._ucbnexp
#define ucbctd    _ucb_union5._ucb_struct16.ucbmt._ucbctd
#define ucbobrid  _ucb_union5._ucb_struct16._ucbobrid
#define ucbmdrid  _ucb_union5._ucb_struct16._ucbmdrid
#define ucbtr     _ucb_union5._ucb_struct16._ucbtr
#define ucbpavbi  _ucb_union5._ucbpavbi
#define ucbmtfl1  _ucb_union5._ucb_struct17._ucbmtfl1
#define ucbtw     _ucb_union5._ucb_struct17._ucbtw
#define ucbsio    _ucb_union5._ucb_struct17._ucbsio
#define ucbnb     ucbser._ucbnb
#define ucbms     ucbser._ucbms
#define ucberg    ucbser._ucberg
#define ucbcln    ucbser._ucbcln

/* Values for field "ucbfl6" */
#define ucbasun     0x80  /* Assign/unassign commands                           */
#define ucbmdisp    0x40  /* Device has message display                         */
#define ucbdbuf     0x20  /* Data is buffered prior to                          */
#define ucbids      0x10  /* Block ID supported on this                         */
#define ucbselfd    0x08  /* Indicates whether the device                       */
#define ucbsmsmm    0x04  /* Indicates that the device is a                     */
#define ucblerp     0x02  /* Flag indicating that basic and                     */
#define ucbiot      0x01  /* Flag indicating that the I/O timing                */

/* Values for field "ucbati" */
#define ucbrsv04    0x80  /* Reserved                                           */
#define ucbrsv05    0x40  /* Reserved                                           */
#define ucbrsv06    0x20  /* Reserved                                           */
#define ucbrsv07    0x10  /* Reserved                                           */
#define ucbrsv08    0x08  /* Reserved                                           */
#define ucbrsv09    0x04  /* Reserved                                           */
#define ucbhali     0x02  /* Optional job entry subsystem (JES)                 */
#define ucbhpdv     0x01  /* Optional job entry subsystem (JES)                 */

/* Values for field "ucbflp1" */
#define ucbnsrch    0x80  /* The currently allocated volume was                 */
#define ucbshrup    0x40  /* Shareable when in uniprocessor mode                */
#define ucbrerp     0x20  /* Resident error routine                             */
#define ucbinhio    0x10  /* Inhibit halt subchannel from SVC 33                */
#define ucbswapf    0x08  /* With bit set, the device is able to                */
#define ucberlog    0x04  /* Indicates presence of an error log                 */
#define ucbdynph    0x02  /* If 1,dynamic pathing availability                  */
#define ucbraloc    0x01  /* Allocations to this device are                     */

/* Values for field "ucbfl7" */
#define ucbmasgn    0x80  /* Multi-system assign done                           */
#define ucbsspnd    0x40  /* Suspended channel program                          */
#define ucbautos    0x20  /* Device is auto-switchable       @P3C               */
#define ucbnosel    0x10  /* Allocation should attempt to select                */
#define ucbeidaw    0x08  /* 4K 8Byte IDAWs supported        @L4C               */
#define ucbasafh    0x04  /* This device is assigned to a                       */
#define ucbprun     0x02  /* This tape device is in unallocation                */
#define ucbponli    0x01  /* For use by Allocation only      @P5C               */

/* Values for field "ucbcsflg" */
#define ucbncc3     0x80  /* Indicates that IOS marked the                      */
#define ucbhswap    0x40  /* The device is enabled for                          */
#define ucbdrsn     0x20  /* Indicates that the device is                       */
#define ucballfc    0x10  /* Indicates if all channels to the                   */
#define ucbonefc    0x08  /* Indicates if at least one of the                   */
#define ucbcnpth    0x04  /* Indicates that a no operational                    */
#define ucbmidaw    0x02  /* Indicates that MIDAWs are supported                */
#define ucbfcx      0x01  /* Indicates that FICON Channel                       */
#define ucbzhpf     0x01  /* Alternate name for UCBFCX       @0DA               */

/* Values for field "ucbfl8" */
#define ucbspecl    0x80  /* Indicates that a device is marked                  */
#define ucbscdry    0x40  /* Indicates that a device is a                       */
#define ucbprrsn    0x20  /* Indicates that the device is                       */
#define ucbsmrsn    0x10  /* Indicates that the device is                       */
#define ucbcmonr    0x08  /* Monitoring is required for this                    */
#define ucbsynchior 0x04  /* Indicates that synchronous I/O                     */
#define ucbsynchiow 0x02  /* Indicates that synchronous I/O                     */
#define ucbcrs      0x01  /* Indicates that consistent read                     */

/* Values for field "ucbioq" */
#define ucbcurpx    0x08  /* Actual prefix stub data length  @P8C               */
#define ucbprfx     0x200 /* Total prefix area length for prefix                */

/* Values for field "ucbjbnr" */
#define ucbvrdev    0x80  /* UCB for VIO device                                 */
#define ucbjes3     0x40  /* All volume mounting and device                     */
#define ucbduc      0x20  /* Display device unit check                          */
#define ucbj3dv     0x10  /* Device is defined to JES3       @09A               */
#define ucboldsm    0x08  /* OLTEP communicating directly with                  */
#define ucbmmsgp    0x04  /* Mount message pending.  The device                 */
#define ucbdcons    0x02  /* Disabled console support controls                  */
#define ucbmont     0x01  /* Volume to be mounted is to be                      */

/* Values for field "ucbfl5" */
#define ucbdcc      0x80  /* Disconnect command chain device                    */
#define ucbaf       0x40  /* Attention for this console device                  */
#define ucbamv      0x40  /* Successful comparison checking of                  */
#define ucbsms      0x20  /* Data management flag            @L6C               */
#define ucbvsdr     0x10  /* Device has variable length SDRs                    */
#define ucbenvrd    0x08  /* Device returns environmental data                  */
#define ucbnaloc    0x04  /* This offline device is being used by               */
#define ucbaltcu    0x02  /* Device has an alternate control unit               */
#define ucbcuir     0x01  /* Indicates whether the device is                    */

/* Values for field "ucbid" */
#define ucbstnd     0xFF  /* UCB identifier                                     */
#define ucbidcpy    0xCC  /* UCB identifier for a UCB copy   @P3A               */
#define ucbgucb     0x11  /* UCB identifier for UCBs that are                   */
#define ucbst1      0x12  /* UCB identifier for UCBs that                       */
#define ucbst3      0xFD  /* UCB identifier for UCBs that                       */

/* Values for field "ucbstat" */
#define ucbonli     0x80  /* Device is online                                   */
#define ucbchgs     0x40  /* Device status is to be changed from                */
#define ucbresv     0x20  /* The mount status of the volume on                  */
#define ucbunld     0x10  /* Unload operator command has been                   */
#define ucbaloc     0x08  /* Device is allocated.                               */
#define ucbpres     0x04  /* The mount status of the volume on                  */
#define ucbsysr     0x02  /* System residence device or primary                 */
#define ucbdadi     0x01  /* Standard tape labels have been                     */

/* Values for field "ucbflb" */
#define ucbincpt    0x80  /* An intercept condition exists                      */
#define ucbnopth    0x40  /* Device has no operational paths.                   */
#define ucbnocon    0x20  /* Device is not connected to a                       */
#define ucbhilvl    0x10  /* Non-normal UCBLEVEL value has been                 */
#define ucbhdet     0x08  /* HOT-I/O detected, device boxed                     */
#define ucbiosn     0x04  /* I/O deferred waiting on         @M3A               */

/* Values for field "ucbwgt" */
#define ucbin       0x80  /* SYSIN                                              */
#define ucbout      0x40  /* SYSOUT                                             */
#define ucbpub      0x20  /* Assumed that this device will be                   */
#define ucbrew      0x10  /* Rewind command has been addressed to               */
#define ucbmtpxp    0x08  /* Parallel access volume          @M3C               */
#define ucbvorsn    0x04  /* Vary command operator reason                       */
#define ucbvhrsn    0x02  /* Vary command hierarchy reason                      */
#define ucbvlrsn    0x01  /* Vary command library reason                        */

/* Values for field "ucbtbyt1" */
#define ucb1fea0    0x80  /* Bit 0                                              */
#define ucb1fea1    0x40  /* Bit 1                                              */
#define ucb1fea2    0x20  /* Bit 2                                              */
#define ucb1fea3    0x10  /* Bit 3                                              */
#define ucb1fea4    0x08  /* Bit 4                                              */
#define ucb1fea5    0x04  /* Bit 5                                              */
#define ucb1fea6    0x02  /* Bit 6                                              */
#define ucb1fea7    0x01  /* Bit 7                                              */

/* Values for field "ucbtbyt2" */
#define ucb2opt0    0x80  /* Flag 0                                             */
#define ucb2opt1    0x40  /* Flag 1                                             */
#define ucb2opt2    0x20  /* Flag 2                                             */
#define ucb2opt3    0x10  /* Flag 3                                             */
#define ucb2opt4    0x08  /* Flag 4                                             */
#define ucb2opt5    0x04  /* Flag 5                                             */
#define ucb2opt6    0x02  /* Flag 6                                             */
#define ucb2opt7    0x01  /* Flag 7                                             */

/* Values for field "ucbtbyt3" */
#define ucb3tape    0x80  /* Tape                                               */
#define ucb3comm    0x40  /* Communications                                     */
#define ucb3ctc     0x41  /* Channel-to-channel adapter                         */
#define ucb3dacc    0x20  /* Direct access                                      */
#define ucb3disp    0x10  /* Display                                            */
#define ucb3urec    0x08  /* Unit record                                        */
#define ucb3char    0x04  /* Character reader                                   */
#define ucbrsv10    0x02  /* Reserved                                           */
#define ucbrsv11    0x01  /* Reserved                                           */

/* Values for field "ucbflc" */
#define ucbattp     0x80  /* Attention pending                                  */
#define ucbitfp     0x40  /* Intercept condition pending                        */
#define ucbude      0x20  /* Unsolicited device end received                    */
#define ucbivrs     0x08  /* Intervention required message                      */
#define ucbivrr     0x04  /* Intervention required message                      */
#define ucbddrsw    0x01  /* DDR switch pending on this device                  */

/* Values for field "ucbaof1" */
#define ucbofmcr    0x80  /* Magnetic card reader adapter                       */
#define ucbofsp     0x40  /* Selector pen - for 3277 only                       */
#define ucbofnl     0x20  /* Numeric lock - for 3277 only                       */
#define ucbofptr    0x10  /* Prepare to read feature                            */
#define ucbrsv65    0x08  /* Reserved - set to zero                             */
#define ucbrsv66    0x04  /* Reserved - set to zero                             */
#define ucbrsv67    0x02  /* Reserved - set to zero                             */
#define ucbrsv68    0x01  /* Reserved - set to zero                             */

/* Values for field "ucbaof2" */
#define ucbrsv69    0x80  /* Reserved - set to zero                             */
#define ucbrsv70    0x40  /* Reserved - set to zero                             */
#define ucbrsv71    0x20  /* Reserved - set to zero                             */
#define ucbrsv72    0x10  /* Reserved - set to zero                             */
#define ucbrsv73    0x08  /* Reserved - set to zero                             */
#define ucbrsv74    0x04  /* Reserved - set to zero                             */
#define ucbrsv75    0x02  /* Reserved - set to zero                             */
#define ucbrsv76    0x01  /* Reserved - set to zero                             */

/* Values for field "_filler5" */
#define ucboltep    0x80  /* OLTEP in control of the device                     */
#define ucbrsv77    0x40  /* Reserved - set to zero                             */
#define ucbrsv78    0x20  /* Reserved - set to zero                             */
#define ucbrsv79    0x10  /* Reserved - set to zero                             */
#define ucbrtiac    0x08  /* Read TI active                                     */
#define ucbripnd    0x04  /* Read initial pending                               */
#define ucbskpfg    0x02  /* Skip flag                                          */
#define ucbatrcd    0x01  /* Attention received from the                        */

/* Values for field "ucbgraf" */
#define ucboip      0x80  /* Open is in progress                                */
#define ucbdro      0x40  /* Device ready in open                               */
#define ucbdrno     0x20  /* Device ready - not in open                         */
#define ucbbtam     0x10  /* Use BTAM - IGG019UP                                */
#define ucbupm      0x08  /* Use provided module                                */
#define ucbrpnd     0x04  /* Ready processing not done                          */
#define ucbdwnr     0x02  /* Device went not ready                              */
#define ucbrv039    0x01  /* Reserved - BTAM                                    */

/* Values for field "ucbstab" */
#define ucbbsvl     0x80  /* Volume demountable by data                         */
#define ucbdvshr    0x80  /* Device not shareable among several                 */
#define ucbpgfl     0x40  /* UCB is open and is being used as a                 */
#define ucbprsrs    0x20  /* During volume attribute processing                 */
#define ucbbalb     0x20  /* Additional volume label processing                 */
#define ucbbprv     0x10  /* Private - volume use status                        */
#define ucbbpub     0x08  /* Public - volume use status                         */
#define ucbbstr     0x04  /* Storage - volume use status                        */
#define ucbshar     0x02  /* Volume shareable among job steps                   */
#define ucbbnul     0x01  /* Control volume - A catalog data set                */

/* Values for field "ucbdmct" */
#define ucbmount    0x80  /* If 0, a mount verification has been                */
#define ucbdmc      0x7F  /* Number of DCB's open for this volume               */

/* Values for field "ucbicncb" */
#define ucb3791l    0xF1  /* 3791 Local control unit                            */
#define ucb42ad1    0x11  /* 2702 Control unit with type 1                      */

/* Values for field "ucbctcf1" */
#define ucbctc80    0x80  /* If this bit is on, above word has                  */
#define ucbrv076    0x40  /* Reserved for CTC owner                             */
#define ucbrv077    0x20  /* Reserved for CTC owner                             */
#define ucbrv078    0x10  /* Reserved for CTC owner                             */
#define ucbrv079    0x08  /* Reserved for CTC owner                             */
#define ucbrv080    0x04  /* Reserved for CTC owner                             */
#define ucbrv081    0x02  /* Reserved for CTC owner                             */
#define ucbrv082    0x01  /* Reserved for CTC owner                             */

/* Values for field "ucbcf2b0" */
#define ucbcclaw    0x80  /* CTC owner is using CLAW protocol,                  */
#define ucbnormf    0x80  /* Prevent RMF from issuing asynch                    */
#define ucbcabyp    0x40  /* If set, bypass attention routine                   */
#define ucbcemua    0x20  /* CTC owner has indicated that                       */
#define ucbcdiag    0x10  /* Diagnostic command is supported                    */

/* Values for field "ucbcdcmd" */
#define ucbpctc     0x00  /* Parallel CTC                    @01A               */
#define ucbsctc     0x01  /* Serial CTC                      @01A               */
#define ucbbctc     0x02  /* Basic Mode ESCON CTC            @02A               */
#define ucbrs6k     0x03  /* RS6000 acting like a CTC        @D1A               */
#define ucb3172     0x04  /* 3172 acting like a CTC          @D1A               */
#define ucbosa      0x05  /* OSA device                      @03A               */
#define ucbosad     0x06  /* OSA diagnostic device           @03A               */
#define ucbiqd      0x07  /* Internal Queued Direct                             */
#define ucbosn      0x08  /* OSA NCP (OSN) device            @07A               */
#define ucbosx      0x09  /* OSX (OSA zBX Data Network)      @08A               */
#define ucbosm      0x0A  /* OSM (OSA zBX Management                            */
#define ucbosaf     0x0F  /* OSA reserved device types B-F   @08C               */
#define ucbfctc     0x20  /* FICON CTC                       @06A               */
#define ucbfbrc     0x21  /* Fabric discovery device         @L6A               */

/* Values for field "ucbfl4" */
#define ucbmdse1    0x80  /* DSE1 is required during MSI    @01C                */
#define ucbwdav     0x40  /* DAVV waiting for mount                             */
#define ucbdpavb    0x20  /* PAV-base capabable device      @01C                */
#define ucbdpava    0x10  /* PAV-alis device                @01C                */
#define ucbsdse1    0x08  /* DSE1 is required during SIO    @01C                */
#define ucbdcmbu    0x04  /* CMB update required            @02A                */
#define ucbdpavh    0x02  /* HiperPAV base or alias device  @9zA                */

/* Values for field "ucbtfl2" */
#define ucbtxms     0x80  /* Extended mode set supported     @H2A               */
#define ucbtpsf     0x40  /* Perform Subsystem Function      @H2A               */
#define ucbtvcmp    0x20  /* Volume contains compacted data  @H2A               */
#define ucbtlpos    0x10  /* ERP detected permanent error -  @H2A               */
#define ucbtdmsi    0x08  /* DSE1 is required during MSI     @H7A               */

/* Values for field "ucbtfl1" */
#define ucbnltp     0x80  /* Tape volume does not contain                       */
#define ucbnsltp    0x40  /* Tape contains non-standard                         */
#define ucbdqdsp    0x20  /* Dequeue tape volume when demounted                 */
#define ucbtfl1s    0x18  /* UCBTFL1 bits swapped by DDR                        */
#define ucbrv005    0x10  /* Unused                          @H1C               */
#define ucbcsl      0x08  /* ACL feature present                                */
#define ucbcslac    0x04  /* ACL active                                         */
#define ucblkahp    0x02  /* Lookahead mount pending                            */
#define ucbblp      0x01  /* Bypass label processing         @01C               */

/* Values for field "ucbvopt" */
#define ucbesv      0x80  /* -             ERROR STATISTICS BY VOLUME (ESV)     */
#define ucbeva      0x40  /* -             ERROR VOLUME ANALYSIS (EVA) RECORDS  */
#define ucbesvc     0x20  /* -             IF 0, ESV RECORDS SENT TO SYS1.MAN   */
#define ucberpc     0x10  /* -             AN ERROR RECOVERY PROCEDURE HAS      */
#define ucbesve     0x08  /* -             AN ESV RECORD HAS BEEN ISSUED FOR    */
#define ucbperr     0x04  /* -             ERP DETECTED PERM ERROR. TAPE        */
#define ucbrsv21    0x02  /* -       RESERVED                                   */
#define ucbrsv22    0x01  /* -       RESERVED                                   */

/* Values for field "ucbmtfl1" */
#define ucbmtdsm    0x80  /* DISP=D (dismount)                                  */
#define ucbmtkep    0x40  /* DISP=K (keep)                                      */
#define ucbmtret    0x20  /* DISP=R (retain)                                    */

/* Values for field "ucbcln" */
#define ucbd1600    0x04  /* 1600 BPI                                           */
#define ucbd6250    0x02  /* 6250 BPI                                           */
#define ucbdudn1    0x20  /* Dual density 800/1600 BPI                          */
#define ucbdudn2    0x10  /* Dual density 1600/6250 BPI                         */
#define ucbrwtau    0x08  /* Read/write tape control                            */
#define ucbcompa    0x04  /* -             Compaction feature              @H2A */
#define ucb3400     0x03  /* 3400 magnetic tape                                 */
#define ucb3423     0x82  /* 3423 magnetic tape              @H4A               */
#define ucb3480     0x80  /* 3480 magnetic tape                                 */
#define ucb3490     0x81  /* 3490 magnetic tape              @H3A               */
#define ucb3591     0x83  /* 3590 magnetic tape              @H5A               */

#endif

#ifndef __ucbocr__
#define __ucbocr__

struct ucbocr {
  unsigned char  ucbfrid[4];  /* Current format record ID (FRID) */
  unsigned char  ucbrdata[4]; /* Command data                    */
  };

#endif

#ifndef __ucb3540x__
#define __ucb3540x__

struct ucb3540x {
  unsigned char  ucbvlser[6]; /* 3540 VOLID             */
  unsigned char  ucbdkbyt;    /* Flag byte              */
  unsigned char  ucbrv073;    /* Reserved - set to zero */
  };

/* Values for field "ucbdkbyt" */
#define ucbdkamx 0x80 /* IBM-supplied diskette reader,       */
#define ucbvlver 0x40 /* Volume verification is required for */
#define ucbrv067 0x20 /* Reserved - set to zero              */
#define ucbrv068 0x10 /* Reserved - set to zero              */
#define ucbrv069 0x08 /* Reserved - set to zero              */
#define ucbrv070 0x04 /* Reserved - set to zero              */
#define ucbrv071 0x02 /* Reserved - set to zero              */
#define ucbrv072 0x01 /* Reserved - set to zero              */

#endif

#ifndef __ucb3800x__
#define __ucb3800x__

struct ucb3800x {
  unsigned char  ucboptns;    /* Optional features installed on     */
  char           ucbcgmno;    /* Number of writeable character      */
  unsigned char  ucbgrafs;    /* Graphic character flag byte        */
  unsigned char  ucbactiv;    /* Active features                    */
  unsigned char  ucbcgmid[4]; /* Four one byte ID's for character   */
  unsigned char  ucbchar1[4]; /* Name of first translate table      */
  unsigned char  ucbchar2[4]; /* Name of second translate table     */
  unsigned char  ucbchar3[4]; /* Name of third translate table      */
  unsigned char  ucbchar4[4]; /* Name of fourth translate table     */
  unsigned char  ucbfcbnm[4]; /* Forms control buffer (FCB) image   */
  unsigned char  ucbimage[4]; /* Forms overlay image identification */
  short int      ucbldata;    /* Lost data page count               */
  short int      ucbpgid;     /* ID of the last fused page for      */
  struct {
    char           _ucbrv075;      /* Reserved - set to zero */
    unsigned int   _ucbmdrba : 24; /* MDR buffer address     */
    } ucbmdrbf;
  };

#define ucbrv075 ucbmdrbf._ucbrv075
#define ucbmdrba ucbmdrbf._ucbmdrba

/* Values for field "ucboptns" */
#define ucbmdlbt 0xF0 /* Model                         */
#define ucbrv055 0x08 /* Reserved - set to zero        */
#define ucbrv056 0x04 /* Reserved - set to zero        */
#define ucbbrstr 0x02 /* Burster/trimmer/stacker       */
#define ucbrv083 0x01 /* Reserved - set to zero        */

/* Values for field "ucbgrafs" */
#define ucbrv046 0x80 /* Reserved - set to zero        */
#define ucbrv047 0x40 /* Reserved - set to zero        */
#define ucbrv048 0x20 /* Reserved - set to zero        */
#define ucbrv049 0x10 /* Reserved - set to zero        */
#define ucbgraf0 0x08 /* WCGM 0 has been modified by a */
#define ucbgraf1 0x04 /* WCGM 1 has been modified by a */
#define ucbgraf2 0x02 /* WCGM 2 has been modified by a */
#define ucbgraf3 0x01 /* WCGM 3 has been modified by a */

/* Values for field "ucbactiv" */
#define ucbrv057 0x80 /* Reserved - set to zero        */
#define ucbrv058 0x40 /* Reserved - set to zero        */
#define ucbrv059 0x20 /* Reserved - set to zero        */
#define ucbrv060 0x10 /* Reserved - set to zero        */
#define ucbrv061 0x08 /* Reserved - set to zero        */
#define ucbrv062 0x04 /* Reserved - set to zero        */
#define ucbrv063 0x02 /* Reserved - set to zero        */
#define ucbbrsta 0x01 /* Reserved - set to zero        */

#endif

#ifndef __ucbucs__
#define __ucbucs__

struct ucbucs {
  unsigned char  ucbucsid[4]; /* UCS image identification in buffer  */
  unsigned char  ucbucsop;    /* Format of UCS image in buffer       */
  unsigned char  ucbfcbop;    /* Reserved (1403) or FCB options      */
  unsigned char  ucbrsv51;    /* Reserved - set to zero              */
  char           ucbercnt;    /* Contains a count of the errors that */
  unsigned char  ucbfcbid[4]; /* The FCB image identification        */
  void * __ptr32 ucberadr;    /* The address of the ERP logout area  */
  unsigned char  ucbipgid[2]; /* Impact printer page ID for last     */
  short int      ucbpdcto;    /* Offset to printer device            */
  };

/* Values for field "ucbucsop" */
#define ucbucso1 0x80 /* UCS image is a default image         */
#define ucbucso2 0x40 /* UCS image is in fold mode            */
#define ucbrsv39 0x20 /* Reserved - set to zero               */
#define ucbrsv40 0x10 /* Reserved - set to zero               */
#define ucbrsv41 0x08 /* Reserved - set to zero               */
#define ucbrsv42 0x04 /* Reserved - set to zero               */
#define ucbrsv43 0x02 /* Reserved - set to zero               */
#define ucbucspe 0x01 /* UCS image has parity error (3211)    */

/* Values for field "ucbfcbop" */
#define ucbfcbo1 0x80 /* FCB image is a default image         */
#define ucbrsv44 0x40 /* Reserved - set to zero               */
#define ucbrsv45 0x20 /* Reserved - set to zero               */
#define ucbrsv46 0x10 /* Reserved - set to zero               */
#define ucbfcbps 0x0C /* Printer speed setting for a variable */
#define ucbrsv49 0x02 /* Reserved - set to zero               */
#define ucbfcbpe 0x01 /* FCB image has parity error           */

#endif

#ifndef __ucbpdcta__
#define __ucbpdcta__

struct ucbpdcta {
  unsigned char  ucbpdct[16]; /* Printer device characteristics */
  };

/* Values for field "ucbpdct" */
#define ucb3211 0x09 /* 3211 Printer                             */
#define ucb3800 0x0E /* 3800 Printing Subsystem                  */
#define ucbafp1 0x0F /* Printer support                          */
#define ucb3263 0x11 /* 3263 Printer                             */
#define ucb4245 0x11 /* 4245 Printer                             */
#define ucb4248 0x13 /* 4248 Printer                             */
#define ucb3895 0x19 /* 3895 device                              */
#define ucbdir  0x3A /* ESCON or FICON Director             @L1A */
#define ucbdsm  0x42 /* Mass Storage Control (MSC) (3851)        */
#define ucb3838 0x4C /* 3838 Array Processor                     */
#define ucbfba  0x60 /* Fixed Block Architecture (FBA)      @FBA */

#endif

#pragma pack(reset)
