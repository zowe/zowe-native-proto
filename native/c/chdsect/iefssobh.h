#pragma pack(packed)

#ifndef __ssob__
#define __ssob__

struct ssob
{
  unsigned char ssobid[4];   /* CONTROL BLOCK IDENTIFIER              */
  unsigned short ssoblen;    /* LENGTH OF SSOB HEADER                 */
  short int ssobfunc;        /* FUNCTION ID                           */
  void *__ptr32 ssobssib;    /* ADDRESS OF SSIB OR ZERO               */
  int ssobretn;              /* RETURN CODE FROM SUBSYSTEM            */
  int ssobindv;              /* FUNCTION DEPENDENT AREA POINTER       */
  void *__ptr32 ssobreta;    /* USED BY SSI TO SAVE RETURN ADDRESS    */
  unsigned char ssobflg1;    /* Flag Byte                        @01A */
  unsigned char ssobrsv1[3]; /* RESERVED                         @01C */
};

/* Values for field "ssobretn" */
#define ssrtok 0    /* SUCCESSFUL COMPLETION - REQUEST WENT  */
#define ssrtnsup 4  /* SUBSYSTEM DOES NOT SUPPORT THIS       */
#define ssrtntup 8  /* SUBSYSTEM EXISTS, BUT IS NOT UP       */
#define ssrtnoss 12 /* SUBSYSTEM DOES NOT EXIST              */
#define ssrtdist 16 /* FUNCTION NOT COMPLETED-DISASTROUS     */
#define ssrtlerr 20 /* LOGICAL ERROR (BAD SSOB FORMAT,       */
#define ssrtnssi 24 /* SSI not initialized              @L1A */

/* Values for field "ssobflg1" */
#define ssobrtry 0x80 /* Retry Requested                  @01A */

/* Values for field "ssobrsv1" */
#define ssobhsiz 0x1C /* SSOB HEADER LENGTH                    */

#endif

#pragma pack(reset)
