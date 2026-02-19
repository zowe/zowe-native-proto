#pragma pack(1)

#ifndef __iazbtokp__
#define __iazbtokp__

struct iazbtokp
{
  unsigned char btokpl1[2]; /* ID LENGTH                            */
  unsigned char btokid[4];  /* ID FIELD                             */
  unsigned char btokpl2[2]; /* VERSION LENGTH                       */
  struct
  {
    unsigned char _btoktype; /* Control block type          @R03LPSO */
    unsigned char _btokvers; /* Version                     @R03LPSO */
  } btokver;
  unsigned char btokpl3[2];  /* Spool token length          @R03LPSO */
  unsigned char btokiotp[4]; /* IOT MTTR (or zero)          @R10LSDB */
  unsigned char btokpl4[2];  /* JOB KEY LENGTH                       */
  unsigned char btokjkey[4]; /* JOB KEY IN HEX                       */
  unsigned char btokpl5[2];  /* ASID LENGTH                          */
  unsigned char btokasid[2]; /* ASID IN HEX                          */
  unsigned char btokpl6[2];  /* NETWORK RECEIVER USERID LENGTH       */
  unsigned char btokrcid[8]; /* NETWORK RECEIVER USERID              */
  unsigned char btokpl7[2];  /* LOG STRING PARAMETER LENGTH          */
  struct
  {
    unsigned char _btoklsdl;      /* LOG STRING DATA LENGTH (0-254 BYTES) */
    unsigned char _btoklsda[254]; /* LOG STRING DATA                      */
  } btoklogs;
};

#define btoktype btokver._btoktype
#define btokvers btokver._btokvers
#define btoklsdl btoklogs._btoklsdl
#define btoklsda btoklogs._btoklsda

/* Values for field "btoktype" */
#define btokbrws 0 /* Block created for browse    @R03LPSO */
#define btoksapi 2 /* Block created by Sysout API @R03LPSO */
#define btokstkn 3 /* SPOOL data set or client    @R10LSDB */

/* Values for field "btokvers" */
#define btokvrnm 3 /* Version OS/390 Release 10   @R10LSDB */

#endif

#pragma pack()
