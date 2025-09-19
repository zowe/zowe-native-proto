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
#include "../zjson.hpp"
#include "common.h"

// Generated C++ structs from cmds.ts

struct IssueConsoleRequest : public CommandRequest {
    std::string commandText;
    zstd::optional<std::string> consoleName;
};
ZJSON_DERIVE(IssueConsoleRequest, commandText, consoleName);

struct IssueTsoRequest : public CommandRequest {
    std::string commandText;
};
ZJSON_DERIVE(IssueTsoRequest, commandText);

struct IssueUnixRequest : public CommandRequest {
    std::string commandText;
};
ZJSON_DERIVE(IssueUnixRequest, commandText);

struct IssueConsoleResponse : public CommandResponse {
    std::string data;
};
ZJSON_DERIVE(IssueConsoleResponse, data);

struct IssueTsoResponse : public CommandResponse {
    std::string data;
};
ZJSON_DERIVE(IssueTsoResponse, data);

struct IssueUnixResponse : public CommandResponse {
    std::string data;
};
ZJSON_DERIVE(IssueUnixResponse, data);

#endif
