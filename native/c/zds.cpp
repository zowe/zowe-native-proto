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

const size_t MAX_DS_LENGTH = 44u;

using namespace std;

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
  string bytes;
  bytes.reserve(size);
  memcpy((char *)bytes.data(), response.c_str(), size);

  if (size > 0 && strlen(zds->encoding_opts.codepage) > 0)
  {
    std::string temp = response;
    try
    {
      const auto bytes_with_encoding = zut_encode(temp, string(zds->encoding_opts.codepage), "UTF-8", zds->diag);
      temp = bytes_with_encoding;
    }
    catch (std::exception &e)
    {
      // TODO: error handling
    }
    if (!temp.empty())
    {
      response = temp;
    }
  }

  return 0;
}

int zds_read_from_dsn(ZDS *zds, string dsn, string &response)
{
  dsn = "//'" + dsn + "'";
  const std::string fopen_flags = zds->encoding_opts.data_type == eDataTypeBinary ? "rb,recfm=U" : "r";
  FILE *fp = fopen(dsn.c_str(), fopen_flags.c_str());
  if (!fp)
  {
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Could not open file '%s'", dsn.c_str());
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
    std::string temp = response;
    try
    {
      const auto bytes_with_encoding = zut_encode(temp, string(zds->encoding_opts.codepage), "UTF-8", zds->diag);
      temp = bytes_with_encoding;
    }
    catch (std::exception &e)
    {
      // TODO: error handling
    }
    if (!temp.empty())
    {
      response = temp;
    }
  }

  return 0;
}

int zds_write_to_dd(ZDS *zds, string ddname, string data)
{
  ddname = "DD:" + ddname;
  ofstream out(ddname.c_str());

  if (!out.is_open())
  {
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Could not open '%s'", ddname.c_str());
    return RTNCD_FAILURE;
  }

  out << data;
  out.close();

  return 0;
}

int zds_write_to_dsn(ZDS *zds, std::string dsn, std::string &data)
{
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
      ss << std::hex << given_etag << std::dec;
      ss << ", actual ";
      ss << std::hex << new_etag << std::dec;

      const auto error_msg = ss.str();
      zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "%s", error_msg.c_str());
      return RTNCD_FAILURE;
    }
  }

  const string dsname = "//'" + dsn + "'";
  std::string temp = data;

  auto *fp = fopen(dsname.c_str(), zds->encoding_opts.data_type == eDataTypeBinary ? "wb,recfm=U" : "w");
  if (nullptr == fp)
  {
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Could not open '%s'", dsname.c_str());
    return RTNCD_FAILURE;
  }

  if (!data.empty())
  {
    if (hasEncoding)
    {
      try
      {
        temp = zut_encode(temp, "UTF-8", codepage, zds->diag);
      }
      catch (std::exception &e)
      {
        zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Failed to convert input data from UTF-8 to %s", codepage.c_str());
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
  etag_stream << std::hex << zut_calc_adler32_checksum(saved_contents);
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

// TODO(Kelosky): add attributues to ZDS and have other functions populate it
int zds_create_dsn(ZDS *zds, string dsn, string &response)
{
  int rc = 0;
  unsigned int code = 0;
  string parm = "ALLOC DA('" + dsn + "') DSORG(PO) SPACE(5,5) CYL LRECL(80) RECFM(F,B) DIR(5) NEW KEEP DSNTYPE(LIBRARY)";

  return zut_bpxwdyn(parm, &code, response);
}

int zds_create_dsn_vb(ZDS *zds, string dsn, string &response)
{
  int rc = 0;
  unsigned int code = 0;
  string parm = "ALLOC DA('" + dsn + "') DSORG(PO) SPACE(5,5) CYL LRECL(255) RECFM(V,B) DIR(5) NEW KEEP DSNTYPE(LIBRARY)";

  return zut_bpxwdyn(parm, &code, response);
}

int zds_create_dsn_adata(ZDS *zds, string dsn, string &response)
{
  int rc = 0;
  unsigned int code = 0;
  string parm = "ALLOC DA('" + dsn + "') DSORG(PO) SPACE(5,5) CYL LRECL(32756) BLKSIZE(32760) RECFM(V,B) DIR(5) NEW KEEP DSNTYPE(LIBRARY)";

  return zut_bpxwdyn(parm, &code, response);
}

int zds_create_dsn_loadlib(ZDS *zds, string dsn, string &response)
{
  int rc = 0;
  unsigned int code = 0;
  string parm = "ALLOC DA('" + dsn + "') DSORG(PO) SPACE(5,5) CYL LRECL(0) BLKSIZE(32760) RECFM(U) DIR(5) NEW KEEP DSNTYPE(LIBRARY)";

  return zut_bpxwdyn(parm, &code, response);
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
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Could not delete data set '%s', rc: '%d'", dsn.c_str());
    zds->diag.detail_rc = ZDS_RTNCD_SERVICE_FAILURE;
    return RTNCD_FAILURE;
  }

  return 0;
}

int zds_list_members(ZDS *zds, string dsn, vector<ZDSMem> &list)
{
  // PO
  // PO-E (PDS)
  dsn = "//'" + dsn + "'";

  int total_entries = 0;

  if (0 == zds->max_entries)
    zds->max_entries = ZDS_DEFAULT_MAX_ENTRIES;

  RECORD rec = {0};
  // https://www.ibm.com/docs/en/zos/3.1.0?topic=pds-reading-directory-sequentially
  // https://www.ibm.com/docs/en/zos/3.1.0?topic=pdse-reading-directory - long alias names omitted, use DESERV for those
  // https://www.ibm.com/docs/en/zos/3.1.0?topic=pds-directory
  FILE *fp = fopen(dsn.c_str(), "rb, blksize=256, recfm=u");

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
          return RTNCD_WARNING;
        }

        unsigned char info = entry.info;
        unsigned char pointer_count = entry.info;
        char name[9] = {0};
        if (info & 0x80) // bit 0 indicates alias
        {
          // TODO(Kelosky): // member name is an alias
        }
        pointer_count & 0x60; // bits 1-2 contain number of user data TTRNs
        pointer_count >>= 5;  // adjust to byte boundary
        info &= 0x1F;         // bits 3-7 contain the number of half words of user data

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
        list.push_back(mem);

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

#if (defined(__IBMCPP__) || defined(__IBMC__))
#pragma pack(packed)
#endif

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

#if (defined(__IBMCPP__) || defined(__IBMC__))
#pragma pack(reset)
#endif

#define BUFF_SIZE 1024
#define FIELD_LEN 8

int zds_list_data_sets(ZDS *zds, string dsn, vector<ZDSEntry> &attributes)
{
  int rc = 0;

  zds->csi = NULL;

  // https://www.ibm.com/docs/en/zos/3.1.0?topic=directory-catalog-field-names
  string fields[][FIELD_LEN] = {
      {"VOLSER"},
      {"NVSMATTR"}};

  int number_of_fields = sizeof(fields) / sizeof(fields[0]);

  int interal_used_buffer_size = sizeof(CSIFIELD) + number_of_fields * FIELD_LEN;

  if (0 == zds->buffer_size)
    zds->buffer_size = ZDS_DEFAULT_BUFFER_SIZE;
  if (0 == zds->max_entries)
    zds->max_entries = ZDS_DEFAULT_MAX_ENTRIES;

#define MIN_BUFFER_FIXED 1024

  int min_buffer_size = MIN_BUFFER_FIXED + interal_used_buffer_size;

  if (zds->buffer_size < min_buffer_size)
  {
    zds->diag.detail_rc = ZDS_RTNCD_INSUFFICIENT_BUFFER;
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Minimum buffer size required is %d but %d was provided", min_buffer_size, zds->buffer_size);
    return RTNCD_FAILURE;
  }

  unsigned char *area = (unsigned char *)__malloc31(zds->buffer_size);
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

    // zut_dump_storage("alias", csi_work_area, 512);

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
      zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Unexpected work area field response preset len %d and return len %d are not equal", number_fields, number_of_fields);
      return RTNCD_FAILURE;
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
        int rc = zut_substitute_sybmol(symbol, value);
        if (0 == rc)
        {
          entry.volser = value;
        }
      }

#define MIGRAT_VOLUME "MIGRAT"
#define ARCIVE_VOLUME "ARCIVE"

      if (entry.volser == MIGRAT_VOLUME || entry.volser == ARCIVE_VOLUME)
      {
        entry.migr = true;
      }
      else
      {
        entry.migr = false;
      }

      // attempt to obtain fldata in all cases and set default data
      if (!entry.migr)
      {
        string dsn = "//'" + entry.name + "'";
        FILE *dir = fopen(dsn.c_str(), "r");
        fldata_t file_info = {0};
        char file_name[64] = {0};

        if (dir)
        {
          if (0 == fldata(dir, file_name, &file_info))
          {
            if (file_info.__dsorgPS)
            {
              entry.dsorg = ZDS_DSORG_PS;
            }
            else if (file_info.__dsorgPO)
            {
              entry.dsorg = ZDS_DSORG_PO;
            }
            else if (file_info.__dsorgVSAM)
            {
              entry.dsorg = ZDS_DSORG_VSAM;
            }
            else
            {
              entry.dsorg = ZDS_DSORG_UNKNOWN;
              entry.volser = ZDS_VOLSER_UNKNOWN;
            }
          }
          else
          {
            entry.dsorg = ZDS_DSORG_UNKNOWN;
            entry.volser = ZDS_VOLSER_UNKNOWN;
          }
          fclose(dir);
        }
      }

      switch (f->type)
      {

      case NON_VSAM_DATA_SET:
      {
        data += *field_len; // point to next data field
        field_len++;        // point to next len field

        char non_vsam_attribute = 0xFF;

        if (*field_len > 0)
        {
          non_vsam_attribute = *(char *)data;

#define SIMPLE_NON_VSAM_DATA_SET 0X00
#define EXTENDED_PARTITIONED_DATA_SET 'L'

          if (EXTENDED_PARTITIONED_DATA_SET == non_vsam_attribute)
          {
            entry.dsorg = ZDS_DSORG_PDSE;
          }
        }
      }

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
      case ALTERNATE_INDEX:
        free(area);
        ZDSDEL(zds);
        zds->diag.detail_rc = ZDS_RTNCD_SERVICE_FAILURE;
        zds->diag.service_rc = ZDS_RTNCD_UNSUPPORTED_ERROR;
        zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Unsupported entry type '%x' ", f->type);
        return RTNCD_FAILURE;
        break;
      case GENERATION_DATA_SET:
        break;
      case INDEX_COMPONENT:
        entry.dsorg = ZDS_DSORG_VSAM;
        entry.volser = ZDS_VOLSER_VSAM;
        break;
      case ATL_LIBRARY_ENTRY:
        free(area);
        ZDSDEL(zds);
        zds->diag.detail_rc = ZDS_RTNCD_SERVICE_FAILURE;
        zds->diag.service_rc = ZDS_RTNCD_UNSUPPORTED_ERROR;
        zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Unsupported entry type '%x' ", f->type);
        return RTNCD_FAILURE;
        break;
      case PATH:
        free(area);
        ZDSDEL(zds);
        zds->diag.detail_rc = ZDS_RTNCD_SERVICE_FAILURE;
        zds->diag.service_rc = ZDS_RTNCD_UNSUPPORTED_ERROR;
        zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Unsupported entry type '%x' ", f->type);
        return RTNCD_FAILURE;
        break;
      case USER_CATALOG_CONNECTOR_ENTRY:
        free(area);
        ZDSDEL(zds);
        zds->diag.detail_rc = ZDS_RTNCD_SERVICE_FAILURE;
        zds->diag.service_rc = ZDS_RTNCD_UNSUPPORTED_ERROR;
        zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Unsupported entry type '%x' ", f->type);
        return RTNCD_FAILURE;
        break;
      case ATL_VOLUME_ENTRY:
        free(area);
        ZDSDEL(zds);
        zds->diag.detail_rc = ZDS_RTNCD_SERVICE_FAILURE;
        zds->diag.service_rc = ZDS_RTNCD_UNSUPPORTED_ERROR;
        zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Unsupported entry type '%x' ", f->type);
        return RTNCD_FAILURE;
        break;
      case ALIAS:
        entry.dsorg = ZDS_DSORG_UNKNOWN;
        entry.volser = ZDS_VOLSER_ALIAS;
        break;
      default:
        free(area);
        ZDSDEL(zds);
        zds->diag.detail_rc = ZDS_RTNCD_SERVICE_FAILURE;
        zds->diag.service_rc = ZDS_RTNCD_UNSUPPORTED_ERROR;
        zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Unsupported entry type '%x' ", f->type);
        return RTNCD_FAILURE;
        break;
      };

      if (attributes.size() + 1 > zds->max_entries)
      {
        zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Reached maximum returned records requested %d", zds->max_entries);
        zds->diag.detail_rc = ZDS_RSNCD_MAXED_ENTRIES_REACHED;
        return RTNCD_WARNING;
      }

      attributes.push_back(entry);

      work_area_total -= ((sizeof(ZDS_CSI_ENTRY) - sizeof(ZDS_CSI_FIELD) + f->response.field.total_len));
      p = p + ((sizeof(ZDS_CSI_ENTRY) - sizeof(ZDS_CSI_FIELD) + f->response.field.total_len)); // next entry
    }

  } while ('Y' == selection_criteria->csiresum);

  free(area);
  ZDSDEL(zds);

  return rc;
}
