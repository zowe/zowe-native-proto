#pragma pack(1)

#ifndef __ssjp__
#define __ssjp__

struct ssjp {
  unsigned char  ssjpid[4];   /* Extension identifier                */
  unsigned short ssjplen;     /* Length of SSOB extension area       */
  short int      ssjpver;     /* I.Version number of SSOB            */
  unsigned char  ssjpfreq;    /* I.Function request byte             */
  unsigned char  ssjprsv1[3]; /* Reserved                            */
  int            ssjpretn;    /* O.Reason code for error return code */
  void * __ptr32 ssjpuser;    /* I.Pointer to user parameter area    */
  int            _filler1[2]; /* Reserved                            */
  unsigned char  _filler2[4];
  };

/* Values for field "ssjpver" */
#define ssjpver1 0x100 /* z/OS 1.11 version (initial)           */
#define ssjpverc 0x100 /* Current version number (z/OS 1.11)    */

/* Values for field "ssjpfreq" */
#define ssjpnjod 4     /* NJE node info obtain                  */
#define ssjpnjrs 8     /* NJE node storage return               */
#define ssjpspod 12    /* Spool info obtain                     */
#define ssjpsprs 16    /* Spool storage return                  */
#define ssjpitod 20    /* Initiator info obtain                 */
#define ssjpitrs 24    /* Initiator storage return              */
#define ssjpjxod 28    /* JESPLEX info obtain                   */
#define ssjpjxrs 32    /* JESPLEX storage return                */
#define ssjpjcod 36    /* Job class info obtain                 */
#define ssjpjcrs 40    /* Job class storage return              */
#define ssjpprod 44    /* PROCLIB concat obtain     @Z22LPRO    */
#define ssjpprrs 48    /* PROCLIB storage return    @Z22LPRO    */
#define ssjpckod 52    /* CKPT information obtain   @Z24LCKS    */
#define ssjpckrs 56    /* CKPT storage return       @Z24LCKS    */
#define ssjprgod 60    /* Resource group obtain     @Z31LJRL    */
#define ssjprgrs 64    /* Resource group release    @Z31LJRL    */

/* Values for field "ssjpretn" */
#define ssjpunsf 4     /* Function code passed in               */
#define ssjpntds 8     /* SSJPUSER pointer is zero              */
#define ssjpunsd 12    /* SSJPUSER CB version number            */
#define ssjpsmle 16    /* SSJPUSER CB length is too small       */
#define ssjpeyee 20    /* SSJPUSER CB eyecatcher is not correct */
#define ssjpinva 136   /* Invalid filter arguments.             */
#define ssjpglbl 140   /* Function not supported on             */
#define ssjpsmap 144   /* Error with storage addressed          */
#define ssjpgetm 128   /* $GETMAIN failed                       */
#define ssjpstgo 132   /* STORAGE OBTAIN failed                 */

#endif

#ifndef __ssob__
#define __ssob__

struct ssob {
  unsigned char  ssobid[4];   /* CONTROL BLOCK IDENTIFIER              */
  unsigned short ssoblen;     /* LENGTH OF SSOB HEADER                 */
  short int      ssobfunc;    /* FUNCTION ID                           */
  void * __ptr32 ssobssib;    /* ADDRESS OF SSIB OR ZERO               */
  int            ssobretn;    /* RETURN CODE FROM SUBSYSTEM            */
  int            ssobindv;    /* FUNCTION DEPENDENT AREA POINTER       */
  void * __ptr32 ssobreta;    /* USED BY SSI TO SAVE RETURN ADDRESS    */
  unsigned char  ssobflg1;    /* Flag Byte                        @01A */
  unsigned char  ssobrsv1[3]; /* RESERVED                         @01C */
  };

/* Values for field "ssobretn" */
#define ssrtok   0    /* SUCCESSFUL COMPLETION - REQUEST WENT  */
#define ssrtnsup 4    /* SUBSYSTEM DOES NOT SUPPORT THIS       */
#define ssrtntup 8    /* SUBSYSTEM EXISTS, BUT IS NOT UP       */
#define ssrtnoss 12   /* SUBSYSTEM DOES NOT EXIST              */
#define ssrtdist 16   /* FUNCTION NOT COMPLETED-DISASTROUS     */
#define ssrtlerr 20   /* LOGICAL ERROR (BAD SSOB FORMAT,       */
#define ssrtnssi 24   /* SSI not initialized              @L1A */

/* Values for field "ssobflg1" */
#define ssobrtry 0x80 /* Retry Requested                  @01A */

/* Values for field "ssobrsv1" */
#define ssobhsiz 0x1C /* SSOB HEADER LENGTH                    */

#endif

#pragma pack()
