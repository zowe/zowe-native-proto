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

int zds_read_from_dsn(ZDS *zds, string dsn, string &response)
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
typedef struct
{
  int total_size;
  int min_size;
  int used_size;
  short number_feilds;
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
#define CSINTICF 0x80
#define CSINOENT 0x40
#define CSINTCMP 0x20
#define CSICERR  0x10
#define CSICERRP 0x08
  unsigned char type;
#define CATALOG_TYPE 0xF0
  ZDS_CSI_ERROR_INFO error_info;
} ZDS_CSI_CATALOG;

// typedef struct
// {

// } ZDS_FIELD;


typedef struct
{
  unsigned char flag;
#define CSIPMENT 0x80
#define CSIENTER 0x40
#define CSIEDATA 0x20
  unsigned char type;
#define NON_VSAM_DATA_SET            'A'
#define GENERATION_DATA_GROUP        'B'
#define CLUSTER                      'C'
#define DATA_COMPONENT               'D'
#define ALTERNATE_INDEX              'G'
#define GENERATION_DATA_SET          'H'
#define INDEX_COMPONENT              'I'
#define ATL_LIBRARY_ENTRY            'L'
#define PATH                         'R'
#define USER_CATALOG_CONNECTOR_ENTRY 'U'
#define ATL_VOLUME_ENTRY             'W'
#define ALIAS                        'X'
  char name[44];

  ZDS_CSI_ERROR_INFO error_info; // if CSIENTER=1
} ZDS_CSI_ENTRY;

typedef struct
{
  int total_size;
  int min_size;
  int used_size;
  short number_feilds;

} ZDS_CSI_WORK_AREA;


#if (defined(__IBMCPP__) || defined(__IBMC__))
#pragma pack(reset)
#endif


#define BUFF_SIZE 1024
#define FIELD_LEN 8

// TODO(Kelosky): allow resume for more records
// TODO(Kelosky): cap number of response
// TODO(Kelosky): preserve loaded module until complete then delete
int zds_list_data_sets(ZDS *zds, string dsn, vector<ZDSEntry> &attributes)
{
  int rc = 0;

  // https://www.ibm.com/docs/en/zos/3.1.0?topic=directory-catalog-field-names
  string fields[][FIELD_LEN] = {
    {"VOLSER"},
    {"NVSMATTR"}
    };

  int number_of_fields = sizeof(fields) / sizeof(fields[0]);

  int total_size_needed = sizeof(CSIFIELD) + (number_of_fields * FIELD_LEN) + BUFF_SIZE;
  unsigned char *area = (unsigned char *)__malloc31(total_size_needed);
  memset(area, 0x00, total_size_needed);

  CSIFIELD *selection_criteria = (CSIFIELD *)area;
  char *csi_fields = (char *)(selection_criteria->csifldnm);
  ZDS_CSI_WORK_AREA *csi_work_area = (ZDS_CSI_WORK_AREA *)(area + sizeof(CSIFIELD) + number_of_fields * FIELD_LEN);

  // set work area
  csi_work_area->total_size = BUFF_SIZE;

  // init blanks in query and set input DSN name
  memset(selection_criteria->csifiltk, ' ', sizeof(selection_criteria->csifiltk));
  transform(dsn.begin(), dsn.end(), dsn.begin(), ::toupper); // upper case
  memcpy(selection_criteria->csifiltk, dsn.c_str(), dsn.size());
  memset(&selection_criteria->csicldi, ' ', sizeof(selection_criteria->csicldi));
  memset(&selection_criteria->csiresum, ' ', sizeof(selection_criteria->csiresum));
  memset(&selection_criteria->csis1cat, ' ', sizeof(selection_criteria->csis1cat));
  memset(&selection_criteria->csioptns, 'F', sizeof(selection_criteria->csioptns)); // 4 byte long output data fields
  memset(selection_criteria->csidtyps, ' ', sizeof(selection_criteria->csidtyps));
  memset(selection_criteria->csidtyps, ' ', sizeof(selection_criteria->csidtyps));
  memset(selection_criteria->csidtyps, ' ', sizeof(selection_criteria->csidtyps));

  for (int i = 0; i < number_of_fields; i++)
  {
    memset(csi_fields, ' ', FIELD_LEN);
    memcpy(csi_fields,  fields[i][0].c_str(), fields[i][0].size());
    csi_fields += FIELD_LEN;
  }

  selection_criteria->csinumen = number_of_fields;

  zut_dump_storage("slection ", selection_criteria, sizeof(CSIFIELD) + number_of_fields * FIELD_LEN);

  rc = ZDSCSI00(zds, selection_criteria, csi_work_area);

  if (0 != rc)
  {
    free(area);
    return RTNCD_FAILURE;
  }

  zut_dump_storage("work area", csi_work_area, 256);

  free(area);

  return rc;
}
