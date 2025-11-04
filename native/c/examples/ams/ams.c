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
#include "zamtypes.h"
#include "zmetal.h"
#include "zutm.h"
#include "zwto.h"
#include "dcbd.h"
#include "zam.h"
#include "zmetal.h"
#include "zdbg.h"
#include "zutm31.h"
#include "ztime.h"
#include <builtins.h>

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
// TODO(Kelosky): cleanup headers
// TODO(Kelosky): use BLDL to get TTR and then on READ instead of FIND
// TODO(Kelosky): test with PDSE, STOW implications
// TODO(Kelosky): IHAPDS to map pds
// TODO(kelosky): what is PROMPT for in ISPF

#pragma prolog(AMSMAIN, " ZWEPROLG NEWDSA=(YES,256) ")
#pragma epilog(AMSMAIN, " ZWEEPILG ")
int AMSMAIN()
{
  zwto_debug("AMSMAIN started");

  int rsn = 0;
  int rc = 0;

  IO_CTRL *PTR32 sysin = open_input_assert("SYSIN", 80, 80, dcbrecf); // NOTE(Kelosky): we won't use this IO for reading apart from this test program
  // IO_CTRL *PTR32 sysprint = open_output_assert("SYSPRINT", 80, 80, dcbrecf);
  // IO_CTRL *PTR32 sysprint = new_write_io_ctrl("SYSPRINT", 80, 80, dcbrecf);
  // set_dcb_dcbe(&sysprint->dcb, eodad); // TODO(Kelosky): is dcbe needed for write?

  IO_CTRL *PTR32 sysprint = new_io_ctrl();
  memcpy(&sysprint->dcb, &open_write_model, sizeof(IHADCB));

  char ddnam[9] = {0};
  sprintf(ddnam, "%-8.8s", "SYSPRINT");
  memcpy(sysprint->dcb.dcbddnam, ddnam, sizeof(sysprint->dcb.dcbddnam));

  rc = read_output_jfcb(sysprint);
  if (0 != rc)
  {
    zwto_debug("@TEST read_output_jfcb failed: %d", rc);
    return -1;
  }

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

  zwto_debug("@TEST sysprint->jfcb.jfcbind1: %x", sysprint->jfcb.jfcbind1);
  zwto_debug("@TEST sysprint->jfcb.jfcbaxbf: %d", sysprint->jfcb.jfcbaxbf);
  zwto_debug("@TEST sysprint->jfcb.jfcrecfm: %d", sysprint->jfcb.jfcrecfm);

  // error if not blocked
  sysprint->dcb.dcbrecfm = sysprint->jfcb.jfcrecfm;

  if (sysprint->dcb.dcbdsrg1 != dcbdsgpo)
  {
    zwto_debug("@TEST sysprint->dcb.dcbdsrg1: %x", sysprint->dcb.dcbdsrg1);
  }

  sysprint->dcb.dcbdsrg1 = dcbdsgpo; // DSORG=PO
  rc = open_output(&sysprint->dcb);
  if (0 != rc)
  {
    zwto_debug("@TEST open_output failed: %d", rc);
    return -1;
  }

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

  zwto_debug("@TEST sysprint->dcb.dcbblksi: %d", sysprint->dcb.dcbblksi);
  return 7;

  BLDL_PL bldl_pl = {0};

  bldl_pl.ff = 1;
  bldl_pl.ll = sizeof(bldl_pl.list);
  memcpy(bldl_pl.list.name, sysprint->jfcb.jfcbelnm, sizeof(sysprint->jfcb.jfcbelnm));
  rc = bldl(sysprint, &bldl_pl, &rsn);

  if (0 != rc)
  {
    zwto_debug("@TEST bldl failed: rc: %d, rsn: %d", rc, rsn);
    return -1;
  }

  rc = find_member(sysprint, &rsn);
  if (0 != rc)
  {
    zwto_debug("@TEST find_member failed: rc: %d, rsn: %d", rc, rsn);
    return -1;
  }

  short int lines_written = 0;
  char inbuff[80] = {80};
  char writebuff[80] = {80};
  while (0 == read_sync(sysin, inbuff))
  {
    memset(writebuff, ' ', 80);
    memcpy(writebuff, inbuff, 80);
    lines_written++;
    write_sync(sysprint, writebuff);
    zwto_debug("@TEST inbuff: %.80s", writebuff);
  }

  NOTE_RESPONSE note_response = {0};
  rc = note(sysprint, &note_response, &rsn);
  if (0 != rc)
  {
    zwto_debug("@TEST note failed: rc: %d, rsn: %d", rc, rsn);
    return -1;
  }

  // Initially copy all user data
  memcpy(sysprint->stow_list.name, bldl_pl.list.name, sizeof(bldl_pl.list.name));
  sysprint->stow_list.c = bldl_pl.list.c;
  memcpy(sysprint->stow_list.ttr, note_response.ttr, sizeof(note_response.ttr));
  memcpy(sysprint->stow_list.user_data, bldl_pl.list.user_data, sizeof(bldl_pl.list.user_data)); // copy all user data

  // address ISPF statistics
  ISPF_STATS *statsp = (ISPF_STATS *)sysprint->stow_list.user_data;
  zut_dump_storage_common("ISPFSTATS", statsp, sizeof(ISPF_STATS), 16, 0, zut_print_debug);

  // update ISPF statistics userid
  char user[8] = {0};
  rc = zutm1gur(user);
  if (0 != rc)
  {
    zwto_debug("@TEST zutm1gur failed: rc: %d", rc);
    return -1;
  }
  memcpy(statsp->userid, user, sizeof(user));

// aupdate ISPF statistics modification level
// https://www.ibm.com/docs/en/zos/3.2.0?topic=environment-version-modification-level-numbers
// level 0x99 is the maximum level
#define MAX_LEVEL 0x99
  if (statsp->level < MAX_LEVEL)
  {
    statsp->level++; // update ISPF statistics level
  }

  // update ISPF statistics number of lines
  statsp->modified_number_of_lines = lines_written; // update ISPF statistics number of lines
  statsp->current_number_of_lines = lines_written;  // update ISPF statistics number of lines

  union
  {
    unsigned int timei;
    struct
    {
      unsigned char HH;
      unsigned char MM;
      unsigned char SS;
      unsigned char unused;
    } times;
  } timel = {0};

  unsigned int datel = 0;
  time_local(&timel.timei, &datel);

  // update ISPF statistics date & time
  memcpy(&statsp->modified_date_century, &datel, sizeof(datel));
  statsp->modified_time_hours = timel.times.HH;   // update ISPF statistics time hours
  statsp->modified_time_minutes = timel.times.MM; // update ISPF statistics time minutes
  statsp->modified_time_seconds = timel.times.SS; // update ISPF statistics time seconds

  rc = stow(sysprint, &rsn);
  if (0 != rc)
  {
    zwto_debug("@TEST stow failed: rc: %d, rsn: %d", rc, rsn);
    return -1;
  }

  close_assert(sysin);
  close_assert(sysprint);

  return 0;
}
