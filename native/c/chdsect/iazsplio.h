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

#ifndef __splio__
#define __splio__

struct splio
{
  unsigned char spiossid[4]; /* I.Eyecatcher                         */
  unsigned short spiolen;    /* I.Length of SPLIO parameter          */
  short int spliovrn;        /* I.Parm list version number           */
  short int spiovero;        /* O.Subsystem version number           */
  short int _filler1;        /* Reserved                             */
  void *__ptr32 spiostrp;    /* Storage management anchor            */
  int _filler2;              /* Reserved                             */
  int _filler3;              /* Reserved                             */
  unsigned char spiospad[8]; /* I.Spool address to be found          */
  unsigned char spioctyp[4]; /* I.Control block ID type              */
  unsigned char spiojnam[8]; /* I.Job name                           */
  unsigned char spiojid[8];  /* I.Job ID (J9999999, etc.)            */
  unsigned char spiojkey[4]; /* I.Job key                            */
  int spiodsky;              /* I.Dataset key                        */
  unsigned char spiossnm[8]; /* I.If instorage data buffers @Z04LHIO */
  short int spioasid;        /* I.If instorage data buffers @Z04LHIO */
  unsigned char spioopt;     /* I.Processing options        @Z09LAMS */
  unsigned char _filler4;    /* Reserved                    @Z09LAMS */
  int _filler5[5];           /* Reserved for future use and must     */
  void *__ptr32 spioouta;    /* O.Address of control block           */
  int spioolen;              /* O.Number of bytes in buffer          */
  int _filler6;              /* Reserved                             */
  int _filler7;              /* Reserved                             */
  unsigned char spioind1;    /* O.Indicator field                    */
  unsigned char _filler8[3]; /* Reserved                             */
};

/* Values for field "spliovrn" */
#define spliovr1 1   /* Service version number of            */
#define spliovr_n_ 1 /* Service version--the        @Z02P699 */

/* Values for field "spioopt" */
#define spioracf 0x80 /* Perform RACF checks even  @Z09LAMS   */

/* Values for field "spioind1" */
#define spionstg 0x80 /* The control block was retrieved      */

/* Values for field "_filler8" */
#define spiosze1 0x70 /* Parameter end-version 1     @Z02P699 */
#define spiosze 0x70  /* Size of SPLIO                        */
#define spiook 0      /* Success                              */
#define spiontvf 4    /* The VERIFY was not successful        */
#define spiocbio 8    /* Spool control block I/O error        */
#define spiocbtk 12   /* Spool control block invalid track    */
#define spiocbng 16   /* General control block problem        */
#define spiostrg 20   /* Error obtaining storage              */
#define spiosjer 24   /* Error obtaining below the line       */
#define spioilog 28   /* A logic error has occurred           */
#define spionspl 32   /* SPIOSTRP not initialized correctly   */
#define spionbuf 36   /* Could not locate instorage  @Z04LHIO */
#define spionsaf 40   /* RACF failure accessing data @Z09LAMS */

#endif

#pragma pack(reset)
