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
#include <sstream>
#endif
#include "ds.hpp"
#include "../extern/picojson.h"
#include "../worker.hpp"
#include "../../c/zds.hpp"
#include "../../c/zdstype.h"
#include "../../c/zbase64.h"
#include "../../c/zut.hpp"
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>

using picojson::value;

namespace
{
// Helper to initialize ZDS structure
void initZDS(ZDS *zds)
{
  memset(zds, 0, sizeof(ZDS));
}

// Helper to convert ZDS error to JSON error
picojson::value createErrorFromZDS(const ZDS *zds, const std::string &operation)
{
  picojson::object error;
  error["code"] = picojson::value(static_cast<double>(zds->diag.service_rc));
  error["message"] = picojson::value(operation + " failed");

  picojson::object data;
  if (zds->diag.e_msg[0] != '\0')
  {
    data["error"] = picojson::value(std::string(zds->diag.e_msg));
  }
  if (zds->diag.service_rsn != 0)
  {
    data["reasonCode"] = picojson::value(static_cast<double>(zds->diag.service_rsn));
  }
  error["data"] = picojson::value(data);

  return picojson::value(error);
}
} // namespace

picojson::value HandleReadDatasetRequest(const picojson::value &params)
{
  // Parse request parameters
  std::string dsname = params.contains("dsname") ? params.get("dsname").get<std::string>() : "";
  std::string encoding = params.contains("encoding") ? params.get("encoding").get<std::string>() : "IBM-1047";
  std::string localEncoding = params.contains("localEncoding") ? params.get("localEncoding").get<std::string>() : "";

  if (dsname.empty())
  {
    throw std::runtime_error("Missing required parameter: dsname");
  }

  ZDS zds;
  initZDS(&zds);
  zut_prepare_encoding(encoding, &zds.encoding_opts);
  // TODO Support streams and local encoding

  std::string response;
  int rc = zds_read_from_dsn(&zds, dsname, response);

  if (rc != 0)
  {
    picojson::value error = createErrorFromZDS(&zds, "Read dataset");
    std::string errorString = serializeJson(error);
    throw std::runtime_error(errorString);
  }

  // Convert response to base64 for JSON transport
  std::string base64Data = zbase64::encode(response);

  picojson::object result;
  result["encoding"] = picojson::value(encoding);
  std::stringstream etag_stream;
  etag_stream << std::hex << zut_calc_adler32_checksum(response) << std::dec;
  result["etag"] = picojson::value(etag_stream.str());
  result["dataset"] = picojson::value(dsname);
  result["data"] = picojson::value(base64Data);
  result["contentLen"] = picojson::value(static_cast<double>(response.length()));

  return picojson::value(result);
}

picojson::value HandleWriteDatasetRequest(const picojson::value &params)
{
  // Parse request parameters
  std::string dsname = params.contains("dsname") ? params.get("dsname").get<std::string>() : "";
  std::string encoding = params.contains("encoding") ? params.get("encoding").get<std::string>() : "IBM-1047";
  std::string localEncoding = params.contains("localEncoding") ? params.get("localEncoding").get<std::string>() : "";
  std::string etag = params.contains("etag") ? params.get("etag").get<std::string>() : "";
  std::string data = params.contains("data") ? params.get("data").get<std::string>() : "";

  if (dsname.empty())
  {
    throw std::runtime_error("Missing required parameter: dsname");
  }

  // Decode base64 data
  std::string decodedData = zbase64::decode(data);

  ZDS zds;
  initZDS(&zds);
  zut_prepare_encoding(encoding, &zds.encoding_opts);
  if (!etag.empty())
  {
    strcpy(zds.etag, etag.c_str());
  }
  // TODO Support streams and local encoding

  int rc = zds_write_to_dsn(&zds, dsname, decodedData);

  if (rc != 0)
  {
    picojson::value error = createErrorFromZDS(&zds, "Write dataset");
    std::string errorString = serializeJson(error);
    throw std::runtime_error(errorString);
  }

  picojson::object result;
  result["success"] = picojson::value(true);
  result["dataset"] = picojson::value(dsname);
  result["etag"] = picojson::value(std::string("")); // TODO: Implement etag support if needed
  result["contentLen"] = picojson::value(static_cast<double>(decodedData.length()));

  return picojson::value(result);
}

picojson::value HandleListDatasetsRequest(const picojson::value &params)
{
  // Parse request parameters
  std::string pattern = params.contains("pattern") ? params.get("pattern").get<std::string>() : "";

  if (pattern.empty())
  {
    throw std::runtime_error("Missing required parameter: pattern");
  }

  ZDS zds;
  initZDS(&zds);

  std::vector<ZDSEntry> entries;
  int rc = zds_list_data_sets(&zds, pattern + ".**", entries);

  if (rc != 0)
  {
    picojson::value error = createErrorFromZDS(&zds, "List datasets");
    std::string errorString = serializeJson(error);
    throw std::runtime_error(errorString);
  }

  picojson::array items;
  for (const auto &entry : entries)
  {
    picojson::object item;
    std::string trimmed_name = entry.name;
    trimmed_name.erase(trimmed_name.find_last_not_of(" ") + 1);
    item["name"] = picojson::value(trimmed_name);
    item["dsorg"] = picojson::value(entry.dsorg);
    item["volser"] = picojson::value(entry.volser);
    item["migr"] = picojson::value(entry.migr);
    items.push_back(picojson::value(item));
  }

  picojson::object result;
  result["items"] = picojson::value(items);
  result["returnedRows"] = picojson::value(static_cast<double>(entries.size()));

  return picojson::value(result);
}

picojson::value HandleListDsMembersRequest(const picojson::value &params)
{
  // Parse request parameters
  std::string dsname = params.contains("dsname") ? params.get("dsname").get<std::string>() : "";

  if (dsname.empty())
  {
    throw std::runtime_error("Missing required parameter: dsname");
  }

  ZDS zds;
  initZDS(&zds);

  std::vector<ZDSMem> members;
  int rc = zds_list_members(&zds, dsname, members);

  if (rc != 0)
  {
    picojson::value error = createErrorFromZDS(&zds, "List dataset members");
    std::string errorString = serializeJson(error);
    throw std::runtime_error(errorString);
  }

  picojson::array items;
  for (const auto &member : members)
  {
    picojson::object item;
    std::string trimmed_name = member.name;
    trimmed_name.erase(trimmed_name.find_last_not_of(" \t\r\n") + 1);
    item["name"] = picojson::value(trimmed_name);
    items.push_back(picojson::value(item));
  }

  picojson::object result;
  result["items"] = picojson::value(items);
  result["returnedRows"] = picojson::value(static_cast<double>(members.size()));

  return picojson::value(result);
}
