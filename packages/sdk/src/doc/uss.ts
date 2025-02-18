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
import type * as common from "./common.ts"

//////////
// source: requests.go

export interface ListFilesRequest extends common.ListOptions {
  command: "listFiles";
  fspath: string;
}
export interface ReadFileRequest {
  command: "readFile";
  encoding?: string;
  fspath: string;
}
export interface WriteFileRequest {
  command: "writeFile";
  encoding?: string;
  fspath: string;
  data: string;
}

//////////
// source: responses.go

export interface ReadFileResponse {
  encoding?: string;
  fspath: string;
  data: Buffer | string;
}
export interface WriteFileResponse {
  success: boolean;
  fspath: string;
}
export interface ListFilesResponse {
  items: common.UssItem[];
  returnedRows: number /* int */;
}
