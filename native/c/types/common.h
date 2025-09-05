#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <string>
#include <vector>
#include <map>
#include <cstdint>

// Generated C++ structs from common.ts

struct RpcNotification {
    std::string jsonrpc;
    std::string method;
    void* params; // optional
};

struct RpcRequest : public RpcNotification {
    void* params; // optional
    // int
    int id;
};

struct ErrorDetails {
    // int
    int code;
    std::string message;
    void* data; // optional
};

struct RpcResponse {
    std::string jsonrpc;
    void* result; // optional
    ErrorDetails* error; // optional
    // int
    int id;
};

struct CommandRequest {
    std::string command;
};

struct CommandResponse {
    bool success;
};

struct Dataset {
    std::string name;
    std::string dsorg;
    std::string volser;
    bool migr;
};

struct DatasetAttributes {
    std::string* alcunit; // optional
    // int
    int* blksize; // optional
    // int
    int* dirblk; // optional
    std::string* dsorg; // optional
    // int
    int primary;
    std::string* recfm; // optional
    // int
    int lrecl;
    std::string* dataclass; // optional
    std::string* unit; // optional
    std::string* dsntype; // optional
    std::string* mgntclass; // optional
    std::string* dsname; // optional
    // int
    int* avgblk; // optional
    // int
    int* secondary; // optional
    std::string* size; // optional
    std::string* storclass; // optional
    std::string* vol; // optional
};

struct DsMember {
    std::string name;
};

struct UssItem {
    std::string name;
    // int
    int links;
    std::string user;
    std::string group;
    // int
    int size;
    std::string* filetag; // optional
    std::string mtime;
    std::string mode;
};

struct Job {
    std::string id;
    std::string name;
    std::string status;
    std::string retcode;
};

struct Spool {
    // int
    int id;
    std::string ddname;
    std::string stepname;
    std::string dsname;
    std::string procstep;
};

struct StatusMessage {
    std::string status;
    std::string message;
    std::map<std::string, void*> data; // optional
};

struct IoserverOptions {
    // int
    int* numWorkers; // optional
    bool* verbose; // optional
};

struct ListOptions {
    // int
    int* maxItems; // optional
    // int
    int* responseTimeout; // optional
};

struct ListDatasetOptions {
    std::string* start; // optional
};

#endif
