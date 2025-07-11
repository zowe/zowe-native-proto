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

register FILE_CTRL *fc ASMREG("r2");

static IO_CTRL *PTR32 newIoCtrl()
{
  IO_CTRL *ioc = storage_obtain24(sizeof(IO_CTRL));
  memset(ioc, 0x00, sizeof(IO_CTRL));
  return ioc;
}

static void setDcbInfo(IHADCB *PTR32 dcb, char *ddname, int lrecl, int blkSize, unsigned char recfm)
{
  char ddnam[9] = {0};
  sprintf(ddnam, "%-8.8s", ddname);
  memcpy(dcb->dcbddnam, ddnam, sizeof(dcb->dcbddnam));
  dcb->dcblrecl = lrecl;
  dcb->dcbblksi = blkSize;
  dcb->dcbrecfm = recfm;
}

static void setDcbDcbe(IHADCB *PTR32 dcb)
{
  // get space for DCBE + buffer
  short ctrlLen = sizeof(FILE_CTRL) + dcb->dcbblksi;
  FILE_CTRL *fc = storage_obtain31(ctrlLen);
  memset(fc, 0x00, ctrlLen);

  // init file control
  fc->ctrlLen = ctrlLen;
  fc->bufferLen = dcb->dcbblksi;

  // buffer is at the end of the structure
  fc->buffer = (unsigned char *)fc + offsetof(FILE_CTRL, buffer) + sizeof(fc->buffer);

  // init DCBE
  fc->dcbe.dcbelen = 56;
  memcpy(fc->dcbe.dcbeid, "DCBE", 4);

  // retain access to DCB / file control
  fc->dcbe.dcbeeoda = (void *)eodad;
  dcb->dcbdcbe = fc;
}

static IO_CTRL *PTR32 newWriteIoCtrl(char *ddname, int lrecl, int blkSize, unsigned char recfm)
{
  IO_CTRL *ioc = newIoCtrl();
  IHADCB *dcb = &ioc->dcb;
  memcpy(dcb, &openWriteModel, sizeof(IHADCB));
  setDcbInfo(dcb, ddname, lrecl, blkSize, recfm);
  return ioc;
}

static IO_CTRL *PTR32 newReadIoCtrl(char *ddname, int lrecl, int blkSize, unsigned char recfm)
{
  IO_CTRL *ioc = newIoCtrl();
  IHADCB *dcb = &ioc->dcb;
  memcpy(dcb, &openReadModel, sizeof(IHADCB));
  setDcbInfo(dcb, ddname, lrecl, blkSize, recfm);
  setDcbDcbe(dcb);
  return ioc;
}

IO_CTRL *open_output_assert(char *ddname, int lrecl, int blkSize, unsigned char recfm)
{
  IO_CTRL *ioc = newWriteIoCtrl(ddname, lrecl, blkSize, recfm);
  IHADCB *dcb = &ioc->dcb;
  int rc = 0;
  rc = open_output(dcb);
  if (0 != rc)
    s0c3_abend(OPEN_OUTPUT_ASSERT_RC);
  if (!(dcbofopn & dcb->dcboflgs))
    s0c3_abend(OPEN_OUTPUT_ASSERT_FAIL);

  return ioc;
}

IO_CTRL *open_input_assert(char *ddname, int lrecl, int blkSize, unsigned char recfm)
{
  IO_CTRL *ioc = newReadIoCtrl(ddname, lrecl, blkSize, recfm);
  IHADCB *dcb = &ioc->dcb;
  int rc = 0;
  rc = open_input(dcb);
  if (0 != rc)
    s0c3_abend(OPEN_INPUT_ASSERT_RC);
  if (!(dcbofopn & dcb->dcboflgs))
    s0c3_abend(OPEN_INPUT_ASSERT_FAIL);
  return ioc;
}

void close_assert(IO_CTRL *ioc)
{
  IHADCB *dcb = &ioc->dcb;
  int rc = close_dcb(dcb);
  if (0 != rc)
    s0c3_abend(CLOSE_ASSERT_RC);

  // free DCBE / file control if obtained
  if (dcb->dcbdcbe)
  {
    FILE_CTRL *fc = dcb->dcbdcbe;
    storage_release(fc->ctrlLen, fc);
  }

  storage_release(sizeof(IO_CTRL), ioc);
}

int open_output(IHADCB *dcb)
{
  int rc = 0;
  OPEN_PL opl = {0};
  opl.option = OPTION_BYTE;

  OPEN_OUTPUT(*dcb, opl, rc);
  return rc;
}

int open_input(IHADCB *dcb)
{
  int rc = 0;
  OPEN_PL opl = {0};
  opl.option = OPTION_BYTE;

  OPEN_INPUT(*dcb, opl, rc);
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
// it is called, NAB will be set.
void forceNab() ATTRIBUTE(noinline);
void forceNab()
{
  return;
}

int check(DECB *cpl)
{
  int rc = 0;
  forceNab();
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

int writeSync(IO_CTRL *ioc, char *buffer)
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

int readSync(IO_CTRL *ioc, char *buffer)
{
  int rc = 0;
  READ_PL *rpl = &ioc->decb;
  IHADCB *dcb = &ioc->dcb;

  if (dcb->dcbdcbe)
  {
    // file control begins at DCBE address
    // __asm(" svc 199 ");
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
