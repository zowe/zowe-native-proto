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
#include "jfcb.h"
#include "ihaexlst.h"
#include "zamtypes.h"

// https://www.ibm.com/docs/en/SSLTBW_3.1.0/pdf/idad500_v3r1.pdf
// see Non-VSAM macro instructions

// IO_CTRL *sysprintIoc = openOutputAssert("SYSPRINT", 132, 132, dcbrecf + dcbrecbr);
// IO_CTRL *snapIoc = openOutputAssert("SNAP", 125, 1632, dcbrecv + dcbrecbr + dcbrecca);
// IO_CTRL *inIoc = openInputAssert("IN", 80, 80, dcbrecf);

// SNAP_HEADER header = {23, "Important Control Block"};

// snap(&snapIoc->dcb, &header, &someCtrl, sizeof(someCtrl));

// while (0 == read_sync(inIoc, inbuff))
// {
//     memset(writeBuf, ' ', 132);
//     memcpy(writeBuf, inbuff, 80);
//     write_sync(sysprintIoc, writeBuf);
// }

// "DCBE=*-*,"                                             \
// TODO(KELOSKY): DCBE?
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

DCB_WRITE_MODEL(open_write_model);

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

DCB_READ_MODEL(open_read_model);

#if defined(__IBM_METAL__)
#define OPEN(dcb, plist, rc, mode)                            \
  __asm(                                                      \
      "*                                                  \n" \
      " OPEN (%0,(" #mode ")),"                               \
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
#define OPEN(dcb, plist, rc, mode)
#endif

#if defined(__IBM_METAL__)
#define SYNADRLS()                                            \
  __asm(                                                      \
      "*                                                  \n" \
      " SYNADRLS                                          \n" \
      "*                                                  \n" \
      "*                                                    " \
      :                                                       \
      :                                                       \
      : "r0", "r1", "r14", "r15");
#else
#define SYNADRLS()
#endif

#if defined(__IBM_METAL__)
#define RDJFCB(dcb, plist, rc, mode)                          \
  __asm(                                                      \
      "*                                                  \n" \
      " RDJFCB (%0,(" #mode ")),"                             \
      "MF=(E,%2)                                          \n" \
      "*                                                  \n" \
      " ST    15,%1     Save RC                           \n" \
      "*                                                    " \
      : "+m"(dcb),                                            \
        "=m"(rc)                                              \
      : "m"(plist)                                            \
      : "r0", "r1", "r14", "r15");
#else
#define RDJFCB(dcb, plist, rc, mode)
#endif

#if defined(__IBM_METAL__)
#define FIND(dcb, member, rc, rsn)                            \
  __asm(                                                      \
      "*                                                  \n" \
      " FIND %0,"                                             \
      "%3,"                                                   \
      "D                                                  \n" \
      "*                                                  \n" \
      " ST    15,%1     Save RC                           \n" \
      " ST    0,%2     Save RSN                           \n" \
      "*                                                    " \
      : "+m"(dcb),                                            \
        "=m"(rc),                                             \
        "=m"(rsn)                                             \
      : "m"(member)                                           \
      : "r0", "r1", "r14", "r15");
#else
#define FIND(dcb, plist, rc, rsn)
#endif

// PDSE member is not connected
#if defined(__IBM_METAL__)
#define BLDL(dcb, list, rc, rsn)                              \
  __asm(                                                      \
      "*                                                  \n" \
      " BLDL %3,"                                             \
      "%0,"                                                   \
      "BYPASSLLA,"                                            \
      "NOCONNECT                                          \n" \
      "*                                                  \n" \
      " ST    15,%1     Save RC                           \n" \
      " ST    0,%2     Save RSN                           \n" \
      "*                                                    " \
      : "+m"(list),                                           \
        "=m"(rc),                                             \
        "=m"(rsn)                                             \
      : "m"(dcb)                                              \
      : "r0", "r1", "r14", "r15");
#else
#define BLDL(dcb, list, rc, rsn)
#endif

#if defined(__IBM_METAL__)
#define STOW(dcb, list, rc, rsn)                              \
  __asm(                                                      \
      "*                                                  \n" \
      " STOW %2,"                                             \
      "%3,"                                                   \
      "R                                                  \n" \
      "*                                                  \n" \
      " ST    15,%1     Save RC                           \n" \
      " ST    0,%2     Save RSN                           \n" \
      "*                                                    " \
      : "=m"(rc),                                             \
        "=m"(rsn)                                             \
      : "m"(dcb),                                             \
        "m"(list)                                             \
      : "r0", "r1", "r14", "r15");
#else
#define STOW(dcb, list, rc, rsn)
#endif

#if defined(__IBM_METAL__)
#define NOTE(dcb, listaddr, rc, rsn)                          \
  __asm(                                                      \
      "*                                                  \n" \
      " NOTE %3,"                                             \
      "REL                                                \n" \
      "*                                                  \n" \
      " ST    1,%0     Save result                        \n" \
      " ST    15,%1    Save RC                            \n" \
      " ST    0,%2     Save RSN                           \n" \
      "*                                                    " \
      : "=m"(listaddr),                                       \
        "=m"(rc),                                             \
        "=m"(rsn)                                             \
      : "m"(dcb)                                              \
      : "r0", "r1", "r14", "r15");
#else
#define NOTE(dcb, listaddr, rc, rsn)
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

int write_sync(IO_CTRL *, char *) ATTRIBUTE(amode31);
int read_sync(IO_CTRL *, char *) ATTRIBUTE(amode31);

#if defined(__IBM_METAL__)
#pragma map(open_output, "openout")
#pragma map(open_input, "openin")
#endif

// individual api methods
int open_output(IHADCB *) ATTRIBUTE(amode31);
int open_update(IHADCB *) ATTRIBUTE(amode31);
int open_input(IHADCB *) ATTRIBUTE(amode31);

#if defined(__IBM_METAL__)
#pragma map(close_dcb, "CLOSEDCB")
#pragma map(write_dcb, "WRITEDCB")
#pragma map(read_dcb, "READDCB")
#endif

int write_dcb(IHADCB *, WRITE_PL *, char *) ATTRIBUTE(amode31);
void read_dcb(IHADCB *, READ_PL *, char *) ATTRIBUTE(amode31);

#if defined(__IBM_METAL__)
#pragma map(open_output_bpam, "OPNOBPAM")
#pragma map(close_output_bpam, "CLOSBPAM")
#endif

int open_output_bpam(ZDIAG *PTR32, IO_CTRL *PTR32 *PTR32, const char *PTR32) ATTRIBUTE(amode31);
int close_output_bpam(ZDIAG *PTR32, IO_CTRL *PTR32) ATTRIBUTE(amode31);

#if defined(__IBM_METAL__)
#pragma map(read_input_jfcb, "RIJFCB")
#pragma map(read_output_jfcb, "ROJFCB")
#endif

int read_input_jfcb(IO_CTRL *ioc) ATTRIBUTE(amode31);
int read_output_jfcb(IO_CTRL *ioc) ATTRIBUTE(amode31);

int bldl(IO_CTRL *, BLDL_PL *, int *rsn) ATTRIBUTE(amode31);
int stow(IO_CTRL *, int *rsn) ATTRIBUTE(amode31);
int note(IO_CTRL *, NOTE_RESPONSE *PTR32 note_response, int *rsn) ATTRIBUTE(amode31);
int find_member(IO_CTRL *ioc, int *rsn) ATTRIBUTE(amode31);

int close_dcb(IHADCB *) ATTRIBUTE(amode31);

int check(DECB *ecb) ATTRIBUTE(amode31);

int snap(IHADCB *, SNAP_HEADER *, void *, int) ATTRIBUTE(amode31);

void eodad() ATTRIBUTE(amode31);

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

static IO_CTRL *PTR32 new_io_ctrl()
{
  IO_CTRL *ioc = storage_obtain24(sizeof(IO_CTRL));
  memset(ioc, 0x00, sizeof(IO_CTRL));
  return ioc;
}

// TODO(Kelosky): remove this function??
static void set_dcb_info(IHADCB *PTR32 dcb, char *PTR32 ddname, int lrecl, int blkSize, unsigned char recfm)
{
  char ddnam[9] = {0};
  sprintf(ddnam, "%-8.8s", ddname);
  memcpy(dcb->dcbddnam, ddnam, sizeof(dcb->dcbddnam));
  dcb->dcblrecl = lrecl;
  dcb->dcbblksi = blkSize;
  dcb->dcbrecfm = recfm;
}

typedef void (*PTR32 EODAD)() ATTRIBUTE(amode31);

static void set_dcb_dcbe(IHADCB *PTR32 dcb)
{
  // get space for DCBE + buffer
  short ctrl_len = sizeof(FILE_CTRL) + dcb->dcbblksi;
  FILE_CTRL *fc = storage_obtain31(ctrl_len);
  memset(fc, 0x00, ctrl_len);

  // init file control
  fc->ctrl_len = ctrl_len;
  fc->buffer_len = dcb->dcbblksi;

  // buffer is at the end of the structure
  fc->buffer = (unsigned char *PTR32)fc + offsetof(FILE_CTRL, buffer) + sizeof(fc->buffer);

  // init DCBE
  fc->dcbe.dcbelen = sizeof(DCBE);
  char *dcbeid = "DCBE";
  memcpy(fc->dcbe.dcbeid, dcbeid, strlen(dcbeid));

  // retain access to DCB / file control
  dcb->dcbdcbe = fc;
}

static void set_eod(IHADCB *PTR32 dcb, EODAD eodad)
{
  FILE_CTRL *fc = dcb->dcbdcbe;
  // retain access to DCB / file control
  fc->dcbe.dcbeeoda = (void *PTR32)eodad;
}

static IO_CTRL *PTR32 new_write_io_ctrl(char *PTR32 ddname, int lrecl, int blkSize, unsigned char recfm)
{
  IO_CTRL *PTR32 ioc = new_io_ctrl();
  IHADCB *dcb = &ioc->dcb;
  memcpy(dcb, &open_write_model, sizeof(IHADCB));
  set_dcb_info(dcb, ddname, lrecl, blkSize, recfm);
  return ioc;
}

static IO_CTRL *PTR32 new_read_io_ctrl(char *PTR32 ddname, int lrecl, int blkSize, unsigned char recfm)
{
  IO_CTRL *ioc = new_io_ctrl();
  IHADCB *dcb = &ioc->dcb;
  memcpy(dcb, &open_read_model, sizeof(IHADCB));
  set_dcb_info(dcb, ddname, lrecl, blkSize, recfm);
  return ioc;
}

#endif
