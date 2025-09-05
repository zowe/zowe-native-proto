#ifndef CMDS_TYPES_H
#define CMDS_TYPES_H

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include "common.h"

// Generated C++ structs from cmds.ts

struct IssueConsoleRequest : public CommandRequest {
    std::string commandText;
    std::string* consoleName; // optional
};

struct IssueTsoRequest : public CommandRequest {
    std::string commandText;
};

struct IssueUnixRequest : public CommandRequest {
    std::string commandText;
};

struct IssueConsoleResponse : public CommandResponse {
    std::string data;
};

struct IssueTsoResponse : public CommandResponse {
    std::string data;
};

struct IssueUnixResponse : public CommandResponse {
    std::string data;
};

#endif
