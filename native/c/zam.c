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

#include "zam.h"
#include "dcbd.h"
#include "zwto.h"

// NOTE(Kelosky): must be assembled in AMODE31 code
#if defined(__IBM_METAL__)
#define RDJFCB_MODEL(rdjfcbm)                                 \
  __asm(                                                      \
      "*                                                  \n" \
      " RDJFCB (,),"                                          \
      "MF=L                                               \n" \
      "*                                                    " \
      : "DS"(rdjfcbm));
#else
#define RDJFCB_MODEL(rdjfcbm)
#endif

RDJFCB_MODEL(rdfjfcb_model);

register FILE_CTRL *fc ASMREG("r8");

IO_CTRL *open_output_assert(char *ddname, int lrecl, int blkSize, unsigned char recfm)
{
  IO_CTRL *ioc = new_write_io_ctrl(ddname, lrecl, blkSize, recfm);
  IHADCB *dcb = &ioc->dcb;
  int rc = 0;
  rc = open_output(dcb);
  dcb->dcbdsrg1 = dcbdsgps; // DSORG=PS
  ioc->output = 1;
  if (0 != rc)
    s0c3_abend(OPEN_OUTPUT_ASSERT_RC);
  if (!(dcbofopn & dcb->dcboflgs))
    s0c3_abend(OPEN_OUTPUT_ASSERT_FAIL);

  return ioc;
}

IO_CTRL *open_input_assert(char *ddname, int lrecl, int blkSize, unsigned char recfm)
{
  IO_CTRL *ioc = new_read_io_ctrl(ddname, lrecl, blkSize, recfm);
  set_dcb_dcbe(&ioc->dcb);
  set_eod(&ioc->dcb, eodad);
  IHADCB *dcb = &ioc->dcb;
  int rc = 0;
  dcb->dcbdsrg1 = dcbdsgps; // DSORG=PS
  rc = open_input(dcb);
  ioc->input = 1;

  // TODO(Kelosky): TM    DCBOFLGS,DCBOFOPN
  // TODO(Kelosky): TM    dcbabend if occurs
  // TODO(Kelosky): duplicate in open_output_assert
  // TODO(Kelosky): handle DUMMY / NULLFILE

  if (0 != rc)
    s0c3_abend(OPEN_INPUT_ASSERT_RC);
  if (!(dcbofopn & dcb->dcboflgs))
    s0c3_abend(OPEN_INPUT_ASSERT_FAIL);
  return ioc;
}

void close_assert(IO_CTRL *ioc)
{
  IHADCB *dcb = &ioc->dcb;
  FILE_CTRL *fc = dcb->dcbdcbe;
  zwto_debug("@TEST close_assert: %p", fc);

  if (dcb->dcboflgs & dcbofopn)
  {
    zwto_debug("@TEST close_assert: closing dcb: %p", dcb);
    int rc = close_dcb(dcb);
    if (0 != rc)
    {
      s0c3_abend(CLOSE_ASSERT_RC);
    }
    // free DCBE / file control if obtained
    if (fc && ioc->input)
    {
      zwto_debug("@TEST close_assert: releasing fc: %p", fc);
      // FILE_CTRL *fc = dcb->dcbdcbe;
      storage_release(fc->ctrl_len, fc);
    }
  }

  zwto_debug("@TEST close_assert: releasing ioc: %p", ioc);
  storage_release(sizeof(IO_CTRL), ioc);
}

int find_member(IO_CTRL *ioc, int *rsn)
{
  int rc = 0;
  int local_rsn = 0;
  FIND(ioc->dcb, ioc->jfcb.jfcbelnm, rc, local_rsn);
  *rsn = local_rsn;
  return rc;
}

int bldl(IO_CTRL *ioc, BLDL_PL *bldl_pl, int *rsn)
{
  int rc = 0;
  int local_rsn = 0;
  BLDL(ioc->dcb, bldl_pl->ff, rc, local_rsn);
  *rsn = local_rsn;
  return rc;
}

int stow(IO_CTRL *ioc, int *rsn)
{
  int rc = 0;
  int local_rsn = 0;
  STOW(ioc->dcb, ioc->stow_list, rc, local_rsn);
  *rsn = local_rsn;
  return rc;
}

int note(IO_CTRL *ioc, NOTE_RESPONSE *PTR32 note_response, int *rsn)
{
  int rc = 0;
  int local_rsn = 0;
  NOTE(ioc->dcb, *note_response, rc, local_rsn);
  *rsn = local_rsn;
  return rc;
}

int read_input_jfcb(IO_CTRL *ioc)
{
  int rc = 0;

  ioc->exlst[0].exlentrb = (unsigned int)0; // NOTE(Kelosky): DCBABEND needs to be copied to 24 bit storage or have some wrapper
  ioc->exlst[0].exlcodes = exldcbab;
  ioc->exlst[1].exlentrb = (unsigned int)&ioc->jfcb;
  ioc->exlst[1].exlcodes = exllaste + exlrjfcb;
  memcpy(&ioc->rpl, &rdfjfcb_model, sizeof(RDJFCB_PL));

  unsigned char recfm = ioc->dcb.dcbrecfm; // save the recfm
  void *PTR32 exlst = &ioc->exlst;
  memcpy(&ioc->dcb.dcbexlst, &exlst, sizeof(ioc->dcb.dcbexlst));
  ioc->dcb.dcbrecfm = recfm; // restore the recfm

  RDJFCB(ioc->dcb, ioc->rpl, rc, INPUT);
  return rc;
}

int read_output_jfcb(IO_CTRL *ioc)
{
  int rc = 0;

  ioc->exlst[0].exlentrb = (unsigned int)0; // NOTE(Kelosky): DCBABEND needs to be copied to 24 bit storage or have some wrapper
  ioc->exlst[0].exlcodes = exldcbab;
  ioc->exlst[1].exlentrb = (unsigned int)&ioc->jfcb;
  ioc->exlst[1].exlcodes = exllaste + exlrjfcb;
  memcpy(&ioc->rpl, &rdfjfcb_model, sizeof(RDJFCB_PL));

  unsigned char recfm = ioc->dcb.dcbrecfm; // save the recfm
  void *PTR32 exlst = &ioc->exlst;
  memcpy(&ioc->dcb.dcbexlst, &exlst, sizeof(ioc->dcb.dcbexlst));
  ioc->dcb.dcbrecfm = recfm; // restore the recfm

  RDJFCB(ioc->dcb, ioc->rpl, rc, OUTPUT);
  return rc;
}

int open_output(IHADCB *dcb)
{
  int rc = 0;
  OPEN_PL opl = {0};
  opl.option = OPTION_BYTE;

  OPEN(*dcb, opl, rc, OUTPUT);
  return rc;
}

int open_update(IHADCB *dcb)
{
  int rc = 0;
  OPEN_PL opl = {0};
  opl.option = OPTION_BYTE;

  OPEN(*dcb, opl, rc, OUTPUT);
  return rc;
}

int open_input(IHADCB *dcb)
{
  int rc = 0;
  OPEN_PL opl = {0};
  opl.option = OPTION_BYTE;

  OPEN(*dcb, opl, rc, INPUT);
  return rc;
}

int snap(IHADCB *dcb, SNAP_HEADER *header, void *start, int len)
{
  int rc = 0;
  SNAP_PLIST plist = {0};
  unsigned char *end = (unsigned char *)start + len;
  SNAP(*dcb, *header, *(unsigned char *)start, *end, plist, rc);
  return rc;
}

int write_dcb(IHADCB *dcb, WRITE_PL *wpl, char *buffer)
{
  int rc = 0;
  memset(wpl, 0x00, sizeof(WRITE_PL));
  WRITE(*dcb, *wpl, *buffer, &rc);
  // TODO(Kelosky): rc doesn't matter for fixed length records
  rc = 0;
  return rc;
}

// NOTE(Kelosky): simple function that is non inline so that when
// it is called, NAB will be set. This is needed for the CHECK macro
// which could trigger a call to EODAD (which requires NAB to be set).
void force_nab() ATTRIBUTE(noinline);
void force_nab()
{
  return;
}

int check(DECB *cpl)
{
  int rc = 0;
  force_nab();
  CHECK(*cpl, rc)
  rc = 0;
  return rc;
}

void read_dcb(IHADCB *dcb, READ_PL *rpl, char *buffer)
{
  // NOTE(Kelosky): READ does not appear to give an RC
  int rc = 0;
  memset(rpl, 0x00, sizeof(READ_PL));
  READ(*dcb, *rpl, *buffer, rc);
}

int close_dcb(IHADCB *dcb)
{
  int rc = 0;
  CLOSE_PL cpl = {0};
  cpl.option = OPTION_BYTE;
  CLOSE(*dcb, cpl, rc);
  return rc;
}

int write_sync(IO_CTRL *ioc, char *buffer)
{
  int rc = 0;
  WRITE_PL *wpl = &ioc->decb;
  rc = write_dcb(&ioc->dcb, wpl, buffer);
  if (0 != rc)
  {
    return rc;
  }

  return check(&ioc->decb);
}

int read_sync(IO_CTRL *ioc, char *buffer)
{
  int rc = 0;
  READ_PL *rpl = &ioc->decb;
  IHADCB *dcb = &ioc->dcb;

  if (dcb->dcbdcbe)
  {
    // file control begins at DCBE address
    fc = dcb->dcbdcbe;

    // fixed only records until rdjfcb
    if (dcbrecf == dcb->dcbrecfm)
    {
      // TODO(Kelosky): skip read and use buffer for blocked records
      // TODO(Kelosky): check for rc
      // right now, for non-blocked, there is no buffer
      read_dcb(dcb, rpl, fc->buffer);
      // read(dcb, rpl, buffer);

      rc = check(rpl);
      if (fc->eod)
      {
        return -1;
      }
      if (0 != rc)
      {
        return rc;
      }

      // TODO(Kelosky): offset into buffer
      memcpy(buffer, fc->buffer, dcb->dcblrecl);
    }
    else
    {
      s0c3_abend(UNSUPPORTED_RECFM);
    }
  }
  else
  {
    s0c3_abend(DCBE_REQUIRED);
  }

  return 0;
}

// NOTE(Kelosky): registers 2-13 should be the same as the time
// the read/check was called for non-VSAM end of data exit.
void eodad()
{
  fc->eod = 1;
}
