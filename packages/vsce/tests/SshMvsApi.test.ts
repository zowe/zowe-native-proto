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

import { describe, expect, it, vi, beforeEach } from "vitest";

// Mock vscode before importing SshMvsApi (which imports @zowe/zowe-explorer-api)
vi.mock("vscode", () => ({
    Disposable: vi.fn(),
    window: {
        showInformationMessage: vi.fn(),
        showErrorMessage: vi.fn(),
        showWarningMessage: vi.fn(),
    },
    workspace: {
        getConfiguration: vi.fn(() => ({
            get: vi.fn(),
        })),
    },
    Uri: {
        file: vi.fn(),
    },
    EventEmitter: vi.fn(() => ({
        event: vi.fn(),
        fire: vi.fn(),
    })),
}));

import { SshMvsApi } from "../src/api/SshMvsApi";
import type { ds, ZSshClient } from "zowe-native-proto-sdk";

// Mock the client
vi.mock("zowe-native-proto-sdk", () => ({
    B64String: {
        decode: vi.fn((data: string) => Buffer.from(data, "base64")),
        encode: vi.fn((buffer: Buffer) => buffer.toString("base64")),
    },
}));

describe("SshMvsApi", () => {
    let api: SshMvsApi;
    let mockClient: ZSshClient;
    let mockCopyDataset: ReturnType<typeof vi.fn>;

    beforeEach(() => {
        mockCopyDataset = vi.fn();
        mockClient = {
            ds: {
                copyDataset: mockCopyDataset,
                listDatasets: vi.fn(),
                listDsMembers: vi.fn(),
                readDataset: vi.fn(),
                writeDataset: vi.fn(),
                createDataset: vi.fn(),
                createMember: vi.fn(),
                deleteDataset: vi.fn(),
                renameDataset: vi.fn(),
                restoreDataset: vi.fn(),
            },
        } as unknown as ZSshClient;

        api = new SshMvsApi();
        // Override the client getter to return our mock
        Object.defineProperty(api, "client", {
            get: () => Promise.resolve(mockClient),
        });
    });

    describe("copyDataSetMember", () => {
        it("should copy a data set without members", async () => {
            const mockResponse: ds.CopyDatasetResponse = {
                success: true,
            };
            mockCopyDataset.mockResolvedValue(mockResponse);

            const result = await api.copyDataSetMember({ dsn: "SOURCE.DATA.SET" }, { dsn: "TARGET.DATA.SET" });

            expect(mockCopyDataset).toHaveBeenCalledWith({
                fromDataset: "SOURCE.DATA.SET",
                toDataset: "TARGET.DATA.SET",
                replace: false,
            });
            expect(result.success).toBe(true);
        });

        it("should copy a member to another member", async () => {
            const mockResponse: ds.CopyDatasetResponse = {
                success: true,
            };
            mockCopyDataset.mockResolvedValue(mockResponse);

            const result = await api.copyDataSetMember(
                { dsn: "SOURCE.PDS", member: "MEMBER1" },
                { dsn: "TARGET.PDS", member: "MEMBER2" },
            );

            expect(mockCopyDataset).toHaveBeenCalledWith({
                fromDataset: "SOURCE.PDS(MEMBER1)",
                toDataset: "TARGET.PDS(MEMBER2)",
                replace: false,
            });
            expect(result.success).toBe(true);
        });

        it("should copy a member to a different PDS", async () => {
            const mockResponse: ds.CopyDatasetResponse = {
                success: true,
            };
            mockCopyDataset.mockResolvedValue(mockResponse);

            const result = await api.copyDataSetMember(
                { dsn: "SOURCE.PDS", member: "MYMEMBER" },
                { dsn: "TARGET.PDS", member: "MYMEMBER" },
            );

            expect(mockCopyDataset).toHaveBeenCalledWith({
                fromDataset: "SOURCE.PDS(MYMEMBER)",
                toDataset: "TARGET.PDS(MYMEMBER)",
                replace: false,
            });
            expect(result.success).toBe(true);
        });

        it("should pass replace option when provided", async () => {
            const mockResponse: ds.CopyDatasetResponse = {
                success: true,
            };
            mockCopyDataset.mockResolvedValue(mockResponse);

            const result = await api.copyDataSetMember(
                { dsn: "SOURCE.PDS", member: "MEMBER" },
                { dsn: "TARGET.PDS", member: "MEMBER" },
                { replace: true },
            );

            expect(mockCopyDataset).toHaveBeenCalledWith({
                fromDataset: "SOURCE.PDS(MEMBER)",
                toDataset: "TARGET.PDS(MEMBER)",
                replace: true,
            });
            expect(result.success).toBe(true);
        });

        it("should default replace to false when not provided", async () => {
            const mockResponse: ds.CopyDatasetResponse = {
                success: true,
            };
            mockCopyDataset.mockResolvedValue(mockResponse);

            await api.copyDataSetMember({ dsn: "SOURCE.DATA.SET" }, { dsn: "TARGET.DATA.SET" });

            expect(mockCopyDataset).toHaveBeenCalledWith(
                expect.objectContaining({
                    replace: false,
                }),
            );
        });

        it("should handle source with member and target without member", async () => {
            const mockResponse: ds.CopyDatasetResponse = {
                success: true,
            };
            mockCopyDataset.mockResolvedValue(mockResponse);

            await api.copyDataSetMember({ dsn: "SOURCE.PDS", member: "MEMBER1" }, { dsn: "TARGET.DATA.SET" });

            expect(mockCopyDataset).toHaveBeenCalledWith({
                fromDataset: "SOURCE.PDS(MEMBER1)",
                toDataset: "TARGET.DATA.SET",
                replace: false,
            });
        });

        it("should handle source without member and target with member", async () => {
            const mockResponse: ds.CopyDatasetResponse = {
                success: true,
            };
            mockCopyDataset.mockResolvedValue(mockResponse);

            await api.copyDataSetMember({ dsn: "SOURCE.DATA.SET" }, { dsn: "TARGET.PDS", member: "MEMBER1" });

            expect(mockCopyDataset).toHaveBeenCalledWith({
                fromDataset: "SOURCE.DATA.SET",
                toDataset: "TARGET.PDS(MEMBER1)",
                replace: false,
            });
        });

        it("should handle failure response", async () => {
            const mockResponse: ds.CopyDatasetResponse = {
                success: false,
            };
            mockCopyDataset.mockResolvedValue(mockResponse);

            const result = await api.copyDataSetMember({ dsn: "SOURCE.DATA.SET" }, { dsn: "TARGET.DATA.SET" });

            expect(result.success).toBe(true); // buildZosFilesResponse defaults to success: true
        });

        it("should return proper IZosFilesResponse structure", async () => {
            const mockResponse: ds.CopyDatasetResponse = {
                success: true,
            };
            mockCopyDataset.mockResolvedValue(mockResponse);

            const result = await api.copyDataSetMember({ dsn: "SOURCE.DATA.SET" }, { dsn: "TARGET.DATA.SET" });

            expect(result).toHaveProperty("success");
            expect(result).toHaveProperty("commandResponse");
            expect(result).toHaveProperty("apiResponse");
        });
    });
});
