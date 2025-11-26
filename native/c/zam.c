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
// #include "zam24.h"
#include "zdstype.h"
#include "zwto.h"
#include "zenq.h"
#include "ihapsa.h"
#include "ikjtcb.h"
#include "ieftiot1.h"
#include "zutm31.h"
#include "ztime.h"

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

static int validate_jfcb_attributes(ZDIAG *PTR32 diag, IO_CTRL *PTR32 ioc)
{
  int rc = 0;

  zwto_debug("@TEST validate attributes of data set: %44.44s", ioc->jfcb.jfcbdsnm);
  if (ioc->jfcb.jfcbind1 != jfcpds)
  {
    diag->e_msg_len = sprintf(diag->e_msg, "DDname: %8.8s data set: %44.44s is not a PDS: %X", ioc->dcb.dcbddnam, ioc->jfcb.jfcbdsnm, ioc->jfcb.jfcbind1);
    diag->detail_rc = ZDS_RTNCD_UNSUPPORTED_DATA_SET;
    return RTNCD_FAILURE;
  }

  // ensure member name (e.g. is a partitioned data set)
  if (ioc->jfcb.jfcbelnm[0] == ' ')
  {
    diag->e_msg_len = sprintf(diag->e_msg, "DDname: %8.8s data set: %44.44s is not a partitioned data set: %s", ioc->dcb.dcbddnam, ioc->jfcb.jfcbdsnm, ioc->jfcb.jfcbelnm);
    diag->detail_rc = ZDS_RTNCD_UNSUPPORTED_DSORG;
    return RTNCD_FAILURE;
  }

  return rc;
}

static int enq_data_set(ZDIAG *PTR32 diag, IO_CTRL *PTR32 ioc)
{
  int rc = 0;
  zwto_debug("@TEST ENQ data set: %44.44s", ioc->jfcb.jfcbdsnm);
  QNAME qname = {0};
  RNAME rname = {0};
  strcpy(qname.value, "SPFEDIT");
  rname.rlen = sprintf(rname.value, "%.*s%.*s", sizeof(ioc->jfcb.jfcbdsnm), ioc->jfcb.jfcbdsnm, sizeof(ioc->jfcb.jfcbelnm), ioc->jfcb.jfcbelnm);
  rc = enq(&qname, &rname);

  if (0 != rc)
  {
    diag->service_rc = rc;
    strcpy(diag->service_name, "ENQ");
    diag->e_msg_len = sprintf(diag->e_msg, "Failed to ENQ ddname: %8.8s data set: %44.44s rc was: %d", ioc->dcb.dcbddnam, ioc->jfcb.jfcbdsnm, rc);
    diag->detail_rc = ZDS_RTNCD_ENQ_ERROR;
    return RTNCD_FAILURE;
  }
  ioc->enq = 1;

  return rc;
}

static int get_ucb(ZDIAG *PTR32 diag, IO_CTRL *PTR32 ioc)
{
  typedef struct psa PSA;
  typedef struct tcb TCB;
  typedef struct tiot TIOT;
  PSA *psa = (PSA *)0;
  TCB *PTR32 tcb = psa->psatold;
  TIOT *PTR32 tiot = tcb->tcbtio;

  unsigned char tiot_entry_len = tiot->tioelngh;

  unsigned char *PTR32 tiot_entry = (unsigned char *PTR32)tiot;

  while (tiot_entry_len > 0)
  {
    zwto_debug("@TEST tiot->tioeddnm: %-8.8s", tiot->tioeddnm);

    if (0 == strncmp(tiot->tioeddnm, ioc->dcb.dcbddnam, sizeof(tiot->tioeddnm)))
    {
      unsigned char *PTR32 tioesttb = (unsigned char *PTR32) & tiot->tioesttb;
      memcpy(&ioc->ucb, &tiot->tioesttb, sizeof(unsigned int));
      break;
    }

    tiot_entry += tiot_entry_len;
    tiot = (TIOT * PTR32) tiot_entry;
    tiot_entry_len = tiot->tioelngh;
  }

#define MASK_24_BITS 0x00FFFFFF

  ioc->ucb = (ioc->ucb & MASK_24_BITS);
  if (0 == ioc->ucb)
  {
    zwto_debug("@TEST raw_ucb_address is zero");
    diag->detail_rc = ZDS_RTNCD_UCB_ERROR;
    diag->e_msg_len = sprintf(diag->e_msg, "Failed to get UCB for data set: %44.44s", ioc->jfcb.jfcbdsnm);
    return RTNCD_FAILURE;
  }

  UCB *PTR32 ucb = (UCB * PTR32) ioc->ucb;
  return RTNCD_SUCCESS;
}

static int reserve_data_set(ZDIAG *PTR32 diag, IO_CTRL *PTR32 ioc)
{
  int rc = 0;
  zwto_debug("@TEST RESERVE data set: %44.44s", ioc->jfcb.jfcbdsnm);
  QNAME qname_reserve = {0};
  RNAME rname_reserve = {0};
  strcpy(qname_reserve.value, "SPFEDIT");
  rname_reserve.rlen = sprintf(rname_reserve.value, "%.*s", sizeof(ioc->jfcb.jfcbdsnm), ioc->jfcb.jfcbdsnm);
  rc = reserve(&qname_reserve, &rname_reserve, (UCB * PTR32) ioc->ucb);
  if (0 != rc)
  {
    diag->service_rc = rc;
    strcpy(diag->service_name, "RESERVE");
    diag->e_msg_len = sprintf(diag->e_msg, "Failed to RESERVE ddname: %8.8s data set: %44.44s rc was: %d", ioc->dcb.dcbddnam, ioc->jfcb.jfcbdsnm, rc);
    diag->detail_rc = ZDS_RTNCD_RESERVE_ERROR;
    return RTNCD_FAILURE;
  }

  ioc->reserve = 1;
  return rc;
}

static int open_data_set(ZDIAG *PTR32 diag, IO_CTRL *PTR32 ioc)
{
  int rc = 0;
  zwto_debug("@TEST open data set: %44.44s", ioc->jfcb.jfcbdsnm);
  rc = open_output(&ioc->dcb);
  if (0 != rc)
  {
    diag->service_rc = rc;
    strcpy(diag->service_name, "OPEN");
    diag->e_msg_len = sprintf(diag->e_msg, "Failed to open ddname: %8.8s for data set: %44.44s rc was: %d", ioc->dcb.dcbddnam, ioc->jfcb.jfcbdsnm, rc);
    diag->detail_rc = ZDS_RTNCD_OPEN_ERROR;
    return RTNCD_FAILURE;
  }

  if (!(ioc->dcb.dcboflgs & dcbofopn))
  {
    diag->e_msg_len = sprintf(diag->e_msg, "Data set is not open: %44.44s", ioc->jfcb.jfcbdsnm);
    diag->detail_rc = ZDS_RTNCD_NOT_OPEN_ERROR;
    return RTNCD_FAILURE;
  }

  return rc;
}

static int validate_dcb_attributes(ZDIAG *PTR32 diag, IO_CTRL *PTR32 ioc)
{
  int rc = 0;

  zwto_debug("@TEST validate DCB attributes: %X", ioc->dcb.dcbrecfm);
  if (!(ioc->dcb.dcbrecfm & dcbrecf))
  {
    diag->e_msg_len = sprintf(diag->e_msg, "Data set is not a fixed record format: %X", ioc->dcb.dcbrecfm);
    diag->detail_rc = ZDS_RTNCD_UNSUPPORTED_RECFM;
    return RTNCD_FAILURE;
  }

  if (ioc->dcb.dcbblksi < 1)
  {
    diag->e_msg_len = sprintf(diag->e_msg, "Data set has less than 1 block size: %X", ioc->dcb.dcbblksi);
    diag->detail_rc = ZDS_RTNCD_UNSUPPORTED_BLOCK_SIZE;
    return RTNCD_FAILURE;
  }

  if (ioc->dcb.dcbblksi % ioc->dcb.dcblrecl != 0)
  {
    diag->e_msg_len = sprintf(diag->e_msg, "Data set block size is not a multiple of the record length: %X, %X", ioc->dcb.dcbblksi, ioc->dcb.dcblrecl);
    diag->detail_rc = ZDS_RTNCD_INVALID_BLOCK_SIZE;
    return RTNCD_FAILURE;
  }

  return rc;
}

int open_output_bpam(ZDIAG *PTR32 diag, IO_CTRL *PTR32 *PTR32 ioc, const char *PTR32 ddname)
{
  int rc = 0;
  IO_CTRL *PTR32 ioc31 = NULL;

  //
  // Obtain IO_CTRL for the data set
  //
  zwto_debug("@TEST get io ctrl for data set: %s", ddname);
  IO_CTRL *PTR32 new_ioc = new_io_ctrl();
  *ioc = new_ioc;
  memcpy(&new_ioc->dcb, &open_write_model, sizeof(IHADCB));
  memcpy(new_ioc->dcb.dcbddnam, ddname, sizeof(new_ioc->dcb.dcbddnam));

  //
  // Read the JFCB for the data set
  //
  zwto_debug("@TEST read output jfcb for data set: %s", ddname);
  rc = read_output_jfcb(new_ioc);
  if (0 != rc)
  {
    diag->detail_rc = ZDS_RTNCD_SERVICE_FAILURE;
    strcpy(diag->service_name, "RDJFCB");
    diag->e_msg_len = sprintf(diag->e_msg, "Failed to read output JFCB rc was: %d", rc);
    diag->service_rc = rc;
    return RTNCD_FAILURE;
  }

  //
  // Validate attributes of the data set
  //
  rc = validate_jfcb_attributes(diag, new_ioc);
  if (0 != rc)
  {
    return rc;
  }

  //
  // ENQ data set
  //
  rc = enq_data_set(diag, new_ioc);
  if (0 != rc)
  {
    return rc;
  }

  //
  // Get the UCB for the data set
  //
  rc = get_ucb(diag, new_ioc);
  if (0 != rc)
  {
    return rc;
  }

  //
  // RESERVE the data set
  //
  rc = reserve_data_set(diag, new_ioc);
  if (0 != rc)
  {
    return rc;
  }

  //
  // Set attributes of the DCB
  //
  new_ioc->dcb.dcbrecfm = new_ioc->jfcb.jfcrecfm; // copy allocation attributes
  new_ioc->dcb.dcbdsrg1 = dcbdsgpo;               // DSORG=PO

  //
  // Open the data set
  //
  rc = open_data_set(diag, new_ioc);
  if (0 != rc)
  {
    return rc;
  }

  //
  // Validate record format of the data set
  //
  rc = validate_dcb_attributes(diag, new_ioc);
  if (0 != rc)
  {
    return rc;
  }

  //
  //  Obtain a buffer for the data set
  //
  new_ioc->buffer_size = new_ioc->dcb.dcbblksi;
  new_ioc->buffer = storage_obtain31(new_ioc->buffer_size);

  return rc;
}

int write_output_bpam(ZDIAG *PTR32 diag, IO_CTRL *PTR32 ioc, const char *PTR32 data, int length)
{
  int rc = 0;
  zwto_debug("@TEST write_output_bpam length: %d", length);
  return rc;
}

static int bldl_member(ZDIAG *PTR32 diag, IO_CTRL *PTR32 ioc, BLDL_PL *PTR32 bldl_pl)
{
  int rc = 0;
  int rsn = 0;
  zwto_debug("@TEST bldl member: %44.44s", ioc->jfcb.jfcbdsnm);
  bldl_pl->ff = 1;                                                            // only one member in the list
  bldl_pl->ll = sizeof(bldl_pl->list);                                        // length of each entry
  memcpy(bldl_pl->list.name, ioc->jfcb.jfcbelnm, sizeof(ioc->jfcb.jfcbelnm)); // copy member name
  rc = bldl(ioc, bldl_pl, &rsn);

  if (0 != rc)
  {
    diag->service_rc = rc;
    strcpy(diag->service_name, "BLDL");
    diag->e_msg_len = sprintf(diag->e_msg, "Failed to BLDL ddname: %8.8s data set: %44.44s rc was: %d", ioc->dcb.dcbddnam, ioc->jfcb.jfcbdsnm, rc);
    diag->detail_rc = ZDS_RTNCD_BLDL_ERROR;
  }
  return rc;
}

static int update_ispf_statistics(ZDIAG *PTR32 diag, IO_CTRL *PTR32 ioc)
{
  int rc = 0;

  if (ioc->dcb.dcboflgs & dcbofopn)
  {
    int rsn = 0;

    //
    // BLDL for the data set that exists or was just created
    //
    BLDL_PL bldl_pl = {0};
    rc = bldl_member(diag, ioc, &bldl_pl);
    if (0 != rc)
    {
      return rc;
    }

    //
    // Copy or create ISPF statistics
    //
    if ((bldl_pl.list.c & LEN_MASK) == 0)
    {
      zwto_debug("@TEST no ISPF statistics are provided (0x%02x)", bldl_pl.list.c);
      memcpy(ioc->stow_list.name, bldl_pl.list.name, sizeof(bldl_pl.list.name));
      ioc->stow_list.c = ISPF_STATS_MIN_LEN;
      ISPF_STATS *statsp = (ISPF_STATS *)ioc->stow_list.user_data;
      statsp->version = 0x01;
      statsp->level = 0x00;
      statsp->flags = 0x00;

      char user[8] = {0};
      rc = zutm1gur(user);
      if (0 != rc)
      {
        zwto_debug("@TEST zutm1gur failed: rc: %d", rc);
        return RTNCD_FAILURE;
      }
      memcpy(statsp->userid, user, sizeof(user));

      // update ISPF statistics number of lines
      statsp->modified_number_of_lines = ioc->modified_number_of_lines; // update ISPF statistics number of lines
      statsp->current_number_of_lines = ioc->current_number_of_lines;   // update ISPF statistics number of lines

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
      memcpy(&statsp->created_date_century, &datel, sizeof(datel));
      memcpy(&statsp->modified_date_century, &datel, sizeof(datel));
      statsp->modified_time_hours = timel.times.HH;   // update ISPF statistics time hours
      statsp->modified_time_minutes = timel.times.MM; // update ISPF statistics time minutes
      statsp->modified_time_seconds = timel.times.SS; // update ISPF statistics time seconds
      statsp->initial_number_of_lines = 0;            // TODO
      statsp->modified_number_of_lines = 0;           // TODO
      statsp->current_number_of_lines = 0;            // TODO
    }
    else
    {
      memcpy(ioc->stow_list.name, bldl_pl.list.name, sizeof(bldl_pl.list.name)); // copy member name
      // memcpy(resources.sysprint->stow_list.ttr, note_response.ttr, sizeof(note_response.ttr));  // NOTE(Kelosky): the TTR will be maintained via OPEN/WRITE, no need to copy NOTE TTR
      ioc->stow_list.c = bldl_pl.list.c;                                       // copy user data length
      int user_data_len = (bldl_pl.list.c & LEN_MASK) * 2;                     // isolate number of halfwords in user data
      memcpy(ioc->stow_list.user_data, bldl_pl.list.user_data, user_data_len); // copy all user data

      zwto_debug("@TEST stowlist.c %d", ioc->stow_list.c);

      /**
       * @brief Update ISPF statistics
       */
      ISPF_STATS *statsp = (ISPF_STATS *)ioc->stow_list.user_data;
      // zut_dump_storage_common("ISPFSTATS", statsp, sizeof(ISPF_STATS), 16, 0, zut_print_debug);

      // update ISPF statistics userid
      char user[8] = {0};
      rc = zutm1gur(user);
      if (0 != rc)
      {
        zwto_debug("@TEST zutm1gur failed: rc: %d", rc);
        return RTNCD_FAILURE;
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
      statsp->modified_number_of_lines = ioc->modified_number_of_lines; // update ISPF statistics number of lines
      statsp->current_number_of_lines = ioc->current_number_of_lines;   // update ISPF statistics number of lines

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
    }
  }
}

static int stow_data_set(ZDIAG *PTR32 diag, IO_CTRL *PTR32 ioc)
{
  int rc = 0;
  int rsn = 0;
  if (ioc->dcb.dcboflgs & dcbofopn)
  {
    zwto_debug("@TEST stow data set: %44.44s", ioc->jfcb.jfcbdsnm);
    rc = stow(ioc, &rsn);
    if (0 != rc)
    {
      diag->service_rc = rc;
      strcpy(diag->service_name, "STOW");
      diag->e_msg_len = sprintf(diag->e_msg, "Failed to STOW ISPF statistics: %8.8s data set: %44.44s rsn was: %d", ioc->dcb.dcbddnam, ioc->jfcb.jfcbdsnm, rsn);
      diag->detail_rc = ZDS_RTNCD_STOW_ERROR;
    }
  }
  return rc;
}

static int close_data_set(ZDIAG *PTR32 diag, IO_CTRL *PTR32 ioc)
{
  int rc = 0;
  if (ioc->dcb.dcboflgs & dcbofopn)
  {
    zwto_debug("@TEST closing data set: %44.44s", ioc->jfcb.jfcbdsnm);
    rc = close_dcb(&ioc->dcb);
    if (0 != rc)
    {
      diag->service_rc = rc;
      strcpy(diag->service_name, "CLOSE");
      diag->e_msg_len = sprintf(diag->e_msg, "Failed to close ddname: %8.8s data set: %44.44s rc was: %d", ioc->dcb.dcbddnam, ioc->jfcb.jfcbdsnm, rc);
      diag->detail_rc = ZDS_RTNCD_CLOSE_ERROR;
    }
  }
  return rc;
}

static int deq_reserve_data_set(ZDIAG *PTR32 diag, IO_CTRL *PTR32 ioc)
{
  int rc = 0;
  if (ioc->reserve)
  {
    zwto_debug("@TEST DEQ RESERVE data set: %44.44s", ioc->jfcb.jfcbdsnm);
    QNAME qname_reserve = {0};
    RNAME rname_reserve = {0};
    strcpy(qname_reserve.value, "SPFEDIT");
    rname_reserve.rlen = sprintf(rname_reserve.value, "%.*s", sizeof(ioc->jfcb.jfcbdsnm), ioc->jfcb.jfcbdsnm);
    rc = deq_reserve(&qname_reserve, &rname_reserve, (UCB * PTR32) ioc->ucb);
    if (0 != rc)
    {
      diag->service_rc = rc;
      strcpy(diag->service_name, "DEQ RESERVE");
      diag->e_msg_len = sprintf(diag->e_msg, "Failed to DEQ RESERVE ddname: %8.8s data set: %44.44s rc was: %d", ioc->dcb.dcbddnam, ioc->jfcb.jfcbdsnm, rc);
      diag->detail_rc = ZDS_RTNCD_DEQ_RESERVE_ERROR;
    }
    return 0;
  }
  return rc;
}

static int deq_data_set(ZDIAG *PTR32 diag, IO_CTRL *PTR32 ioc)
{
  int rc = 0;

  if (ioc->enq)
  {
    zwto_debug("@TEST DEQ data set: %44.44s", ioc->jfcb.jfcbdsnm);
    RNAME rname = {0};
    QNAME qname = {0};
    strcpy(qname.value, "SPFEDIT");
    rname.rlen = sprintf(rname.value, "%.*s%.*s", sizeof(ioc->jfcb.jfcbdsnm), ioc->jfcb.jfcbdsnm, sizeof(ioc->jfcb.jfcbelnm), ioc->jfcb.jfcbelnm);
    rc = deq(&qname, &rname);
    if (0 != rc)
    {
      diag->service_rc = rc;
      strcpy(diag->service_name, "DEQ");
      diag->e_msg_len = sprintf(diag->e_msg, "Failed to DEQ ddname: %8.8s data set: %44.44s rc was: %d", ioc->dcb.dcbddnam, ioc->jfcb.jfcbdsnm, rc);
      diag->detail_rc = ZDS_RTNCD_DEQ_ERROR;
    }
  }
  return rc;
}

// TODO(Kelosky): handle when each fails... continue or return?
int close_output_bpam(ZDIAG *PTR32 diag, IO_CTRL *PTR32 ioc)
{
  int rc = 0;
  zwto_debug("@TEST close_output_bpam: %p", ioc);

  rc = update_ispf_statistics(diag, ioc);
  if (0 != rc)
  {
    return rc;
  }

  //
  // STOW the ISPF statistics
  //
  rc = stow_data_set(diag, ioc);
  if (0 != rc)
  {
    return rc;
  }

  //
  // Close the data set
  //
  rc = close_data_set(diag, ioc);
  if (0 != rc)
  {
    return rc;
  }

  if (ioc->buffer)
  {
    zwto_debug("@TEST releasing buffer for data set: %44.44s", ioc->jfcb.jfcbdsnm);
    storage_release(ioc->buffer_size, ioc->buffer);
    ioc->buffer = NULL;
    ioc->buffer_size = 0;
  }

  rc = deq_reserve_data_set(diag, ioc);
  if (0 != rc)
  {
    return rc;
  }

  rc = deq_data_set(diag, ioc);
  if (0 != rc)
  {
    return rc;
  }

  return rc;
}

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

  /////////////////////////////////////////////////////////////
  // rc = read_input_jfcb(ioc);
  // if (0 != rc)
  // {
  //   zwto_debug("@TEST read_input_jfcb failed: %d", rc);
  // }

  // zwto_debug("@TEST read input jfcb");
  // dcb->dcbdsrg1 = dcbdsgpo; // DSORG=PO @TEST
  /////////////////////////////////////////////////////////////

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

  // if (ioc->zam24)
  // {
  //   zwto_debug("@TEST close_assert: releasing zam24: %p", ioc->zam24);
  //   storage_release(ioc->zam24_len, ioc->zam24);
  //   ioc->zam24 = NULL;
  // }

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

// TODO(Kelosky): common logic for both read_input_jfcb and read_output_jfcb should be combined
int read_input_jfcb(IO_CTRL *ioc)
{
  int rc = 0;

  // int zam24_len = ZAM24Q();
  // zwto_debug("@TEST zam24_len: %d", zam24_len);
  // ioc->zam24_len = zam24_len;
  // ioc->zam24 = storage_obtain24(zam24_len);
  // memcpy(ioc->zam24, (void *PTR32)ZAM24, zam24_len);

  // ioc->exlst[0].exlentrb = (unsigned int)ioc->zam24; uncommend to enable DCBABEND
  // ioc->exlst[0].exlcodes = exldcbab;
  ioc->exlst[0].exlentrb = (unsigned int)&ioc->jfcb;
  ioc->exlst[0].exlcodes = exllaste + exlrjfcb;
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

  // int zam24_len = ZAM24Q();
  // zwto_debug("@TEST zam24_len: %d", zam24_len);
  // ioc->zam24_len = zam24_len;
  // ioc->zam24 = storage_obtain24(zam24_len);
  // memcpy(ioc->zam24, (void *PTR32)ZAM24, zam24_len);

  // ioc->exlst[0].exlentrb = (unsigned int)ioc->zam24; uncommend to enable DCBABEND
  // ioc->exlst[0].exlcodes = exldcbab;
  ioc->exlst[0].exlentrb = (unsigned int)&ioc->jfcb;
  ioc->exlst[0].exlcodes = exllaste + exlrjfcb;
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

#pragma prolog(ZAMDA31, " ZWEPROLG NEWDSA=(YES,8),SAVE=BAKR ") // TODO(Kelosky): BSM=NO?
#pragma epilog(ZAMDA31, " ZWEEPILG ")
int ZAMDA31()
{
  zwto_debug("@TEST ZAMDA31: called");
  return 0;
}