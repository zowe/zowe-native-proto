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
 * Generated from common.ts - edit there instead
 */

#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include "../zstd.hpp"
#define ZJSON_ENABLE_STRUCT_SUPPORT
#include "../zjson.hpp"

// Generated C++ structs from common.ts

struct RpcNotification
{
  std::string jsonrpc;
  std::string method;
  zstd::optional<zjson::Value> params;
};
ZJSON_DERIVE(RpcNotification, jsonrpc, method, params);

struct RpcRequest : RpcNotification
{
  // int
  int id;
};
ZJSON_DERIVE(RpcRequest, jsonrpc, method, params, id);

struct ErrorDetails
{
  // int
  int code;
  std::string message;
  zstd::optional<zjson::Value> data;
};
ZJSON_DERIVE(ErrorDetails, code, message, data);

struct RpcResponse
{
  std::string jsonrpc;
  zstd::optional<zjson::Value> result;
  zstd::optional<ErrorDetails> error;
  // int
  int id;
};
ZJSON_DERIVE(RpcResponse, jsonrpc, result, error, id);

struct CommandRequest
{
  std::string command;
};
ZJSON_DERIVE(CommandRequest, command);

struct CommandResponse
{
  bool success;
};
ZJSON_DERIVE(CommandResponse, success);

struct Dataset
{
  std::string name;
  std::string dsorg;
  std::string volser;
  bool migr;
};
ZJSON_DERIVE(Dataset, name, dsorg, volser, migr);

struct DatasetAttributes
{
  zstd::optional<std::string> alcunit;
  // int
  zstd::optional<int> blksize;
  // int
  zstd::optional<int> dirblk;
  zstd::optional<std::string> dsorg;
  // int
  int primary;
  zstd::optional<std::string> recfm;
  // int
  int lrecl;
  zstd::optional<std::string> dataclass;
  zstd::optional<std::string> unit;
  zstd::optional<std::string> dsntype;
  zstd::optional<std::string> mgntclass;
  zstd::optional<std::string> dsname;
  // int
  zstd::optional<int> avgblk;
  // int
  zstd::optional<int> secondary;
  zstd::optional<std::string> size;
  zstd::optional<std::string> storclass;
  zstd::optional<std::string> vol;
};
ZJSON_DERIVE(DatasetAttributes, alcunit, blksize, dirblk, dsorg, primary, recfm, lrecl, dataclass, unit, dsntype, mgntclass, dsname, avgblk, secondary, size, storclass, vol);

struct DsMember
{
  std::string name;
};
ZJSON_DERIVE(DsMember, name);

struct UssItem
{
  std::string name;
  // int
  int links;
  std::string user;
  std::string group;
  // int
  int size;
  zstd::optional<std::string> filetag;
  std::string mtime;
  std::string mode;
};
ZJSON_DERIVE(UssItem, name, links, user, group, size, filetag, mtime, mode);

struct Job
{
  std::string id;
  std::string name;
  std::string status;
  std::string retcode;
};
ZJSON_DERIVE(Job, id, name, status, retcode);

struct Spool
{
  // int
  int id;
  std::string ddname;
  std::string stepname;
  std::string dsname;
  std::string procstep;
};
ZJSON_DERIVE(Spool, id, ddname, stepname, dsname, procstep);

struct StatusMessage
{
  std::string status;
  std::string message;
  zstd::optional<zjson::Value> data;
};
ZJSON_DERIVE(StatusMessage, status, message, data);

struct ListOptions
{
  // int
  zstd::optional<int> maxItems;
  // int
  zstd::optional<int> responseTimeout;
};
ZJSON_DERIVE(ListOptions, maxItems, responseTimeout);

struct ListDatasetOptions
{
  zstd::optional<std::string> start;
};
ZJSON_DERIVE(ListDatasetOptions, start);

#endif
