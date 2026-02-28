#pragma pack(packed)

#ifndef __ifgacb__
#define __ifgacb__

struct ifgacb {
  unsigned char  acbid;    /* ACB IDENTIFIER           */
  unsigned char  acbstyp;  /* ACB SUBTYPE              */
  struct {
    struct {
      short int      _acbleng2; /* ALTERNATE NAME FOR */
      } acblen2;
    } acbleng;
  struct {
    struct {
      struct {
        struct {
          void * __ptr32 _acbamwap; /* ACCESS METHOD WORKAREA */
          } acbsubnm;
        } acbibct;
      } acbjwa;
    } acbambl;
  void * __ptr32 acbinrtn; /* DATA MANAGEMENT          */
  struct {
    unsigned char  _acbmacr1; /* MACRF FIRST BYTE  */
    unsigned char  _acbmacr2; /* MACRF SECOND BYTE */
    } acbmacrf;
  char           acbbstno; /* NUMBER OF CONCURRENT     */
  char           acbstrno; /* NUMBER OF CONCURRENT     */
  short int      acbbufnd; /* NUMBER OF DATA RECORD    */
  short int      acbbufni; /* NUMBER OF INDEX RECORD   */
  struct {
    struct {
      unsigned char  _acbmacr3; /* MACRF THIRD BYTE X04SVHS */
      unsigned char  _acbshrp;  /* SHARED RESOURCE POOL     */
      short int      _acbjbuf;  /* NUMBER OF JOURNAL        */
      } acblfb;
    } acbbufpl;
  unsigned char  acbrecfm; /* RECORD FORMAT            */
  unsigned char  acbcctyp; /* CONTROL CHARACTER TYPE   */
  struct {
    struct {
      unsigned char  _acbdsor1; /* DSORG FIRST BYTE  */
      unsigned char  _acbdsor2; /* DSORG SECOND BYTE */
      } acbdsorg;
    } acbopt;
  void * __ptr32 acbmsgar; /* MSG AREA PTR     X04SVHS */
  void * __ptr32 acbpassw; /* PASSWORD ADDRESS         */
  struct {
    void * __ptr32 _acbuel; /* ALTERNATE NAME FOR */
    } acbexlst;
  union {
    unsigned char  _acbddnm[8]; /* DDNAME - MUST BE THE */
    struct {
      short int      _acbtiot;     /* OFFSET FROM TIOT ORIGIN */
      unsigned char  _acbinfl;     /* CONTENTS AND MEANING    */
      struct {
        unsigned char  _acbameth; /* ACCESS METHOD TYPE */
        } acbam;
      unsigned char  _acberfl;     /* FOR JES, CONTENTS AND   */
      unsigned int   _acbdeb : 24; /* DEB ADDRESS             */
      } _ifgacb_struct1;
    } _ifgacb_union1;
  unsigned char  acboflgs; /* OPEN / CLOSE FLAGS       */
  unsigned char  acberflg; /* ERROR FLAGS - FOR        */
  struct {
    unsigned char  _acbinfl1; /* FIRST IND FLAGS @ZA16012 */
    unsigned char  _acbinfl2; /* 2ND IND FLAGS   @ZA16012 */
    } acbinflg;
  struct {
    unsigned char  _acboptn;     /* JAM UCS INDICATORX04SVHS */
    unsigned char  _filler1[3];  /* RESERVED         X04SVHS */
    } acbujfcb;
  int            acbbufsp; /* VIRTUAL CORE AVAILABLE   */
  struct {
    short int      _acbmsgln; /* LNG OF MSG AREA  X04SVHS */
    } acbblksz;
  short int      acblrecl; /* LOGICAL RECORD LENGTH    */
  void * __ptr32 acbuaptr; /* USER WORKAREA ADDRESS;   */
  void * __ptr32 acbcbmwa; /* CONTROL BLOCK            */
  struct {
    void * __ptr32 _acbamax; /* ACCESS METHOD ACB */
    } acbapid;
  };

#define acbleng2 acbleng.acblen2._acbleng2
#define acbamwap acbambl.acbjwa.acbibct.acbsubnm._acbamwap
#define acbmacr1 acbmacrf._acbmacr1
#define acbmacr2 acbmacrf._acbmacr2
#define acbmacr3 acbbufpl.acblfb._acbmacr3
#define acbshrp  acbbufpl.acblfb._acbshrp
#define acbjbuf  acbbufpl.acblfb._acbjbuf
#define acbdsor1 acbopt.acbdsorg._acbdsor1
#define acbdsor2 acbopt.acbdsorg._acbdsor2
#define acbuel   acbexlst._acbuel
#define acbddnm  _ifgacb_union1._acbddnm
#define acbtiot  _ifgacb_union1._ifgacb_struct1._acbtiot
#define acbinfl  _ifgacb_union1._ifgacb_struct1._acbinfl
#define acbameth _ifgacb_union1._ifgacb_struct1.acbam._acbameth
#define acberfl  _ifgacb_union1._ifgacb_struct1._acberfl
#define acbdeb   _ifgacb_union1._ifgacb_struct1._acbdeb
#define acbinfl1 acbinflg._acbinfl1
#define acbinfl2 acbinflg._acbinfl2
#define acboptn  acbujfcb._acboptn
#define acbmsgln acbblksz._acbmsgln
#define acbamax  acbapid._acbamax

/* Values for field "acbid" */
#define acbidval 0xA0 /* IDENTIFIER VALUE - X'A0' */

/* Values for field "acbstyp" */
#define acbsvsam 0x10 /* VSAM SUBTYPE     X04SVHS */
#define acbsvrp  0x11 /* VRP SUBTYPE      X04SVHS */
#define acbscntl 0x12 /* VSAM CONTROL ACB    @LYA */
#define acbsvtam 0x20 /* VTAM SUBTYPE     X04SVHS */
#define acbs3540 0x40 /* 3540 SUBTYPE     X04SVHS */

/* Values for field "acbmacr1" */
#define acbkey   0x80 /* KEYED PROCESSING VIA     */
#define acbadr   0x40 /* ADDRESSED PROCESSING     */
#define acbadd   0x40 /* ALTERNATE NAME FOR       */
#define acbcnv   0x20 /* PROCESSING BY            */
#define acbblk   0x20 /* ALTERNATE NAME FOR       */
#define acbseq   0x10 /* SEQUENTIAL PROCESSING    */
#define acbdir   0x08 /* DIRECT PROCESSING        */
#define acbin    0x04 /* INPUT PROCESSING USING   */
#define acbout   0x02 /* OUTPUT PROCESSING USING  */
#define acbubf   0x01 /* USER CONTROLS BUFFERS -  */

/* Values for field "acbmacr2" */
#define acbntrun 0x80 /* DEFINED ONLY WHEN        */
#define acbccany 0x40 /* THE CONTROL CHARACTER    */
#define acbbwo   0x20 /* ELIGIBLE FOR BACKUP      */
#define acbskp   0x10 /* SKIP SEQUENTIAL          */
#define acblogon 0x08 /* LOGON REQUESTS TO AN     */
#define acbrst   0x04 /* SET DATA SET TO  X04SVHS */
#define acbdsn   0x02 /* BASIC SUBTASK SHARED     */
#define acbaix   0x01 /* ENTITY TO BE PROCESSED   */

/* Values for field "acbmacr3" */
#define acbnlw   0x80 /* NO EXCL CTL WAIT@YA41855 */
#define acblsr   0x40 /* LOCAL SHARED RESOURCE    */
#define acbgsr   0x20 /* GLOBAL SHARED RESOURCE   */
#define acbici   0x10 /* IMPROVED CONTROL         */
#define acbdfr   0x08 /* DEFER WRITES     X04SVHS */
#define acbsis   0x04 /* SEQUENTIAL INSERT        */
#define acbncfx  0x02 /* NFX=0/CFX=1      X04SVHS */
#define acbmode  0x01 /* 31-BIT BUFFER ADDRESS    */

/* Values for field "acbrecfm" */
#define acbrecaf 0x80 /* JES FORMAT               */

/* Values for field "acbcctyp" */
#define acbtrcid 0xC0 /* 3800 TRANSLATE TABLE+8   */
#define acbdlixi 0x10 /* CROSS INVALIDATE   @LSA  */
#define acbccasa 0x04 /* ASA CONTROL CHARACTERS   */
#define acbccmch 0x02 /* MACHINE CONTROL          */
#define acbccdsi 0x01 /* DATA STREAM. NO CONTROL  */

/* Values for field "acbdsor1" */
#define acbcrnck 0x80 /* NO CHECK BY RESTART      */
#define acbcrnre 0x40 /* DATA ADDED SINCE         */
#define acbdvind 0x20 /* DEVICE INDICATR @ZA26638 */
#define acboptj  0x20 /* 3800 CONTROL    @ZA26638 */

/* Values for field "acbdsor2" */
#define acbbypdb 0x80 /* 1=DO NOT ACCESS THE DATA */
#define acbbldix 0x40 /* BLDINDEX CALLING    @T0A */
#define acbzhw   0x20 /* ZHYPERWRITE         @02A */
#define acbdacom 0x10 /* DISABLE TVSAMCOM    @03A */
#define acbdorga 0x08 /* ACB INDICATOR            */

/* Values for field "acbameth" */
#define acbvtam  0x60 /* VTAM              X03004 */
#define acbsubs  0x41 /* SUBSYSTEMS               */
#define acbtcam  0x31 /* TCAM                     */
#define acbrci   0x23 /* JES/RCI                  */
#define acbrtam  0x22 /* JES/RTAM                 */
#define acbjam   0x21 /* JES/JAM                  */
#define acbvsam  0x11 /* VSAM                     */

/* Values for field "acboflgs" */
#define acbr31b  0x80 /* 31-BIT BUFR REQUEST @LPA */
#define acbr31c  0x40 /* 31-BIT CB REQUEST   @LPA */
#define acbeov   0x20 /* EOV CONCATENATION        */
#define acbopen  0x10 /* THE ACB IS OPEN          */
#define acbdserr 0x08 /* NO FURTHER REQUESTS ARE  */
#define acbrecov 0x04 /* ALLOW THE OPEN OF  @LYA  */
#define acbexfg  0x02 /* USER EXIT FLAG - SET TO  */
#define acblock  0x02 /* ALTERNATE NAME FOR       */
#define acbiosfg 0x01 /* OPEN/CLOSE IN CONTROL -  */
#define acbbusy  0x01 /* ALTERNATE NAME FOR       */

/* Values for field "acberflg" */
#define acboalr  0x04 /* THE ACB IS ALREADY OPEN  */
#define acbcalr  0x04 /* THE ACB IS NOT OPEN      */

/* Values for field "acbinfl1" */
#define acbjeps  0x40 /* JEPS IS USING THIS ACB   */
#define acbijrqe 0x20 /* AN RQE IS HELD BY JAM    */
#define acbcat   0x10 /* ACB FOR VSAM CATALOG     */
#define acbscra  0x08 /* CATALOG CONTROL BLOCK    */
#define acbucra  0x04 /* CATALOG CONTROL BLOCK    */
#define acbvvic  0x02 /* DATA SET BEING OPENED    */
#define acbsds   0x02 /* OPEN AS SYSTEM DATA SET  */
#define acbbypss 0x01 /* BYPASS SECURITY ON OPEN  */

/* Values for field "acbinfl2" */
#define acbswarn 0x80 /* SUPPRESS OPEN WARNING    */
#define acbsopen 0x40 /* SUPPRESS CLOSE CATALOG   */
#define acbcbic  0x20 /* OPEN WITH CONTROL BLOCKS */
#define acbcatx  0x10 /* CATX OPEN           @L1A */
#define acbcasrs 0x08 /* CAS RESTART         @PAS */
#define acbishrd 0x04 /* IGNORE SHR DASD @YA03157 */
#define acbshrop 0x03 /* SHARE OPTIONS       @X1A */
#define acbshr02 0x02 /* CROSS REG SHARE 2   @X1A */
#define acbshr01 0x01 /* CROSS REG SHARE 1   @X1A */

#endif

#pragma pack(reset)
