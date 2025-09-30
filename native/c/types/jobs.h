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
 * Generated from jobs.ts - edit there instead
 */

#ifndef JOBS_TYPES_H
#define JOBS_TYPES_H

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include "../zstd.hpp"
#include "../zjson.hpp"
#include "common.h"

// Generated C++ structs from jobs.ts

struct ListJobsRequest : CommandRequest
{
    ListOptions listOptions;
    zstd::optional<std::string> owner;
    zstd::optional<std::string> prefix;
    zstd::optional<std::string> status;
};
ZJSON_SERIALIZABLE(ListJobsRequest, ZJSON_FIELD(ListJobsRequest, listOptions).flatten(), ZJSON_FIELD(ListJobsRequest, owner), ZJSON_FIELD(ListJobsRequest, prefix), ZJSON_FIELD(ListJobsRequest, status));

struct ListSpoolsRequest : CommandRequest
{
    std::string jobId;
};
ZJSON_DERIVE(ListSpoolsRequest, jobId);

struct ReadSpoolRequest : CommandRequest
{
    zstd::optional<std::string> encoding;
    zstd::optional<std::string> localEncoding;
    // int
    int spoolId;
    std::string jobId;
};
ZJSON_DERIVE(ReadSpoolRequest, encoding, localEncoding, spoolId, jobId);

struct GetJclRequest : CommandRequest
{
    std::string jobId;
};
ZJSON_DERIVE(GetJclRequest, jobId);

struct GetJobStatusRequest : CommandRequest
{
    std::string jobId;
};
ZJSON_DERIVE(GetJobStatusRequest, jobId);

struct SubmitJobRequest : CommandRequest
{
    std::string dsname;
};
ZJSON_DERIVE(SubmitJobRequest, dsname);

struct SubmitUssRequest : CommandRequest
{
    std::string fspath;
};
ZJSON_DERIVE(SubmitUssRequest, fspath);

struct SubmitJclRequest : CommandRequest
{
    zstd::optional<std::string> encoding;
    zstd::optional<std::string> localEncoding;
    std::vector<uint8_t> jcl;
};
ZJSON_DERIVE(SubmitJclRequest, encoding, localEncoding, jcl);

struct CancelJobRequest : CommandRequest
{
    std::string jobId;
};
ZJSON_DERIVE(CancelJobRequest, jobId);

struct DeleteJobRequest : CommandRequest
{
    std::string jobId;
};
ZJSON_DERIVE(DeleteJobRequest, jobId);

struct HoldJobRequest : CommandRequest
{
    std::string jobId;
};
ZJSON_DERIVE(HoldJobRequest, jobId);

struct ReleaseJobRequest : CommandRequest
{
    std::string jobId;
};
ZJSON_DERIVE(ReleaseJobRequest, jobId);

struct ListJobsResponse : CommandResponse
{
    std::vector<Job> items;
};
ZJSON_DERIVE(ListJobsResponse, success, items);

struct ListSpoolsResponse : CommandResponse
{
    std::vector<Spool> items;
};
ZJSON_DERIVE(ListSpoolsResponse, success, items);

struct GetJclResponse : CommandResponse
{
    std::string jobId;
    std::string data;
};
ZJSON_DERIVE(GetJclResponse, success, jobId, data);

struct ReadSpoolResponse : CommandResponse
{
    zstd::optional<std::string> encoding;
    // int
    int spoolId;
    std::string jobId;
    std::vector<uint8_t> data;
};
ZJSON_DERIVE(ReadSpoolResponse, success, encoding, spoolId, jobId, data);

struct GetJobStatusResponse : CommandResponse
{
    Job job;
};
ZJSON_SERIALIZABLE(GetJobStatusResponse, ZJSON_FIELD(GetJobStatusResponse, job).flatten());

struct SubmitJobResponse : CommandResponse
{
    std::string jobId;
    std::string dsname;
};
ZJSON_DERIVE(SubmitJobResponse, success, jobId, dsname);

struct SubmitUssResponse : CommandResponse
{
    std::string jobId;
    std::string fspath;
};
ZJSON_DERIVE(SubmitUssResponse, success, jobId, fspath);

struct SubmitJclResponse : CommandResponse
{
    std::string jobId;
};
ZJSON_DERIVE(SubmitJclResponse, success, jobId);

struct DeleteJobResponse : CommandResponse
{
    std::string jobId;
};
ZJSON_DERIVE(DeleteJobResponse, success, jobId);

struct CancelJobResponse : CommandResponse
{
    std::string jobId;
};
ZJSON_DERIVE(CancelJobResponse, success, jobId);

struct HoldJobResponse : CommandResponse
{
    std::string jobId;
};
ZJSON_DERIVE(HoldJobResponse, success, jobId);

struct ReleaseJobResponse : CommandResponse
{
    std::string jobId;
};
ZJSON_DERIVE(ReleaseJobResponse, success, jobId);

#endif
