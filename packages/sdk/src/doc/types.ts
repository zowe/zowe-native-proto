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

export type Dataset = {
    name: string;
    dsorg: string;
    volser: string;
};

export type DsMember = {
    name: string;
};

export type UssItem = {
    name: string;
    isDir: boolean;
};

export type Job = {
    id: string;
    name: string;
    status: string;
    retcode: string;
};

export type Spool = {
    id: number;
    ddname: string;
    stepname: string;
    procstep?: string;
    dsname?: string;
};
