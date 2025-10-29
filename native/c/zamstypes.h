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

#ifndef ZAMS_TYPES_H
#define ZAMS_TYPES_H

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

#define NUM_EXLIST_ENTRIES 2
typedef struct
{
  IHADCB dcb;
  DECB decb;
  JFCB jfcb;
  EXLIST exlst[NUM_EXLIST_ENTRIES];
  RDJFCB_PL rpl;
  OPEN_PL opl;
  int input : 1;
  int output : 1;
} IO_CTRL;

#endif