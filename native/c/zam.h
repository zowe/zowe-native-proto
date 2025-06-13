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

#ifndef ZAM_H
#define ZAM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zmetal.h"
#include "dcbd.h"
#include "ihaecb.h"
#include "zecb.h"
#include "zstorage.h"
#include "ihadcbe.h"

// IO_CTRL *sysprintIoc = openOutputAssert("SYSPRINT", 132, 132, dcbrecf + dcbrecbr);
// IO_CTRL *snapIoc = openOutputAssert("SNAP", 125, 1632, dcbrecv + dcbrecbr + dcbrecca);
// IO_CTRL *inIoc = openInputAssert("IN", 80, 80, dcbrecf);

// SNAP_HEADER header = {23, "Important Control Block"};

// snap(&snapIoc->dcb, &header, &someCtrl, sizeof(someCtrl));

// while (0 == readSync(inIoc, inbuff))
// {
//     memset(writeBuf, ' ', 132);
//     memcpy(writeBuf, inbuff, 80);
//     writeSync(sysprintIoc, writeBuf);
// }

#if defined(__IBM_METAL__)
#define DCB_WRITE_MODEL(dcbwm)                                \
  __asm(                                                      \
      "*                                                  \n" \
      " DCB DDNAME=*-*,"                                      \
      "DSORG=PS,"                                             \
      "MACRF=W                                            \n" \
      "*                                                    " \
      : "DS"(dcbwm));
#else
#define DCB_WRITE_MODEL(dcbwm)
#endif

DCB_WRITE_MODEL(openWriteModel);

#if defined(__IBM_METAL__)
#define DCB_READ_MODEL(dcbrm)                                 \
  __asm(                                                      \
      "*                                                  \n" \
      " DCB DDNAME=*-*,"                                      \
      "DSORG=PS,"                                             \
      "DCBE=*-*,"                                             \
      "MACRF=R                                            \n" \
      "*                                                    " \
      : "DS"(dcbrm));
#else
#define DCB_READ_MODEL(dcbrm)
#endif

DCB_READ_MODEL(openReadModel);

#if defined(__IBM_METAL__)
#define OPEN_OUTPUT(dcb, plist, rc)                           \
  __asm(                                                      \
      "*                                                  \n" \
      " OPEN (%0,(OUTPUT)),"                                  \
      "MODE=31,"                                              \
      "MF=(E,%2)                                          \n" \
      "*                                                  \n" \
      " ST    15,%1     Save RC                           \n" \
      "*                                                    " \
      : "+m"(dcb),                                            \
        "=m"(rc)                                              \
      : "m"(plist)                                            \
      : "r0", "r1", "r14", "r15");
#else
#define OPEN_OUTPUT(dcb, plist, rc)
#endif

// TODO(Kelosky): "TYPE=J,"
#if defined(__IBM_METAL__)
#define OPEN_INPUT(dcb, plist, rc)                            \
  __asm(                                                      \
      "*                                                  \n" \
      " OPEN (%0,(INPUT)),"                                   \
      "MODE=31,"                                              \
      "MF=(E,%2)                                          \n" \
      "*                                                  \n" \
      " ST    15,%1     Save RC                           \n" \
      "*                                                    " \
      : "+m"(dcb),                                            \
        "=m"(rc)                                              \
      : "m"(plist)                                            \
      : "r0", "r1", "r14", "r15");
#else
#define OPEN_INPUT(dcb, plist, rc)
#endif

#if defined(__IBM_METAL__)
#define RDJFCB_INPUT(dcb, plist, rc)                          \
  __asm(                                                      \
      "*                                                  \n" \
      " RDJFCB (%0,(INPUT)),"                                 \
      "MF=(E,%2)                                          \n" \
      "*                                                  \n" \
      " ST    15,%1     Save RC                           \n" \
      "*                                                    " \
      : "+m"(dcb),                                            \
        "=m"(rc)                                              \
      : "m"(plist)                                            \
      : "r0", "r1", "r14", "r15");
#else
#define RDJFCB_INPUT(dcb, plist, rc)
#endif

#if defined(__IBM_METAL__)
#define FIND(dcb, ddname, rc)                                 \
  __asm(                                                      \
      "*                                                  \n" \
      " FIND %0,"                                             \
      "%2,"                                                   \
      "D                                                  \n" \
      "*                                                  \n" \
      " ST    15,%1     Save RC                           \n" \
      "*                                                    " \
      : "+m"(dcb),                                            \
        "=m"(rc)                                              \
      : "m"(ddname)                                           \
      : "r0", "r1", "r14", "r15");
#else
#define FIND(dcb, plist, rc)
#endif

#if defined(__IBM_METAL__)
#define CLOSE(dcb, plist, rc)                                 \
  __asm(                                                      \
      "*                                                  \n" \
      " CLOSE (%0),"                                          \
      "MODE=31,"                                              \
      "MF=(E,%2)                                          \n" \
      "*                                                  \n" \
      " ST    15,%1     Save RC                           \n" \
      "*                                                    " \
      : "+m"(dcb),                                            \
        "=m"(rc)                                              \
      : "m"(plist)                                            \
      : "r0", "r1", "r14", "r15");
#else
#define CLOSE(dcb, plist, rc)
#endif

#if defined(__IBM_METAL__)
#define WRITE(dcb, ecb, buf, rc)                              \
  __asm(                                                      \
      "*                                                  \n" \
      " WRITE %0,"                                            \
      "SF,"                                                   \
      "%2,"                                                   \
      "%3,"                                                   \
      "MF=E                                               \n" \
      "*                                                  \n" \
      " ST    15,%1     Save RC                           \n" \
      "*                                                    " \
      : "+m"(ecb),                                            \
        "=m"(*rc)                                             \
      : "m"(dcb),                                             \
        "m"(buf)                                              \
      : "r0", "r1", "r14", "r15");
#else
#define WRITE(dcb, ecb, buf, rc)
#endif

#if defined(__IBM_METAL__)
#define READ(dcb, ecb, buf, rc)                               \
  __asm(                                                      \
      "*                                                  \n" \
      " READ %0,"                                             \
      "SF,"                                                   \
      "%2,"                                                   \
      "%3,"                                                   \
      "'S',"                                                  \
      "MF=E                                               \n" \
      "*                                                  \n" \
      " ST    15,%1     Save RC                           \n" \
      "*                                                    " \
      : "+m"(ecb),                                            \
        "=m"(rc)                                              \
      : "m"(dcb),                                             \
        "m"(buf)                                              \
      : "r0", "r1", "r14", "r15");
#else
#define READ(dcb, ecb, buf, rc)
#endif

#if defined(__IBM_METAL__)
#define SNAP(dcb, header, start, end, plist, rc)              \
  __asm(                                                      \
      "*                                                  \n" \
      " SNAP DCB=%1,"                                         \
      "ID=1,"                                                 \
      "STORAGE=(%2,%3),"                                      \
      "STRHDR=%4,"                                            \
      "MF=(E,%5)                                          \n" \
      "*                                                  \n" \
      " ST    15,%0     Save RC                           \n" \
      "*                                                    " \
      : "=m"(rc)                                              \
      : "m"(dcb),                                             \
        "m"(start),                                           \
        "m"(end),                                             \
        "m"(header),                                          \
        "m"(plist)                                            \
      : "r0", "r1", "r14", "r15");
#else
#define SNAP(dcb, header, start, end, plist, rc)
#endif

#if defined(__IBM_METAL__)
#define CHECK(ecb, rc)                                        \
  __asm(                                                      \
      "*                                                  \n" \
      " CHECK %1                                          \n" \
      "*                                                  \n" \
      " ST    15,%0     Save RC                           \n" \
      "*                                                    " \
      : "=m"(rc)                                              \
      : "m"(ecb)                                              \
      : "r0", "r1", "r14", "r15");
#else
#define CHECK(ecb, rc)
#endif

#define OPTION_BYTE 0X80

typedef struct ihadcb IHADCB;
typedef struct dcbe DCBE;

//
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

typedef DECB WRITE_PL;
typedef DECB READ_PL;

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

typedef struct
{
  IHADCB dcb;
  DECB decb;
} IO_CTRL;

// 8-char entry points for z
#if defined(__IBM_METAL__)
#pragma map(open_output_assert, "opnoasrt")
#pragma map(open_input_assert, "opniasrt")
#pragma map(close_assert, "closasrt")
#endif

// API methods
IO_CTRL *PTR32 open_output_assert(char *, int, int, unsigned char) ATTRIBUTE(amode31);
IO_CTRL *PTR32 open_input_assert(char *, int, int, unsigned char) ATTRIBUTE(amode31);
void close_assert(IO_CTRL *) ATTRIBUTE(amode31);

int writeSync(IO_CTRL *, char *) ATTRIBUTE(amode31);
int readSync(IO_CTRL *, char *) ATTRIBUTE(amode31);

#if defined(__IBM_METAL__)
#pragma map(open_output, "openout")
#pragma map(open_input, "openin")
#endif

// individual api methods
int open_output(IHADCB *) ATTRIBUTE(amode31);
int open_input(IHADCB *) ATTRIBUTE(amode31);

#if defined(__IBM_METAL__)
#pragma map(close_dcb, "CLOSEDCB")
#pragma map(write_dcb, "WRITEDCB")
#pragma map(read_dcb, "READDCB")
#endif

int write_dcb(IHADCB *, WRITE_PL *, char *) ATTRIBUTE(amode31);
void read_dcb(IHADCB *, READ_PL *, char *) ATTRIBUTE(amode31);

int close_dcb(IHADCB *) ATTRIBUTE(amode31);

int check(DECB *ecb) ATTRIBUTE(amode31);

int snap(IHADCB *, SNAP_HEADER *, void *, int) ATTRIBUTE(amode31);

void eodad();

enum AMS_ERR
{
  UNKOWN_MODE,
  OPEN_OUTPUT_ASSERT_RC,
  OPEN_OUTPUT_ASSERT_FAIL,
  OPEN_INPUT_ASSERT_RC,
  OPEN_INPUT_ASSERT_FAIL,
  CLOSE_ASSERT_RC,
  DCBE_REQUIRED,
  UNSUPPORTED_RECFM
};

// TODO(Kelosky): dbcabend
// TODO(Kelosky): synad
// TODO(Kelosky): rdjfcb

#endif