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
#include <stdio.h>
#include "zmetal.h"
#include "zwto.h"
#include "dcbd.h"
#include "zam.h"
#include "zmetal.h"
#include "zdbg.h"

// NOTE(Kelosky): We only use this path for write operations and to preserve and update ISPF statistics.  Read operations or DSORG=PS will use `fopen`.
// In this path we must perform dynamic allocation on the data set.  We must perform RDJFCB to validate the data set and get the attributes prior to performing the OPEN.
// To use a STOW macro, you must specify DSORG=PO|POU.

// TODO(Kelosky): determine if we need to use BPAM up front & that we are writing / updating stats
// if BPAM, do dynalloc, RDJFCB to valid attributes or perhaps use something like fldata
// open without TYPE=J
// TODO(Kelosky): ensure resources are released in abends or thread issues
// TODO(Kelosky): TEST dcbabend
// TODO(Kelosky): TEST synad
// TODO(Kelosky): DCBE for write?
// TODO(Kelosky): handling wriing via DD name
// TODO(Kelosky): handle supported formats

#pragma prolog(AMSMAIN, " ZWEPROLG NEWDSA=(YES,256) ")
#pragma epilog(AMSMAIN, " ZWEEPILG ")
int AMSMAIN()
{
  zwto_debug("AMSMAIN started");

  // IO_CTRL *PTR32 sysin = new_read_io_ctrl("SYSIN", 80, 80, dcbrecf);
  // set_dcb_dcbe(&sysin->dcb, eodad);

  // int rc = read_input_jfcb(sysin);
  // if (0 != rc)
  // {
  //   zwto_debug("@TEST read_input_jfcb failed: %d", rc);
  //   s0c3_abend(1);
  // }

  // zwto_debug("@TEST sysin->jfcb.jfcbelnm: %.8s", sysin->jfcb.jfcbelnm); // if first by has`x'BF'` then member name exists and not a PS file, clear DSORG if needed
  // zwto_debug("@TEST sysin->jfcb.jfcnlrec: %x", sysin->jfcb.jfcnlrec);
  // zwto_debug("@TEST sysin->jfcb.jfcbaxbf: %x", sysin->jfcb.jfcbaxbf);
  // zwto_debug("@TEST sysin->jfcb.jfcdsrg1: %x", sysin->jfcb.jfcdsrg1);
  // zwto_debug("@TEST sysin->jfcb.jfcdsrg2: %x", sysin->jfcb.jfcdsrg2);
  // zwto_debug("@TEST sysin->jfcb.jfcbind1: %x", sysin->jfcb.jfcbind1);
  // zwto_debug("@TEST sysin->jfcb.jfcbind2: %x", sysin->jfcb.jfcbind2);

  // if (sysin->jfcb.jfcbind1 != jfcpds)
  // {
  //   zwto_debug("@TEST sysin->jfcb.jfcbind1 is not PS (0x%x)", sysin->jfcb.jfcbind1);
  //   return -1;
  // }

  // rc = open_input(&sysin->dcb);
  // // rc = funcs.open_input_j(ioc);
  // // if (0 != rc)
  // {
  //   zwto_debug("@TEST open_input_j failed: %d", rc);
  //   s0c3_abend(1);
  // }

  // return 0;

  IO_CTRL *PTR32 sysin = open_input_assert("SYSIN", 80, 80, dcbrecf);
  // IO_CTRL *PTR32 sysprint = open_output_assert("SYSPRINT", 80, 80, dcbrecf);
  IO_CTRL *PTR32 sysprint = new_write_io_ctrl("SYSPRINT", 80, 80, dcbrecf);
  set_dcb_dcbe(&sysprint->dcb, eodad);
  int rc = read_output_jfcb(sysprint);
  if (0 != rc)
  {
    zwto_debug("@TEST read_output_jfcb failed: %d", rc);
    return -1;
  }
  zwto_debug("@TEST sysprint->jfcb.jfcbelnm: %.8s", sysprint->jfcb.jfcbelnm);
  zwto_debug("@TEST sysprint->jfcb.jfcbelnm hex: %x", sysprint->jfcb.jfcbelnm[0]);
  zwto_debug("@TEST sysprint->jfcb.jfcnlrec: %x", sysprint->jfcb.jfcnlrec);
  zwto_debug("@TEST sysprint->jfcb.jfcbaxbf: %x", sysprint->jfcb.jfcbaxbf);
  zwto_debug("@TEST sysprint->jfcb.jfcdsrg1: %x", sysprint->jfcb.jfcdsrg1);
  zwto_debug("@TEST sysprint->jfcb.jfcdsrg2: %x", sysprint->jfcb.jfcdsrg2);
  zwto_debug("@TEST sysprint->jfcb.jfcbind1: %x", sysprint->jfcb.jfcbind1);
  zwto_debug("@TEST sysprint->jfcb.jfcbind2: %x", sysprint->jfcb.jfcbind2);

  // ensure PDS
  if (sysprint->jfcb.jfcbind1 != jfcpds)
  {
    zwto_debug("@TEST sysprint->jfcb.jfcbind1 is not PS (0x%x)", sysprint->jfcb.jfcbind1);
    return -1;
  }

  // ensure member name (e.g. is a partitioned data set)
  if (sysprint->jfcb.jfcbelnm[0] == ' ')
  {
    zwto_debug("@TEST sysprint->jfcb.jfcbelnm is empty");
    return -1;
  }

  zwto_debug("@TEST sysprint->dcb.dcbdsrg1: %x", sysprint->dcb.dcbdsrg1);

  sysprint->dcb.dcbdsrg1 = dcbdsgpo; // DSORG=PO
  rc = open_output(&sysprint->dcb);
  if (0 != rc)
  {
    zwto_debug("@TEST open_output failed: %d", rc);
    return -1;
  }

  zwto_debug("@TEST sysprint->dcb.dcboflgs: %x", sysprint->dcb.dcboflgs);

  if (!(sysprint->dcb.dcboflgs & dcbofopn))
  {
    zwto_debug("@TEST sysprint->dcb.dcboflgs is not open (0x%x)", sysprint->dcb.dcboflgs);
    return -1;
  }

  // TODO(Kelosky): check for DCBABEND

  if (!(sysprint->dcb.dcbrecfm & dcbrecf))
  {
    zwto_debug("@TEST sysprint->dcb.dcbrecfm is not fixed (0x%x)", sysprint->dcb.dcbrecfm);
    return -1;
  }

  if (sysprint->dcb.dcblrecl != 80)
  {
    zwto_debug("@TEST sysprint->dcb.dcblrecl is not 80 (0x%x)", sysprint->dcb.dcblrecl);
    return -1;
  }

  zwto_debug("@TEST find member");
  int rsn = 0;
  rc = find_member(sysprint, &rsn);
  if (0 != rc)
  {
    zwto_debug("@TEST find_member failed: rc: %d, rsn: %d", rc, rsn);
    return -1;
  }
  zwto_debug("@TEST find member success");

  char inbuff[80] = {80};
  char writebuff[80] = {80};
  while (0 == read_sync(sysin, inbuff))
  {
    memset(writebuff, ' ', 80);
    memcpy(writebuff, inbuff, 80);
    // write_sync(sysprint, writebuff);
    zwto_debug("@TEST inbuff: %.80s", writebuff);
  }

  close_assert(sysin);
  zwto_debug("@TEST closing sysin");
  // sysprint->dcb.dcbdcbe = NULL; // NOTE(Kelosky): this can cause an I/O error which may drive synad or DCBABEND
  zwto_debug("@TEST closing sysprint");
  close_assert(sysprint);
  zwto_debug("@TEST closed sysprint");

  return 0;
}