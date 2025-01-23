/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0 & Apache-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

#pragma pack(packed)

#ifndef __iazssji__
#define __iazssji__

struct iazssji
{
  unsigned char ssjiid[4];   /* EXTENSION IDENTIFIER                  */
  unsigned short ssjilen;    /* LENGTH OF SSOB EXTENSION AREA         */
  short int ssjisvrn;        /* SERVICE VERSION NUMBER                */
  unsigned char ssjifreq;    /* FUNCTION REQUEST BYTE                 */
  unsigned char ssjirsv1[3]; /* RESERVED                              */
  int ssjiretn;              /* REASON CODE FOR ERROR RETURN CODE     */
  void *__ptr32 ssjiuser;    /* Pointer to user parm area    @Z23LRES */
};

/* Values for field "ssjisvrn" */
#define ssjisvr_n_ 1 /* SERVICE VERSION NUMBER OF THIS LEVEL    */

/* Values for field "ssjifreq" */
#define ssjifobt 4  /* FUNCTION REQUEST_OBTAIN                 */
#define ssjifrel 8  /* FUNCTION REQUEST_RELEASE                */
#define ssjifjco 12 /* Function Jobclass_Data Obtain  @R04LWLM */
#define ssjifjcr 16 /* Function Jobclass_Data Return  @R04LWLM */
#define ssjisiom 20 /* Function SPOOL I/O: obtain     @Z02LSIO */
#define ssjisirs 24 /* Function SPOOL I/O: return     @Z02LSIO */
#define ssjicvdv 28 /* Function Convert Device ID     @Z02LSIO */
#define ssjimnod 32 /* Function Monitor info obtain   @Z07LMSS */
#define ssjimnrs 36 /* Function Monitor info return   @Z07LMSS */
#define ssjickpd 40 /* Internal function to get CKPT  @Z23LICK */
#define ssjilmod 44 /* Function Resource Limits       @Z24LRES */
#define ssjilmrs 48 /* Function Resource Limits       @Z24LRES */

/* Values for field "ssjiretn" */
#define ssjioldd 20 /* The data may be obsolete     @Z07LMSS   */
#define ssji64da 40 /* Some 64 bit pointers not       @Z31BF   */
#define ssjiunsf 4  /* Function code passed in      @Z07LMSS   */
#define ssjintds 24 /* SSJIUSER does not point to   @Z07LMSS   */
#define ssjiunsd 28 /* SSJIUSER CB version number   @Z07LMSS   */
#define ssjismds 32 /* SSJIUSER CB length is too    @Z07LMSS   */
#define ssji2obt 8  /* SUCCESSIVE OBTAINS WITHOUT              */
#define ssjidisa 12 /* SUBTASK DISABLED, TRY AGAIN SHORTLY     */
#define ssjivina 16 /* VERSIONING INACTIVE, ACTIVATE IT        */
#define ssjiinvr 36 /* INVALID INPUT DATA TO RELEASE,          */

/* Values for field "ssjiuser" */
#define ssjisize 0x14 /* SSOB EXTENSION LENGTH                   */
#define ssjilen8 0x30 /* TOTAL SSOB LENGTH W/JI EXTENSION        */

#endif

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
