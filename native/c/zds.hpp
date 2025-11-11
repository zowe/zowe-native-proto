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

#ifndef ZDS_HPP
#define ZDS_HPP

#include <vector>
#include <string>
#include "zdstype.h"

extern const size_t MAX_DS_LENGTH;

struct ZDSMem
{
  std::string name;
  // std::string dsorg;
};

struct ZDSEntry
{
  std::string name;
  int alloc;
  int allocx;
  int blksize;
  std::string cdate;
  std::string dataclass;
  int devtype;
  std::string dsorg;
  std::string dsntype;
  std::string edate;
  bool encrypted;
  int lrecl;
  std::string mgmtclass;
  bool migrated;
  int primary;
  std::string rdate;
  std::string recfm;
  int secondary;
  std::string spacu;
  std::string storclass;
  int usedp;
  int usedx;
  std::string volser;
  // ISPF shows the following fields, but we omit them since they require reading PDS directory (too slow)
  // int maxdb;
  // int members;
  // int useddb;
};

typedef struct
{
  std::string alcunit;   // Allocation Unit
  int blksize;           // Block Size
  int dirblk;            // Directory Blocks
  std::string dsorg;     // Data Set Organization
  int primary;           // Primary Space
  std::string recfm;     // Record Format
  int lrecl;             // Record Length
  std::string dataclass; // Data Class
  std::string unit;      // Device Type
  std::string dsntype;   // Data Set Type
  std::string mgntclass; // Management Class
  std::string dsname;    // Data Set Name
  int avgblk;            // Average Block Length
  int secondary;         // Secondary Space
  int size;              // Size
  std::string storclass; // Storage Class
  std::string vol;       // Volume Serial
} DS_ATTRIBUTES;

#ifdef SWIG
extern "C"
{
#endif
/**
 * @brief Read data from a z/OS data set
 *
 * @param zds data set returned attributes and error information
 * @param dsn data set name from which to read
 * @param response data read
 * @param encoding The desired encoding for the data set (optional)
 * @return int 0 for success; non zero otherwise
 */
int zds_read_from_dsn(ZDS *zds, const std::string &dsn, std::string &response);

/**
 * @brief Write data to a z/OS data set name
 *
 * @param zds data set returned attributes and error information
 * @param dsn data set name to write to
 * @param data data to write
 * @return int 0 for success; non zero otherwise
 */
int zds_write_to_dsn(ZDS *zds, const std::string &dsn, std::string &data);

/**
 * @brief Create a data set
 *
 * @param zds data set returned attributes and error information
 * @param dsn data set name to create
 * @param response messages from dynamic allocation (which may be present even when successful requests are made)
 * @return int 0 for success; non zero otherwise
 */
int zds_create_dsn(ZDS *zds, std::string dsn, DS_ATTRIBUTES attributes, std::string &response);

/**
 * @brief Delete a data set
 *
 * @param zds data set returned attributes and error information
 * @param dsn data set name to delete to
 * @return int 0 for success; non zero otherwise
 */
int zds_delete_dsn(ZDS *zds, std::string dsn);

/**
 * @brief Obtain list of members in a z/OS data set
 *
 * @param zds data set returned attributes and error information
 * @param dsn data set name to obtain attributes for
 * @param members populated list returned containing member names within a z/OS data set
 * @return int 0 for success; non zero otherwise
 */
int zds_list_members(ZDS *zds, std::string dsn, std::vector<ZDSMem> &members);

int zds_list_data_sets(ZDS *zds, std::string dsn, std::vector<ZDSEntry> &datasets, bool show_attributes = false);
#ifdef SWIG
}
#endif

/**
 * @brief Read data from a DDNAME
 *
 * @param zds data set returned attributes and error information
 * @param ddname ddname from which to read
 * @param response data read
 * @return int 0 for success; non zero otherwise
 */
int zds_read_from_dd(ZDS *zds, std::string ddname, std::string &response);

/**
 * @brief Write data to a DDNAME
 *
 * @param zds data set returned attributes and error information
 * @param ddname DDNAME to write to
 * @param data data to write
 * @return int 0 for success; non zero otherwise
 */
int zds_write_to_dd(ZDS *zds, std::string ddname, const std::string &data);

/**
 * @brief Create a data set
 *
 * @param zds data set returned attributes and error information
 * @param dsn data set name to create
 * @param response messages from dynamic allocation (which may be present even when successful requests are made)
 * @return int 0 for success; non zero otherwise
 */
int zds_create_dsn_fb(ZDS *zds, const std::string &dsn, std::string &response);

/**
 * @brief Create a data set
 *
 * @param zds data set returned attributes and error information
 * @param dsn data set name to create
 * @param response messages from dynamic allocation (which may be present even when successful requests are made)
 * @return int 0 for success; non zero otherwise
 */
int zds_create_dsn_vb(ZDS *zds, const std::string &dsn, std::string &response);

/**
 * @brief Create an ADATA data set
 *
 * @param zds data set returned attributes and error information
 * @param dsn data set name to create
 * @param response messages from dynamic allocation (which may be present even when successful requests are made)
 * @return int 0 for success; non zero otherwise
 */
int zds_create_dsn_adata(ZDS *zds, const std::string &dsn, std::string &response);

/**
 * @brief Create a loadlib data set
 *
 * @param zds data set returned attributes and error information
 * @param dsn data set name to create
 * @param response messages from dynamic allocation (which may be present even when successful requests are made)
 * @return int 0 for success; non zero otherwise
 */
int zds_create_dsn_loadlib(ZDS *zds, const std::string &dsn, std::string &response);

/**
 * @brief Delete a data set
 *
 * @param zds data set returned attributes and error information
 * @param dsn data set name to delete to
 * @return int 0 for success; non zero otherwise
 */
int zds_delete_dsn(ZDS *zds, std::string dsn);

int zdsReadDynalloc(std::string, std::string, std::string, std::string &); // NOTE(Kelosky): testing only

/**
 * @brief Read data from a z/OS data set in streaming mode
 *
 * @param zds data set returned attributes and error information
 * @param dsn data set name from which to read
 * @param pipe name of the output pipe
 * @param content_len pointer where the length of the data set contents will be stored
 * @return int 0 for success; non zero otherwise
 */
int zds_read_from_dsn_streamed(ZDS *zds, const std::string &dsn, const std::string &pipe, size_t *content_len);

/**
 * @brief Write data to a z/OS data set in streaming mode
 *
 * @param zds data set returned attributes and error information
 * @param dsn data set name to write to
 * @param pipe name of the input pipe
 * @param content_len pointer where the length of the data set contents will be stored
 * @return int 0 for success; non zero otherwise
 */
int zds_write_to_dsn_streamed(ZDS *zds, const std::string &dsn, const std::string &pipe, size_t *content_len);

#endif
