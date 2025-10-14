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

import type { Readable, Writable } from "node:stream";
import type { B64String } from "../types";
import type * as common from "./common";

export interface CreateDatasetRequest extends common.CommandRequest<"createDataset"> {
    /**
     * Dataset name
     */
    dsname: string;
    /**
     * Dataset attributes
     */
    attributes: common.DatasetAttributes;
}

export type CreateDatasetResponse = common.CommandResponse;

export interface CreateMemberRequest extends common.CommandRequest<"createMember"> {
    /**
     * Dataset name
     */
    dsname: string;
}

export type CreateMemberResponse = common.CommandResponse;

export interface DeleteDatasetRequest extends common.CommandRequest<"deleteDataset"> {
    /**
     * Dataset name
     */
    dsname: string;
}

export type DeleteDatasetResponse = common.CommandResponse;

export interface ListDatasetsRequest
    extends common.CommandRequest<"listDatasets">,
        common.ListOptions,
        common.ListDatasetOptions {
    /**
     * Pattern to match against dataset names
     */
    pattern: string;
    /**
     * Whether to include attributes in the response
     */
    attributes?: boolean;
}

export interface ListDatasetsResponse extends common.CommandResponse {
    /**
     * List of returned datasets
     */
    items: common.Dataset[];
    /**
     * Number of rows returned
     */
    returnedRows: number;
}

export interface ListDsMembersRequest
    extends common.CommandRequest<"listDsMembers">,
        common.ListOptions,
        common.ListDatasetOptions {
    /**
     * Dataset name
     */
    dsname: string;
    /**
     * Whether to include attributes in the response
     */
    attributes?: boolean;
}

export interface ListDsMembersResponse extends common.CommandResponse {
    /**
     * List of returned dataset members
     */
    items: common.DsMember[];
    /**
     * Number of rows returned
     */
    returnedRows: number;
}

export interface ReadDatasetRequest extends common.CommandRequest<"readDataset"> {
    /**
     * Desired encoding for the dataset (optional)
     */
    encoding?: string;
    /**
     * Source encoding of the dataset content (optional, defaults to UTF-8)
     */
    localEncoding?: string;
    /**
     * Volume serial for the data set (optional)
     */
    volume?: string;
    /**
     * Dataset name
     */
    dsname: string;
    /**
     * Stream to write contents to
     */
    stream?: Writable;
}

export interface ReadDatasetResponse extends common.CommandResponse {
    /**
     * Desired encoding for the dataset (optional)
     */
    encoding?: string;
    /**
     * Returned e-tag for the data set
     */
    etag: string;
    /**
     * Dataset contents (omitted if streaming)
     */
    data: B64String;
    /**
     * Length of dataset contents in bytes (only used for streaming)
     */
    contentLen?: number;
}

export interface RestoreDatasetRequest extends common.CommandRequest<"restoreDataset"> {
    /**
     * Dataset name
     */
    dsname: string;
}

export type RestoreDatasetResponse = common.CommandResponse;

export interface WriteDatasetRequest extends common.CommandRequest<"writeDataset"> {
    /**
     * Desired encoding for the dataset (optional)
     */
    encoding?: string;
    /**
     * Source encoding of the dataset content (optional, defaults to UTF-8)
     */
    localEncoding?: string;
    /**
     * Last e-tag for the data set (optional, omit to overwrite)
     */
    etag?: string;
    /**
     * Volume serial for the data set (optional)
     */
    volume?: string;
    /**
     * Dataset name
     */
    dsname: string;
    /**
     * Dataset contents
     */
    data?: B64String;
    /**
     * Stream to read contents from
     */
    stream?: Readable;
}

export interface WriteDatasetResponse extends common.CommandResponse {
    /**
     * Returned e-tag for the data set
     */
    etag: string;
    /**
     * Length of dataset contents in bytes (only used for streaming)
     */
    contentLen?: number;
}
