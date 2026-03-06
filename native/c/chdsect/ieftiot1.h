#ifdef __open_xl__
#pragma pack(1)
#else
#pragma pack(packed)
#endif

#ifndef __tiot__
#define __tiot__

struct tiot
{
  unsigned char tiocnjob[8]; /* -         JOB NAME                                  */
  struct
  {
    struct
    {
      unsigned char _tiocpstn[8]; /* -         8-BYTE PROC STEP NAME FOR PROCS     @P1A */
    } tiocstpn;
    unsigned char _tiocjstn[8]; /* -         8-BYTE JOBSTEP NAME FOR PROCS       @P1A */
  } tiocstep;
  char tioelngh;          /* -          LENGTH, IN BYTES, OF THIS ENTRY          */
  unsigned char tioestta; /* -            STATUS BYTE A                          */
  struct
  {
    unsigned char _tioewtct; /* -          DURING ALLOCATION, NUMBER OF DEVICES */
    unsigned char _tioelink; /* -          DURING ALLOCATION, LINK TO THE       */
  } tioerloc;
  unsigned char tioeddnm[8];  /* -          DD NAME                                  */
  unsigned char tioejfcb[3];  /* -          SWA virtual address token, mapped        */
  unsigned char tioesttc;     /* -            STATUS BYTE C.  USED DURING ALLOCATION */
  unsigned char tioesttb;     /* -            STATUS BYTE B - DURING ALLOCATION AND  */
  unsigned int tioefsrt : 24; /* -          DURING PROBLEM PROGRAM, ADDRESS OF UCB.  */
  unsigned char _filler1;     /* -          RESERVED                                 */
  char tiopnslt;              /* -          NUMBER OF SLOTS FOR POOL                 */
  unsigned char _filler2;     /* -          RESERVED                                 */
  char tiopnsrt;              /* -          NUMBER OF DEVICES (FILLED SLOTS)         */
  unsigned char tioppool[8];  /* -          POOL NAME                                */
  unsigned char tiopsttb;     /* -          STATUS OF SLOT                           */
  unsigned int tiopslot : 24; /* -          UCB ADDRESS OR EMPTY SLOT                */
  unsigned char tiotfend[4];  /* -          FINAL END OF THE TIOT - BINARY ZEROS     */
};

#define tiocpstn tiocstep.tiocstpn._tiocpstn
#define tiocjstn tiocstep._tiocjstn
#define tioewtct tioerloc._tioewtct
#define tioelink tioerloc._tioelink

/* Values for field "tioestta" */
#define tiosltyp 0x80 /* -        NONSTANDARD LABEL (TAPE) (OS/VS1)         */
#define tiospltp 0x40 /* -        DURING ALLOCATION, SPLIT CYLINDER         */
#define tiosplts 0x20 /* -        DURING ALLOCATION, SPLIT CYLINDER         */
#define tiosjblb 0x10 /* -        JOBLIB INDICATOR                          */
#define tiosdads 0x08 /* -        DADSM ALLOCATION NECESSRY                 */
#define tioslabl 0x04 /* -        LABELED TAPE.  IF BIT 0 IS OFF, SL OR     */
#define tiosdsp1 0x02 /* -        REWIND/UNLOAD THE TAPE VOLUME (TAPE)      */
#define tiosdsp2 0x01 /* -        REWIND THE TAPE VOLUME (TAPE)             */

/* Values for field "tioelink" */
#define tiosyout 0x80 /* -        THIS IS A SYSOUT DATA SET THAT CONTAINS   */
#define tiotrv01 0x40 /* -        RESERVED                           MDC006 */
#define tiotterm 0x20 /* -        DEVICE IS A TERMINAL                      */
#define tioedynm 0x10 /* -        DYNAM CODED ON DD STATEMENT               */
#define tioeqnam 0x08 /* -        QNAME CODED ON DD STATEMENT               */
#define tioesyin 0x04 /* -        ENTRY FOR SPOOLED SYSIN DATA SET          */
#define tioesyot 0x02 /* -        ENTRY FOR SPOOLED SYSOUT DATA SET         */
#define tioessds 0x02 /* -        ENTRY FOR A SUBSYSTEM DATA SET            */
#define tiotrem 0x01  /* -        ENTRY FOR A REMOTE DEVICE          ICB340 */

/* Values for field "tioesttc" */
#define tiosdkcr 0x80 /* -        MAIN STORAGE OR DASD ADDRESS              */
#define tiosdefr 0x40 /* -        DEFERRED MOUNT                            */
#define tiosaffp 0x20 /* -        PRIMARY UNIT AFFINITY                     */
#define tiosaffs 0x10 /* -        SECONDARY UNIT AFFINITY                   */
#define tiosvolp 0x08 /* -        PRIMARY VOLUME AFFINITY                   */
#define tiosvols 0x04 /* -        SECONDARY VOLUME AFFINITY                 */
#define tiosbalp 0x02 /* -        PRIMARY SUBALLOCATE                       */
#define tiosbals 0x01 /* -        SECONDARY SUBALLOCATE                     */

/* Values for field "tioesttb" */
#define tiosused 0x80 /* -        DATA SET IS ON DEVICE                     */
#define tiosreqd 0x40 /* -        DATA SET WILL USE DEVICE                  */
#define tiospvio 0x20 /* -        DEVICE VIOLATES SEPARATION                */
#define tiosvlsr 0x10 /* -        VOLUME SERIAL PRESENT                     */
#define tiossetu 0x08 /* -        SETUP MESSAGE REQUIRED                    */
#define tiosmntd 0x04 /* -        IF 0, DELETE UNLOADED VOLUME IF UNLOAD    */
#define tiosunld 0x02 /* -        UNLOAD REQUIRED                           */
#define tiosverf 0x01 /* -        VERIFICATION REQUIRED                     */

#endif

#ifdef __open_xl__
#pragma pack()
#else
#pragma pack(reset)
#endif
