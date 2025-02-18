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

export interface IssueConsoleRequest {
  command: "consoleCommand";
  commandText: string;
  consoleName: string;
}
export interface IssueTsoRequest {
  command: "tsoCommand";
  commandText: string;
}
export interface IssueUnixRequest {
  command: "unixCommand";
  commandText: string;
}

//////////
// source: responses.go

export interface IssueConsoleResponse {
  data: string;
}
export interface IssueTsoResponse {
  data: string;
}
export interface IssueUnixResponse {
  data: string;
}
