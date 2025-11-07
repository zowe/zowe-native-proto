#pragma pack(packed)

#ifndef __ucbcmext__
#define __ucbcmext__

struct ucbcmext {
  char           ucbeti;   /* A binary number used by the exit     */
  char           ucbsti;   /* Increment which, when multiplied by  */
  unsigned char  ucbfl6;   /* Device features byte                 */
  char           ucbati;   /* Index to the attention table (ANTAB) */
  char           ucbsnsct; /* Count of sense bytes presented       */
  unsigned char  ucbflp1;  /* Flag byte                            */
  unsigned char  ucbstli;  /* Statistics table lookup index        */
  unsigned char  ucbfl7;   /* Miscellaneous usage flags            */
  void * __ptr32 ucbiext;  /* Pointer to IOS UCB extension    @01C */
  unsigned char  ucbchprm; /* Channel path recovery mask           */
  char           ucbsati;  /* Attention table index                */
  short int      ucbasid;  /* ASID of the memory to which this     */
  struct {
    unsigned char  _filler1;     /* Reserved               */
    unsigned char  _ucbwtoid[3]; /* WTO message identifier */
    } ucbwtowd;
  struct {
    short int      _ucbddti;  /* Contains DDT name list index during */
    short int      _filler2;  /* Remainder of DDT address            */
    } ucbddt;
  void * __ptr32 ucbclext; /* Pointer to device class              */
  short int      ucbdctof; /* Device connect time                  */
  unsigned char  ucbcsflg; /* Miscellaneous flags which should     */
  unsigned char  ucbfl8;   /* Miscellaneous usage flags       @A6C */
  };

#define ucbwtoid ucbwtowd._ucbwtoid
#define ucbddti  ucbddt._ucbddti

/* Values for field "ucbfl6" */
#define ucbasun     0x80 /* Assign/unassign commands             */
#define ucbmdisp    0x40 /* Device has message display           */
#define ucbdbuf     0x20 /* Data is buffered prior to            */
#define ucbids      0x10 /* Block ID supported on this           */
#define ucbselfd    0x08 /* Indicates whether the device         */
#define ucbsmsmm    0x04 /* Indicates that the device is a       */
#define ucblerp     0x02 /* Flag indicating that basic and       */
#define ucbiot      0x01 /* Flag indicating that the I/O timing  */

/* Values for field "ucbati" */
#define ucbrsv04    0x80 /* Reserved                             */
#define ucbrsv05    0x40 /* Reserved                             */
#define ucbrsv06    0x20 /* Reserved                             */
#define ucbrsv07    0x10 /* Reserved                             */
#define ucbrsv08    0x08 /* Reserved                             */
#define ucbrsv09    0x04 /* Reserved                             */
#define ucbhali     0x02 /* Optional job entry subsystem (JES)   */
#define ucbhpdv     0x01 /* Optional job entry subsystem (JES)   */

/* Values for field "ucbflp1" */
#define ucbnsrch    0x80 /* The currently allocated volume was   */
#define ucbshrup    0x40 /* Shareable when in uniprocessor mode  */
#define ucbrerp     0x20 /* Resident error routine               */
#define ucbinhio    0x10 /* Inhibit halt subchannel from SVC 33  */
#define ucbswapf    0x08 /* With bit set, the device is able to  */
#define ucberlog    0x04 /* Indicates presence of an error log   */
#define ucbdynph    0x02 /* If 1,dynamic pathing availability    */
#define ucbraloc    0x01 /* Allocations to this device are       */

/* Values for field "ucbfl7" */
#define ucbmasgn    0x80 /* Multi-system assign done             */
#define ucbsspnd    0x40 /* Suspended channel program            */
#define ucbautos    0x20 /* Device is auto-switchable       @P3C */
#define ucbnosel    0x10 /* Allocation should attempt to select  */
#define ucbeidaw    0x08 /* 4K 8Byte IDAWs supported        @L4C */
#define ucbasafh    0x04 /* This device is assigned to a         */
#define ucbprun     0x02 /* This tape device is in unallocation  */
#define ucbponli    0x01 /* For use by Allocation only      @P5C */

/* Values for field "ucbcsflg" */
#define ucbncc3     0x80 /* Indicates that IOS marked the        */
#define ucbhswap    0x40 /* The device is enabled for            */
#define ucbdrsn     0x20 /* Indicates that the device is         */
#define ucballfc    0x10 /* Indicates if all channels to the     */
#define ucbonefc    0x08 /* Indicates if at least one of the     */
#define ucbcnpth    0x04 /* Indicates that a no operational      */
#define ucbmidaw    0x02 /* Indicates that MIDAWs are supported  */
#define ucbfcx      0x01 /* Indicates that FICON Channel         */
#define ucbzhpf     0x01 /* Alternate name for UCBFCX       @0DA */

/* Values for field "ucbfl8" */
#define ucbspecl    0x80 /* Indicates that a device is marked    */
#define ucbscdry    0x40 /* Indicates that a device is a         */
#define ucbprrsn    0x20 /* Indicates that the device is         */
#define ucbsmrsn    0x10 /* Indicates that the device is         */
#define ucbcmonr    0x08 /* Monitoring is required for this      */
#define ucbsynchior 0x04 /* Indicates that synchronous I/O       */
#define ucbsynchiow 0x02 /* Indicates that synchronous I/O       */
#define ucbcrs      0x01 /* Indicates that consistent read       */

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
