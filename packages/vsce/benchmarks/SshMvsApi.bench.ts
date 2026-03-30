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

import "./setup"; // installs vscode mock before any other imports
import { bench, describe, beforeAll, afterAll } from "vitest";
import { PassThrough } from "node:stream";
import { targets, PREFIX, DS_OPTS, setupTargets } from "./setup";

beforeAll(() => setupTargets(), 60000);

describe("Data Sets (MVS)", () => {
    beforeAll(async () => {
        for (const target of targets) {
            try {
                await target.mvs.createDataSet(3 as any, target.dsName, DS_OPTS);
                await target.mvs.uploadFromBuffer(Buffer.from("HELLO BENCH"), `${target.dsName}(MEMBER1)`);
            } catch (e: any) {
                console.error(`[${target.name}] MVS setup failed:`, e?.mDetails?.msg ?? e?.message);
            }
        }
    }, 60000);

    afterAll(async () => {
        for (const target of targets) {
            try { await target.mvs?.deleteDataSet(target.dsName); } catch {}
        }
    });

    describe("List data sets", () => {
        for (const target of targets) {
            bench(target.name, async () => {
                await target.mvs.dataSet(`${PREFIX}.B*`);
            }, { throws: true });
        }
    });

    describe("List PDS members", () => {
        for (const target of targets) {
            bench(target.name, async () => {
                await target.mvs.allMembers(target.dsName);
            }, { throws: true });
        }
    });

    describe("Write member", () => {
        for (const target of targets) {
            bench(target.name, async () => {
                await target.mvs.uploadFromBuffer(Buffer.from("HELLO BENCH"), `${target.dsName}(MEMBER1)`);
            }, { throws: true });
        }
    });

    describe("Read member", () => {
        for (const target of targets) {
            bench(target.name, async () => {
                await target.mvs.getContents(`${target.dsName}(MEMBER1)`, { binary: false, stream: new PassThrough() });
            }, { throws: true });
        }
    });
});
