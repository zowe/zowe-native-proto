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

struct ListDatasetsRequest : public CommandRequest {
    ListOptions listoptions;
    ListDatasetOptions listdatasetoptions;
    std::string pattern;
    zstd::optional<bool> attributes;
};
ZJSON_SERIALIZABLE(ListDatasetsRequest, ZJSON_FIELD(ListDatasetsRequest, listoptions).flatten(), ZJSON_FIELD(ListDatasetsRequest, listdatasetoptions).flatten(), ZJSON_FIELD(ListDatasetsRequest, pattern), ZJSON_FIELD(ListDatasetsRequest, attributes));

struct ListDsMembersRequest : public CommandRequest {
    ListOptions listoptions;
    ListDatasetOptions listdatasetoptions;
    std::string dsname;
    zstd::optional<bool> attributes;
};
ZJSON_SERIALIZABLE(ListDsMembersRequest, ZJSON_FIELD(ListDsMembersRequest, listoptions).flatten(), ZJSON_FIELD(ListDsMembersRequest, listdatasetoptions).flatten(), ZJSON_FIELD(ListDsMembersRequest, dsname), ZJSON_FIELD(ListDsMembersRequest, attributes));

struct ReadDatasetRequest : public CommandRequest {
    zstd::optional<std::string> encoding;
    zstd::optional<std::string> localEncoding;
    zstd::optional<std::string> volume;
    std::string dsname;
    zstd::optional<int> streamId;
};
ZJSON_DERIVE(ReadDatasetRequest, encoding, localEncoding, volume, dsname, streamId);

struct WriteDatasetRequest : public CommandRequest {
    zstd::optional<std::string> encoding;
    zstd::optional<std::string> localEncoding;
    zstd::optional<std::string> etag;
    zstd::optional<std::string> volume;
    std::string dsname;
    zstd::optional<std::vector<uint8_t>> data;
    zstd::optional<int> streamId;
};
ZJSON_DERIVE(WriteDatasetRequest, encoding, localEncoding, etag, volume, dsname, data, streamId);

struct DeleteDatasetRequest : public CommandRequest {
    std::string dsname;
};
ZJSON_DERIVE(DeleteDatasetRequest, dsname);

struct RestoreDatasetRequest : public CommandRequest {
    std::string dsname;
};
ZJSON_DERIVE(RestoreDatasetRequest, dsname);

struct CreateDatasetRequest : public CommandRequest {
    std::string dsname;
    DatasetAttributes attributes;
};
ZJSON_DERIVE(CreateDatasetRequest, dsname, attributes);

struct CreateMemberRequest : public CommandRequest {
    std::string dsname;
};
ZJSON_DERIVE(CreateMemberRequest, dsname);

struct WriteDatasetResponse : public CommandResponse {
    std::string dataset;
    std::string etag;
    // int
    zstd::optional<int> contentLen;
};
ZJSON_DERIVE(WriteDatasetResponse, dataset, etag, contentLen);

struct RestoreDatasetResponse : public CommandResponse {
};
ZJSON_DERIVE(RestoreDatasetResponse);

struct ReadDatasetResponse : public CommandResponse {
    zstd::optional<std::string> encoding;
    std::string etag;
    std::string dataset;
    std::vector<uint8_t> data;
    // int
    zstd::optional<int> contentLen;
};
ZJSON_DERIVE(ReadDatasetResponse, encoding, etag, dataset, data, contentLen);

struct ListDatasetsResponse : public CommandResponse {
    std::vector<Dataset> items;
    // int
    int returnedRows;
};
ZJSON_DERIVE(ListDatasetsResponse, items, returnedRows);

struct ListDsMembersResponse : public CommandResponse {
    std::vector<DsMember> items;
    // int
    int returnedRows;
};
ZJSON_DERIVE(ListDsMembersResponse, items, returnedRows);

struct GenericDatasetResponse : public CommandResponse {
    std::string dsname;
};
ZJSON_DERIVE(GenericDatasetResponse, dsname);

#endif
