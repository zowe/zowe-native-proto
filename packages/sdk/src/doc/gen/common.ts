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

//////////
// source: common.go

export interface RpcRequest {
  jsonrpc: "2.0";
  method: string;
  params?: any;
  id: number /* int */;
}
export interface RpcResponse {
  jsonrpc: "2.0";
  result?: any;
  error?: ErrorDetails;
  id?: number /* int */;
}
export interface CommandRequest {
  /**
   * Requested command to execute
   */
  command: string;
}
export interface CommandResponse {
  /**
   * True if command succeeded
   */
  success: boolean;
}
export interface ErrorDetails {
  code: number /* int */;
  message: string;
  data?: any;
}
export interface Dataset {
  /**
   * Dataset name
   */
  name: string;
  /**
   * Dataset organization
   */
  dsorg: string;
  /**
   * Volume serial number
   */
  volser: string;
  /**
   * Dataset migrated
   */
  migr: boolean;
}
export interface DsMember {
  /**
   * Dataset member name
   */
  name: string;
}
export interface UssItem {
  /**
   * File name
   */
  name: string;
  /**
   * Whether the item is a directory
   */
  isDir: boolean;
}
export interface Job {
  /**
   * Job ID
   */
  id: string;
  /**
   * Job name
   */
  name: string;
  /**
   * Job status
   */
  status: string;
  /**
   * Job return code
   */
  retcode: string;
}
export interface Spool {
  /**
   * Spool ID
   */
  id: number /* int */;
  /**
   * DD name
   */
  ddname: string;
  /**
   * Step name in the job
   */
  stepname: string;
  /**
   * Dataset name
   */
  dsname: string;
  /**
   * Procedure name for the step
   */
  procstep: string;
}
/**
 * StatusMessage represents the status of `zowed` and serves as a health check
 */
export interface StatusMessage {
  status: string;
  message: string;
}

//////////
// source: options.go

export interface IoserverOptions {
  numWorkers?: number /* int */;
  verbose?: boolean;
}
export interface ListOptions {
  /**
   * Maximum number of items to return
   */
  maxItems?: number /* int */;
  /**
   * Response timeout in seconds
   */
  responseTimeout?: number /* int */;
}
export interface ListDatasetOptions {
  /**
   * Skip data sets that come before this data set name
   */
  start?: string;
}
