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

// Base Request/Response interfaces

export interface RpcNotification {
    jsonrpc: "2.0";
    method: string;
    // biome-ignore lint/suspicious/noExplicitAny: RPC params can be any object
    params?: any;
}

export interface RpcRequest extends RpcNotification {
    id: number;
}

export interface ErrorDetails {
    code: number;
    message: string;
    // biome-ignore lint/suspicious/noExplicitAny: RPC data can be any object
    data?: any;
}

export interface RpcResponse {
    jsonrpc: "2.0";
    // biome-ignore lint/suspicious/noExplicitAny: RPC result can be any object
    result?: any;
    error?: ErrorDetails;
    id: number;
}

export interface CommandRequest<CommandT extends string = string> {
    /**
     * Requested command to execute
     */
    command: CommandT;
}

export interface CommandResponse {
    /**
     * True if command succeeded
     */
    success: boolean;
}

/**
 * StatusMessage represents the status of `zowed` and serves as a health check
 */
export interface StatusMessage {
    status: string;
    message: string;
    // biome-ignore lint/suspicious/noExplicitAny: Data properties are unknown type
    data?: { [key: string]: any };
}

// Options interfaces

export interface IoserverOptions {
    numWorkers?: number;
    verbose?: boolean;
}

export interface ListOptions {
    /**
     * Maximum number of items to return
     */
    maxItems?: number;
    /**
     * Response timeout in seconds
     */
    responseTimeout?: number;
}

export interface ListDatasetOptions {
    /**
     * Skip data sets that come before this data set name
     */
    start?: string;
}

// Dataset-related types

export interface Dataset {
    /**
     * Dataset name
     */
    name: string;
    /**
     * Dataset organization
     */
    dsorg?: string;
    /**
     * Volume serial number
     */
    volser?: string;
    /**
     * Dataset migrated
     */
    migr?: boolean;
    /**
     * Record format
     */
    recfm?: string;
}

export interface DatasetAttributes {
    alcunit?: string; // Allocation Unit
    blksize?: number; // Block Size
    dirblk?: number; // Directory Blocks
    dsorg?: string; // Data Set Organization
    primary: number; // Primary Space
    recfm?: string; // Record Format
    lrecl: number; // Record Length
    dataclass?: string; // Data Class
    unit?: string; // Device Type
    dsntype?: string; // Data Set Type
    mgntclass?: string; // Management Class
    dsname?: string; // Data Set Name
    avgblk?: number; // Average Block Length
    secondary?: number; // Secondary Space
    size?: string; // Size
    storclass?: string; // Storage Class
    vol?: string; // Volume Serial
}

export interface DsMember {
    /**
     * Dataset member name
     */
    name: string;
}

// USS-related types

export interface UssItem {
    /**
     * File name
     */
    name: string;
    /**
     * Number of links to the item
     */
    links?: number;
    /**
     * User (owner) of the item
     */
    user?: string;
    /**
     * Group of the item
     */
    group?: string;
    /**
     * Size of the item
     */
    size?: number;
    /**
     * The filetag of the item
     */
    filetag?: string;
    /**
     * Modification date of the item
     */
    mtime?: string;
    /**
     * The permission string of the item
     */
    mode?: string;
}

// Job-related types

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
    id: number;
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
