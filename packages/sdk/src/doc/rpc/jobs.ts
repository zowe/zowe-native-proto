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

import type { B64String } from "../types";
import type * as common from "./common";

export interface CancelJobRequest extends common.CommandRequest<"cancelJob"> {
    /**
     * Job ID to cancel
     */
    jobId: string;
}

export type CancelJobResponse = common.CommandResponse;

export interface DeleteJobRequest extends common.CommandRequest<"deleteJob"> {
    /**
     * Job ID to cancel
     */
    jobId: string;
}

export type DeleteJobResponse = common.CommandResponse;

export interface GetJclRequest extends common.CommandRequest<"getJcl"> {
    /**
     * Job ID to get JCL for
     */
    jobId: string;
}

export interface GetJclResponse extends common.CommandResponse {
    /**
     * JCL contents
     */
    data: string;
}

export interface GetJobStatusRequest extends common.CommandRequest<"getJobStatus"> {
    /**
     * Job ID to get status for
     */
    jobId: string;
}

export interface GetJobStatusResponse extends common.CommandResponse, common.Job {}

export interface HoldJobRequest extends common.CommandRequest<"holdJob"> {
    /**
     * Job ID to hold
     */
    jobId: string;
}

export type HoldJobResponse = common.CommandResponse;

export interface ListJobsRequest extends common.CommandRequest<"listJobs">, common.ListOptions {
    /**
     * Filter jobs by matching job owner (optional)
     */
    owner?: string;
    /**
     * Filter jobs by prefix (optional)
     */
    prefix?: string;
    /**
     * Filter jobs by status (optional)
     */
    status?: string;
}

export interface ListJobsResponse extends common.CommandResponse {
    /**
     * List of returned jobs
     */
    items: common.Job[];
}

export interface ListSpoolsRequest extends common.CommandRequest<"listSpools"> {
    /**
     * Job ID to list spools for
     */
    jobId: string;
}

export interface ListSpoolsResponse extends common.CommandResponse {
    /**
     * List of returned spools
     */
    items: common.Spool[];
}

export interface ReadSpoolRequest extends common.CommandRequest<"readSpool"> {
    /**
     * Desired encoding for the spool file (optional)
     */
    encoding?: string;
    /**
     * Source encoding of the spool file content (optional, defaults to UTF-8)
     */
    localEncoding?: string;
    /**
     * Spool ID to read under the given job ID
     */
    spoolId: number;
    /**
     * Job ID with spools to read from
     */
    jobId: string;
}

export interface ReadSpoolResponse extends common.CommandResponse {
    /**
     * Desired encoding for the spool file (optional)
     */
    encoding?: string;
    /**
     * Spool ID matching the returned spool contents
     */
    spoolId: number;
    /**
     * Job ID associated with the returned spool
     */
    jobId: string;
    /**
     * Spool contents
     */
    data: B64String;
}

export interface ReleaseJobRequest extends common.CommandRequest<"releaseJob"> {
    /**
     * Job ID to release
     */
    jobId: string;
}

export type ReleaseJobResponse = common.CommandResponse;

export interface SubmitJclRequest extends common.CommandRequest<"submitJcl"> {
    /**
     * Desired encoding for the spool file (optional)
     */
    encoding?: string;
    /**
     * Source encoding of the JCL content (optional, defaults to UTF-8)
     */
    localEncoding?: string;
    /**
     * JCL contents to submit as a job
     */
    jcl: B64String;
}

export interface SubmitJclResponse extends common.CommandResponse {
    /**
     * The ID of the new job
     */
    jobId: string;
}

export interface SubmitJobRequest extends common.CommandRequest<"submitJob"> {
    /**
     * Dataset name w/ contents to submit as JCL
     */
    dsname: string;
}

export type SubmitJobResponse = SubmitJclResponse;

export interface SubmitUssRequest extends common.CommandRequest<"submitUss"> {
    /**
     * File path w/ contents to submit as JCL
     */
    fspath: string;
}

export type SubmitUssResponse = SubmitJclResponse;
