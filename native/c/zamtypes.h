/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

#ifndef ZAMTYPES_H
#define ZAMTYPES_H

#include "dcbd.h"
#include "ihadcbe.h"
#include "jfcb.h"
#include "ihaexlst.h"
#include "zecb.h"

typedef struct ihadcb IHADCB;
typedef struct dcbe DCBE;

#define OPTION_BYTE 0X80

// NOTE(Kelosky): mapping for __asm(" OPEN,MODE=31,MF=L" : "DS"(plist));
typedef struct
{
  unsigned char option;
  unsigned char reserved[3];
  IHADCB *PTR32 dcb;
} OPEN_PL;

typedef OPEN_PL CLOSE_PL;

typedef struct
{
  unsigned char option;
  unsigned char reserved[3];
} RDJFCB_PL;

// the residual count is the halfword, 14 bytes from the start of the status area
typedef struct
{
  unsigned char filler[14];
  short int residualCount;

} STATUS_AREA;

// must be below 16MB (see Using Data Sets publication)
typedef struct
{
  ECB ecb;
  unsigned char typeField1;
  unsigned char typeField2;
  unsigned short length;
  IHADCB *PTR32 dcb;
  char *PTR32 area;
  STATUS_AREA *PTR32 statusArea;
} DECB;

typedef struct jfcb JFCB;

typedef DECB WRITE_PL;
typedef DECB READ_PL;

typedef struct exlst EXLIST;

#define MAX_HEADER_LEN 100
typedef struct
{
  unsigned char len;
  char title[MAX_HEADER_LEN];
} SNAP_HEADER;

typedef struct
{
  unsigned char id;
  unsigned char flags;
  unsigned char flag2;
  unsigned char reserved;
  unsigned char sdataFlagsOne;
  unsigned char sdataFlagsTwo;
  unsigned char pdataFlags;
  unsigned char reserved2;
  IHADCB *PTR32 dcb;
  void *PTR32 tcb;
  void *PTR32 list;
  SNAP_HEADER *PTR32 header;
} SNAP_PLIST;

typedef struct
{
  DCBE dcbe;
  int ctrlLen;
  int bufferLen;
  int bufferCtrl;
  unsigned int eod : 1;
  char *PTR32 buffer;
} FILE_CTRL;

#define MAX_USER_DATA_LEN 62
typedef struct
{
  char name[8]; // padded with blanks
  unsigned char ttr[3];
  unsigned char k; // concatention
  unsigned char z; // where found, 0=private, 1=link, 2=job, task, step, 3-16=job, task, step of parent
  unsigned char c; // name type bit0=0member, bit0=1alias, bit1-2=number of TTRN in user data (max 3), bit3-7 number of halfwords in the user data
  unsigned char user_data[MAX_USER_DATA_LEN];
} BLDL_LIST;

typedef struct
{
  unsigned char prefix[8]; // you must provide a prefix of 8 bytes immediately precedes the list of member names; listadd most point to FF field
  unsigned short int ff;   // number of entries in the list
  unsigned short int ll;   // length of each entry
  BLDL_LIST list;
} BLDL_PL;

typedef struct
{
  char name[8]; // padded with blanks
  unsigned char ttr[3];
  unsigned char c;
  unsigned char user_data[MAX_USER_DATA_LEN];
} STOW_LIST;

typedef struct
{
  unsigned char ttr[3];
  unsigned char z;
} NOTE_RESPONSE;

#define NUM_EXLIST_ENTRIES 2 // dcbabend and jfcb
typedef struct
{
  IHADCB dcb;
  DECB decb;
  JFCB jfcb;
  EXLIST exlst[NUM_EXLIST_ENTRIES];
  RDJFCB_PL rpl;
  OPEN_PL opl;
  STOW_LIST stow_list;
  int input : 1;
  int output : 1;
} IO_CTRL;

#endif