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

export interface ListFilesRequest extends common.CommandRequest, common.ListOptions {
  command: "listFiles";
  /**
   * Directory to list files for
   */
  fspath: string;
}
export interface ReadFileRequest extends common.CommandRequest {
  command: "readFile";
  /**
   * Desired encoding for the file (optional)
   */
  encoding?: string;
  /**
   * Remote file path to read contents from
   */
  fspath: string;
}
export interface WriteFileRequest extends common.CommandRequest {
  command: "writeFile";
  /**
   * Desired encoding for the file (optional)
   */
  encoding?: string;
  /**
   * E-tag for the file to detect conflicts during save (optional)
   */
  etag?: string;
  /**
   * Remote file path to write contents to
   */
  fspath: string;
  /**
   * New contents for the file
   */
  data: string;
}
export interface CreateFileRequest extends common.CommandRequest {
  command: "createFile";
  /**
   * Permissions for the new path
   */
  permissions?: string;
  /**
   * Remote file path to create
   */
  fspath: string;
  /**
   * Whether to create a directory (true) or a file (false)
   */
  isDir?: boolean;
}
export interface DeleteFileRequest extends common.CommandRequest {
  command: "deleteFile";
  /**
   * Remote file path to delete
   */
  fspath: string;
  /**
   * Whether to delete the file recursively
   */
  recursive: boolean;
}
export interface ChmodFileRequest extends common.CommandRequest {
  command: "chmodFile";
  /**
   * Desired permissions for the file (represented as an octal value, e.g. "755")
   */
  mode: string;
  /**
   * Remote file path to change permissions for
   */
  fspath: string;
  /**
   * Whether to change permissions recursively
   */
  recursive: boolean;
}
export interface ChownFileRequest extends common.CommandRequest {
  command: "chownFile";
  /**
   * New owner for the file
   */
  owner: string;
  /**
   * Remote file path to change ownership for
   */
  fspath: string;
  /**
   * Whether to apply ownership to inner files and directories
   */
  recursive: boolean;
}
export interface ChtagFileRequest extends common.CommandRequest {
  command: "chtagFile";
  /**
   * Remote file path to change tags for
   */
  fspath: string;
  /**
   * New tag for the file
   */
  tag: string;
  /**
   * Whether to apply the tag to inner files and directories
   */
  recursive: boolean;
}

//////////
// source: responses.go

export interface GenericFileResponse extends common.CommandResponse {
  /**
   * Whether the operation was successful
   */
  success: boolean;
  /**
   * Remote file path
   */
  fspath: string;
}
export interface ReadFileResponse extends common.CommandResponse {
  /**
   * Returned encoding for the file
   */
  encoding?: string;
  /**
   * Returned e-tag for the file
   */
  etag: string;
  /**
   * Remote file path
   */
  fspath: string;
  /**
   * File contents
   */
  data: Buffer | string;
}
export interface WriteFileResponse extends GenericFileResponse {
  etag: string;
}
export interface ListFilesResponse extends common.CommandResponse {
  items: common.UssItem[];
  returnedRows: number /* int */;
}
export type CreateFileResponse = GenericFileResponse;
export type DeleteFileResponse = GenericFileResponse;
export type ChmodFileResponse = GenericFileResponse;
export type ChownFileResponse = GenericFileResponse;
export type ChtagFileResponse = GenericFileResponse;
