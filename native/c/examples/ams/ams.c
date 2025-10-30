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
#include "zams24.h"
#include "zmetal.h"
#include "zdbg.h"

// TODO(Kelosky): determine if we need to use BPAM up front
// if BPAM, do dynalloc, RDJFCB to valid attributes or perhaps use something like fldata
// open without TYPE=J

#pragma prolog(AMSMAIN, " ZWEPROLG NEWDSA=(YES,256) ")
#pragma epilog(AMSMAIN, " ZWEEPILG ")
int AMSMAIN()
{
  zwto_debug("AMSMAIN started");

  IO_CTRL *PTR32 ioc = new_read_io_ctrl("SYSIN", 80, 80, dcbrecf);
  set_dcb_dcbe(&ioc->dcb, eodad);

  int rc = read_input_jfcb(ioc);
  if (0 != rc)
  {
    zwto_debug("@TEST read_input_jfcb failed: %d", rc);
    s0c3_abend(1);
  }

  zwto_debug("@TEST ioc->jfcb.jfcbelnm: %.8s", ioc->jfcb.jfcbelnm); // if first by has`x'BF'` then member name exists and not a PS file, clear DSORG if needed
  zwto_debug("@TEST ioc->jfcb.jfcnlrec: %d", ioc->jfcb.jfcnlrec);
  zwto_debug("@TEST ioc->jfcb.jfcbaxbf: %d", ioc->jfcb.jfcbaxbf); // JFCBLKSI

  // ZAMS24_FUNCS funcs = {0};
  // AMS24_fn AMS24 = (AMS24_fn)load_module31("ZAMS24");
  // if (!AMS24)
  // {
  //   zwto_debug("@TEST AMS24 not found");
  //   s0c3_abend(1);
  // }

  // rc = AMS24(&funcs);
  // if (0 != rc)
  // {
  //   zwto_debug("@TEST AMS24 failed: %d", rc);
  //   s0c3_abend(1);
  // }

  rc = open_input(&ioc->dcb);
  // rc = funcs.open_input_j(ioc);
  // if (0 != rc)
  // {
  //   zwto_debug("@TEST open_input_j failed: %d", rc);
  //   s0c3_abend(1);
  // }

  return 0;

  IO_CTRL *PTR32 sysin = open_input_assert("SYSIN", 80, 80, dcbrecf);
  IO_CTRL *PTR32 sysprint = open_output_assert("SYSPRINT", 80, 80, dcbrecf);

  char inbuff[80] = {80};
  char writebuff[80] = {80};
  while (0 == read_sync(sysin, inbuff))
  {
    memset(writebuff, ' ', 80);
    memcpy(writebuff, inbuff, 80);
    write_sync(sysprint, writebuff);
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