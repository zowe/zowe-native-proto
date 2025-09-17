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
#include "common.h"

// Generated C++ structs from jobs.ts

struct ListJobsRequest : public CommandRequest, public ListOptions {
    std::string* owner; // optional
    std::string* prefix; // optional
    std::string* status; // optional
};

struct ListSpoolsRequest : public CommandRequest {
    std::string jobId;
};

struct ReadSpoolRequest : public CommandRequest {
    std::string* encoding; // optional
    std::string* localEncoding; // optional
    // int
    int spoolId;
    std::string jobId;
};

struct GetJclRequest : public CommandRequest {
    std::string jobId;
};

struct GetJobStatusRequest : public CommandRequest {
    std::string jobId;
};

struct SubmitJobRequest : public CommandRequest {
    std::string dsname;
};

struct SubmitUssRequest : public CommandRequest {
    std::string fspath;
};

struct SubmitJclRequest : public CommandRequest {
    std::string* encoding; // optional
    std::string* localEncoding; // optional
    std::vector<uint8_t> jcl;
};

struct CancelJobRequest : public CommandRequest {
    std::string jobId;
};

struct DeleteJobRequest : public CommandRequest {
    std::string jobId;
};

struct HoldJobRequest : public CommandRequest {
    std::string jobId;
};

struct ReleaseJobRequest : public CommandRequest {
    std::string jobId;
};

struct ListJobsResponse : public CommandResponse {
    std::vector<Job> items;
};

struct ListSpoolsResponse : public CommandResponse {
    std::vector<Spool> items;
};

struct GetJclResponse : public CommandResponse {
    std::string jobId;
    std::string data;
};

struct ReadSpoolResponse : public CommandResponse {
    std::string* encoding; // optional
    // int
    int spoolId;
    std::string jobId;
    std::vector<uint8_t> data;
};

struct GetJobStatusResponse : public CommandResponse, public Job {
};

struct SubmitJobResponse : public CommandResponse {
    std::string jobId;
    std::string dsname;
};

struct SubmitUssResponse : public CommandResponse {
    std::string jobId;
    std::string fspath;
};

struct SubmitJclResponse : public CommandResponse {
    std::string jobId;
};

struct DeleteJobResponse : public CommandResponse {
    std::string jobId;
};

struct CancelJobResponse : public CommandResponse {
    std::string jobId;
};

struct HoldJobResponse : public CommandResponse {
    std::string jobId;
};

struct ReleaseJobResponse : public CommandResponse {
    std::string jobId;
};

#endif
