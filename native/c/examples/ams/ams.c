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
#include "zwto.h"
#include "dcbd.h"
#include "zam.h"
#include "zdbg.h"

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