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
#include "../zstd.hpp"
#include "../zjson.hpp"
#include "common.h"

// Generated C++ structs from ds.ts

struct ListDatasetsRequest : CommandRequest
{
    ListOptions listOptions;
    ListDatasetOptions listDatasetOptions;
    std::string pattern;
    zstd::optional<bool> attributes;
};
ZJSON_SERIALIZABLE(ListDatasetsRequest, ZJSON_FIELD(ListDatasetsRequest, listOptions).flatten(), ZJSON_FIELD(ListDatasetsRequest, listDatasetOptions).flatten(), ZJSON_FIELD(ListDatasetsRequest, pattern), ZJSON_FIELD(ListDatasetsRequest, attributes));

struct ListDsMembersRequest : CommandRequest
{
    ListOptions listOptions;
    ListDatasetOptions listDatasetOptions;
    std::string dsname;
    zstd::optional<bool> attributes;
};
ZJSON_SERIALIZABLE(ListDsMembersRequest, ZJSON_FIELD(ListDsMembersRequest, listOptions).flatten(), ZJSON_FIELD(ListDsMembersRequest, listDatasetOptions).flatten(), ZJSON_FIELD(ListDsMembersRequest, dsname), ZJSON_FIELD(ListDsMembersRequest, attributes));

struct ReadDatasetRequest : CommandRequest
{
    zstd::optional<std::string> encoding;
    zstd::optional<std::string> localEncoding;
    zstd::optional<std::string> volume;
    std::string dsname;
    zstd::optional<int> streamId;
};
ZJSON_DERIVE(ReadDatasetRequest, encoding, localEncoding, volume, dsname, streamId);

struct WriteDatasetRequest : CommandRequest
{
    zstd::optional<std::string> encoding;
    zstd::optional<std::string> localEncoding;
    zstd::optional<std::string> etag;
    zstd::optional<std::string> volume;
    std::string dsname;
    zstd::optional<std::vector<uint8_t>> data;
    zstd::optional<int> streamId;
};
ZJSON_DERIVE(WriteDatasetRequest, encoding, localEncoding, etag, volume, dsname, data, streamId);

struct DeleteDatasetRequest : CommandRequest
{
    std::string dsname;
};
ZJSON_DERIVE(DeleteDatasetRequest, dsname);

struct RestoreDatasetRequest : CommandRequest
{
    std::string dsname;
};
ZJSON_DERIVE(RestoreDatasetRequest, dsname);

struct CreateDatasetRequest : CommandRequest
{
    std::string dsname;
    DatasetAttributes attributes;
};
ZJSON_DERIVE(CreateDatasetRequest, dsname, attributes);

struct CreateMemberRequest : CommandRequest
{
    std::string dsname;
};
ZJSON_DERIVE(CreateMemberRequest, dsname);

struct WriteDatasetResponse : CommandResponse
{
    std::string dataset;
    std::string etag;
    // int
    zstd::optional<int> contentLen;
};
ZJSON_DERIVE(WriteDatasetResponse, success, dataset, etag, contentLen);

struct RestoreDatasetResponse : CommandResponse
{
};
ZJSON_DERIVE(RestoreDatasetResponse, success);

struct ReadDatasetResponse : CommandResponse
{
    zstd::optional<std::string> encoding;
    std::string etag;
    std::string dataset;
    std::vector<uint8_t> data;
    // int
    zstd::optional<int> contentLen;
};
ZJSON_DERIVE(ReadDatasetResponse, success, encoding, etag, dataset, data, contentLen);

struct ListDatasetsResponse : CommandResponse
{
    std::vector<Dataset> items;
    // int
    int returnedRows;
};
ZJSON_DERIVE(ListDatasetsResponse, success, items, returnedRows);

struct ListDsMembersResponse : CommandResponse
{
    std::vector<DsMember> items;
    // int
    int returnedRows;
};
ZJSON_DERIVE(ListDsMembersResponse, success, items, returnedRows);

struct GenericDatasetResponse : CommandResponse
{
    std::string dsname;
};
ZJSON_DERIVE(GenericDatasetResponse, success, dsname);

#endif
