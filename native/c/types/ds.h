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
 * Generated from ds.ts - edit there instead
 */

#ifndef DS_TYPES_H
#define DS_TYPES_H

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include "common.h"

// Generated C++ structs from ds.ts

struct ListDatasetsRequest : public CommandRequest, public ListOptions, public ListDatasetOptions {
    std::string pattern;
    bool* attributes; // optional
};

struct ListDsMembersRequest : public CommandRequest, public ListOptions, public ListDatasetOptions {
    std::string dsname;
    bool* attributes; // optional
};

struct ReadDatasetRequest : public CommandRequest {
    std::string* encoding; // optional
    std::string* localEncoding; // optional
    std::string* volume; // optional
    std::string dsname;
    int* streamId; // optional
};

struct WriteDatasetRequest : public CommandRequest {
    std::string* encoding; // optional
    std::string* localEncoding; // optional
    std::string* etag; // optional
    std::string* volume; // optional
    std::string dsname;
    std::vector<uint8_t> data; // optional
    int* streamId; // optional
};

struct DeleteDatasetRequest : public CommandRequest {
    std::string dsname;
};

struct RestoreDatasetRequest : public CommandRequest {
    std::string dsname;
};

struct CreateDatasetRequest : public CommandRequest {
    std::string dsname;
    DatasetAttributes attributes;
};

struct CreateMemberRequest : public CommandRequest {
    std::string dsname;
};

struct WriteDatasetResponse : public CommandResponse {
    std::string dataset;
    std::string etag;
    // int
    int* contentLen; // optional
};

struct RestoreDatasetResponse : public CommandResponse {
};

struct ReadDatasetResponse : public CommandResponse {
    std::string* encoding; // optional
    std::string etag;
    std::string dataset;
    std::vector<uint8_t> data;
    // int
    int* contentLen; // optional
};

struct ListDatasetsResponse : public CommandResponse {
    std::vector<Dataset> items;
    // int
    int returnedRows;
};

struct ListDsMembersResponse : public CommandResponse {
    std::vector<DsMember> items;
    // int
    int returnedRows;
};

struct GenericDatasetResponse : public CommandResponse {
    std::string dsname;
};

#endif
