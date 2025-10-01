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

/*
 * AUTO-GENERATED - DO NOT EDIT
 * Generated from uss.ts - edit there instead
 */

#ifndef USS_TYPES_H
#define USS_TYPES_H

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include "../zstd.hpp"
#define ZJSON_ENABLE_STRUCT_SUPPORT
#include "../zjson.hpp"
#include "common.h"

// Generated C++ structs from uss.ts

struct ListFilesRequest : CommandRequest
{
  ListOptions listOptions;
  std::string fspath;
  zstd::optional<bool> all;
  zstd::optional<bool> long;
};
ZJSON_SERIALIZABLE(ListFilesRequest, ZJSON_FIELD(ListFilesRequest, listOptions).flatten(), ZJSON_FIELD(ListFilesRequest, fspath), ZJSON_FIELD(ListFilesRequest, all), ZJSON_FIELD(ListFilesRequest, long));

struct ReadFileRequest : CommandRequest
{
  zstd::optional<std::string> encoding;
  zstd::optional<std::string> localEncoding;
  std::string fspath;
  zstd::optional<int> streamId;
};
ZJSON_DERIVE(ReadFileRequest, encoding, localEncoding, fspath, streamId);

struct WriteFileRequest : CommandRequest
{
  zstd::optional<std::string> encoding;
  zstd::optional<std::string> localEncoding;
  zstd::optional<std::string> etag;
  std::string fspath;
  zstd::optional<std::vector<uint8_t>> data;
  zstd::optional<int> streamId;
  // int
  zstd::optional<int> contentLen;
};
ZJSON_DERIVE(WriteFileRequest, encoding, localEncoding, etag, fspath, data, streamId, contentLen);

struct CreateFileRequest : CommandRequest
{
  zstd::optional<std::string> permissions;
  std::string fspath;
  zstd::optional<bool> isDir;
};
ZJSON_DERIVE(CreateFileRequest, permissions, fspath, isDir);

struct DeleteFileRequest : CommandRequest
{
  std::string fspath;
  bool recursive;
};
ZJSON_DERIVE(DeleteFileRequest, fspath, recursive);

struct ChmodFileRequest : CommandRequest
{
  std::string mode;
  std::string fspath;
  bool recursive;
};
ZJSON_DERIVE(ChmodFileRequest, mode, fspath, recursive);

struct ChownFileRequest : CommandRequest
{
  std::string owner;
  std::string fspath;
  bool recursive;
};
ZJSON_DERIVE(ChownFileRequest, owner, fspath, recursive);

struct ChtagFileRequest : CommandRequest
{
  std::string fspath;
  std::string tag;
  bool recursive;
};
ZJSON_DERIVE(ChtagFileRequest, fspath, tag, recursive);

struct GenericFileResponse : CommandResponse
{
  std::string fspath;
};
ZJSON_DERIVE(GenericFileResponse, success, fspath);

struct ReadFileResponse : CommandResponse
{
  zstd::optional<std::string> encoding;
  std::string etag;
  std::string fspath;
  std::vector<uint8_t> data;
  // int
  zstd::optional<int> contentLen;
};
ZJSON_DERIVE(ReadFileResponse, success, encoding, etag, fspath, data, contentLen);

struct WriteFileResponse : GenericFileResponse
{
  std::string etag;
  bool created;
  // int
  zstd::optional<int> contentLen;
};
ZJSON_DERIVE(WriteFileResponse, success, fspath, etag, created, contentLen);

struct ListFilesResponse : CommandResponse
{
  std::vector<UssItem> items;
  // int
  int returnedRows;
};
ZJSON_DERIVE(ListFilesResponse, success, items, returnedRows);

#endif
