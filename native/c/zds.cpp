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

#ifndef _OPEN_SYS_ITOA_EXT
#define _OPEN_SYS_ITOA_EXT
#endif
#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <algorithm>
#include "zds.hpp"
#include "zdyn.h"
#include "zdstype.h"
#include "zut.hpp"
#include "iefzb4d2.h"
#include "zdsm.h"
#include <fcntl.h>
#include <stdlib.h>
#include "zbase64.h"
#include "zdsm.h"

const size_t MAX_DS_LENGTH = 44u;

using namespace std;

// https://www.ibm.com/docs/en/zos/2.5.0?topic=functions-fldata-retrieve-file-information#fldata__fldat
string zds_get_recfm(const fldata_t &file_info)
{
  string recfm = ZDS_RECFM_U;

  if (file_info.__recfmF)
  {
    recfm = ZDS_RECFM_F;
    if (file_info.__recfmBlk || file_info.__recfmB)
      recfm = file_info.__recfmS ? ZDS_RECFM_FBS : ZDS_RECFM_FB;
    if (file_info.__recfmASA)
      recfm += "A";
    if (file_info.__recfmM)
      recfm += "M";
  }
  else if (file_info.__recfmV)
  {
    recfm = ZDS_RECFM_V;
    if (file_info.__recfmBlk || file_info.__recfmB)
      recfm = file_info.__recfmS ? ZDS_RECFM_VBS : ZDS_RECFM_VB;
    if (file_info.__recfmASA)
      recfm += "A";
    if (file_info.__recfmM)
      recfm += "M";
  }
  else if (file_info.__recfmU)
  {
    recfm = ZDS_RECFM_U;
  }

  return recfm;
}

bool zds_dataset_exists(const string &dsn)
{
  const auto member_idx = dsn.find('(');
  const string dsn_without_member = member_idx == string::npos ? dsn : dsn.substr(0, member_idx);
  FILE *fp = fopen(("//'" + dsn_without_member + "'").c_str(), "r");
  if (fp)
  {
    fclose(fp);
    return true;
  }
  return false;
}

int zds_read_from_dd(ZDS *zds, string ddname, string &response)
{
  ddname = "DD:" + ddname;

  ifstream in(ddname.c_str());
  if (!in.is_open())
  {
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Could not open file '%s'", ddname.c_str());
    return RTNCD_FAILURE;
  }

  int index = 0;

  string line;
  while (getline(in, line))
  {
    if (index > 0 || line.size() > 0)
    {
      response += line;
      response.push_back('\n');
      index++;
    }
  }
  in.close();

  const size_t size = response.size() + 1;
  if (size > 0 && strlen(zds->encoding_opts.codepage) > 0)
  {
    string temp = response;
    const auto source_encoding = strlen(zds->encoding_opts.source_codepage) > 0 ? string(zds->encoding_opts.source_codepage) : "UTF-8";
    try
    {
      const auto bytes_with_encoding = zut_encode(temp, string(zds->encoding_opts.codepage), source_encoding, zds->diag);
      temp = bytes_with_encoding;
    }
    catch (exception &e)
    {
      zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Failed to convert input data from %s to %s", source_encoding.c_str(), zds->encoding_opts.codepage);
      return RTNCD_FAILURE;
    }
    if (!temp.empty())
    {
      response = temp;
    }
  }

  return 0;
}

int zds_read_from_dsn(ZDS *zds, const string &dsn, string &response)
{
  string dsname = "//'" + dsn + "'";
  if (strlen(zds->ddname) > 0)
  {
    dsname = "//DD:" + string(zds->ddname);
  }
  const string fopen_flags = zds->encoding_opts.data_type == eDataTypeBinary ? "rb" : "r";

  FILE *fp = fopen(dsname.c_str(), fopen_flags.c_str());
  if (!fp)
  {
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Could not open dsn '%s'", dsn.c_str());
    return RTNCD_FAILURE;
  }

  size_t bytes_read = 0;
  size_t total_size = 0;
  char buffer[4096] = {0};
  while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0)
  {
    total_size += bytes_read;
    response.append(buffer, bytes_read);
  }
  fclose(fp);

  const auto encodingProvided = zds->encoding_opts.data_type == eDataTypeText && strlen(zds->encoding_opts.codepage) > 0;

  if (total_size > 0 && encodingProvided)
  {
    string temp = response;
    const auto source_encoding = strlen(zds->encoding_opts.source_codepage) > 0 ? string(zds->encoding_opts.source_codepage) : "UTF-8";
    try
    {
      const auto bytes_with_encoding = zut_encode(temp, string(zds->encoding_opts.codepage), source_encoding, zds->diag);
      temp = bytes_with_encoding;
    }
    catch (exception &e)
    {
      zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Failed to convert input data from %s to %s", source_encoding.c_str(), zds->encoding_opts.codepage);
      return RTNCD_FAILURE;
    }
    if (!temp.empty())
    {
      response = temp;
    }
  }

  return 0;
}

int zds_write_to_dd(ZDS *zds, string ddname, const string &data)
{
  ddname = "DD:" + ddname;
  ofstream out(ddname.c_str());

  if (!out.is_open())
  {
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Could not open file '%s'", ddname.c_str());
    return RTNCD_FAILURE;
  }

  out << data;
  out.close();

  return 0;
}

int zds_write_to_dsn(ZDS *zds, const string &dsn, string &data)
{
  if (!zds_dataset_exists(dsn))
  {
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Could not access '%s'", dsn.c_str());
    return RTNCD_FAILURE;
  }

  const auto hasEncoding = zds->encoding_opts.data_type == eDataTypeText && strlen(zds->encoding_opts.codepage) > 0;
  const auto codepage = string(zds->encoding_opts.codepage);

  if (strlen(zds->etag) > 0)
  {
    ZDS read_ds = {0};
    string current_contents = "";
    if (hasEncoding)
    {
      memcpy(&read_ds.encoding_opts, &zds->encoding_opts, sizeof(ZEncode));
    }
    const auto read_rc = zds_read_from_dsn(&read_ds, dsn, current_contents);
    if (0 != read_rc)
    {
      zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Failed to read contents of data set for e-tag comparison: %s", read_ds.diag.e_msg);
      return RTNCD_FAILURE;
    }

    const auto given_etag = strtoul(zds->etag, nullptr, 16);
    const auto new_etag = zut_calc_adler32_checksum(current_contents);

    if (given_etag != new_etag)
    {
      ostringstream ss;
      ss << "Etag mismatch: expected ";
      ss << hex << given_etag << dec;
      ss << ", actual ";
      ss << hex << new_etag << dec;

      const auto error_msg = ss.str();
      zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "%s", error_msg.c_str());
      return RTNCD_FAILURE;
    }
  }

  string dsname = "//'" + dsn + "'";
  if (strlen(zds->ddname) > 0)
  {
    dsname = "//DD:" + string(zds->ddname);
  }
  const string fopen_extra_flags = zds->encoding_opts.data_type == eDataTypeBinary ? "b" : "" + string(",recfm=*");

  // If file already exists, open in read+write mode to avoid losing ISPF stats
  auto *fp = fopen(dsname.c_str(), ("r+" + fopen_extra_flags).c_str());
  if (nullptr == fp)
  {
    fp = fopen(dsname.c_str(), ("w" + fopen_extra_flags).c_str());
    if (nullptr == fp)
    {
      zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Could not open dsn '%s'", dsn.c_str());
      return RTNCD_FAILURE;
    }
  }

  string temp = data;
  if (!data.empty())
  {
    if (hasEncoding)
    {
      const auto source_encoding = strlen(zds->encoding_opts.source_codepage) > 0 ? string(zds->encoding_opts.source_codepage) : "UTF-8";
      try
      {
        temp = zut_encode(temp, source_encoding, codepage, zds->diag);
      }
      catch (exception &e)
      {
        fclose(fp);
        zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Failed to convert input data from %s to %s", source_encoding.c_str(), codepage.c_str());
        return RTNCD_FAILURE;
      }
    }
    if (!temp.empty())
    {
      fwrite(temp.c_str(), 1u, temp.length(), fp);
    }
  }

  fclose(fp);

  // Print new e-tag to stdout as response
  string saved_contents = "";
  const auto read_rc = zds_read_from_dsn(zds, dsn, saved_contents);
  if (0 != read_rc)
  {
    return RTNCD_FAILURE;
  }

  stringstream etag_stream;
  etag_stream << hex << zut_calc_adler32_checksum(saved_contents);
  strcpy(zds->etag, etag_stream.str().c_str());

  return 0;
}

// https://www.ibm.com/docs/en/zos/3.1.0?topic=examples-listing-partitioned-data-set-members
#define RECLEN 254

typedef struct
{
  unsigned short int count;
  char rest[RECLEN];
} RECORD;

typedef struct
{
  char name[8];
  unsigned char ttr[3];
  unsigned char info;
} RECORD_ENTRY;

int alloc_and_free(const string &alloc_dd, const string &dsn, unsigned int *code, string &resp)
{
  int rc = zut_bpxwdyn(alloc_dd, code, resp);
  if (RTNCD_SUCCESS == rc)
  {
    rc = zut_bpxwdyn("FREE DA('" + dsn + "')", code, resp);
  }
  return rc;
}

// TODO(Kelosky): add attributues to ZDS and have other functions populate it
int zds_create_dsn(ZDS *zds, std::string dsn, DS_ATTRIBUTES attributes, std::string &response)
{
  int rc = 0;
  unsigned int code = 0;
  string parm = "ALLOC DA('" + dsn + "')";
  transform(attributes.alcunit.begin(), attributes.alcunit.end(), attributes.alcunit.begin(), ::toupper);
  if (attributes.alcunit.empty() || attributes.alcunit == "TRACKS" || attributes.alcunit == "TRK")
  {
    attributes.alcunit = "TRACKS"; // Allocation Unit
  }
  else if (attributes.alcunit == "CYLINDERS" || attributes.alcunit == "CYL")
  {
    attributes.alcunit = "CYL"; // Allocation Unit
  }
  else
  {
    response = "Invalid allocation unit '" + attributes.alcunit + "'";
    return RTNCD_FAILURE;
  }
  if (attributes.blksize == 0)
  {
    attributes.blksize = 80; // Block Size
  }
  if (attributes.primary == 0)
  {
    attributes.primary = 1; // Primary Space
  }
  if (attributes.lrecl == 0)
  {
    attributes.lrecl = 80; // Record Length
  }

  char numberAsString[6];

  // Required options
  if (!attributes.dsorg.empty())
    parm += " DSORG(" + attributes.dsorg + ")";

  if (attributes.primary > 0)
  {
    memset(numberAsString, 0, sizeof(numberAsString));
    parm += " SPACE(" + std::string(itoa(attributes.primary, numberAsString, 10));

    if (attributes.secondary > 0)
    {
      memset(numberAsString, 0, sizeof(numberAsString));
      parm += "," + std::string(itoa(attributes.secondary, numberAsString, 10));
    }

    parm += ") " + attributes.alcunit;
  }

  if (attributes.lrecl > 0)
  {
    memset(numberAsString, 0, sizeof(numberAsString));
    parm += " LRECL(" + std::string(itoa(attributes.lrecl, numberAsString, 10)) + ")";
  }

  if (!attributes.recfm.empty())
    parm += " RECFM(" + attributes.recfm + ")";

  if (attributes.dirblk > 0)
  {
    memset(numberAsString, 0, sizeof(numberAsString));
    parm += " DIR(" + std::string(itoa(attributes.dirblk, numberAsString, 10)) + ")";
  }

  parm += " NEW KEEP";

  if (!attributes.dsntype.empty())
    parm += " DSNTYPE(" + attributes.dsntype + ")";
  if (!attributes.storclass.empty())
    parm += " STORCLAS(" + attributes.storclass + ")";
  if (!attributes.dataclass.empty())
    parm += " DATACLAS(" + attributes.dataclass + ")";
  if (!attributes.mgntclass.empty())
    parm += " MGMTCLAS(" + attributes.mgntclass + ")";
  if (!attributes.vol.empty())
    parm += " VOL(" + attributes.vol + ")";
  if (!attributes.unit.empty())
    parm += " UNIT(" + attributes.unit + ")";

  if (attributes.blksize > 0)
  {
    memset(numberAsString, 0, sizeof(numberAsString));
    parm += " BLKSIZE(" + std::string(itoa(attributes.blksize, numberAsString, 10)) + ")";
  }

  return alloc_and_free(parm, dsn, &code, response);
}

int zds_create_dsn_fb(ZDS *zds, const string &dsn, string &response)
{
  int rc = 0;
  unsigned int code = 0;
  string parm = "ALLOC DA('" + dsn + "') DSORG(PO) SPACE(5,5) CYL LRECL(80) RECFM(F,B) DIR(5) NEW KEEP DSNTYPE(LIBRARY)";

  return alloc_and_free(parm, dsn, &code, response);
}

int zds_create_dsn_vb(ZDS *zds, const string &dsn, string &response)
{
  int rc = 0;
  unsigned int code = 0;
  string parm = "ALLOC DA('" + dsn + "') DSORG(PO) SPACE(5,5) CYL LRECL(255) RECFM(V,B) DIR(5) NEW KEEP DSNTYPE(LIBRARY)";

  return alloc_and_free(parm, dsn, &code, response);
}

int zds_create_dsn_adata(ZDS *zds, const string &dsn, string &response)
{
  int rc = 0;
  unsigned int code = 0;
  string parm = "ALLOC DA('" + dsn + "') DSORG(PO) SPACE(5,5) CYL LRECL(32756) BLKSIZE(32760) RECFM(V,B) DIR(5) NEW KEEP DSNTYPE(LIBRARY)";

  return alloc_and_free(parm, dsn, &code, response);
}

int zds_create_dsn_loadlib(ZDS *zds, const string &dsn, string &response)
{
  int rc = 0;
  unsigned int code = 0;
  string parm = "ALLOC DA('" + dsn + "') DSORG(PO) SPACE(5,5) CYL LRECL(0) BLKSIZE(32760) RECFM(U) DIR(5) NEW KEEP DSNTYPE(LIBRARY)";

  return alloc_and_free(parm, dsn, &code, response);
}

#define NUM_DELETE_TEXT_UNITS 2
int zds_delete_dsn(ZDS *zds, string dsn)
{
  int rc = 0;

  dsn = "//'" + dsn + "'";

  rc = remove(dsn.c_str());

  if (0 != rc)
  {
    strcpy(zds->diag.service_name, "remove");
    zds->diag.service_rc = rc;
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Could not delete data set '%s', rc: '%d'", dsn.c_str(), rc);
    zds->diag.detail_rc = ZDS_RTNCD_SERVICE_FAILURE;
    return RTNCD_FAILURE;
  }

  return 0;
}

int zds_list_members(ZDS *zds, string dsn, vector<ZDSMem> &members)
{
  // PO
  // PO-E (PDS)
  dsn = "//'" + dsn + "'";

  int total_entries = 0;

  if (0 == zds->max_entries)
    zds->max_entries = ZDS_DEFAULT_MAX_MEMBER_ENTRIES;

  members.reserve(zds->max_entries);

  RECORD rec = {0};
  // https://www.ibm.com/docs/en/zos/3.1.0?topic=pds-reading-directory-sequentially
  // https://www.ibm.com/docs/en/zos/3.1.0?topic=pdse-reading-directory - long alias names omitted, use DESERV for those
  // https://www.ibm.com/docs/en/zos/3.1.0?topic=pds-directory
  FILE *fp = fopen(dsn.c_str(), "rb,recfm=u");

  const int bufsize = 256;
  char buffer[bufsize] = {0};

  if (!fp)
  {
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Could not open dsn '%s'", dsn.c_str());
    return RTNCD_FAILURE;
  }

  while (fread(&rec, sizeof(rec), 1, fp))
  {
    unsigned char *data = nullptr;
    data = (unsigned char *)&rec;
    data += sizeof(rec.count); // increment past halfword length

    int len = sizeof(RECORD_ENTRY);
    for (int i = 0; i < rec.count; i = i + len)
    {
      RECORD_ENTRY entry = {0};
      memcpy(&entry, data, sizeof(entry));
      long long int end = 0xFFFFFFFFFFFFFFFF; // indicates end of entries
      if (memcmp(entry.name, &end, sizeof(end)) == 0)
      {
        break;
      }
      else
      {
        total_entries++;

        if (total_entries > zds->max_entries)
        {
          zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Reached maximum returned members requested %d", zds->max_entries);
          zds->diag.detail_rc = ZDS_RSNCD_MAXED_ENTRIES_REACHED;
          fclose(fp);
          return RTNCD_WARNING;
        }

        unsigned char info = entry.info;
        unsigned char pointer_count = entry.info;
        char name[9] = {0};
        if (info & 0x80) // bit 0 indicates alias
        {
          // TODO(Kelosky): // member name is an alias
        }
        pointer_count &= 0x60; // bits 1-2 contain number of user data TTRNs
        pointer_count >>= 5;   // adjust to byte boundary
        info &= 0x1F;          // bits 3-7 contain the number of half words of user data

        memcpy(name, entry.name, sizeof(entry.name));

        for (int j = 8; j >= 0; j--)
        {
          if (name[j] == ' ')
          {
            name[j] = 0x00;
          }
        }

        ZDSMem mem = {0};
        mem.name = string(name);
        members.push_back(mem);

        data = data + sizeof(entry) + (info * 2); // skip number of half words
        len = sizeof(entry) + (info * 2);

        int remainder = rec.count - (i + len);
        if (remainder < sizeof(entry))
          break;
      }
    }
  }

  fclose(fp);
  return 0;
}

#pragma pack(1)

// https://www.ibm.com/docs/en/zos/3.1.0?topic=format-work-area-table
// https://www.ibm.com/docs/en/zos/3.1.0?topic=format-work-area-picture
typedef struct
{
  int total_size;
  int min_size;
  int used_size;
  short number_fields;
} ZDS_CSI_HEADER;

typedef struct
{
  char modid[2];
  unsigned char rsn;
  unsigned char rc;
} ZDS_CSI_ERROR_INFO;

typedef struct
{
  unsigned char flag;
#define NOT_SUPPORTED 0x80         // CSINTICF
#define NO_ENTRY 0x40              // CSINOENT
#define DATA_NOT_COMPLETE 0x20     // CSINTCMP
#define PROCESS_ERROR 0x10         // CSICERR
#define PARTIAL_PROCESS_ERROR 0x08 // CSICERRP
#define ERROR_CONDITION 0XF8       // all flags
  unsigned char type;
#define CATALOG_TYPE 0xF0
  char name[MAX_DS_LENGTH];
  ZDS_CSI_ERROR_INFO error_info;
} ZDS_CSI_CATALOG;

typedef struct
{
  int total_len;
  unsigned int reserved;
  int field_lens; // data after field_lens
} ZDS_CSI_FIELD;

typedef struct
{
  unsigned char flag;
#define PRIMARY_ENTRY 0x80 // CSIPMENT
#define ERROR 0x40         // CSIENTER
#define DATA 0x20          // CSIEDATA
  unsigned char type;
#define NOT_FOUND 0x00
#define NON_VSAM_DATA_SET 'A'
#define GENERATION_DATA_GROUP 'B'
#define CLUSTER 'C'
#define DATA_COMPONENT 'D'
#define ALTERNATE_INDEX 'G'
#define GENERATION_DATA_SET 'H'
#define INDEX_COMPONENT 'I'
#define ATL_LIBRARY_ENTRY 'L'
#define PATH 'R'
#define USER_CATALOG_CONNECTOR_ENTRY 'U'
#define ATL_VOLUME_ENTRY 'W'
#define ALIAS 'X'
  char name[MAX_DS_LENGTH];

  union
  {
    ZDS_CSI_ERROR_INFO error_info; // if CSIENTER=1
    ZDS_CSI_FIELD field;
  } response;
} ZDS_CSI_ENTRY;

typedef struct
{
  ZDS_CSI_HEADER header;
  ZDS_CSI_CATALOG catalog;
  ZDS_CSI_ENTRY entry;
} ZDS_CSI_WORK_AREA;

#pragma pack() // restore default packing

#define BUFF_SIZE 1024
#define FIELD_LEN 8

#define DS1DSGPS_MASK 0x4000 // PS: Bit 2 is set
#define DS1DSGDA_MASK 0x2000 // DA: Bit 3 is set
#define DS1DSGPO_MASK 0x0200 // PO: Bit 7 is set
#define DS1DSGU_MASK 0x0100  // Unmovable: Bit 8 is set
#define DS1ACBM_MASK 0x0008  // VSAM: Bit 13 is set
#define DS1PDSE_MASK 0x10    // PDSE: Bit 4 in ds1smsfg
#define DS1ENCRP_MASK 0x04   // Encrypted: Bit 5 in ds1flag1

void load_dsorg_from_dscb(const DSCBFormat1 *dscb, string *dsorg)
{
  // Bitmasks translated from binary to hex from "DFSMSdfp advanced services" PDF, Chapter 1 page 7 (PDF page 39)
  if (dscb->ds1dsorg & DS1DSGPS_MASK)
  {
    *dsorg = ZDS_DSORG_PS;
  }
  else if (dscb->ds1dsorg & DS1DSGDA_MASK)
  {
    *dsorg = ZDS_DSORG_DA;
  }
  else if (dscb->ds1dsorg & DS1DSGPO_MASK)
  {
    *dsorg = ZDS_DSORG_PO;
  }
  else if (dscb->ds1dsorg & DS1ACBM_MASK)
  {
    *dsorg = ZDS_DSORG_VSAM;
  }

  // Unmovable: Last bit of first half is set
  if (dscb->ds1dsorg & DS1DSGU_MASK)
  {
    *dsorg += 'U';
  }

  if (dsorg->empty())
  {
    *dsorg = ZDS_DSORG_UNKNOWN;
  }
}

void load_recfm_from_dscb(const DSCBFormat1 *dscb, string *recfm)
{
  // Bitmasks translated from binary to hex from "DFSMSdfp advanced services" PDF, Chapter 1 page 7 (PDF page 39)
  // Fixed: First bit is set
  if ((dscb->ds1recfm & 0xC0) == 0x80)
  {
    *recfm = ZDS_RECFM_F;
  }
  // Variable: Second bit is set
  else if ((dscb->ds1recfm & 0xC0) == 0x40)
  {
    *recfm = ZDS_RECFM_V;
  }
  // Undefined: First and second bits are set
  else if ((dscb->ds1recfm & 0xC0) == 0xC0)
  {
    *recfm = ZDS_RECFM_U;
  }

  // Blocked records: Fourth bit is set
  if ((dscb->ds1recfm & 0x10) > 0)
  {
    *recfm += 'B';
  }

  // Sequential: Fifth bit is set
  if ((dscb->ds1recfm & 0x08) > 0 && recfm[0] != ZDS_RECFM_U)
  {
    *recfm += 'S';
  }

  // ANSI control characters/ASA: Sixth bit is set
  if ((dscb->ds1recfm & 0x04) > 0)
  {
    *recfm += 'A';
  }

  // Machine-control characters: Seventh bit is set
  if ((dscb->ds1recfm & 0x02) > 0)
  {
    *recfm += 'M';
  }

  if (recfm->empty())
  {
    *recfm = ZDS_RECFM_U;
  }
}

void load_date_from_dscb(const char *date_in, string *date_out, bool is_expiration_date = false)
{
  // Date is in 'YDD' format (3 bytes): Year offset and day of year
  // If all zeros, date is not maintained
  unsigned char year_offset = static_cast<unsigned char>(date_in[0]);
  unsigned char day_high = static_cast<unsigned char>(date_in[1]);
  unsigned char day_low = static_cast<unsigned char>(date_in[2]);

  // Check if date is zero (not maintained)
  if (year_offset == 0 && day_high == 0 && day_low == 0)
  {
    *date_out = "";
    return;
  }

  // Parse year: add 1900 to the year offset
  int year = 1900 + year_offset;

  // Parse day of year from 2-byte value (big-endian)
  int day_of_year = (day_high << 8) | day_low;

  // Check for sentinel date 1999/12/31 (year_offset=99, day=365)
  // This is used to indicate "no expiration" for expiration dates only
  if (is_expiration_date && year == 1999 && day_of_year == 365)
  {
    *date_out = "";
    return;
  }

  // Convert day of year to month/day
  static const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  // Check for leap year
  bool is_leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);

  int month = 1;
  int day = day_of_year;

  for (int i = 0; i < 12 && day > days_in_month[i]; i++)
  {
    int days_this_month = days_in_month[i];
    if (i == 1 && is_leap) // February in leap year
      days_this_month = 29;

    day -= days_this_month;
    month++;
  }

  // Format as "YYYY/MM/DD"
  char buffer[11];
  sprintf(buffer, "%04d/%02d/%02d", year, month, day);
  *date_out = buffer;
}

void load_extents_from_dscb(const DSCBFormat1 *dscb, ZDSEntry &entry, uint32_t pds_max_track = 0)
{
  // allocx: Number of extents allocated on this volume
  entry.allocx = dscb->ds1noepv;

  // Parse DS1SCAL1 (ds1scalo[0]) for space allocation type
  uint8_t scal1 = static_cast<uint8_t>(dscb->ds1scalo[0]);
  uint8_t dspac = (scal1 & 0xC0) >> 6; // Top 2 bits: space request type

  // Parse DS1SCAL3 (ds1scalo[1-3]) - secondary allocation quantity (3 bytes)
  uint32_t scal3 = (static_cast<unsigned char>(dscb->ds1scalo[1]) << 16) |
                   (static_cast<unsigned char>(dscb->ds1scalo[2]) << 8) |
                   static_cast<unsigned char>(dscb->ds1scalo[3]);

  // Parse the extent information from ds1exnts (3 extents, 10 bytes each)
  // Each extent has: type(1), seq(1), lower_limit(4), upper_limit(4)
  const char *extent_data = dscb->ds1exnts;
  int total_cylinders = 0;
  int total_tracks = 0;
  bool use_cylinders = (dspac == 0x03); // true if allocated in cylinders

  for (int i = 0; i < 3; i++)
  {
    const char *extent = extent_data + (i * 10);
    uint8_t extent_type = static_cast<uint8_t>(extent[0]);

    // Check if this is a valid extent (not X'00')
    if (extent_type == 0x00)
      break;

    // Parse lower limit CCHH (4 bytes)
    // CCHH format: CC (bytes 2-3) = low 16 bits of cylinder
    //              HH (bytes 4-5) = high 12 bits of cylinder (bits 0-11) + 4-bit head (bits 12-15)
    uint16_t lower_cyl_low = (static_cast<unsigned char>(extent[2]) << 8) |
                             static_cast<unsigned char>(extent[3]);
    uint16_t lower_cyl_high = ((static_cast<unsigned char>(extent[4]) << 4) |
                               (static_cast<unsigned char>(extent[5]) >> 4)) &
                              0x0FFF;
    uint16_t lower_head = static_cast<unsigned char>(extent[5]) & 0x0F;

    // Parse upper limit CCHH (4 bytes)
    uint16_t upper_cyl_low = (static_cast<unsigned char>(extent[6]) << 8) |
                             static_cast<unsigned char>(extent[7]);
    uint16_t upper_cyl_high = ((static_cast<unsigned char>(extent[8]) << 4) |
                               (static_cast<unsigned char>(extent[9]) >> 4)) &
                              0x0FFF;
    uint16_t upper_head = static_cast<unsigned char>(extent[9]) & 0x0F;

    // Calculate full cylinder numbers (28-bit)
    uint32_t lower_cyl = (lower_cyl_high << 16) | lower_cyl_low;
    uint32_t upper_cyl = (upper_cyl_high << 16) | upper_cyl_low;

    // Calculate cylinders and tracks in this extent
    if (upper_cyl >= lower_cyl)
    {
      int cylinders_in_extent = upper_cyl - lower_cyl + 1;
      int tracks_in_extent = ((upper_cyl - lower_cyl) * 15) + (upper_head - lower_head) + 1;
      total_cylinders += cylinders_in_extent;
      total_tracks += tracks_in_extent;
    }
  }

  // alloc: Total allocated space in native units (cylinders, tracks, or blocks)
  if (dspac == 0x01) // Blocks (average block length)
  {
    // For block-allocated datasets, calculate blocks based on track capacity
    // This is an approximation: tracks * (track_capacity / average_blksize)
    // We'll report tracks for now as block calculation requires blksize from DSCB
    entry.alloc = total_tracks;
  }
  else
  {
    entry.alloc = use_cylinders ? total_cylinders : total_tracks;
  }

  // primary: Primary extent size in allocation units (cylinders or tracks)
  // The first extent is the primary allocation
  if (entry.allocx > 0)
  {
    const char *first_extent = extent_data;
    uint16_t lower_cyl_low = (static_cast<unsigned char>(first_extent[2]) << 8) |
                             static_cast<unsigned char>(first_extent[3]);
    uint16_t lower_cyl_high = ((static_cast<unsigned char>(first_extent[4]) << 4) |
                               (static_cast<unsigned char>(first_extent[5]) >> 4)) &
                              0x0FFF;
    uint16_t upper_cyl_low = (static_cast<unsigned char>(first_extent[6]) << 8) |
                             static_cast<unsigned char>(first_extent[7]);
    uint16_t upper_cyl_high = ((static_cast<unsigned char>(first_extent[8]) << 4) |
                               (static_cast<unsigned char>(first_extent[9]) >> 4)) &
                              0x0FFF;

    uint32_t lower_cyl = (lower_cyl_high << 16) | lower_cyl_low;
    uint32_t upper_cyl = (upper_cyl_high << 16) | upper_cyl_low;

    if (dspac == 0x03) // Cylinders
    {
      entry.primary = (upper_cyl - lower_cyl + 1);
    }
    else if (dspac == 0x02) // Tracks - report in tracks
    {
      uint16_t lower_head = static_cast<unsigned char>(first_extent[5]) & 0x0F;
      uint16_t upper_head = static_cast<unsigned char>(first_extent[9]) & 0x0F;
      int tracks = ((upper_cyl - lower_cyl) * 15) + (upper_head - lower_head) + 1;
      entry.primary = tracks;
    }
    else if (dspac == 0x01) // Blocks (average block length)
    {
      // For block allocation, primary is calculated from extent size in tracks
      // Convert to approximate block count
      uint16_t lower_head = static_cast<unsigned char>(first_extent[5]) & 0x0F;
      uint16_t upper_head = static_cast<unsigned char>(first_extent[9]) & 0x0F;
      int tracks = ((upper_cyl - lower_cyl) * 15) + (upper_head - lower_head) + 1;
      entry.primary = tracks; // Report in tracks for now
    }
    else // Unknown allocation type
    {
      entry.primary = 0;
    }
  }
  else
  {
    entry.primary = 0;
  }

  // spacu: Determine allocation unit and whether to convert to bytes
  // Note: ISPF and z/OSMF display track-allocated datasets with contiguous allocation as BYTES
  // We need to determine this BEFORE calculating secondary, as it affects which field we read from
  bool convert_to_bytes = false;

  if (dspac == 0x03) // 11 = DS1CYL - Cylinders
  {
    entry.spacu = "CYLINDERS";
  }
  else if (dspac == 0x02) // 10 = DS1TRK - Tracks
  {
    // Check bit 4 (0x10) - DS1CONTG (contiguous request)
    bool is_contiguous = (scal1 & 0x10) != 0;
    if (is_contiguous)
    {
      entry.spacu = "BYTES"; // Contiguous track allocation displayed as BYTES
      convert_to_bytes = true;
    }
    else
    {
      entry.spacu = "TRACKS"; // Non-contiguous track allocation displayed as TRACKS
    }
  }
  else if (dspac == 0x01) // 01 = DS1AVR - Average block length (blocks)
  {
    entry.spacu = "BLOCKS";
  }
  else if ((scal1 & 0x20) != 0) // Bit 2 set: DS1EXT - Extension to secondary space
  {
    // Fall back to ds1scext[0] (ds1scxtf) for extended units
    uint8_t scxtf = static_cast<uint8_t>(dscb->ds1scext[0]);

    if (scxtf & 0x40) // DS1SCMB - megabytes
    {
      entry.spacu = "MEGABYTES";
    }
    else if (scxtf & 0x20) // DS1SCKB - kilobytes
    {
      entry.spacu = "KILOBYTES";
    }
    else if (scxtf & 0x10) // DS1SCUB - bytes
    {
      entry.spacu = "BYTES";
      convert_to_bytes = true;
    }
    else
    {
      entry.spacu = "TRACKS"; // Default fallback
    }
  }
  else
  {
    // dspac == 0x00 (00 . 0) - Absolute track request (DS1DSABS)
    // ISPF displays this allocation in bytes to the user
    entry.spacu = "BYTES";
    convert_to_bytes = true;
  }

  // secondary: Secondary allocation quantity
  // For contiguous track allocations (convert_to_bytes=true), use DS1SCEXT instead of DS1SCAL3
  // DS1SCEXT contains the secondary allocation in bytes (possibly compacted)
  if (convert_to_bytes)
  {
    // Parse DS1SCEXT: ds1scext[0] = DS1SCXTF (flags), ds1scext[1-2] = DS1SCXTV (value)
    uint8_t scxtf = static_cast<uint8_t>(dscb->ds1scext[0]);
    uint16_t scxtv = (static_cast<unsigned char>(dscb->ds1scext[1]) << 8) |
                     static_cast<unsigned char>(dscb->ds1scext[2]);

    // Check if value is compacted
    if (scxtf & 0x08) // DS1SCCP1 - compacted by 256
    {
      entry.secondary = scxtv * 256;
    }
    else if (scxtf & 0x04) // DS1SCCP2 - compacted by 65,536
    {
      entry.secondary = scxtv * 65536;
    }
    else
    {
      entry.secondary = scxtv;
    }
  }
  else if (dspac == 0x03) // Cylinders
  {
    entry.secondary = scal3;
  }
  else if (dspac == 0x02) // Tracks - report in tracks
  {
    entry.secondary = scal3;
  }
  else if (dspac == 0x01) // Blocks (average block length)
  {
    // For block allocation, scal3 is in blocks
    // But since we're reporting space in tracks for consistency, convert or report as-is
    entry.secondary = scal3; // Report the block count from DSCB
  }
  else // Unknown allocation type
  {
    entry.secondary = 0;
  }

  // Note: Conversion to bytes for alloc/primary/used is handled in zds_get_attrs_from_dscb after blksize is known

  // used: Calculate used space in native units (cylinders or tracks)
  // usedx: Calculate number of used extents
  // DS1LSTAR contains the last used track (TTR format)
  // Not defined/meaningful for VSAM, PDSE, HFS, and DA (direct/BDAM)

  // Check if this is a PDSE
  bool is_pdse = (dscb->ds1smsfg & DS1PDSE_MASK) != 0;

  if ((dscb->ds1dsorg & DS1DSGDA_MASK) || (dscb->ds1dsorg & DS1ACBM_MASK) || is_pdse)
  {
    // DA (Direct Access/BDAM), VSAM, or PDSE: DS1LSTAR not meaningful
    entry.used = -1;
    entry.usedx = -1;
  }
  else
  {
    // DS1LSTAR is 3 bytes: TT (track) + R (record)
    // We only need the TT part (bytes 0-1)
    uint32_t last_used_track;
    if (pds_max_track > 0)
    {
      last_used_track = pds_max_track;
    }
    else
    {
      uint8_t lstar_tt_high = static_cast<unsigned char>(dscb->ds1lstar[0]);
      uint8_t lstar_tt_low = static_cast<unsigned char>(dscb->ds1lstar[1]);

      // Parse last used track number (TT from TTR)
      // TT is a 2-byte big-endian value representing the relative track number (0-based)
      last_used_track = (lstar_tt_high << 8) | lstar_tt_low;

      // Check if this is a large format dataset that uses DS1TTHI
      bool is_large = (dscb->ds1flag1 & 0x10) != 0; // DS1LARGE bit
      if (is_large)
      {
        // Include the high-order byte from DS1TTHI
        uint8_t lstar_tt_highest = static_cast<unsigned char>(dscb->ds1ttthi);
        last_used_track |= (lstar_tt_highest << 16);
      }
    }
    // Special case: DS1LSTAR = 0x000000 can mean either:
    // 1. Track 0, record 0 is used (dataset has been written to)
    // 2. Dataset has never been written to (unused)
    // For datasets with blksize=0, DS1LSTAR = 0x000000 means unused
    // This is common for checkpoint datasets and preallocated system datasets
    bool is_blksize_zero = (entry.blksize == 0);

    if (last_used_track == 0 && pds_max_track == 0 && is_blksize_zero)
    {
      // DS1LSTAR = 0x000000 for blksize=0 dataset = unused
      entry.used = 0;
      entry.usedx = 0;
    }
    else
    {
      // Report used space in native units
      if (use_cylinders)
      {
        // Convert tracks to cylinders (round up)
        entry.used = (last_used_track + 1 + 14) / 15;
      }
      else
      {
        // Report in tracks
        entry.used = last_used_track + 1;
      }
    }

    // Calculate how many extents contain data (count of used extents)
    // Skip this calculation if we already determined the dataset is unused
    if (entry.used > 0)
    {
      int cumulative_tracks = 0;
      entry.usedx = 0;

      for (int i = 0; i < 3; i++)
      {
        const char *extent = extent_data + (i * 10);
        uint8_t extent_type = static_cast<uint8_t>(extent[0]);

        if (extent_type == 0x00)
          break;

        // Parse extent to get track count
        uint16_t lower_cyl_low = (static_cast<unsigned char>(extent[2]) << 8) |
                                 static_cast<unsigned char>(extent[3]);
        uint16_t lower_cyl_high = ((static_cast<unsigned char>(extent[4]) << 4) |
                                   (static_cast<unsigned char>(extent[5]) >> 4)) &
                                  0x0FFF;
        uint16_t lower_head = static_cast<unsigned char>(extent[5]) & 0x0F;
        uint16_t upper_cyl_low = (static_cast<unsigned char>(extent[6]) << 8) |
                                 static_cast<unsigned char>(extent[7]);
        uint16_t upper_cyl_high = ((static_cast<unsigned char>(extent[8]) << 4) |
                                   (static_cast<unsigned char>(extent[9]) >> 4)) &
                                  0x0FFF;
        uint16_t upper_head = static_cast<unsigned char>(extent[9]) & 0x0F;

        uint32_t lower_cyl = (lower_cyl_high << 16) | lower_cyl_low;
        uint32_t upper_cyl = (upper_cyl_high << 16) | upper_cyl_low;

        if (upper_cyl >= lower_cyl)
        {
          int tracks_in_extent = ((upper_cyl - lower_cyl) * 15) + (upper_head - lower_head) + 1;
          cumulative_tracks += tracks_in_extent;

          // Count this extent as used if it contains data up to the last used track
          // last_used_track is 0-based, cumulative_tracks is the total tracks so far
          if (last_used_track < cumulative_tracks)
          {
            entry.usedx = i + 1; // This is the count of extents with data
            break;
          }
        }
      }
    } // End if (entry.used > 0)
  }
}

// Helper function to read PDS directory and get member count and directory block info
uint32_t load_pds_directory_info(const string &dsn, ZDSEntry &entry)
{
  string full_dsn = "//'" + dsn + "'";

  FILE *fp = fopen(full_dsn.c_str(), "rb,recfm=u");
  if (!fp)
  {
    // Can't open directory - leave values as-is
    return 0;
  }

  RECORD rec = {0};
  int total_members = 0;
  int directory_blocks_used = 0;
  int total_directory_blocks = 0;
  bool found_end_marker = false;
  uint32_t max_track = 0;

  // Read all directory blocks - continue even after end marker to count all allocated blocks
  while (fread(&rec, sizeof(rec), 1, fp))
  {
    total_directory_blocks++;

    // Check if this looks like a valid directory block
    // rec.count should be reasonable (between 0 and RECLEN)
    if (rec.count > RECLEN)
    {
      // This doesn't look like a directory block - probably hit member data
      total_directory_blocks--; // Don't count this invalid block
      break;
    }

    // Check if this block contains actual directory entries
    bool has_entries = false;

    unsigned char *data = (unsigned char *)&rec;
    data += sizeof(rec.count); // increment past halfword length

    int len = sizeof(RECORD_ENTRY);
    for (int i = 0; i < rec.count; i = i + len)
    {
      RECORD_ENTRY dir_entry = {0};
      memcpy(&dir_entry, data, sizeof(dir_entry));

      // Check for end of directory marker (8 bytes of 0xFF)
      long long int end = 0xFFFFFFFFFFFFFFFF;
      if (memcmp(dir_entry.name, &end, sizeof(end)) == 0)
      {
        // End of directory reached - this block is used up to this point
        directory_blocks_used = total_directory_blocks;
        found_end_marker = true;
        // Don't exit - continue to count remaining allocated but unused directory blocks
        break; // Break inner loop only
      }

      // Check for unused directory space (all zeros)
      long long int zero = 0;
      if (memcmp(dir_entry.name, &zero, sizeof(zero)) == 0)
      {
        // Empty entry - skip it but continue
        data = data + sizeof(dir_entry);
        len = sizeof(dir_entry);
        continue;
      }

      uint32_t current_track = (dir_entry.ttr[0] << 8) | dir_entry.ttr[1];
      if (current_track > max_track)
      {
        max_track = current_track;
      }

      total_members++;
      has_entries = true;

      unsigned char info = dir_entry.info;
      info &= 0x1F; // bits 3-7 contain the number of half words of user data

      data = data + sizeof(dir_entry) + (info * 2); // skip number of half words
      len = sizeof(dir_entry) + (info * 2);

      int remainder = rec.count - (i + len);
      if (remainder < sizeof(dir_entry))
        break;
    }

    if (has_entries && !found_end_marker)
    {
      directory_blocks_used = total_directory_blocks;
    }
  }

done:
  fclose(fp);

  // Update entry with directory information
  entry.members = total_members;
  entry.useddb = directory_blocks_used;

  // maxdb is the total allocated directory blocks
  // This includes used blocks plus any unused allocated blocks before data starts
  entry.maxdb = total_directory_blocks;
  return max_track;
}

void zds_get_attrs_from_dscb(ZDS *zds, ZDSEntry &entry)
{
  auto *dscb = (DSCBFormat1 *)__malloc31(sizeof(DSCBFormat1));
  if (dscb == nullptr)
  {
    return;
  }

  memset(dscb, 0x00, sizeof(DSCBFormat1));
  const auto rc = ZDSDSCB1(zds, entry.name.c_str(), entry.volser.c_str(), dscb);

  // DEBUG: Dump DSCB to file
  if (rc == RTNCD_SUCCESS)
  {
    // FILE *debug_fp = fopen("/tmp/znp-ds-attrs.txt", "a");
    // if (debug_fp)
    // {
    //   fprintf(debug_fp, "\n=== Dataset: %s ===\n", entry.name.c_str());
    //   // DSCB Format-1 is 140 bytes (0x8C), use fixed size instead of sizeof
    //   const size_t DSCB_SIZE = 140;
    //   fprintf(debug_fp, "DSCB Hex Dump (%zu bytes, sizeof=%zu):\n", DSCB_SIZE, sizeof(DSCBFormat1));

    //   unsigned char *dscb_bytes = (unsigned char *)dscb;
    //   for (size_t i = 0; i < DSCB_SIZE; i++)
    //   {
    //     if (i % 16 == 0)
    //       fprintf(debug_fp, "%04zX: ", i);
    //     fprintf(debug_fp, "%02X ", dscb_bytes[i]);
    //     if ((i + 1) % 16 == 0)
    //       fprintf(debug_fp, "\n");
    //   }
    //   if (DSCB_SIZE % 16 != 0)
    //     fprintf(debug_fp, "\n");

    //   fclose(debug_fp);
    // }
  }

  if (rc == RTNCD_SUCCESS)
  {
    load_dsorg_from_dscb(dscb, &entry.dsorg);

    // Check if this is a VSAM dataset
    bool is_vsam = (dscb->ds1dsorg & DS1ACBM_MASK) != 0;

    if (is_vsam)
    {
      // VSAM datasets: blksize, lrecl, and recfm are not meaningful
      entry.blksize = -1;
      entry.lrecl = -1;
      entry.recfm = "";
    }
    else
    {
      load_recfm_from_dscb(dscb, &entry.recfm);
      entry.blksize = (static_cast<unsigned char>(dscb->ds1blkl[0]) << 8) |
                      static_cast<unsigned char>(dscb->ds1blkl[1]);
      entry.lrecl = (static_cast<unsigned char>(dscb->ds1lrecl[0]) << 8) |
                    static_cast<unsigned char>(dscb->ds1lrecl[1]);
    }

    // Determine dsntype: PDS or LIBRARY (PDSE)
    // DS1DSGPO (0x0200) in ds1dsorg indicates partitioned organization
    bool is_partitioned = (dscb->ds1dsorg & DS1DSGPO_MASK) != 0;
    // DS1PDSE (0x10) in ds1smsfg indicates PDSE
    bool is_pdse = (dscb->ds1smsfg & DS1PDSE_MASK) != 0;
    uint32_t max_member_track = 0;

    if (is_partitioned && is_pdse)
    {
      entry.dsntype = "LIBRARY";
    }
    else if (is_partitioned)
    {
      entry.dsntype = "PDS";
    }
    else
    {
      entry.dsntype = "";
    }

    // Check if dataset is encrypted
    // DS1ENCRP (0x04) in ds1flag1 indicates access method encrypted dataset
    entry.encrypted = (dscb->ds1flag1 & DS1ENCRP_MASK) != 0;

    // Load directory block information for PDS/PDSE
    if (is_partitioned)
    {
      // Initialize with defaults
      entry.maxdb = -1;
      entry.useddb = -1;
      entry.members = -1;

      // For PDS (not PDSE), read the directory to get accurate counts
      // PDSE directories are more complex and would require DESERV
      if (!is_pdse)
      {
        // Read the PDS directory to get member count and directory block info
        max_member_track = load_pds_directory_info(entry.name, entry);
      }
      else
      {
        // For PDSE, we'd need to use DESERV macro which is more complex
        // For now, just set DS1NOBDB value
        entry.useddb = dscb->ds1nobdb;
      }
    }
    else
    {
      // Not a partitioned dataset
      entry.maxdb = -1;
      entry.useddb = -1;
      entry.members = -1;
    }

    load_date_from_dscb(dscb->ds1credt, &entry.cdate, false);
    load_date_from_dscb(dscb->ds1expdt, &entry.edate, true); // expiration date can have sentinel
    load_date_from_dscb(dscb->ds1refd, &entry.rdate, false);
    load_extents_from_dscb(dscb, entry, max_member_track);

    // Convert track-based values to bytes if spacu is BYTES
    // This uses the block size to calculate usable bytes per track
    if (entry.spacu == "BYTES" && entry.blksize > 0)
    {
      // For 3390 DASD, track capacity is approximately 56,664 bytes
      const int TRACK_CAPACITY_3390 = 56664;

      // Calculate usable bytes per track: floor(capacity / blksize) * blksize
      int blocks_per_track = TRACK_CAPACITY_3390 / entry.blksize;
      int usable_bytes_per_track = blocks_per_track * entry.blksize;

      // Convert alloc, primary, and used from tracks to bytes
      if (entry.alloc > 0)
        entry.alloc *= usable_bytes_per_track;
      if (entry.primary > 0)
        entry.primary *= usable_bytes_per_track;
      if (entry.used > 0)
      {
        // Calculate from DS1LSTAR and DS1TRBAL
        // ISPF calculates used space more precisely using DS1TRBAL (space remaining on last track)
        // Formula: used_bytes = (used_tracks × bytes_per_track) - bytes_remaining
        uint16_t trbal = (static_cast<unsigned char>(dscb->ds1trbal[0]) << 8) |
                         static_cast<unsigned char>(dscb->ds1trbal[1]);

        // For PDS datasets, ISPF calculates "used" by scanning the directory
        // and finding the highest TTR among all members, then subtracting DS1TRBAL.
        // DS1LSTAR includes both directory and member data, which can lead to
        // discrepancies of 1-2 blocks compared to ISPF's member-based calculation.
        // For now, we use the simpler DS1LSTAR-based calculation for all datasets.
        // TODO: For exact ISPF matching on PDS, scan directory for highest member TTR

        entry.used = (entry.used * usable_bytes_per_track) - trbal;
      }

      // Secondary allocation is already in bytes from DS1SCEXT for contiguous allocations
      // No conversion needed
    }
  }
  else
  {
    entry.dsorg = ZDS_DSORG_UNKNOWN;
    entry.volser = ZDS_VOLSER_UNKNOWN;
    entry.recfm = ZDS_RECFM_U;
  }

  free(dscb);
}

int zds_list_data_sets(ZDS *zds, string dsn, vector<ZDSEntry> &datasets, bool show_attributes)
{
  int rc = 0;

  zds->csi = NULL;

  // https://www.ibm.com/docs/en/zos/3.1.0?topic=directory-catalog-field-names
  string fields_long[][FIELD_LEN] = {{"VOLSER"}, {"DATACLAS"}, {"MGMTCLAS"}, {"STORCLAS"}, {"DEVTYP"}};
  string fields_short[][FIELD_LEN] = {{"VOLSER"}};

  string(*fields)[FIELD_LEN];
  int number_of_fields;

  if (show_attributes)
  {
    fields = fields_long;
    number_of_fields = sizeof(fields_long) / sizeof(fields_long[0]);
  }
  else
  {
    fields = fields_short;
    number_of_fields = sizeof(fields_short) / sizeof(fields_short[0]);
  }

  int internal_used_buffer_size = sizeof(CSIFIELD) + number_of_fields * FIELD_LEN;

  if (0 == zds->buffer_size)
    zds->buffer_size = ZDS_DEFAULT_BUFFER_SIZE;
  if (0 == zds->max_entries)
    zds->max_entries = ZDS_DEFAULT_MAX_ENTRIES;

#define MIN_BUFFER_FIXED 1024

  int min_buffer_size = MIN_BUFFER_FIXED + internal_used_buffer_size;

  if (zds->buffer_size < min_buffer_size)
  {
    zds->diag.detail_rc = ZDS_RTNCD_INSUFFICIENT_BUFFER;
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Minimum buffer size required is %d but %d was provided", min_buffer_size, zds->buffer_size);
    return RTNCD_FAILURE;
  }

  auto *area = (unsigned char *)__malloc31(zds->buffer_size);
  if (area == nullptr)
  {
    zds->diag.detail_rc = ZDS_RTNCD_INSUFFICIENT_BUFFER;
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Failed to allocate 31-bit buffer for workarea to list %s", dsn.c_str());
    return RTNCD_FAILURE;
  }
  memset(area, 0x00, zds->buffer_size);

  CSIFIELD *selection_criteria = (CSIFIELD *)area;
  char *csi_fields = (char *)(selection_criteria->csifldnm);
  ZDS_CSI_WORK_AREA *csi_work_area = (ZDS_CSI_WORK_AREA *)(area + sizeof(CSIFIELD) + number_of_fields * FIELD_LEN);

  // set work area
  csi_work_area->header.total_size = zds->buffer_size - min_buffer_size;

#define FOUR_BYTE_RETURN 'F'

  // init blanks in query and set input DSN name
  memset(selection_criteria->csifiltk, ' ', sizeof(selection_criteria->csifiltk));
  transform(dsn.begin(), dsn.end(), dsn.begin(), ::toupper); // upper case
  memcpy(selection_criteria->csifiltk, dsn.c_str(), dsn.size());
  memset(&selection_criteria->csicldi, 'Y', sizeof(selection_criteria->csicldi));
  memset(&selection_criteria->csiresum, ' ', sizeof(selection_criteria->csiresum));
  memset(&selection_criteria->csicatnm, ' ', sizeof(selection_criteria->csicatnm));
  memset(&selection_criteria->csis1cat, 'Y', sizeof(selection_criteria->csis1cat)); // do not search master catalog if alias is found
  memset(&selection_criteria->csioptns, FOUR_BYTE_RETURN, sizeof(selection_criteria->csioptns));
  memset(selection_criteria->csidtyps, ' ', sizeof(selection_criteria->csidtyps));

  for (int i = 0; i < number_of_fields; i++)
  {
    memset(csi_fields, ' ', FIELD_LEN);
    memcpy(csi_fields, fields[i][0].c_str(), fields[i][0].size());
    csi_fields += FIELD_LEN;
  }

  selection_criteria->csinumen = number_of_fields;

  do
  {
    rc = ZDSCSI00(zds, selection_criteria, csi_work_area);

    if (0 != rc)
    {
      free(area);
      ZDSDEL(zds);
      strcpy(zds->diag.service_name, "ZDSCSI00");
      zds->diag.service_rc = rc;
      zds->diag.detail_rc = ZDS_RTNCD_SERVICE_FAILURE;
      zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "ZDSCSI00 failed with rc %d", rc);
      return RTNCD_FAILURE;
    }

    int number_fields = csi_work_area->header.number_fields - 1;

    if (number_fields != number_of_fields)
    {
      free(area);
      ZDSDEL(zds);
      zds->diag.detail_rc = ZDS_RTNCD_UNEXPECTED_ERROR;
      zds->diag.e_msg_len =
          sprintf(zds->diag.e_msg,
                  "Unexpected work area field response preset len %d and "
                  "return len %d are not equal",
                  number_fields, number_of_fields);
      return RTNCD_FAILURE;
    }

    if (ERROR_CONDITION == csi_work_area->catalog.flag)
    {
      free(area);
      ZDSDEL(zds);
      zds->diag.detail_rc = ZDS_RTNCD_PARSING_ERROR;
      zds->diag.service_rc = ZDS_RTNCD_CATALOG_ERROR;
      zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Unexpected catalog flag '%x' ", csi_work_area->catalog.flag);
      return RTNCD_FAILURE;
    }

    if (DATA_NOT_COMPLETE & csi_work_area->catalog.flag)
    {
      free(area);
      ZDSDEL(zds);
      zds->diag.detail_rc = ZDS_RTNCD_PARSING_ERROR;
      zds->diag.service_rc = ZDS_RTNCD_CATALOG_ERROR;
      zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Unexpected catalog flag '%x' ", csi_work_area->catalog.flag);
      return RTNCD_FAILURE;
    }

    if (NO_ENTRY & csi_work_area->catalog.flag)
    {
      free(area);
      ZDSDEL(zds);
      zds->diag.detail_rc = ZDS_RSNCD_NOT_FOUND;
      zds->diag.service_rc = ZDS_RTNCD_CATALOG_ERROR;
      zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Not found in catalog, flag '%x' ", csi_work_area->catalog.flag);
      return RTNCD_WARNING;
    }

    if (CATALOG_TYPE != csi_work_area->catalog.type)
    {
      free(area);
      ZDSDEL(zds);
      zds->diag.detail_rc = ZDS_RTNCD_PARSING_ERROR;
      zds->diag.service_rc = ZDS_RTNCD_CATALOG_ERROR;
      zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Unexpected type '%x' ", csi_work_area->catalog.type);
      return RTNCD_FAILURE;
    }

    int work_area_total = csi_work_area->header.used_size;
    unsigned char *p = (unsigned char *)&csi_work_area->entry;
    ZDS_CSI_ENTRY *f = nullptr;

    work_area_total -= sizeof(ZDS_CSI_HEADER);
    work_area_total -= sizeof(ZDS_CSI_CATALOG);

    ZDSEntry entry = {0};
    char buffer[sizeof(f->name) + 1] = {0};

    while (work_area_total > 0)
    {
      f = (ZDS_CSI_ENTRY *)p;

      if (ERROR == f->flag)
      {
        free(area);
        ZDSDEL(zds);
        zds->diag.detail_rc = ZDS_RTNCD_SERVICE_FAILURE;
        zds->diag.service_rc = ZDS_RTNCD_ENTRY_ERROR;
        zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Unexpected entry flag '%x' ", f->flag);
        return RTNCD_FAILURE;
      }

      if (NOT_FOUND == f->type)
      {
        free(area);
        ZDSDEL(zds);
        zds->diag.detail_rc = ZDS_RTNCD_NOT_FOUND;
        zds->diag.service_rc = ZDS_RTNCD_ENTRY_ERROR;
        zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "No entry found '%x' ", f->type);
        return RTNCD_FAILURE;
      }

      // NOTE(Kelosky): rejecting entities we haven't tested... this can be removed as we verify the logic works for all return types
      if (
          NON_VSAM_DATA_SET != f->type &&
          CLUSTER != f->type &&
          DATA_COMPONENT != f->type &&
          INDEX_COMPONENT != f->type &&
          GENERATION_DATA_GROUP != f->type &&
          GENERATION_DATA_SET != f->type &&
          ALIAS != f->type)
      {
        free(area);
        ZDSDEL(zds);
        zds->diag.detail_rc = ZDS_RTNCD_SERVICE_FAILURE;
        zds->diag.service_rc = ZDS_RTNCD_UNSUPPORTED_ERROR;
        zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Unsupported entry type '%x' ", f->type);
        return RTNCD_FAILURE;
      }

      memset(buffer, 0x00, sizeof(buffer));     // clear buffer
      memcpy(buffer, f->name, sizeof(f->name)); // copy all & leave a null
      entry.name = string(buffer);

      int *field_len = &f->response.field.field_lens;
      unsigned char *data = (unsigned char *)&f->response.field.field_lens;
      data += (sizeof(f->response.field.field_lens) * number_fields);

      memset(buffer, 0x00, sizeof(buffer)); // clear buffer
      memcpy(buffer, data, *field_len);     // copy VOLSER
      entry.volser = string(buffer);

#define IPL_VOLUME "******"
#define IPL_VOLUME_SYMBOL "&SYSR1" // https://www.ibm.com/docs/en/zos/3.1.0?topic=symbols-static-system

      if (0 == strcmp(IPL_VOLUME, entry.volser.c_str()))
      {
        string symbol(IPL_VOLUME_SYMBOL);
        string value;
        rc = zut_substitute_symbol(symbol, value);
        if (0 == rc)
        {
          entry.volser = value;
        }
      }

#define MIGRAT_VOLUME "MIGRAT"
#define ARCIVE_VOLUME "ARCIVE"

      // Only load attributes if show_attributes is true
      if (show_attributes)
      {
        if (entry.volser == MIGRAT_VOLUME || entry.volser == ARCIVE_VOLUME)
        {
          entry.migrated = true;
          entry.recfm = ZDS_RECFM_U;
        }
        else
        {
          entry.migrated = false;
        }

        // Load dsorg and recfm from DSCB if not migrated
        // This needs to happen before the switch statement as it sets entry.dsorg
        if (!entry.migrated)
        {
          zds_get_attrs_from_dscb(zds, entry);
        }

        switch (f->type)
        {

        case NON_VSAM_DATA_SET:
          // DSORG and PDSE/PDS determination is done via DSCB parsing in zds_get_attrs_from_dscb()
          // No need to process NVSMATTR field from catalog
          break;
        case GENERATION_DATA_GROUP:
          entry.volser = ZDS_VOLSER_GDG;
          break;
        case CLUSTER:
          entry.dsorg = ZDS_DSORG_VSAM;
          entry.volser = ZDS_VOLSER_VSAM;
          break;
        case DATA_COMPONENT:
          entry.dsorg = ZDS_DSORG_VSAM;
          entry.volser = ZDS_VOLSER_VSAM;
          break;
        case GENERATION_DATA_SET:
          break;
        case INDEX_COMPONENT:
          entry.dsorg = ZDS_DSORG_VSAM;
          entry.volser = ZDS_VOLSER_VSAM;
          break;
        case ALIAS:
          entry.dsorg = ZDS_DSORG_UNKNOWN;
          entry.volser = ZDS_VOLSER_ALIAS;
          break;
        case ALTERNATE_INDEX:
        case ATL_LIBRARY_ENTRY:
        case PATH:
        case USER_CATALOG_CONNECTOR_ENTRY:
        case ATL_VOLUME_ENTRY:
        default:
          free(area);
          ZDSDEL(zds);
          zds->diag.detail_rc = ZDS_RTNCD_SERVICE_FAILURE;
          zds->diag.service_rc = ZDS_RTNCD_UNSUPPORTED_ERROR;
          zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Unsupported entry type '%x' ", f->type);
          return RTNCD_FAILURE;
        };
      } // End if (show_attributes)

      // Parse additional fields only if show_attributes is true
      if (show_attributes)
      {
        // Parse DATACLAS field (8 bytes)
        data += *field_len;
        field_len++;
        entry.dataclass = "";
        if (*field_len > 2)
        {
          // First 2 bytes are a length prefix, extract actual length
          uint16_t actual_len = (static_cast<unsigned char>(data[0]) << 8) | static_cast<unsigned char>(data[1]);
          if (actual_len > 0 && actual_len <= (*field_len - 2))
          {
            entry.dataclass = string((char *)(data + 2), actual_len);
          }
        }

        // Parse MGMTCLAS field (8 bytes)
        data += *field_len;
        field_len++;
        entry.mgmtclass = "";
        if (*field_len > 2)
        {
          // First 2 bytes are a length prefix, extract actual length
          uint16_t actual_len = (static_cast<unsigned char>(data[0]) << 8) | static_cast<unsigned char>(data[1]);
          if (actual_len > 0 && actual_len <= (*field_len - 2))
          {
            entry.mgmtclass = string((char *)(data + 2), actual_len);
          }
        }

        // Parse STORCLAS field (8 bytes)
        data += *field_len;
        field_len++;
        entry.storclass = "";
        if (*field_len > 2)
        {
          // First 2 bytes are a length prefix, extract actual length
          uint16_t actual_len = (static_cast<unsigned char>(data[0]) << 8) | static_cast<unsigned char>(data[1]);
          if (actual_len > 0 && actual_len <= (*field_len - 2))
          {
            entry.storclass = string((char *)(data + 2), actual_len);
          }
        }

        // Parse DEVTYP field (4-byte UCB device type)
        data += *field_len;
        field_len++;
        entry.devtype = "";
        if (*field_len >= 4)
        {
          uint32_t devtyp = (static_cast<unsigned char>(data[0]) << 24) |
                            (static_cast<unsigned char>(data[1]) << 16) |
                            (static_cast<unsigned char>(data[2]) << 8) |
                            static_cast<unsigned char>(data[3]);

          // If devtyp is all zeros, default to 3390 (most common modern DASD)
          if (devtyp == 0x00000000)
          {
            entry.devtype = "3390";
          }
          else
          {
            // Extract the device type code from byte 3 (last byte)
            uint8_t dev_code = static_cast<unsigned char>(data[3]);

            // Map UCB device type to device name
            // Reference: z/OS DFSMSdfp Advanced Services, UCB device type codes
            switch (dev_code)
            {
            case 0x0B:
              entry.devtype = "3340";
              break;
            case 0x0E:
              entry.devtype = "3350";
              break;
            case 0x0F:
              entry.devtype = "3390";
              break; // Most common modern DASD
            case 0x10:
              entry.devtype = "9345";
              break;
            case 0x2E:
              entry.devtype = "3380";
              break;
            default:
              // For unknown types, format as hex string
              char dev_buffer[9];
              sprintf(dev_buffer, "%08X", devtyp);
              entry.devtype = string(dev_buffer);
              break;
            }
          }
        }
      }

      if (datasets.size() + 1 > zds->max_entries)
      {
        free(area);
        ZDSDEL(zds);
        zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Reached maximum returned records requested %d", zds->max_entries);
        zds->diag.detail_rc = ZDS_RSNCD_MAXED_ENTRIES_REACHED;
        return RTNCD_WARNING;
      }

      datasets.push_back(entry);

      work_area_total -= ((sizeof(ZDS_CSI_ENTRY) - sizeof(ZDS_CSI_FIELD) + f->response.field.total_len));
      p = p + ((sizeof(ZDS_CSI_ENTRY) - sizeof(ZDS_CSI_FIELD) + f->response.field.total_len)); // next entry
    }
  } while ('Y' == selection_criteria->csiresum);

  free(area);
  ZDSDEL(zds);

  return RTNCD_SUCCESS;
}

/**
 * Reads data from a data set in streaming mode.
 *
 * @param zds pointer to a ZDS object
 * @param dsn name of the data set
 * @param pipe name of the output pipe
 * @param content_len pointer where the length of the data set contents will be stored
 *
 * @return RTNCD_SUCCESS on success, RTNCD_FAILURE on failure
 */
int zds_read_from_dsn_streamed(ZDS *zds, const string &dsn, const string &pipe, size_t *content_len)
{
  if (content_len == nullptr)
  {
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "content_len must be a valid size_t pointer");
    return RTNCD_FAILURE;
  }

  string dsname = "//'" + dsn + "'";
  if (strlen(zds->ddname) > 0)
  {
    dsname = "//DD:" + string(zds->ddname);
  }
  const std::string fopen_flags = zds->encoding_opts.data_type == eDataTypeBinary ? "rb" : "r";
  FileGuard fin(dsname.c_str(), fopen_flags.c_str());
  if (!fin)
  {
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Could not open dsn '%s'", dsn.c_str());
    return RTNCD_FAILURE;
  }

  int fifo_fd = open(pipe.c_str(), O_WRONLY);
  FileGuard fout(fifo_fd, "w");
  if (!fout)
  {
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Could not open output pipe '%s'", pipe.c_str());
    close(fifo_fd);
    return RTNCD_FAILURE;
  }

  const auto hasEncoding = zds->encoding_opts.data_type == eDataTypeText && strlen(zds->encoding_opts.codepage) > 0;
  const auto codepage = string(zds->encoding_opts.codepage);

  const size_t chunk_size = FIFO_CHUNK_SIZE * 3 / 4;
  std::vector<char> buf(chunk_size);
  size_t bytes_read;
  std::vector<char> temp_encoded;
  std::vector<char> left_over;

  while ((bytes_read = fread(&buf[0], 1, chunk_size, fin)) > 0)
  {
    int chunk_len = bytes_read;
    const char *chunk = &buf[0];

    if (hasEncoding)
    {
      const auto source_encoding = strlen(zds->encoding_opts.source_codepage) > 0 ? string(zds->encoding_opts.source_codepage) : "UTF-8";
      try
      {
        temp_encoded = zut_encode(chunk, chunk_len, codepage, source_encoding, zds->diag);
        chunk = &temp_encoded[0];
        chunk_len = temp_encoded.size();
      }
      catch (std::exception &e)
      {
        zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Failed to convert input data from %s to %s", codepage.c_str(), source_encoding.c_str());
        return RTNCD_FAILURE;
      }
    }

    *content_len += chunk_len;
    temp_encoded = zbase64::encode(chunk, chunk_len, &left_over);
    fwrite(&temp_encoded[0], 1, temp_encoded.size(), fout);
    temp_encoded.clear();
  }

  if (!left_over.empty())
  {
    temp_encoded = zbase64::encode(&left_over[0], left_over.size());
    fwrite(&temp_encoded[0], 1, temp_encoded.size(), fout);
  }

  fflush(fout);

  return RTNCD_SUCCESS;
}

/**
 * Writes data to a data set in streaming mode.
 *
 * @param zds pointer to a ZDS object
 * @param dsn name of the data set
 * @param pipe name of the input pipe
 * @param content_len pointer where the length of the data set contents will be stored
 *
 * @return RTNCD_SUCCESS on success, RTNCD_FAILURE on failure
 */
int zds_write_to_dsn_streamed(ZDS *zds, const string &dsn, const string &pipe, size_t *content_len)
{
  if (content_len == nullptr)
  {
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "content_len must be a valid size_t pointer");
    return RTNCD_FAILURE;
  }
  else if (!zds_dataset_exists(dsn))
  {
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Could not access '%s'", dsn.c_str());
    return RTNCD_FAILURE;
  }

  string dsname = "//'" + dsn + "'";
  if (strlen(zds->ddname) > 0)
  {
    dsname = "//DD:" + string(zds->ddname);
  }

  if (strlen(zds->etag) > 0)
  {
    // Get current data set content for etag check
    ZDS read_ds = {0};
    string current_contents = "";
    if (zds->encoding_opts.data_type == eDataTypeText && strlen(zds->encoding_opts.codepage) > 0)
    {
      memcpy(&read_ds.encoding_opts, &zds->encoding_opts, sizeof(ZEncode));
    }
    const auto read_rc = zds_read_from_dsn(&read_ds, dsn, current_contents);
    if (0 != read_rc)
    {
      zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Failed to read contents of data set for e-tag comparison: %s", read_ds.diag.e_msg);
      return RTNCD_FAILURE;
    }

    const auto given_etag = strtoul(zds->etag, nullptr, 16);
    const auto new_etag = zut_calc_adler32_checksum(current_contents);

    if (given_etag != new_etag)
    {
      ostringstream ss;
      ss << "Etag mismatch: expected ";
      ss << std::hex << given_etag << std::dec;
      ss << ", actual ";
      ss << std::hex << new_etag << std::dec;

      const auto error_msg = ss.str();
      zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "%s", error_msg.c_str());
      return RTNCD_FAILURE;
    }
  }

  const auto hasEncoding = zds->encoding_opts.data_type == eDataTypeText && strlen(zds->encoding_opts.codepage) > 0;
  const auto codepage = string(zds->encoding_opts.codepage);
  const auto fopen_extra_flags = zds->encoding_opts.data_type == eDataTypeBinary ? "b" : "" + string(",recfm=*");

  // If file already exists, open in read+write mode to avoid losing ISPF stats
  FileGuard fout(dsname.c_str(), ("r+" + fopen_extra_flags).c_str());
  if (!fout)
  {
    fout.reset(dsname.c_str(), ("w" + fopen_extra_flags).c_str());
    if (!fout)
    {
      zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Could not open dsn '%s'", dsn.c_str());
      return RTNCD_FAILURE;
    }
  }

  int fifo_fd = open(pipe.c_str(), O_RDONLY);
  if (fifo_fd == -1)
  {
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "open() failed on input pipe '%s', errno %d", pipe.c_str(), errno);
    return RTNCD_FAILURE;
  }

  FileGuard fin(fifo_fd, "r");
  if (!fin)
  {
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Could not open input pipe '%s'", pipe.c_str());
    close(fifo_fd);
    return RTNCD_FAILURE;
  }

  std::vector<char> buf(FIFO_CHUNK_SIZE);
  size_t bytes_read;
  std::vector<char> temp_encoded;
  std::vector<char> left_over;

  while ((bytes_read = fread(&buf[0], 1, FIFO_CHUNK_SIZE, fin)) > 0)
  {
    temp_encoded = zbase64::decode(&buf[0], bytes_read, &left_over);
    const char *chunk = &temp_encoded[0];
    int chunk_len = temp_encoded.size();
    *content_len += chunk_len;

    if (hasEncoding)
    {
      const auto source_encoding = strlen(zds->encoding_opts.source_codepage) > 0 ? string(zds->encoding_opts.source_codepage) : "UTF-8";
      try
      {
        temp_encoded = zut_encode(chunk, chunk_len, source_encoding, codepage, zds->diag);
        chunk = &temp_encoded[0];
        chunk_len = temp_encoded.size();
      }
      catch (std::exception &e)
      {
        zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Failed to convert input data from %s to %s", source_encoding.c_str(), codepage.c_str());
        return RTNCD_FAILURE;
      }
    }

    size_t bytes_written = fwrite(chunk, 1, chunk_len, fout);
    if (bytes_written != chunk_len)
    {
      zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Failed to write to '%s' (possibly out of space)", dsname.c_str());
      return RTNCD_FAILURE;
    }
    temp_encoded.clear();
  }

  fflush(fout);

  // Update the etag
  string saved_contents = "";
  const auto read_rc = zds_read_from_dsn(zds, dsn, saved_contents);
  if (0 != read_rc)
  {
    return RTNCD_FAILURE;
  }

  stringstream etag_stream;
  etag_stream << std::hex << zut_calc_adler32_checksum(saved_contents) << std::dec;
  strcpy(zds->etag, etag_stream.str().c_str());

  return RTNCD_SUCCESS;
}
