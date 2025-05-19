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

export declare const CreateDefaults: {
    /**
     * Specifies all the defaults to create non-vsam data sets
     */
    DATA_SET: {
        /**
         * Specifies the defaults used by the Zos Files API to create a partitioned data set
         * @type {ICreateDataSetOptions}
         */
        PARTITIONED: {
            alcunit: string;
            dsorg: string;
            primary: number;
            dirblk: number;
            recfm: string;
            blksize: number;
            lrecl: number;
        };
        /**
         * Specifies the defaults used by the Zos Files API to create a sequential data set
         * @type {ICreateDataSetOptions}
         */
        SEQUENTIAL: {
            alcunit: string;
            dsorg: string;
            primary: number;
            recfm: string;
            blksize: number;
            lrecl: number;
        };
        /**
         * Specifies the defaults used by the Zos Files API to create a classic data set
         * @type {ICreateDataSetOptions}
         */
        CLASSIC: {
            alcunit: string;
            dsorg: string;
            primary: number;
            recfm: string;
            blksize: number;
            lrecl: number;
            dirblk: number;
        };
        /**
         * Specifies the defaults used by the Zos Files API to create a data set used for C code
         * @type {ICreateDataSetOptions}
         */
        C: {
            dsorg: string;
            alcunit: string;
            primary: number;
            recfm: string;
            blksize: number;
            lrecl: number;
            dirblk: number;
        };
        /**
         * Specifies the defaults used by the Zos Files API to create a data set used for binaries
         * @type {ICreateDataSetOptions}
         */
        BINARY: {
            dsorg: string;
            alcunit: string;
            primary: number;
            recfm: string;
            blksize: number;
            lrecl: number;
            dirblk: number;
        };
        /**
         * Specifies the defaults used by the Zos Files API to create a blank data set
         * @type {ICreateDataSetOptions}
         */
        BLANK: {
            primary: number;
        };
    };
    /**
     * Specifies the defaults used by the Zos Files API to create a VSAM cluster
     * @type {ICreateVsamOptions}
     */
    VSAM: {
        dsorg: string;
        alcunit: string;
        primary: number;
    };
};
