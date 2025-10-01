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
 * Generated from cmds.ts - edit there instead
 */

#ifndef CMDS_TYPES_H
#define CMDS_TYPES_H

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include "../zstd.hpp"
#define ZJSON_ENABLE_STRUCT_SUPPORT
#include "../zjson.hpp"
#include "common.h"

// Generated C++ structs from cmds.ts

struct IssueConsoleRequest : CommandRequest
{
  std::string commandText;
  zstd::optional<std::string> consoleName;
};
ZJSON_DERIVE(IssueConsoleRequest, commandText, consoleName);

struct IssueTsoRequest : CommandRequest
{
  std::string commandText;
};
ZJSON_DERIVE(IssueTsoRequest, commandText);

struct IssueUnixRequest : CommandRequest
{
  std::string commandText;
};
ZJSON_DERIVE(IssueUnixRequest, commandText);

struct IssueConsoleResponse : CommandResponse
{
  std::string data;
};
ZJSON_DERIVE(IssueConsoleResponse, success, data);

struct IssueTsoResponse : CommandResponse
{
  std::string data;
};
ZJSON_DERIVE(IssueTsoResponse, success, data);

struct IssueUnixResponse : CommandResponse
{
  std::string data;
};
ZJSON_DERIVE(IssueUnixResponse, success, data);

#endif
