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

#ifndef USS_TYPES_H
#define USS_TYPES_H

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include "common.h"

// Generated C++ structs from uss.ts

struct ListFilesRequest : public CommandRequest, public ListOptions {
    std::string fspath;
    bool* all; // optional
    bool* long; // optional
};

struct ReadFileRequest : public CommandRequest {
    std::string* encoding; // optional
    std::string* localEncoding; // optional
    std::string fspath;
    int* streamId; // optional
};

struct WriteFileRequest : public CommandRequest {
    std::string* encoding; // optional
    std::string* localEncoding; // optional
    std::string* etag; // optional
    std::string fspath;
    std::vector<uint8_t> data; // optional
    int* streamId; // optional
    // int
    int* contentLen; // optional
};

struct CreateFileRequest : public CommandRequest {
    std::string* permissions; // optional
    std::string fspath;
    bool* isDir; // optional
};

struct DeleteFileRequest : public CommandRequest {
    std::string fspath;
    bool recursive;
};

struct ChmodFileRequest : public CommandRequest {
    std::string mode;
    std::string fspath;
    bool recursive;
};

struct ChownFileRequest : public CommandRequest {
    std::string owner;
    std::string fspath;
    bool recursive;
};

struct ChtagFileRequest : public CommandRequest {
    std::string fspath;
    std::string tag;
    bool recursive;
};

struct GenericFileResponse : public CommandResponse {
    std::string fspath;
};

struct ReadFileResponse : public CommandResponse {
    std::string* encoding; // optional
    std::string etag;
    std::string fspath;
    std::vector<uint8_t> data; // optional
    // int
    int* contentLen; // optional
};

struct WriteFileResponse : public GenericFileResponse {
    std::string etag;
    bool created;
    // int
    int* contentLen; // optional
};

struct ListFilesResponse : public CommandResponse {
    std::vector<UssItem> items;
    // int
    int returnedRows;
};

#endif
