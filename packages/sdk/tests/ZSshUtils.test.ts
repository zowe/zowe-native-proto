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

import * as fs from "node:fs";
import { ZSshUtils } from "../src/ZSshUtils";

vi.mock("node:fs", { spy: true });

describe("ZSshUtils", () => {
    describe("checkIfOutdated", () => {
        it("should return false for dev builds without remote checksums", async () => {
            const readFileSyncSpy = vi.spyOn(fs, "readFileSync");
            const isOutdated = await ZSshUtils.checkIfOutdated();
            expect(isOutdated).toBe(false);
            expect(readFileSyncSpy).not.toHaveBeenCalled();
        });

        it("should return false for matching checksums with different order", async () => {
            const readFileSyncSpy = vi.spyOn(fs, "readFileSync").mockReturnValueOnce("123 abc\n456 def");
            const isOutdated = await ZSshUtils.checkIfOutdated({ def: "456", abc: "123" });
            expect(isOutdated).toBe(false);
            expect(readFileSyncSpy).toHaveBeenCalled();
        });

        it("should return false for same checksums and local file removed", async () => {
            const readFileSyncSpy = vi.spyOn(fs, "readFileSync").mockReturnValueOnce("123 abc");
            const isOutdated = await ZSshUtils.checkIfOutdated({ abc: "123", def: "456" });
            expect(isOutdated).toBe(false);
            expect(readFileSyncSpy).toHaveBeenCalled();
        });

        it("should return true for different checksums", async () => {
            const readFileSyncSpy = vi.spyOn(fs, "readFileSync").mockReturnValueOnce("123 abc\n456 def");
            const isOutdated = await ZSshUtils.checkIfOutdated({ abc: "789", def: "456" });
            expect(isOutdated).toBe(true);
            expect(readFileSyncSpy).toHaveBeenCalled();
        });

        it("should return true for same checksums and local file added", async () => {
            const readFileSyncSpy = vi.spyOn(fs, "readFileSync").mockReturnValueOnce("123 abc\n456 def\n789 ghi");
            const isOutdated = await ZSshUtils.checkIfOutdated({ abc: "123", def: "456" });
            expect(isOutdated).toBe(true);
            expect(readFileSyncSpy).toHaveBeenCalled();
        });
    });
});
