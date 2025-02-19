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

export interface ListDatasetsRequest extends common.ListOptions, common.ListDatasetOptions {
  command: "listDatasets";
  /**
   * Pattern to match against dataset names
   */
  pattern: string;
  /**
   * Whether to include attributes in the response
   */
  attributes?: boolean;
}
export interface ListDsMembersRequest extends common.ListOptions, common.ListDatasetOptions {
  command: "listDsMembers";
  /**
   * Dataset name
   */
  dsname: string;
  /**
   * Whether to include attributes in the response
   */
  attributes?: boolean;
}
export interface ReadDatasetRequest {
  command: "readDataset";
  /**
   * Desired encoding for the dataset (optional)
   */
  encoding?: string;
  /**
   * Dataset name
   */
  dsname: string;
}
export interface WriteDatasetRequest {
  command: "writeDataset";
  /**
   * Desired encoding for the dataset (optional)
   */
  encoding?: string;
  /**
   * Dataset name
   */
  dsname: string;
  /**
   * Dataset contents
   */
  data: string;
}
export interface DeleteDatasetRequest {
  command: "deleteDataset";
  /**
   * Dataset name
   */
  dsname: string;
}
export interface RestoreDatasetRequest {
  command: "restoreDataset";
  /**
   * Dataset name
   */
  dsname: string;
}
/**
 * default: DSORG=PO, RECFM=FB, LRECL=80
 * vb: DSORG=PO, RECFM=VB, LRECL=255
 * adata: DSORG=PO, RECFM=VB, LRECL=32756
 */
export interface CreateDatasetRequest {
  command: "createDataset";
  /**
   * Dataset name
   */
  dsname: string;
  /**
   * Type of the dataset to make
   */
  dstype: 'default' | 'vb' | 'adata';
}

//////////
// source: responses.go

export interface WriteDatasetResponse {
  /**
   * Whether the new data was stored successfully
   */
  success: boolean;
  /**
   * Dataset name
   */
  dataset: string;
}
export interface RestoreDatasetResponse {
  /**
   * Whether the dataset was restored successfully
   */
  success: boolean;
}
export interface ReadDatasetResponse {
  /**
   * Desired encoding for the dataset (optional)
   */
  encoding?: string;
  /**
   * Dataset name
   */
  dataset: string;
  /**
   * Dataset contents
   */
  data: Buffer | string;
}
export interface ListDatasetsResponse {
  /**
   * List of returned datasets
   */
  items: common.Dataset[];
  /**
   * Number of rows returned
   */
  returnedRows: number /* int */;
}
export interface ListDsMembersResponse {
  /**
   * List of returned dataset members
   */
  items: common.DsMember[];
  /**
   * Number of rows returned
   */
  returnedRows: number /* int */;
}
export interface GenericDatasetResponse {
  /**
   * Whether the dataset operation was successful
   */
  success: boolean;
  /**
   * Dataset name
   */
  dsname: string;
}
export type CreateDatasetResponse = GenericDatasetResponse;
export type DeleteDatasetResponse = GenericDatasetResponse;
