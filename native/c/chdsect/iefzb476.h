#pragma pack(1)

#ifndef __emdsect1__
#define __emdsect1__

struct emdsect1
{
  unsigned char emfunct;    /* Function indicator flags       */
  unsigned char emidnum;    /* Caller identifier number  @D1C */
  unsigned char emnmsgbk;   /* Number of messages to be       */
  unsigned char emrsv02;    /* Reserved                  @01C */
  void *__ptr32 ems99rbp;   /* Address of the failing SVC 99  */
  int emretcod;             /* The SVC 99 or the DAIR reg 15  */
  void *__ptr32 emcpplp;    /* Address of the CPPL This is    */
  void *__ptr32 embufp;     /* Address of message buffers if  */
  unsigned char emrsv03[4]; /* Reserved                  @01C */
  void *__ptr32 emwtpcdp;   /* When EmWtpCde is set, this is  */
};

/* Values for field "emfunct" */
#define emputlin 0x80 /* ON for message output via      */
#define emwtp 0x40    /* ON if the caller wants a Write */
#define emreturn 0x20 /* ON if the caller wants message */
#define emkeep 0x10   /* ON if caller wants to keep     */
#define emwtpcde 0x08 /* DESC & ROUTCDE codes are       */

/* Values for field "emwtpcdp" */
#define emlen1 0x1C

#endif

#ifndef __emdsect4__
#define __emdsect4__

struct emdsect4
{
  struct
  {
    unsigned char _emwtdesc[2];  /* WTO Descriptor codes      @01A */
    unsigned char _emwtrtcd[16]; /* WTO Routing codes         @01A */
  } emwtdert;
};

#define emwtdesc emwtdert._emwtdesc
#define emwtrtcd emwtdert._emwtrtcd

#endif

#ifndef __emdsect2__
#define __emdsect2__

struct emdsect2
{
  struct
  {
    unsigned char _embufl1[2]; /* Length of area used in EMBUF1 */
    unsigned char _embufo1[2]; /* Offset is zero on return      */
  } embufs;
  unsigned char embuft1[251]; /* Text of first level message  */
  unsigned char _filler1;
  struct
  {
    unsigned char _embufl2[2]; /* Length of area used in EMBUF2 */
  } embuf2;
  unsigned char embufo2[2];   /* Offset is zero on return     */
  unsigned char embuft2[251]; /* Text of second level message */
};

#define embufl1 embufs._embufl1
#define embufo1 embufs._embufo1
#define embufl2 embuf2._embufl2

/* Values for field "embuft2" */
#define emlen2 0x1FF /* Length of buffer parameters */

#endif

#ifndef __emdsect3__
#define __emdsect3__

struct emdsect3
{
  union
  {
    struct
    {
      unsigned char _emabuff[255]; /* @D2A */
      unsigned char _filler1;
    } emabuffs;
    struct
    {
      unsigned char _emabufln[2];   /* @D2A */
      unsigned char _emabufof[2];   /* @D2A */
      unsigned char _emabuftx[251]; /* @D2A */
      unsigned char _filler2;       /* @D2A */
    } _emdsect3_struct1;
  } _emdsect3_union1;
};

#define emabuff _emdsect3_union1.emabuffs._emabuff
#define emabufln _emdsect3_union1._emdsect3_struct1._emabufln
#define emabufof _emdsect3_union1._emdsect3_struct1._emabufof
#define emabuftx _emdsect3_union1._emdsect3_struct1._emabuftx

/* Values for field "_filler2" */
#define emlen3 0x100 /* Length of array element  @D2A   */
#define emsvc99 50   /* General caller with an SVC 99   */
#define emfree 51    /* Free command with an SVC 99     */
#define emdair 1     /* General caller with a DAIR      */
#define emdynalc 99  /* Call is Dynamic Allocation @P1A */

#endif

#pragma pack()
