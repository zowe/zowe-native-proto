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
#include <builtins.h>
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
#include "zenq.h"
#include "iezdeb.h"

typedef struct deb DEB;

/**
 * https://www.ibm.com/docs/en/zos/3.1.0?topic=defaults-ispf-ispfpdf
 *
To serialize access to resources with concurrent batch or TSO/E use of the resources, ISPF relies on MVS allocation (qname of SYSDSN).
To ensure the integrity of shared data, batch, or TSO/E, users who are updating a data set must allocate it with DISP=OLD.
Because MVS allocation does not satisfy an exclusive request and a shared request for the same resource at the same time, data set integrity is maintained between ISPF users and batch or TSO/E users.
To serialize access to partitioned data sets among multiple ISPF users, ISPF also issues its own ENQ, DEQ, and RESERVE macros.
To allow users to update a data set that has a record format of "U", ISPF serializes with the linkage editor to protect the entire partitioned data set.
 *
 */

/**
 *
 * We can comment out the SPFEDIT enq and test ISPF + VS Code writing via STWO with ISGENQ RESERVEVOL=YES
 * if bldl not found create stats
 * handle the case matt found on 65356 line changes
 * function to return stats
 * stow works with any ttr value
 *
 */

/**
 *
 * if in view mode, not spfedit enq on member name
 * if in edit mode, spfedit enq on member name
 * when writin isgenq with SGENQ SCOPE=SYSTEMS RESERVEVOLUME=YES is probably what we need to do
 * no need to find - remove
 * stow ttr set to zero still "works" and needs debugged
 *  try wtritting two members on the same open with different data to see what happens
 * bldl if member doesnt exist needs to create ispf stats
 *
 */

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

typedef struct
{
  IO_CTRL *PTR32 sysin;
  IO_CTRL *PTR32 sysprint;
} RESOURCES;

static void release_resources(RESOURCES *resources)
{
  zwto_debug("AMSMAIN cleanup started");

  if (resources->sysprint->buffer)
  {
    zwto_debug("@TEST releasing buffer for sysprint");
    zwto_debug("@TEST dcbe: %p", resources->sysprint->dcb.dcbdcbe);
    storage_release(resources->sysprint->buffer_size, resources->sysprint->buffer);
    resources->sysprint->buffer = NULL;
    resources->sysprint->buffer_size = 0;
  }

  if (resources->sysin)
  {
    zwto_debug("@TEST releasing sysin");
    zwto_debug("@TEST dcbe: %p", resources->sysprint->dcb.dcbdcbe);
    close_assert(resources->sysin);
    resources->sysin = NULL;
  }

  if (resources->sysprint)
  {
    zwto_debug("@TEST dcbe: %p", resources->sysprint->dcb.dcbdcbe);
    zwto_debug("@TEST releasing sysprint");
    // TODO(Kelosky): caller lower level services to avoid S0C3 in this function
    close_assert(resources->sysprint);
    resources->sysprint = NULL;
  }

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

  RESOURCES resources = {0};

  resources.sysin = open_input_assert("SYSIN", 80, 80, dcbrecf); // NOTE(Kelosky): we won't use this IO for reading apart from this test program

  /**
   * @brief Obtain 24 bit structures for legacy macros for non-VSAM data sets and initialize the DCB.
   */
  resources.sysprint = new_io_ctrl();
  memcpy(&resources.sysprint->dcb, &open_write_model, sizeof(IHADCB));

  /**
   * @brief Set DD of data set we intend to open.  In the future, we'll probably have to require that the system provide use with a unique DD name.
   */
  char ddnam[9] = {0};
  sprintf(ddnam, "%-8.8s", "SYSPRINT");
  memcpy(resources.sysprint->dcb.dcbddnam, ddnam, sizeof(resources.sysprint->dcb.dcbddnam));

  /**
   * @brief Perform a read of the Job File Control Block to see what has been allocated to this "job"
   */
  rc = read_output_jfcb(resources.sysprint);
  if (0 != rc)
  {
    zwto_debug("@TEST read_output_jfcb failed: %d", rc);
    return -1;
  }

  /**
   * @brief Validate that this is a member of a data set via PDS and member name
   */
  if (resources.sysprint->jfcb.jfcbind1 != jfcpds)
  {
    zwto_debug("@TEST resources.sysprint->jfcb.jfcbind1 is not PDS (0x%x)", resources.sysprint->jfcb.jfcbind1);
    return -1;
  }

  // ensure member name (e.g. is a partitioned data set)
  if (resources.sysprint->jfcb.jfcbelnm[0] == ' ')
  {
    zwto_debug("@TEST resources.sysprint->jfcb.jfcbelnm is empty");
    return -1;
  }

  /**
   * @brief IBM recommends using ENQ SCOPE=SYSTEMS.  We may use RESERVE if one of the following is true:
   * - GRS is not active (only applies to early IPL)
   * - Installation is not using SMS
   * https://www.ibm.com/docs/en/zos/3.2.0?topic=dasd-macros-used-shared-reserve-extract-getdsab
   *
   * ____ SYSDSN   DKELOSKY.IO.I.F80                             17 SYSTEM  SYSTEM  SHRD O
   * ____ SPFEDIT  DKELOSKY.IO.I.F80                          +  52 SYSTEMS SYSTEMS EXCL O
   */

  QNAME qname = {0};
  RNAME rname = {0};

  QNAME qname_reserve = {0};
  RNAME rname_reserve = {0};

  strcpy(qname_reserve.value, "SPFEDIT ");
  rname_reserve.rlen = sprintf(rname_reserve.value, "%.*s", sizeof(resources.sysprint->jfcb.jfcbdsnm), resources.sysprint->jfcb.jfcbdsnm);
  zwto_debug("@TEST qname_reserve: %s and length: %d and rname_reserve: %.*s", qname_reserve.value, rname_reserve.rlen, rname_reserve.rlen, rname_reserve.value);

  strcpy(qname.value, "SPFEDIT ");
  zwto_debug("@TEST resources.sysprint->jfcb.jfcbdsnm: %.*s", sizeof(resources.sysprint->jfcb.jfcbdsnm), resources.sysprint->jfcb.jfcbdsnm);
  zwto_debug("@TEST resources.sysprint->jfcb.jfcbelnm: %.*s", sizeof(resources.sysprint->jfcb.jfcbelnm), resources.sysprint->jfcb.jfcbelnm);
  rname.rlen = sprintf(rname.value, "%.*s%.*s", sizeof(resources.sysprint->jfcb.jfcbdsnm), resources.sysprint->jfcb.jfcbdsnm, sizeof(resources.sysprint->jfcb.jfcbelnm), resources.sysprint->jfcb.jfcbelnm);
  zwto_debug("@TEST qname: %s and length: %d and rname: %.*s", qname.value, rname.rlen, rname.rlen, rname.value);

  rc = enq(&qname, &rname); // TODO(Kelosky): before open? How is directory entry protected?
  if (0 != rc)
  {
    zwto_debug("@TEST reserve failed: %d", rc);
    release_resources(&resources);
  }

  DEB *PTR32 deb = (DEB * PTR32) & resources.sysprint->dcb.dcbiflgs; // flags followed by DEB
  deb = (DEB * PTR32)((unsigned int)deb & 0x00FFFFFF);               // clear flags

  zwto_debug("@TEST deb->debflgs2: %02x", deb->debflgs2);

  if (deb->debflgs2 & deb31ucb)
  {
    zwto_debug("@TEST deb31ucb is set");
  }
  else
  {
    zwto_debug("@TEST deb31ucb is not set");
  }

  // TODO(Kelosky): ensure proper sequence, ensure conditional LOC= on RESREVE and perhaps the DEQ, ensure proper RESERVE model set
  // rc = reserve(&qname_reserve, &rname_reserve, (UCB *)&resources.sysprint->dcb);
  // if (0 != rc)
  // {
  //   zwto_debug("@TEST reserve failed: %d", rc);
  //   release_resources(&resources);
  //   return -1;
  // }

  // zwto_debug("@TEST reserve successful");

  /////////////////////////////////////////////////////////////

  // WTO_BUF buf = {0};
  // buf.len = sprintf(buf.msg, "Awaiting reply");

  // ECB ecb = {0};

  // WTOR_REPLY_BUF reply = {0};
  // rc = wtor(&buf, &reply, &ecb);
  // if (0 != rc)
  // {
  //   zwto_debug("@TEST wtor failed: %d", rc);
  //   return -1;
  // }
  // zwto_debug("@TEST wtor reply: %s", reply.msg);

  // ecb_wait(&ecb);
  // zwto_debug("@TEST wtor reply: %s", reply.msg);
  /////////////////////////////////////////////////////////////

  /**
   * @brief Set DCBE
   */

  // set_dcb_dcbe(&resources.sysprint->dcb);
  zwto_debug("@TEST resources.sysprint->dcb.dcbdcbe: %p", resources.sysprint->dcb.dcbdcbe);

  /**
   * @brief Set items that we obtained from JFCB
   */
  resources.sysprint->dcb.dcbrecfm = resources.sysprint->jfcb.jfcrecfm; // copy allocation attributes
  resources.sysprint->dcb.dcbdsrg1 = dcbdsgpo;                          // DSORG=PO

  zwto_debug("@TEST opening for output");

  /**
   * @brief Perform open
   */
  rc = open_output(&resources.sysprint->dcb);
  if (0 != rc)
  {
    zwto_debug("@TEST open_output failed: %d", rc);
    return -1;
  }
  zwto_debug("@TEST opened for output");
  zwto_debug("@TEST resources.sysprint->dcb.dcbdcbe: %p", resources.sysprint->dcb.dcbdcbe);

  /**
   * @brief Verify file is indeed open and no DCBABEND has occurred
   */
  if (!(resources.sysprint->dcb.dcboflgs & dcbofopn))
  {
    zwto_debug("@TEST resources.sysprint->dcb.dcboflgs is not open (0x%x)", resources.sysprint->dcb.dcboflgs);
    return -1;
  }

  // TODO(Kelosky): check for DCBABEND
  // TODO(Kelosky): invoke CLOSE

  /**
   * @brief Validate data set attributes are Fixed, Variable, and/or blocked
   */
  if (!(resources.sysprint->dcb.dcbrecfm & dcbrecf)) // validate block or variable??
  {
    zwto_debug("@TEST resources.sysprint->dcb.dcbrecfm is not fixed (0x%x)", resources.sysprint->dcb.dcbrecfm);
    release_resources(&resources);
    return -1;
  }

  zwto_debug("@TEST resources.sysprint->dcb.dcblrecl: %d", resources.sysprint->dcb.dcblrecl);
  if (resources.sysprint->dcb.dcblrecl != 80)
  {
    zwto_debug("@TEST resources.sysprint->dcb.dcblrecl is not 80 (0x%x)", resources.sysprint->dcb.dcblrecl);
    return -1;
  }

  zwto_debug("@TEST resources.sysprint->dcb.dcbblksi: %d", resources.sysprint->dcb.dcbblksi);
  if (resources.sysprint->dcb.dcbblksi < 1)
  {
    zwto_debug("@TEST resources.sysprint->dcb.dcbblksi is less than 1 (0x%x)", resources.sysprint->dcb.dcbblksi);
    release_resources(&resources);
    return -1;
  }

  if (resources.sysprint->dcb.dcbblksi % resources.sysprint->dcb.dcblrecl != 0)
  {
    zwto_debug("@TEST resources.sysprint->dcb.dcbblksi is not a multiple of resources.sysprint->dcb.dcblrecl (0x%x, 0x%x)", resources.sysprint->dcb.dcbblksi, resources.sysprint->dcb.dcblrecl);
    release_resources(&resources);
    return -1;
  }

  /**
   * @brief Find the position last block written
   */
  NOTE_RESPONSE note_response = {0};
  rc = note(resources.sysprint, &note_response, &rsn);
  if (0 != rc)
  {
    zwto_debug("@TEST note failed: rc: %d, rsn: %d", rc, rsn);
    release_resources(&resources);
    return -1;
  }
  zut_dump_storage_common("NOTE TTR after open before memset", &note_response.ttr, 3, 16, 0, zut_print_debug);

  memset(&note_response, 0x00, sizeof(NOTE_RESPONSE));

  zut_dump_storage_common("NOTE TTR after open after memset", &note_response.ttr, 3, 16, 0, zut_print_debug);

  /**
   * @brief Obtain TTR and other attributes
   */
  BLDL_PL bldl_pl = {0};

  bldl_pl.ff = 1;                                                                                          // only one member in the list
  bldl_pl.ll = sizeof(bldl_pl.list);                                                                       // length of each entry
  memcpy(bldl_pl.list.name, resources.sysprint->jfcb.jfcbelnm, sizeof(resources.sysprint->jfcb.jfcbelnm)); // copy member name
  rc = bldl(resources.sysprint, &bldl_pl, &rsn);                                                           // obtain TTR and other attributes

  // TODO(Kelosky): if BLDL fails with RC = 4, not found, should we create stats?
  if (0 != rc)
  {
    zwto_debug("@TEST bldl failed: rc: %d, rsn: %d", rc, rsn);
    release_resources(&resources);
    return -1;
  }

  zut_dump_storage_common("initial BLDL TTR", &bldl_pl.list.ttr, 3, 16, 0, zut_print_debug);

  /**
   * @brief Validate that ISPF statistics are provided
   */
  // TODO(Kelosky): if no stats are provided, should we add them?
  if ((bldl_pl.list.c & LEN_MASK) == 0)
  {
    zwto_debug("@TEST no ISPF statistics are provided (0x%02x)", bldl_pl.list.c);
    release_resources(&resources);
    return -1;
  }

  // /**
  //  * @brief Find the member in the data set to obtain the TTR and other attributes
  //  * Establish the beggingin of a data set member (BPAM)
  //  * // TODO(Kelosky): this could be a POINT
  //  * // TODO(Kelosky): this find may not be needed ... .
  //  */
  // rc = find_member(resources.sysprint, &rsn);
  // if (0 != rc)
  // {
  //   zwto_debug("@TEST find_member failed: rc: %d, rsn: %d", rc, rsn);
  //   release_resources(&resources);
  //   return -1;
  // }

  char inbuff[80] = {0};

  /**
   * @brief Allocate a buffer to write the data set member into
   */
  resources.sysprint->buffer_size = resources.sysprint->dcb.dcbblksi;
  resources.sysprint->buffer = storage_obtain31(resources.sysprint->buffer_size);

  // init buffer variables
  int bytes_in_buffer = 0;
  char *PTR32 free_location = resources.sysprint->buffer;
  memset(resources.sysprint->buffer, 0x00, resources.sysprint->buffer_size);
  int lrecl = resources.sysprint->dcb.dcblrecl;
  int blocksize = resources.sysprint->dcb.dcbblksi;

  int lines_written = 0;

  /////////////////////////////////////////////////////////////

  // resources.sysprint->dcb.dcbblksi = 7; // NOTE(Kelosky): this can be used to drive a SYNAD exit
  // allocate a data set for output, write to it, then attempt to read it

  /////////////////////////////////////////////////////////////

  // loop read
  while (0 == read_sync(resources.sysin, inbuff))
  {
    // zwto_debug("@TEST read: %.80s", inbuff);
    memset(free_location, ' ', lrecl);
    memcpy(free_location, inbuff, lrecl);

    lines_written++;

    // track bytes in buffer and free space
    bytes_in_buffer += lrecl;
    free_location += lrecl;

    // write block if buffer is full
    if (bytes_in_buffer >= blocksize)
    {
      write_sync(resources.sysprint, resources.sysprint->buffer);
      // reset buffer variables
      bytes_in_buffer = 0;
      free_location = resources.sysprint->buffer;
      memset(resources.sysprint->buffer, 0x00, resources.sysprint->buffer_size);
      zwto_debug("@TEST wrote block");
    }
  }

  // write any remaining bytes in the buffer
  if (bytes_in_buffer > 0)
  {
    resources.sysprint->dcb.dcbblksi = bytes_in_buffer;         // temporary update block size before writing
    write_sync(resources.sysprint, resources.sysprint->buffer); // TODO(Kelosky): if we abend, we MUST restore the original block size before CLOSE
    resources.sysprint->dcb.dcbblksi = blocksize;

    bytes_in_buffer = 0;
    free_location = resources.sysprint->buffer;
    memset(resources.sysprint->buffer, 0x00, resources.sysprint->buffer_size);
    // zwto_debug("@TEST wrote block");
  }

  zwto_debug("@TEST dcbe: %p", resources.sysprint->dcb.dcbdcbe);

  // /**
  //  * @brief Find the position last block written
  //  */
  // NOTE_RESPONSE note_response = {0};
  // rc = note(resources.sysprint, &note_response, &rsn);
  // if (0 != rc)
  // {
  //   zwto_debug("@TEST note failed: rc: %d, rsn: %d", rc, rsn);
  //   release_resources(&resources);
  //   return -1;
  // }

  // zut_dump_storage_common("NOTE TTR", &note_response.ttr, 3, 16, 0, zut_print_debug);

  /**
   * @brief Copy ISPF statistics
   */
  // Copy all user data
  memcpy(resources.sysprint->stow_list.name, bldl_pl.list.name, sizeof(bldl_pl.list.name)); // copy member name
  memcpy(resources.sysprint->stow_list.ttr, note_response.ttr, sizeof(note_response.ttr));  // copy NOTE TTR
  resources.sysprint->stow_list.c = bldl_pl.list.c;                                         // copy user data length
  int user_data_len = (bldl_pl.list.c & LEN_MASK) * 2;                                      // isolate number of halfwords in user data
  memcpy(resources.sysprint->stow_list.user_data, bldl_pl.list.user_data, user_data_len);   // copy all user data

  /**
   * @brief Update ISPF statistics
   */
  ISPF_STATS *statsp = (ISPF_STATS *)resources.sysprint->stow_list.user_data;
  zut_dump_storage_common("ISPFSTATS", statsp, sizeof(ISPF_STATS), 16, 0, zut_print_debug);

  // update ISPF statistics userid
  char user[8] = {0};
  rc = zutm1gur(user);
  if (0 != rc)
  {
    zwto_debug("@TEST zutm1gur failed: rc: %d", rc);
    release_resources(&resources);
    return -1;
  }
  memcpy(statsp->userid, user, sizeof(user));
  zwto_debug("@TEST1 dcbe: %p", resources.sysprint->dcb.dcbdcbe);

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

  zwto_debug("@TEST6 dcbe: %p", resources.sysprint->dcb.dcbdcbe);

  zut_dump_storage_common("DIRECTORY ENTRY before stow", &resources.sysprint->stow_list.user_data, sizeof(STOW_LIST), 16, 0, zut_print_debug);

  /**
   * @brief Update the directory entry with the new ISPF statistics
   */
  rc = stow(resources.sysprint, &rsn);
  if (0 != rc)
  {
    zwto_debug("@TEST stow failed: rc: %d, rsn: %d", rc, rsn);
    release_resources(&resources);
    return -1;
  }
  zwto_debug("@TEST7 dcbe: %p", resources.sysprint->dcb.dcbdcbe);

  zut_dump_storage_common("DIRECTORY ENTRY after stow", &resources.sysprint->stow_list.user_data, sizeof(STOW_LIST), 16, 0, zut_print_debug);

  // TODO(Kelosky): this enq should drop after we decided we're done editing the data set
  rc = deq(&qname, &rname);
  if (0 != rc)
  {
    zwto_debug("@TEST deq failed: rc: %d", rc);
    release_resources(&resources);
    return -1;
  }
  zwto_debug("@TEST1 dcbe: %p", resources.sysprint->dcb.dcbdcbe);

  zwto_debug("@TEST releasing resources");

  /**
   * @brief Release resources
   */
  release_resources(&resources);

  return 0;
}
