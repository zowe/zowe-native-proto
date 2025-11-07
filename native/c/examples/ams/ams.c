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
// TODO(Kelosky): ensure resources are released in abends or thread issues
// TODO(Kelosky): TEST dcbabend
// TODO(Kelosky): TEST synad
// TODO(Kelosky): DCBE for write?
// TODO(Kelosky): handling writing via DD name
// TODO(Kelosky): handle supported formats
// TODO(Kelosky): cleanup headers
// TODO(Kelosky): use BLDL to get TTR and then on READ instead of FIND
// TODO(Kelosky): test with PDSE, STOW implications
// TODO(Kelosky): what is PROMPT for in ISPF stats?
// TODO(Kelosky): return errors as messages

static void release_resources(IO_CTRL *PTR32 sysin, IO_CTRL *PTR32 sysprint)
{
  zwto_debug("AMSMAIN cleanup started");

  if (sysprint->buffer)
  {
    storage_release(sysprint->buffer_size, sysprint->buffer);
    sysprint->buffer = NULL;
    sysprint->buffer_size = 0;
  }

  close_assert(sysin);
  close_assert(sysprint);

  zwto_debug("AMSMAIN cleanup ended");
}

/**
 * @brief By the time this routine is called, dynamic allocation of the data set we are writing to must have occurred.
 * @assign ???? bpxwydn2 may be able to obtain a unique dd name otherwise use SVC 99 (__svc99)
 */
#pragma prolog(AMSMAIN, " ZWEPROLG NEWDSA=(YES,256) ")
#pragma epilog(AMSMAIN, " ZWEEPILG ")
int AMSMAIN()
{
  zwto_debug("AMSMAIN started");

  int rsn = 0;
  int rc = 0;

  IO_CTRL *PTR32 sysin = open_input_assert("SYSIN", 80, 80, dcbrecf); // NOTE(Kelosky): we won't use this IO for reading apart from this test program

  /**
   * @brief Obtain 24 bit structures for legacy macros for non-VSAM data sets and initialize the DCB.
   */
  IO_CTRL *PTR32 sysprint = new_io_ctrl();
  memcpy(&sysprint->dcb, &open_write_model, sizeof(IHADCB));

  /**
   * @brief Set DD of data set we intend to open.  In the future, we'll probably have to require that the system provide use with a unique DD name.
   */
  char ddnam[9] = {0};
  sprintf(ddnam, "%-8.8s", "SYSPRINT");
  memcpy(sysprint->dcb.dcbddnam, ddnam, sizeof(sysprint->dcb.dcbddnam));

  /**
   * @brief Perform a read of the Job File Control Block to see what has been allocated to this "job"
   */
  rc = read_output_jfcb(sysprint);
  if (0 != rc)
  {
    zwto_debug("@TEST read_output_jfcb failed: %d", rc);
    return -1;
  }

  /**
   * @brief Validate that this is a member of a data set via PDS and member name
   */
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

  /**
   * @brief Set items that we obtained from JFCB
   */
  sysprint->dcb.dcbrecfm = sysprint->jfcb.jfcrecfm; // copy allocation attributes
  sysprint->dcb.dcbdsrg1 = dcbdsgpo;                // DSORG=PO

  /**
   * @brief Perform open
   */
  rc = open_output(&sysprint->dcb);
  if (0 != rc)
  {
    zwto_debug("@TEST open_output failed: %d", rc);
    return -1;
  }

  /**
   * @brief Verify file is indeed open and no DCBABEND has occurred
   */
  if (!(sysprint->dcb.dcboflgs & dcbofopn))
  {
    zwto_debug("@TEST sysprint->dcb.dcboflgs is not open (0x%x)", sysprint->dcb.dcboflgs);
    return -1;
  }

  // TODO(Kelosky): check for DCBABEND
  // TODO(Kelosky): invoke CLOSE

  /**
   * @brief Validate data set attributes are Fixed, Variable, and/or blocked
   */
  if (!(sysprint->dcb.dcbrecfm & dcbrecf)) // validate block or variable??
  {
    zwto_debug("@TEST sysprint->dcb.dcbrecfm is not fixed (0x%x)", sysprint->dcb.dcbrecfm);
    release_resources(sysin, sysprint);
    return -1;
  }

  zwto_debug("@TEST sysprint->dcb.dcblrecl: %d", sysprint->dcb.dcblrecl);
  if (sysprint->dcb.dcblrecl != 80)
  {
    zwto_debug("@TEST sysprint->dcb.dcblrecl is not 80 (0x%x)", sysprint->dcb.dcblrecl);
    return -1;
  }

  zwto_debug("@TEST sysprint->dcb.dcbblksi: %d", sysprint->dcb.dcbblksi);
  if (sysprint->dcb.dcbblksi < 1)
  {
    zwto_debug("@TEST sysprint->dcb.dcbblksi is less than 1 (0x%x)", sysprint->dcb.dcbblksi);
    release_resources(sysin, sysprint);
    return -1;
  }

  if (sysprint->dcb.dcbblksi % sysprint->dcb.dcblrecl != 0)
  {
    zwto_debug("@TEST sysprint->dcb.dcbblksi is not a multiple of sysprint->dcb.dcblrecl (0x%x, 0x%x)", sysprint->dcb.dcbblksi, sysprint->dcb.dcblrecl);
    release_resources(sysin, sysprint);
    return -1;
  }

  /**
   * @brief Obtain TTR and other attributes
   */
  BLDL_PL bldl_pl = {0};

  bldl_pl.ff = 1;                                                                      // only one member in the list
  bldl_pl.ll = sizeof(bldl_pl.list);                                                   // length of each entry
  memcpy(bldl_pl.list.name, sysprint->jfcb.jfcbelnm, sizeof(sysprint->jfcb.jfcbelnm)); // copy member name
  rc = bldl(sysprint, &bldl_pl, &rsn);                                                 // obtain TTR and other attributes

  if (0 != rc)
  {
    zwto_debug("@TEST bldl failed: rc: %d, rsn: %d", rc, rsn);
    release_resources(sysin, sysprint);
    return -1;
  }

  /**
   * @brief Validate that ISPF statistics are provided
   */
  // TODO(Kelosky): if no stats are provided, should we add them?
  if ((bldl_pl.list.c & LEN_MASK) == 0)
  {
    zwto_debug("@TEST no ISPF statistics are provided (0x%02x)", bldl_pl.list.c);
    release_resources(sysin, sysprint);
    return -1;
  }

  /**
   * @brief Find the member in the data set to obtain the TTR and other attributes
   * Establish the beggingin of a data set member (BPAM)
   */
  rc = find_member(sysprint, &rsn);
  if (0 != rc)
  {
    zwto_debug("@TEST find_member failed: rc: %d, rsn: %d", rc, rsn);
    release_resources(sysin, sysprint);
    return -1;
  }

  char inbuff[80] = {0};

  /**
   * @brief Allocate a buffer to write the data set member into
   */
  sysprint->buffer_size = sysprint->dcb.dcbblksi;
  sysprint->buffer = storage_obtain31(sysprint->buffer_size);

  // init buffer variables
  int bytes_in_buffer = 0;
  char *PTR32 free_location = sysprint->buffer;
  memset(sysprint->buffer, 0x00, sysprint->buffer_size);
  int lrecl = sysprint->dcb.dcblrecl;
  int blocksize = sysprint->dcb.dcbblksi;

  int lines_written = 0;

  // loop read
  while (0 == read_sync(sysin, inbuff))
  {
    zwto_debug("@TEST read: %.80s", inbuff);
    memset(free_location, ' ', lrecl);
    memcpy(free_location, inbuff, lrecl);

    lines_written++;

    // track bytes in buffer and free space
    bytes_in_buffer += lrecl;
    free_location += lrecl;

    // write block if buffer is full
    if (bytes_in_buffer >= blocksize)
    {
      write_sync(sysprint, sysprint->buffer);
      // reset buffer variables
      bytes_in_buffer = 0;
      free_location = sysprint->buffer;
      memset(sysprint->buffer, 0x00, sysprint->buffer_size);
      zwto_debug("@TEST wrote block");
    }
  }

  // write any remaining bytes in the buffer
  if (bytes_in_buffer > 0)
  {
    sysprint->dcb.dcbblksi = bytes_in_buffer; // temporary update block size before writing
    write_sync(sysprint, sysprint->buffer);   // TODO(Kelosky): if we abend, we MUST restore the original block size before CLOSE
    sysprint->dcb.dcbblksi = blocksize;

    bytes_in_buffer = 0;
    free_location = sysprint->buffer;
    memset(sysprint->buffer, 0x00, sysprint->buffer_size);
    zwto_debug("@TEST wrote block");
  }

  /**
   * @brief Find the position last block written
   */
  NOTE_RESPONSE note_response = {0};
  rc = note(sysprint, &note_response, &rsn);
  if (0 != rc)
  {
    zwto_debug("@TEST note failed: rc: %d, rsn: %d", rc, rsn);
    release_resources(sysin, sysprint);
    return -1;
  }

  /**
   * @brief Copy ISPF statistics
   */
  // Copy all user data
  memcpy(sysprint->stow_list.name, bldl_pl.list.name, sizeof(bldl_pl.list.name)); // copy member name
  memcpy(sysprint->stow_list.ttr, note_response.ttr, sizeof(note_response.ttr));  // copy NOTE TTR
  sysprint->stow_list.c = bldl_pl.list.c;                                         // copy user data length
  int user_data_len = (bldl_pl.list.c & LEN_MASK) * 2;                            // isolate number of halfwords in user data
  memcpy(sysprint->stow_list.user_data, bldl_pl.list.user_data, user_data_len);   // copy all user data

  /**
   * @brief Update ISPF statistics
   */
  ISPF_STATS *statsp = (ISPF_STATS *)sysprint->stow_list.user_data;
  zut_dump_storage_common("ISPFSTATS", statsp, sizeof(ISPF_STATS), 16, 0, zut_print_debug);

  // update ISPF statistics userid
  char user[8] = {0};
  rc = zutm1gur(user);
  if (0 != rc)
  {
    zwto_debug("@TEST zutm1gur failed: rc: %d", rc);
    release_resources(sysin, sysprint);
    return -1;
  }
  memcpy(statsp->userid, user, sizeof(user));

// update ISPF statistics modification level
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

  /**
   * @brief Obtain the current date and time
   */
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

  /**
   * @brief Update the directory entry with the new ISPF statistics
   */
  rc = stow(sysprint, &rsn);
  if (0 != rc)
  {
    zwto_debug("@TEST stow failed: rc: %d, rsn: %d", rc, rsn);
    release_resources(sysin, sysprint);
    return -1;
  }

  /**
   * @brief Release resources
   */
  release_resources(sysin, sysprint);

  return 0;
}
