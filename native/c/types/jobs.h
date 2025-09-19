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

struct ListJobsRequest : public CommandRequest {
    ListOptions listoptions;
    zstd::optional<std::string> owner;
    zstd::optional<std::string> prefix;
    zstd::optional<std::string> status;
};
ZJSON_SERIALIZABLE(ListJobsRequest, ZJSON_FIELD(ListJobsRequest, listoptions).flatten(), ZJSON_FIELD(ListJobsRequest, owner), ZJSON_FIELD(ListJobsRequest, prefix), ZJSON_FIELD(ListJobsRequest, status));

struct ListSpoolsRequest : public CommandRequest {
    std::string jobId;
};
ZJSON_DERIVE(ListSpoolsRequest, jobId);

struct ReadSpoolRequest : public CommandRequest {
    zstd::optional<std::string> encoding;
    zstd::optional<std::string> localEncoding;
    // int
    int spoolId;
    std::string jobId;
};
ZJSON_DERIVE(ReadSpoolRequest, encoding, localEncoding, spoolId, jobId);

struct GetJclRequest : public CommandRequest {
    std::string jobId;
};
ZJSON_DERIVE(GetJclRequest, jobId);

struct GetJobStatusRequest : public CommandRequest {
    std::string jobId;
};
ZJSON_DERIVE(GetJobStatusRequest, jobId);

struct SubmitJobRequest : public CommandRequest {
    std::string dsname;
};
ZJSON_DERIVE(SubmitJobRequest, dsname);

struct SubmitUssRequest : public CommandRequest {
    std::string fspath;
};
ZJSON_DERIVE(SubmitUssRequest, fspath);

struct SubmitJclRequest : public CommandRequest {
    zstd::optional<std::string> encoding;
    zstd::optional<std::string> localEncoding;
    std::vector<uint8_t> jcl;
};
ZJSON_DERIVE(SubmitJclRequest, encoding, localEncoding, jcl);

struct CancelJobRequest : public CommandRequest {
    std::string jobId;
};
ZJSON_DERIVE(CancelJobRequest, jobId);

struct DeleteJobRequest : public CommandRequest {
    std::string jobId;
};
ZJSON_DERIVE(DeleteJobRequest, jobId);

struct HoldJobRequest : public CommandRequest {
    std::string jobId;
};
ZJSON_DERIVE(HoldJobRequest, jobId);

struct ReleaseJobRequest : public CommandRequest {
    std::string jobId;
};
ZJSON_DERIVE(ReleaseJobRequest, jobId);

struct ListJobsResponse : public CommandResponse {
    std::vector<Job> items;
};
ZJSON_DERIVE(ListJobsResponse, items);

struct ListSpoolsResponse : public CommandResponse {
    std::vector<Spool> items;
};
ZJSON_DERIVE(ListSpoolsResponse, items);

struct GetJclResponse : public CommandResponse {
    std::string jobId;
    std::string data;
};
ZJSON_DERIVE(GetJclResponse, jobId, data);

struct ReadSpoolResponse : public CommandResponse {
    zstd::optional<std::string> encoding;
    // int
    int spoolId;
    std::string jobId;
    std::vector<uint8_t> data;
};
ZJSON_DERIVE(ReadSpoolResponse, encoding, spoolId, jobId, data);

struct GetJobStatusResponse : public CommandResponse {
    Job job;
};
ZJSON_SERIALIZABLE(GetJobStatusResponse, ZJSON_FIELD(GetJobStatusResponse, job).flatten());

struct SubmitJobResponse : public CommandResponse {
    std::string jobId;
    std::string dsname;
};
ZJSON_DERIVE(SubmitJobResponse, jobId, dsname);

struct SubmitUssResponse : public CommandResponse {
    std::string jobId;
    std::string fspath;
};
ZJSON_DERIVE(SubmitUssResponse, jobId, fspath);

struct SubmitJclResponse : public CommandResponse {
    std::string jobId;
};
ZJSON_DERIVE(SubmitJclResponse, jobId);

struct DeleteJobResponse : public CommandResponse {
    std::string jobId;
};
ZJSON_DERIVE(DeleteJobResponse, jobId);

struct CancelJobResponse : public CommandResponse {
    std::string jobId;
};
ZJSON_DERIVE(CancelJobResponse, jobId);

struct HoldJobResponse : public CommandResponse {
    std::string jobId;
};
ZJSON_DERIVE(HoldJobResponse, jobId);

struct ReleaseJobResponse : public CommandResponse {
    std::string jobId;
};
ZJSON_DERIVE(ReleaseJobResponse, jobId);

#endif
