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
import { targets, USS_DIR, setupTargets } from "./setup";

beforeAll(() => setupTargets(), 60000);

describe("USS Files", () => {
    beforeAll(async () => {
        for (const target of targets) {
            await target.uss.create(target.ussFile, "file");
        }
    }, 60000);

    afterAll(async () => {
        for (const target of targets) {
            try {
                await target.uss?.delete(target.ussFile);
            } catch {}
        }
    });

    describe("List directory", () => {
        for (const target of targets) {
            bench(
                target.name,
                async () => {
                    await target.uss.fileList(USS_DIR);
                },
                { throws: true },
            );
        }
    });

    describe("Write file", () => {
        for (const target of targets) {
            bench(
                target.name,
                async () => {
                    await target.uss.uploadFromBuffer(Buffer.from("HELLO BENCH"), target.ussFile);
                },
                { throws: true },
            );
        }
    });

    describe("Read file", () => {
        for (const target of targets) {
            bench(
                target.name,
                async () => {
                    await target.uss.getContents(target.ussFile, { stream: new PassThrough() });
                },
                { throws: true },
            );
        }
    });
});
