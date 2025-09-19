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
#include "../../c/zjson.hpp"
#include "../worker.hpp"
#include "../../c/zds.hpp"
#include "../../c/zdstype.h"
#include "../../c/zbase64.h"
#include "../../c/zut.hpp"
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>

using zjson::Value;

namespace
{
// Helper to initialize ZDS structure
void initZDS(ZDS *zds)
{
  memset(zds, 0, sizeof(ZDS));
}

// Helper to convert ZDS error to JSON error
zjson::Value createErrorFromZDS(const ZDS *zds, const std::string &operation)
{
  zjson::Value error = zjson::Value::create_object();
  error.add_to_object("code", zjson::Value(zds->diag.service_rc));
  error.add_to_object("message", zjson::Value(operation + " failed"));

  zjson::Value data = zjson::Value::create_object();
  if (zds->diag.e_msg[0] != '\0')
  {
    data.add_to_object("error", zjson::Value(std::string(zds->diag.e_msg)));
  }
  if (zds->diag.service_rsn != 0)
  {
    data.add_to_object("reasonCode", zjson::Value(zds->diag.service_rsn));
  }
  error.add_to_object("data", data);

  return error;
}
} // namespace

zjson::Value HandleReadDatasetRequest(const zjson::Value &params)
{
  // Parse request parameters
  if (!params.is_object())
  {
    throw std::runtime_error("Invalid parameters: expected object");
  }
  const auto &obj = params.as_object();

  auto dsname_it = obj.find("dsname");
  std::string dsname = (dsname_it != obj.end() && dsname_it->second.is_string()) ? dsname_it->second.as_string() : "";

  auto encoding_it = obj.find("encoding");
  std::string encoding = (encoding_it != obj.end() && encoding_it->second.is_string()) ? encoding_it->second.as_string() : "IBM-1047";

  auto localEncoding_it = obj.find("localEncoding");
  std::string localEncoding = (localEncoding_it != obj.end() && localEncoding_it->second.is_string()) ? localEncoding_it->second.as_string() : "";

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
    zjson::Value error = createErrorFromZDS(&zds, "Read dataset");
    std::string errorString = serializeJson(error);
    throw std::runtime_error(errorString);
  }

  // Convert response to base64 for JSON transport
  std::string base64Data = zbase64::encode(response);

  zjson::Value result = zjson::Value::create_object();
  result.add_to_object("encoding", zjson::Value(encoding));
  std::stringstream etag_stream;
  etag_stream << std::hex << zut_calc_adler32_checksum(response) << std::dec;
  result.add_to_object("etag", zjson::Value(etag_stream.str()));
  result.add_to_object("dataset", zjson::Value(dsname));
  result.add_to_object("data", zjson::Value(base64Data));
  result.add_to_object("contentLen", zjson::Value(static_cast<int>(response.length())));

  return result;
}

zjson::Value HandleWriteDatasetRequest(const zjson::Value &params)
{
  // Parse request parameters
  if (!params.is_object())
  {
    throw std::runtime_error("Invalid parameters: expected object");
  }
  const auto &obj = params.as_object();

  auto dsname_it = obj.find("dsname");
  std::string dsname = (dsname_it != obj.end() && dsname_it->second.is_string()) ? dsname_it->second.as_string() : "";

  auto encoding_it = obj.find("encoding");
  std::string encoding = (encoding_it != obj.end() && encoding_it->second.is_string()) ? encoding_it->second.as_string() : "IBM-1047";

  auto localEncoding_it = obj.find("localEncoding");
  std::string localEncoding = (localEncoding_it != obj.end() && localEncoding_it->second.is_string()) ? localEncoding_it->second.as_string() : "";

  auto etag_it = obj.find("etag");
  std::string etag = (etag_it != obj.end() && etag_it->second.is_string()) ? etag_it->second.as_string() : "";

  auto data_it = obj.find("data");
  std::string data = (data_it != obj.end() && data_it->second.is_string()) ? data_it->second.as_string() : "";

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
    zjson::Value error = createErrorFromZDS(&zds, "Write dataset");
    std::string errorString = serializeJson(error);
    throw std::runtime_error(errorString);
  }

  zjson::Value result = zjson::Value::create_object();
  result.add_to_object("success", zjson::Value(true));
  result.add_to_object("dataset", zjson::Value(dsname));
  result.add_to_object("etag", zjson::Value(std::string(""))); // TODO: Implement etag support if needed
  result.add_to_object("contentLen", zjson::Value(static_cast<int>(decodedData.length())));

  return result;
}

zjson::Value HandleListDatasetsRequest(const zjson::Value &params)
{
  // Parse request parameters
  if (!params.is_object())
  {
    throw std::runtime_error("Invalid parameters: expected object");
  }
  const auto &obj = params.as_object();

  auto pattern_it = obj.find("pattern");
  std::string pattern = (pattern_it != obj.end() && pattern_it->second.is_string()) ? pattern_it->second.as_string() : "";

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
    zjson::Value error = createErrorFromZDS(&zds, "List datasets");
    std::string errorString = serializeJson(error);
    throw std::runtime_error(errorString);
  }

  zjson::Value items = zjson::Value::create_array();
  for (const auto &entry : entries)
  {
    zjson::Value item = zjson::Value::create_object();
    std::string trimmed_name = entry.name;
    trimmed_name.erase(trimmed_name.find_last_not_of(" ") + 1);
    item.add_to_object("name", zjson::Value(trimmed_name));
    item.add_to_object("dsorg", zjson::Value(entry.dsorg));
    item.add_to_object("volser", zjson::Value(entry.volser));
    item.add_to_object("migr", zjson::Value(entry.migr));
    items.add_to_array(item);
  }

  zjson::Value result = zjson::Value::create_object();
  result.add_to_object("items", items);
  result.add_to_object("returnedRows", zjson::Value(static_cast<int>(entries.size())));

  return result;
}

zjson::Value HandleListDsMembersRequest(const zjson::Value &params)
{
  // Parse request parameters
  if (!params.is_object())
  {
    throw std::runtime_error("Invalid parameters: expected object");
  }
  const auto &obj = params.as_object();

  auto dsname_it = obj.find("dsname");
  std::string dsname = (dsname_it != obj.end() && dsname_it->second.is_string()) ? dsname_it->second.as_string() : "";

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
    zjson::Value error = createErrorFromZDS(&zds, "List dataset members");
    std::string errorString = serializeJson(error);
    throw std::runtime_error(errorString);
  }

  zjson::Value items = zjson::Value::create_array();
  for (const auto &member : members)
  {
    zjson::Value item = zjson::Value::create_object();
    std::string trimmed_name = member.name;
    trimmed_name.erase(trimmed_name.find_last_not_of(" \t\r\n") + 1);
    item.add_to_object("name", zjson::Value(trimmed_name));
    items.add_to_array(item);
  }

  zjson::Value result = zjson::Value::create_object();
  result.add_to_object("items", items);
  result.add_to_object("returnedRows", zjson::Value(static_cast<int>(members.size())));

  return result;
}
