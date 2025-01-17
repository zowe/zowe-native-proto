/*
  This program and the accompanying materials are
  made available under the terms of the Eclipse Public License v2.0 which accompanies
  this distribution, and is available at https://www.eclipse.org/legal/epl-v20.html

  SPDX-License-Identifier: EPL-2.0

  Copyright Contributors to the Zowe Project.
*/
#include <stdio.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <iomanip>
#include <algorithm>
#include "zds.hpp"
#include "zdyn.h"
#include "zdstype.h"
#include "zut.hpp"
#include "iefzb4d2.h"
#include "zdsm.h"

using namespace std;

// int zds_read_from_dd(ZDS *zds, string ddname, string &response)
// {
//   string prefix_ddname = "DD:" + ddname;

//   FILE *fp = fopen(prefix_ddname.c_str(), "r"); // e.g. DD:SYS00001

//   if (NULL == fp)
//   {
//     zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Failed to open ddname '%s'", prefix_ddname.c_str());
//     return RTNCD_FAILURE;
//   }

//   int readlen = 0;
//   char buffer[256 + 1] = {0};
//   while ((readlen = fread(buffer, 1, sizeof(buffer), fp)) > 0)
//   {
//     response += string(buffer, readlen);
//   }
//   fclose(fp);

//   return 0;
// }

int zds_read_from_dd(ZDS *zds, string ddname, string &response)
{
  ddname = "DD:" + ddname;

  ifstream in(ddname.c_str());
  if (!in.is_open())
  {
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Could not open file '%s'", ddname.c_str());
    return RTNCD_FAILURE;
  }

  string line;
  while (getline(in, line))
  {
    response += line;
    response.push_back('\n');
  }

  in.close();

  return 0;
}

int zds_read_from_dsn(ZDS *zds, string dsn, string &response, string *encoding)
{
  dsn = "//'" + dsn + "'";

  ifstream in(dsn.c_str());
  if (!in.is_open())
  {
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Could not open file '%s'", dsn.c_str());
    return RTNCD_FAILURE;
  }

  string line;
  while (getline(in, line))
  {
    response += line;
    response.push_back('\n');
  }

  in.close();

  return 0;
}

int zds_write_to_dd(ZDS *zds, string ddname, string &data)
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

int zds_write_to_dsn(ZDS *zds, string dsn, string &data)
{
  dsn = "//'" + dsn + "'";
  ofstream out(dsn.c_str());

  if (!out.is_open())
  {
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Could not open '%s'", dsn.c_str());
    return RTNCD_FAILURE;
  }

  out << data;
  out.close();

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
  string parm = "ALLOC DA('" + dsn + "') DSORG(PO) SPACE(5,5) CYL LRECL(80) RECFM(F,B) DIR(5) NEW KEEP";

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

// #include <dirent.h>
// int obtain_member_info(ZCLIResult result)
// {
//   dsn = "//'" + dsn + "'";
//   FILE *dir = fopen(dsn.c_str(), "r");
//   if (dir) {
//     cout << "got a doir" << endl;

//     int rc = fldata(dir, filename, &fileinfo);

//     zut_dump_storage("wowo", &fileinfo, sizeof(fldata_t));
//     if (fileinfo.__recfmF) cout << "Fixed\n";
//     if (fileinfo.__recfmV) cout << "Variable\n";
//     if (fileinfo.__recfmU) cout << "Undefined\n";
//     if (fileinfo.__recfmS) cout << "Standard\n";
//     if (fileinfo.__recfmBlk) cout << "Blocked\n";
//     if (fileinfo.__recfmASA) cout << "ASA\n";
//     if (fileinfo.__recfmM) cout << "M\n";
//     if (fileinfo.__dsorgPO) cout << "Partitioned\n";
//     if (fileinfo.__dsorgPDSmem) cout << "Member\n";
//     if (fileinfo.__dsorgPDSdir) cout << "PDS or PDSE directory\n";
//     if (fileinfo.__dsorgPS) cout << "Sequention\n";
//     if (fileinfo.__dsorgVSAM) cout << "VSAM\n";
//     if (fileinfo.__dsorgPDSE) cout << "PDSE\n";

//     printf("dsn %s and macxlrecl %d \n", fileinfo.__dsname, fileinfo.__maxreclen);

//     cout << "rc was " << rc << endl;
//   }
// }

int zds_list_members(ZDS *zds, string dsn, vector<ZDSMem> &list)
{
  // PO
  // PO-E (PDS)
  dsn = "//'" + dsn + "'";

  RECORD rec = {0};
  // https://www.ibm.com/docs/en/zos/3.1.0?topic=pds-reading-directory-sequentially
  // https://www.ibm.com/docs/en/zos/3.1.0?topic=pdse-reading-directory - long alias names omitted, use DESERV for those
  // bldl / deserv
  // https://www.ibm.com/docs/en/zos/3.1.0?topic=pds-directory
  FILE *fp = fopen(dsn.c_str(), "rb, blksize=256, recfm=fb");

  const int bufsize = 256;
  char buffer[bufsize] = {0};

  if (!fp)
  {
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Could not open dsn '%s'", dsn.c_str());
    return RTNCD_FAILURE;
  }

  while (fread(&rec, sizeof *buffer, sizeof(RECORD), fp))
  {
    unsigned char *data = NULL;
    data = (unsigned char *)&rec;
    data += sizeof(rec.count); // increment past halfword length
    int len = sizeof(RECORD_ENTRY);
    for (int i = 0; i < rec.count; i = i + len)
    {
      RECORD_ENTRY RECORD_ENTRY = {0};
      memcpy(&RECORD_ENTRY, data, sizeof(RECORD_ENTRY));
      long long int end = 0xFFFFFFFFFFFFFFFF;
      if (memcmp(RECORD_ENTRY.name, &end, sizeof(end)) == 0)
      {
        break;
      }
      else
      {
        unsigned char info = RECORD_ENTRY.info;
        char name[9] = {0};
        info &= 0x1F;

        memcpy(name, RECORD_ENTRY.name, sizeof(RECORD_ENTRY.name));

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

        data = data + sizeof(RECORD_ENTRY) + (info * 2); // skip number of half workds
        len += (info * 2);
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
  char name[44];
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
#define ATL_VOLUME_ENTRY 'W'
#define ALIAS 'X'
  char name[44];

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

  if (0 == zds->buffer_size)
    zds->buffer_size = ZDS_DEFAULT_BUFFER_SIZE;
  if (0 == zds->max_entries)
    zds->max_entries = ZDS_DEFAULT_MAX_ENTRIES;

#define MIN_BUFFER 1024

  if (zds->buffer_size < MIN_BUFFER)
  {
    zds->diag.detail_rc = ZDS_RTNCD_INSUFFICIENT_BUFFER;
    zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Minimum buffer size required is %d but %d was provided", MIN_BUFFER, zds->buffer_size);
    return RTNCD_FAILURE;
  }

  // https://www.ibm.com/docs/en/zos/3.1.0?topic=directory-catalog-field-names
  string fields[][FIELD_LEN] = {
      {"VOLSER"},
      {"NVSMATTR"}};

  int number_of_fields = sizeof(fields) / sizeof(fields[0]);

  unsigned char *area = (unsigned char *)__malloc31(zds->buffer_size);
  memset(area, 0x00, zds->buffer_size);

  CSIFIELD *selection_criteria = (CSIFIELD *)area;
  char *csi_fields = (char *)(selection_criteria->csifldnm);
  ZDS_CSI_WORK_AREA *csi_work_area = (ZDS_CSI_WORK_AREA *)(area + sizeof(CSIFIELD) + number_of_fields * FIELD_LEN);

  // set work area
  csi_work_area->header.total_size = zds->buffer_size;

#define FOUR_BYTE_RETURN 'F'

  // init blanks in query and set input DSN name
  memset(selection_criteria->csifiltk, ' ', sizeof(selection_criteria->csifiltk));
  transform(dsn.begin(), dsn.end(), dsn.begin(), ::toupper); // upper case
  memcpy(selection_criteria->csifiltk, dsn.c_str(), dsn.size());
  memset(&selection_criteria->csicldi, 'Y', sizeof(selection_criteria->csicldi));
  memset(&selection_criteria->csiresum, ' ', sizeof(selection_criteria->csiresum));
  memset(&selection_criteria->csis1cat, ' ', sizeof(selection_criteria->csis1cat));
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
      zds->diag.detail_rc = ZDS_RTNCD_UNEXPECTED_ERROR;
      zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Unexpected work area field response preset len %d and return len %d are not equal", number_fields, number_of_fields);
      return RTNCD_FAILURE;
    }

    if (CATALOG_TYPE != csi_work_area->catalog.type)
    {
      free(area);
      zds->diag.detail_rc = ZDS_RTNCD_PARSING_ERROR;
      zds->diag.service_rc = ZDS_RTNCD_CATALOG_ERROR;
      zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Unexpected type '%x' ", csi_work_area->catalog.type);
      return RTNCD_FAILURE;
    }

    if (ERROR_CONDITION == csi_work_area->catalog.flag)
    {
      free(area);
      zds->diag.detail_rc = ZDS_RTNCD_PARSING_ERROR;
      zds->diag.service_rc = ZDS_RTNCD_CATALOG_ERROR;
      zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Unexpected catalog flag '%x' ", csi_work_area->catalog.flag);
      return RTNCD_FAILURE;
    }

    if (DATA_NOT_COMPLETE & csi_work_area->catalog.flag)
    {
      free(area);
      zds->diag.detail_rc = ZDS_RTNCD_PARSING_ERROR;
      zds->diag.service_rc = ZDS_RTNCD_CATALOG_ERROR;
      zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Unexpected catalog flag '%x' ", csi_work_area->catalog.flag);
      return RTNCD_FAILURE;
    }

    if (NO_ENTRY & csi_work_area->catalog.flag)
    {
      free(area);
      zds->diag.detail_rc = ZDS_RSNCD_NOT_FOUND;
      zds->diag.service_rc = ZDS_RTNCD_CATALOG_ERROR;
      zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Not found in catalog, flag '%x' ", csi_work_area->catalog.flag);
      return RTNCD_WARNING;
    }

    int work_area_total = csi_work_area->header.used_size;
    unsigned char *p = (unsigned char *)&csi_work_area->entry;
    ZDS_CSI_ENTRY *f = NULL;

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
        zds->diag.detail_rc = ZDS_RTNCD_SERVICE_FAILURE;
        zds->diag.service_rc = ZDS_RTNCD_ENTRY_ERROR;
        zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Unexpected entry flag '%x' ", f->flag);
        return RTNCD_FAILURE;
      }

      if (NOT_FOUND == f->type)
      {
        free(area);
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
          INDEX_COMPONENT != f->type)
      {
        free(area);
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

      if (0 == *field_len)
      {

        string dsn = "//'" + entry.name + "'";
        FILE *dir = fopen(dsn.c_str(), "r");
        fldata_t file_info = {0};
        char file_name[64] = {0};

        if (dir)
        {
          if (0 == fldata(dir, file_name, &file_info))
          {
            if (file_info.__dsorgVSAM)
            {
              entry.dsorg = DSORG_VSAM;
              entry.volser = VOLSER_VSAM;
            }
            else if (file_info.__dsorgHFS)
            {
              entry.dsorg = DSORG_VSAM;
              entry.volser = VOLSER_VSAM;
            }
          }
          else
            entry.dsorg = DSORG_UNKNWON;
          entry.volser = VOLSER_UNKNOWN;
        }
        else
        {
          entry.dsorg = DSORG_UNKNWON;
          entry.volser = VOLSER_UNKNOWN;
        }
      }

      else
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
            entry.dsorg = DSORG_PDSE;
          }
          else if (SIMPLE_NON_VSAM_DATA_SET == non_vsam_attribute)
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
                  entry.dsorg = DSORG_PS;
                }
                else if (file_info.__dsorgPO)
                {
                  entry.dsorg = DSORG_PO;
                }
                else
                {
                  entry.dsorg = DSORG_UNKNWON;
                  entry.volser = VOLSER_UNKNOWN;
                }
              }
              else
              {
                entry.dsorg = DSORG_UNKNWON;
                entry.volser = VOLSER_UNKNOWN;
              }
            }
          }
        }
      }

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

  return rc;
}
