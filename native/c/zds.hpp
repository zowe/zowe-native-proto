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
#include "zamtypes.h"

extern const size_t MAX_DS_LENGTH;

struct ZDSMem
{
  std::string name;
  int vers;
  int mod;
  std::string c4date;
  std::string m4date;
  std::string mtime;
  int cnorc;
  int inorc;
  int mnorc;
  std::string user;
  bool sclm;
};

struct ZDSEntry
{
  std::string name;
  long long alloc;
  int allocx;
  int blksize;
  std::string cdate;
  std::string dataclass;
  uint16_t devtype;
  std::string dsorg;
  std::string dsntype;
  std::string edate;
  bool encrypted;
  int lrecl;
  std::string mgmtclass;
  bool migrated;
  bool multivolume;
  long long primary;
  std::string rdate;
  std::string recfm;
  long long secondary;
  std::string spacu;
  std::string storclass;
  int usedp;
  int usedx;
  std::string volser;
  std::vector<std::string> volsers;
  // ISPF shows the following fields, but we omit them since they require reading PDS directory (too slow)
  // Maximum dir. blocks, Used dir. blocks, Number of members
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

/**
 * Helper struct to hold data set attributes from DSCB lookup.
 */
struct DscbAttributes
{
  std::string recfm;
  int lrecl;
  bool is_asa;

  DscbAttributes()
      : recfm(""), lrecl(0), is_asa(false)
  {
  }
};

#ifdef SWIG
extern "C"
{
#endif

/**
 * @brief Options and results for data set copy operation
 */
struct ZDSCopyOptions
{
  // Input options
  bool replace;               // Replace like-named members in target (for PDS copy)
  bool delete_target_members; // Delete all members from target PDS before copying (PDS-to-PDS only)

  // Output results
  bool target_created; // Set to true if target data set was created
  bool member_created; // Set to true if target member was created

  ZDSCopyOptions()
      : replace(false), delete_target_members(false), target_created(false), member_created(false)
  {
  }
};

/**
 * @brief Copy a data set or member
 *
 * @param zds data set returned attributes and error information
 * @param dsn1 source data set name
 * @param dsn2 destination data set name
 * @param options copy options and results (optional, uses defaults if nullptr)
 * @return int 0 for success; non zero otherwise
 */
int zds_copy_dsn(ZDS *zds, const std::string &dsn1, const std::string &dsn2, ZDSCopyOptions *options = nullptr);

/**
 * @brief Check if a data set exists
 *
 * @param dsn data set name to check
 * @return true if it exists; false otherwise
 */
bool zds_dataset_exists(const std::string &dsn);

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
 * @brief Open a data set for using BPAM mode
 *
 * @param zds data set returned attributes and error information
 * @param dsname data set name to open
 * @param ioc IO_CTRL pointer to the data set
 * @return int 0 for success; non zero otherwise
 */
int zds_open_output_bpam(ZDS *zds, std::string dsname, IO_CTRL *&ioc);

/**
 * @brief Write data to a data set using BPAM mode
 *
 * @param zds data set returned attributes and error information
 * @param ioc IO_CTRL pointer to the data set
 * @param data data to write
 * @param asa_char optional ASA control character to prepend (default '\0' means none)
 * @return int 0 for success; non zero otherwise
 */
int zds_write_output_bpam(ZDS *zds, IO_CTRL *ioc, std::string &data, char asa_char = '\0');

/**
 * @brief Close a data set using BPAM mode
 *
 * @param zds data set returned attributes and error information
 * @param ioc IO_CTRL pointer to the data set
 * @return int 0 for success; non zero otherwise
 */
int zds_close_output_bpam(ZDS *zds, IO_CTRL *ioc);

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
 * @brief Rename a data set
 *
 * @param zds data set returned attributes and error information
 * @param dsn_Before data set name to rename
 * @param dsn_After new data set name
 * @return int 0 for success; non zero otherwise
 */
int zds_rename_dsn(ZDS *zds, std::string dsn_before, std::string dsn_after);

/**
 * @brief Rename a data set member
 *
 * @param zds
 * @param member_before
 * @param member_after
 * @return int 0 for success; non zero otherwise
 */
int zds_rename_members(ZDS *zds, const std::string &dsn, const std::string &member_before, const std::string &member_after);

/**
 * @brief Obtain list of members in a z/OS data set
 *
 * @param zds data set returned attributes and error information
 * @param dsn data set name to obtain attributes for
 * @param members populated list returned containing member names within a z/OS data set
 * @return int 0 for success; non zero otherwise
 */
int zds_list_members(ZDS *zds, std::string dsn, std::vector<ZDSMem> &members, bool show_attributes = false);

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
