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

#include <iostream>
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
  std::string dsorg;
  std::string volser;
  bool migr;
};

/**
 * @brief Read data from a z/OS data set
 *
 * @param zds data set returned attributes and error information
 * @param dsn data set name from which to read
 * @param response data read
 * @param encoding The desired encoding for the data set (optional)
 * @return int 0 for success; non zero otherwise
 */
int zds_read_from_dsn(ZDS *zds, std::string dsn, std::string &response);

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
int zds_write_to_dd(ZDS *zds, std::string ddname, std::string &data);

/**
 * @brief Write data to a z/OS data set name
 *
 * @param zds data set returned attributes and error information
 * @param dsn data set name to write to
 * @param data data to write
 * @return int 0 for success; non zero otherwise
 */
int zds_write_to_dsn(ZDS *zds, std::string dsn, std::string &data, std::string etag_value = "");

/**
 * @brief Create a data set
 *
 * @param zds data set returned attributes and error information
 * @param dsn data set name to create
 * @param response messages from dynamic allocation (which may be present even when successful requests are made)
 * @return int 0 for success; non zero otherwise
 */
int zds_create_dsn(ZDS *zds, std::string dsn, std::string &response);

/**
 * @brief Create a data set
 *
 * @param zds data set returned attributes and error information
 * @param dsn data set name to create
 * @param response messages from dynamic allocation (which may be present even when successful requests are made)
 * @return int 0 for success; non zero otherwise
 */
int zds_create_dsn_vb(ZDS *zds, std::string dsn, std::string &response);

/**
 * @brief Create a data set
 *
 * @param zds data set returned attributes and error information
 * @param dsn data set name to create
 * @param response messages from dynamic allocation (which may be present even when successful requests are made)
 * @return int 0 for success; non zero otherwise
 */
int zds_create_dsn_adata(ZDS *zds, std::string dsn, std::string &response);

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

int zds_list_data_sets(ZDS *zds, std::string dsn, std::vector<ZDSEntry> &attributes);

int zdsReadDynalloc(std::string, std::string, std::string, std::string &); // NOTE(Kelosky): testing only

#endif
