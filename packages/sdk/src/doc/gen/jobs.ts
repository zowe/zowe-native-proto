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

// Code generated by tygo. DO NOT EDIT.
import type * as common from "./common.ts";

//////////
// source: requests.go

export interface ListJobsRequest extends common.CommandRequest, common.ListOptions {
  command: "listJobs";
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
export interface ListSpoolsRequest extends common.CommandRequest {
  command: "listSpools";
  /**
   * Job ID to list spools for
   */
  jobId: string;
}
export interface ReadSpoolRequest extends common.CommandRequest {
  command: "readSpool";
  /**
   * Desired encoding for the spool file (optional)
   */
  encoding?: string;
  /**
   * Spool ID to read under the given job ID
   */
  spoolId: number /* int */;
  /**
   * Job ID with spools to read from
   */
  jobId: string;
}
export interface GetJclRequest extends common.CommandRequest {
  command: "getJcl";
  /**
   * Job ID to get JCL for
   */
  jobId: string;
}
export interface GetJobStatusRequest extends common.CommandRequest {
  command: "getJobStatus";
  /**
   * Job ID to get status for
   */
  jobId: string;
}
export interface SubmitJobRequest extends common.CommandRequest {
  command: "submitJob";
  /**
   * Dataset name w/ contents to submit as JCL
   */
  dsname: string;
}
export interface SubmitUssRequest extends common.CommandRequest {
  command: "submitUss";
  /**
   * File path w/ contents to submit as JCL
   */
  fspath: string;
}
export interface SubmitJclRequest extends common.CommandRequest {
  command: "submitJcl";
  /**
   * JCL contents to submit as a job
   */
  jcl: string;
}
export interface CancelJobRequest extends common.CommandRequest {
  command: "cancelJob";
  /**
   * Job ID to cancel
   */
  jobId: string;
}
export interface DeleteJobRequest extends common.CommandRequest {
  command: "deleteJob";
  /**
   * Job ID to cancel
   */
  jobId: string;
}
export interface HoldJobRequest extends common.CommandRequest {
  command: "holdJob";
  /**
   * Job ID to hold
   */
  jobId: string;
}
export interface ReleaseJobRequest extends common.CommandRequest {
  command: "releaseJob";
  /**
   * Job ID to release
   */
  jobId: string;
}

//////////
// source: responses.go

export interface ListJobsResponse extends common.CommandResponse {
  /**
   * List of returned jobs
   */
  items: common.Job[];
}
export interface ListSpoolsResponse extends common.CommandResponse {
  /**
   * List of returned spools
   */
  items: common.Spool[];
}
export interface GetJclResponse extends common.CommandResponse {
  /**
   * Job ID for the returned JCL
   */
  jobId: string;
  /**
   * JCL contents
   */
  data: string;
}
export interface ReadSpoolResponse extends common.CommandResponse {
  /**
   * Desired encoding for the spool file (optional)
   */
  encoding?: string;
  /**
   * Spool ID matching the returned spool contents
   */
  spoolId: number /* int */;
  /**
   * Job ID associated with the returned spool
   */
  jobId: string;
  /**
   * Spool contents
   */
  data: Buffer | string;
}
export interface GetJobStatusResponse extends common.CommandResponse, common.Job {
}
export interface SubmitJobResponse extends common.CommandResponse {
  /**
   * Whether the job was successfully submitted
   */
  success: boolean;
  /**
   * The job ID of the newly-submitted job
   */
  jobId: string;
  /**
   * The data set name where the JCL was read from
   */
  dsname: string;
}
export interface SubmitUssResponse extends common.CommandResponse {
  /**
   * Whether the job was successfully submitted
   */
  success: boolean;
  /**
   * The job ID of the newly-submitted job
   */
  jobId: string;
  /**
   * The USS file where the JCL was read from
   */
  fspath: string;
}
export interface SubmitJclResponse extends common.CommandResponse {
  /**
   * Whether the JCL was successfully submitted
   */
  success: boolean;
  /**
   * The ID of the new job
   */
  jobId: string;
}
export interface DeleteJobResponse extends common.CommandResponse {
  /**
   * Whether the job was successfully deleted
   */
  success: boolean;
  /**
   * The ID for the job that was deletede
   */
  jobId: string;
}
export interface CancelJobResponse extends common.CommandResponse {
  /**
   * Whether the job was successfully cancelled
   */
  success: boolean;
  /**
   * The ID for the job that was cancelled
   */
  jobId: string;
}
export interface HoldJobResponse extends common.CommandResponse {
  /**
   * Whether the job was successfully held
   */
  success: boolean;
  /**
   * The ID for the job that was held
   */
  jobId: string;
}
export interface ReleaseJobResponse extends common.CommandResponse {
  /**
   * Whether the job was successfully released
   */
  success: boolean;
  /**
   * The ID for the job that was released
   */
  jobId: string;
}
