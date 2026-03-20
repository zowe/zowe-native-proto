#ifdef __open_xl__
#pragma pack(1)
#else
#pragma pack(packed)
#endif

#ifndef __ifgrpl__
#define __ifgrpl__

struct ifgrpl
{
  unsigned char rplid;   /* RPL IDENTIFIER         */
  unsigned char rplstyp; /* RPL SUBTYPE - SET TO   */
  unsigned char rplreq;  /* RPL REQUEST TYPE       */
  struct
  {
    unsigned char _rpllen2; /* ALTERNATE NAME FOR */
  } rpllen;
  void *__ptr32 rplplhpt; /* POINTER TO PLACEHOLDER */
  void *__ptr32 rplecb;   /* INTERNAL ECB OR        */
  struct
  {
    unsigned char _rplstat; /* CURRENT RPL STATUS */
    struct
    {
      struct
      {
        unsigned char _rplerreg; /* ALTERNATE NAME FOR */
      } rplrtncd;
      struct
      {
        struct
        {
          unsigned char _rplfdb2; /* REASON CODE(VTAM) X03004 */
        } rplcmpon;
        struct
        {
          unsigned char _rplfdb3; /* DATA FLAGS(VTAM)  X03004 */
        } rplerrcd;
      } rplcndcd;
    } rplfdbk;
  } rplfdbwd;
  struct
  {
    short int _rplkeyl; /* ALTERNATE NAME FOR */
  } rplkeyle;
  short int rplstrid;     /* CCW STRING IDENTIFIER  */
  void *__ptr32 rplcchar; /* POINTER TO CONTROL     */
  void *__ptr32 rpldacb;  /* POINTER TO DATA ACB    */
  void *__ptr32 rpltcbpt; /* POINTER TO TCB         */
  void *__ptr32 rplarea;  /* POINTER TO AREA        */
  struct
  {
    unsigned char _rplsaf[2]; /* SOURCE ADDRESS      */
    unsigned char _rpldaf[2]; /* DESTINATION ADDRESS */
  } rplarg;
  struct
  {
    unsigned char _rplopt1; /* OPTION BYTE 1 */
    unsigned char _rplopt2; /* OPTION BYTE 2 */
    unsigned char _rplopt3; /* OPTION BYTE 3 */
    unsigned char _rplopt4; /* OPTCD BYTE 4  */
  } rploptcd;
  struct
  {
    void *__ptr32 _rplchain; /* ALTERNATE NAME FOR */
  } rplnxtrp;
  void *__ptr32 rplrlen; /* LENGTH OF RECORD       */
  void *__ptr32 rplbufl; /* USER BUFFER LENGTH     */
  union
  {
    struct
    {
      unsigned char _rplopt5; /* OPTION BYTE 5     X03004 */
      unsigned char _rplopt6; /* OPTION BYTE 6     X03004 */
      unsigned char _rplopt7; /* OPTION BYTE 7     X03004 */
      unsigned char _rplopt8; /* OPTION BYTE 8     X03004 */
    } rploptc2;
    struct
    {
      unsigned char _filler1;  /* VSAM RPL - RESERVED @ZZA */
      unsigned char _rplaixid; /* AIX POINTER TYPE    @ZZM */
      unsigned char _rplappky; /* VSAM RPL - RESERVED @T1C */
      unsigned char _filler2;  /* USER KEY FOR RPLAREA@T1A */
    } _ifgrpl_struct1;
  } _ifgrpl_union1;
  struct
  {
    unsigned char _rplaixpc[2]; /* AIX POINTER COUNT   @ZZC */
    struct
    {
      unsigned char _rplrbap1[2]; /* PART 1 OF RBA       @03C */
      struct
      {
        struct
        {
          unsigned char _rplargln[4]; /* RPL Argument length for */
        } rplrbap2;
      } rpldddd;
    } rplrbarx;
  } rplrbar;
  struct
  {
    unsigned char _rplextd1; /* ALTERNATE NAME FOR */
  } rplextds;
  unsigned char rplactiv; /* ACTIVE INDICATOR -     */
  short int rplemlen;     /* LENGTH OF THE ERROR    */
  void *__ptr32 rplermsa; /* POINTER TO THE ERROR   */
};

#define rpllen2 rpllen._rpllen2
#define rplstat rplfdbwd._rplstat
#define rplerreg rplfdbwd.rplfdbk.rplrtncd._rplerreg
#define rplfdb2 rplfdbwd.rplfdbk.rplcndcd.rplcmpon._rplfdb2
#define rplfdb3 rplfdbwd.rplfdbk.rplcndcd.rplerrcd._rplfdb3
#define rplkeyl rplkeyle._rplkeyl
#define rplsaf rplarg._rplsaf
#define rpldaf rplarg._rpldaf
#define rplopt1 rploptcd._rplopt1
#define rplopt2 rploptcd._rplopt2
#define rplopt3 rploptcd._rplopt3
#define rplopt4 rploptcd._rplopt4
#define rplchain rplnxtrp._rplchain
#define rplopt5 _ifgrpl_union1.rploptc2._rplopt5
#define rplopt6 _ifgrpl_union1.rploptc2._rplopt6
#define rplopt7 _ifgrpl_union1.rploptc2._rplopt7
#define rplopt8 _ifgrpl_union1.rploptc2._rplopt8
#define rplaixid _ifgrpl_union1._ifgrpl_struct1._rplaixid
#define rplappky _ifgrpl_union1._ifgrpl_struct1._rplappky
#define rplaixpc rplrbar._rplaixpc
#define rplrbap1 rplrbar.rplrbarx._rplrbap1
#define rplargln rplrbar.rplrbarx.rpldddd.rplrbap2._rplargln
#define rplextd1 rplextds._rplextd1

/* Values for field "rplid" */
#define rplidd 0x00 /* IDENTIFIER VALUE - X'00' */

/* Values for field "rplstyp" */
#define rplsvsam 0x10 /* VSAM SUBTYPE     X04SVHS */
#define rplsvtam 0x20 /* VTAM SUBTYPE     X04SVHS */
#define rpls3540 0x40 /* 3540 SUBTYPE     X04SVHS */
#define rplcrid 0xFF  /* CRPL ID (VTAM)    X03004 */

/* Values for field "rplreq" */
#define rplget 0x00   /* GET                      */
#define rplput 0x01   /* PUT                      */
#define rplpoint 0x03 /* POINT                    */
#define rplerase 0x05 /* ERASE                    */
#define rpljsfmt 0x07 /* JES FORMAT REQUEST       */
#define rplcheck 0x02 /* CHECK                    */
#define rplendre 0x04 /* ENDREQ                   */
#define rplverif 0x06 /* VERIFY                   */
#define rplimprt 0x07 /* IMPORT                   */
#define rplpfmtd 0x08 /* DATA PREFORMAT           */
#define rplpfmti 0x09 /* INDEX PREFORMAT          */
#define rplfrcio 0x0A /* FORCE I/O                */
#define rplcnvta 0x10 /* CNVTAD          @ZA37315 */
#define rplmntac 0x11 /* MNTACQ(VSAM)    @ZA37315 */
#define rplwrite 0x11 /* WRITE(VTAM)       X03004 */
#define rplacqra 0x12 /* ACQRANGE(VSAM)  @ZA37315 */
#define rplreset 0x12 /* RESET(VTAM)       X03004 */
#define rplterm 0x13  /* TERMRPL         @ZA32757 */
#define rpldo 0x13    /* DO(VTAM)          X03004 */
#define rplverrf 0x14 /* VERIFY REFRESH      @L2A */
#define rpllkadd 0x15 /* LKADD REQUEST(VSAM) @L3A */
#define rpllkrel 0x16 /* LKREL REQUEST(VSAM) @L3A */
#define rpllkcd 0x17  /* LKCD  REQUEST(VSAM) @L3A */
#define rplinqrc 0x18 /* INQRC REQUEST(VSAM) @L3A */
#define rplretlk 0x19 /* RETLK REQUEST(VSAM) @L3A */
#define rplrecov 0x1A /* RECOV REQUEST(VSAM) @L3A */
#define rplqui 0x1B   /* QUIESCE REQ  (VSAM) @L3A */
#define rpleadd 0x1C  /* EADD  REQUEST(VSAM) @L3A */
#define rplquise 0x15 /* SETLOGON(VTAM)    X03004 */
#define rplsmlgo 0x16 /* SIMLOGON(VTAM)    X03004 */
#define rplopnds 0x17 /* OPNDST(VTAM)      X03004 */
#define rplchng 0x19  /* CHANGE(VTAM)      X03004 */
#define rplinqir 0x1A /* INQUIRE(VTAM)     X03004 */
#define rplintpt 0x1B /* INTRPRET(VTAM)    X03004 */
#define rplread 0x1D  /* READ(VTAM)        X03004 */
#define rplslict 0x1E /* SOLICIT(VTAM)     X03004 */
#define rplclose 0x1F /* CLSDST(VTAM)      X03004 */
#define rplclacb 0x21 /* CLOSEACB(VTAM)    X03004 */
#define rplsndcd 0x22 /* SEND(VTAM)       X3004BS */
#define rplrcvcd 0x23 /* RECEIVE(VTAM)    X3004BS */
#define rplrsrcd 0x24 /* RESETSR(VTAM)    X3004BS */
#define rplssccd 0x25 /* SESSIONC(VTAM)   X3004BS */
#define rplsdcmd 0x27 /* SENDCMD(VTAM)   @Z40BHUC */
#define rplrvcmd 0x28 /* RCVCMD(VTAM)    @Z40BHUC */
#define rpltreqs 0x29 /* REQSESS(VTAM)   @G40AKCM */
#define rpltopns 0x2A /* OPNSEC(VTAM)    @G40AKCM */
#define rpltclss 0x2B /* CLSSEC(VTAM)    @G40AKCM */
#define rpltrms 0x2C  /* TRMSESS(VTAM)   @G40AKCM */

/* Values for field "rplecb" */
#define rplwait 0x80 /* A REQUEST HAS BEEN       */
#define rplpost 0x40 /* THE REQUEST HAS          */

/* Values for field "rplrtncd" */
#define rplnoerr 0x00 /* NORMAL RETURN            */
#define rplblker 0x04 /* INVALID CONTROL BLOCK    */
#define rplcblke 0x04 /* ALTERNATE NAME FOR       */
#define rplloger 0x08 /* ILLOGICAL REQUEST        */
#define rpllogic 0x08 /* ALTERNATE NAME FOR       */
#define rplphyer 0x0C /* PHYSICAL I/O ERROR       */
#define rplphysc 0x0C /* ALTERNATE NAME FOR       */
#define rplngrcc 0x10 /* A CONDITIONAL COMMAND    */
#define rplnovas 0x10 /* SMSVSAM SERVER ADDRESS   */
#define rplspecc 0x14 /* A TEMPORARY OUT-OF-CORE  */
#define rplcmdrt 0x18 /* THE REQUEST WAS          */
#define rplpurge 0x1C /* THE COMMAND WAS          */
#define rplvtmna 0x20 /* VTAM IS NOT ACTIVE(VTAM) */
#define rplsyerr 0x24 /* SYSTEM ERROR(VTAM)X03004 */
#define rpldevdc 0x28 /* DIAL LINE IS             */
#define rpllimex 0x2C /* RESPONSE LIMIT           */
#define rplexrq 0x30  /* EXCEPTION REQUEST        */
#define rplexrs 0x34  /* EXCEPTION RESPONSE       */
#define rplnoin 0x38  /* NO INPUT                 */
#define rplvabnd 0x3C /* VTAM ENCOUNTERED ABEND   */

/* Values for field "rplfdb2" */
#define rplerlk 0x80  /* ERROR LOCK SET    X03004 */
#define rplrvid 0x40  /* RVI RECEIVED      X03004 */
#define rplatnd 0x20  /* ATTN RECEIVED     X03004 */
#define rpldvuns 0x10 /* DEVICE UNUSABLE   X03004 */
#define rplioerr 0x08 /* I/O ERROR TYPE- 0=INPUT/ */
#define rpldlgfl 0x04 /* DIALOG INIT FAILED       */
#define rplcuerr 0x02 /* CONTROL UNIT FAILURE     */
#define rplstsav 0x01 /* SENSE BYTES PRESENT      */

/* Values for field "rplfdb3" */
#define rpluinpt 0x80 /* UNSOLICITED INPUT X03004 */
#define rplsv32 0x40  /* RESERVED          X03004 */
#define rplreob 0x20  /* END OF BLOCK      X03004 */
#define rplreom 0x10  /* END OF MESSAGE    X03004 */
#define rplreot 0x08  /* END OF TRANSMISSION      */
#define rpllgfrc 0x04 /* LOGOFF DETECTED   X03004 */
#define rplrlg 0x02   /* LEADING GRAPHICS         */
#define rplrdsoh 0x01 /* START OF HEADER (SOH)    */

/* Values for field "rplopt1" */
#define rplloc 0x80   /* LOCATE MODE; MOVE MODE   */
#define rpldir 0x40   /* DIRECT ACCESS            */
#define rplseq 0x20   /* SEQUENTIAL ACCESS        */
#define rplskp 0x10   /* SKIP SEQUENTIAL ACCESS   */
#define rplasy 0x08   /* ASYNCHRONOUS PROCESSING  */
#define rplkge 0x04   /* SEARCH KEY GT/EQ         */
#define rplgen 0x02   /* GENERIC KEY REQUEST      */
#define rplecbsw 0x01 /* EXTERNAL ECB             */
#define rplecbin 0x01 /* ALTERNATE NAME FOR       */

/* Values for field "rplopt2" */
#define rplkey 0x80   /* KEYED ACCESS             */
#define rpladr 0x40   /* ADDRESSED ACCESS         */
#define rpladd 0x40   /* ALTERNATE NAME FOR       */
#define rplcnv 0x20   /* CONTROL INTERVAL ACCESS  */
#define rplara64 0x20 /* 1=RPLAREA INDIRECTLY PTS */
#define rplbwd 0x10   /* FWD=0/BWD=1      X04SVHS */
#define rpllrd 0x08   /* ARD=0/LRD=1      X04SVHS */
#define rplwaitx 0x04 /* AYNCH PROC WAIT @ZA07549 */
#define rplupd 0x02   /* UPDATE                   */
#define rplnsp 0x01   /* NOTE STRING POSITION     */

/* Values for field "rplopt3" */
#define rpleods 0x80  /* END OF USER SYSOUT       */
#define rplsform 0x40 /* SPECIAL FORM ON REMOTE   */
#define rplblk 0x20   /* BLOCKED UCS DATA CHECKS  */
#define rplvfy 0x10   /* VERIFY UCS/FCB           */
#define rplfld 0x08   /* LOAD UCS BUFFER IN       */
#define rplfmt 0x02   /* FCB LOAD                 */
#define rplfrmt 0x06  /* UCS LOAD IF 00           */
#define rplalign 0x01 /* ALIGN FCB BUFFER LOADING */

/* Values for field "rplopt4" */
#define rplendtr 0x80 /* 3800 END OF TRANSMISSION */
#define rplxrba 0x80  /* EXTENDED ADDRESSABILTY   */
#define rplmkfrm 0x40 /* 3800 MARK FORM  (VS1)    */
#define rplgtfa 0x40  /* GTF IS ACTIVE FOR VSAM   */
#define rplapky 0x40  /* USE USER APPLICATION     */
#define rplnocir 0x20 /* NO CI RECLAIM       @L1A */
#define rplcta 0x10   /* RPLCCHAR POINTS TO AN    */
#define rplpson 0x10  /* POSITION ONLY, DO NOT    */
#define rplctm 0x08   /* RPLCCHAR POINTS TO A     */
#define rplcto 0x04   /* OTHER FORMAT. RPLCCHAR   */

/* Values for field "rplopt5" */
#define rpldlgin 0x80 /* CONTINUE READING IN      */
#define rplssnin 0x40 /* CONTINUE DIALOG WITH THE */
#define rplpsopt 0x20 /* PASS TERMINAL TO         */
#define rplneras 0x10 /* WRITE TO 3270 BUT DO NOT */
#define rpleau 0x08   /* WRITE TO 3270 AND ERASE  */
#define rplerace 0x04 /* WRITE TO 3270 AND ERASE  */
#define rplnode 0x02  /* READ FROM ANY TERMINAL;  */
#define rplwropt 0x01 /* CONVERSATIONAL MODE;     */

/* Values for field "rplopt6" */
#define rpleob 0x80   /* WRITE A BLOCK OF DATA    */
#define rpleom 0x40   /* WRITE THE LAST BLOCK     */
#define rpleot 0x20   /* WRITE THE LAST BLOCK     */
#define rplcond 0x10  /* DO NOT STOP OPERATION    */
#define rplncond 0x08 /* STOP OPERATION           */
#define rpllock 0x04  /* RESET ERROR LOCK TO      */
#define rplrsv67 0x02 /* RESERVED          X03004 */
#define rplrsv68 0x01 /* RESERVED          X03004 */

/* Values for field "rplopt7" */
#define rplcnall 0x80 /* ALL TERMINALS IN OPNDST  */
#define rplcnany 0x40 /* CONNECT ANY ONE TERMINAL */
#define rplcnimm 0x20 /* RESERVED          X03004 */
#define rplqopt 0x10  /* QUEUE THE OPNDST REQUEST */
#define rpltpost 0x08 /* RPL ALREADY UNDER PSS    */
#define rplrlsop 0x04 /* SCHEDULE THE RELREQ EXIT */
#define rpltcrno 0x02 /* CLOSE IN PROCESS FOR PO  */
#define rplrsv78 0x01 /* RESERVED          X03004 */

/* Values for field "rplopt8" */
#define rplodacq 0x80 /* THE APPLICATION REQUIRES */
#define rplodacp 0x40 /* THE APPLICATION WILL     */
#define rplodprm 0x20 /* A SPECIFIC TERMINAL IS   */
#define rplpend 0x10  /* PREEMPT THE TERMINAL     */
#define rplsess 0x08  /* PREEMPT THE TERMINAL     */
#define rplactv 0x04  /* PREEMPT THE TERMINAL IF  */
#define rpluncon 0x02 /* PREEMPT THE TERMINAL     */
#define rplrsv88 0x01 /* RESERVED          X03004 */

/* Values for field "rplaixid" */
#define rplaxpkp 0x80 /* RBA=1/PRIME=0       @ZZM */
#define rplkrnrq 0x04 /* KEEP RECORD LOCK    @01A */
#define rplkenrq 0x02 /* KEEP ESDS LOCK      @01A */
#define rplbldix 0x01 /* BLDINDEX CALLING    @T0A */

/* Values for field "rplextd1" */
#define rplexsch 0x80 /* AN EXIT HAS BEEN         */
#define rplnexit 0x40 /* NO EXIT WAS SPECIFIED    */
#define rplexit 0x20  /* ASYNCH EXIT     @XM01127 */
#define rplkeyon 0x10 /* IF ON, RPLKEY WAS ON     */
#define rpltcryp 0x08 /* IF ON, ENCRYPTION        */
#define rplzhw 0x08   /* zHyperWrite         @04A */
#define rplnib 0x04   /* THE RPLARG FIELD         */
#define rplbranc 0x02 /* USE A BRANCH ENTRY       */

#endif

#ifdef __open_xl__
#pragma pack()
#else
#pragma pack(reset)
#endif
