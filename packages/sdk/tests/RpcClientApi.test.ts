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

import { beforeEach, describe, expect, it, vi } from "vitest";
import type { ds } from "../src/doc/rpc";
import { RpcClientApi } from "../src/RpcClientApi";

// Create a concrete implementation for testing
class TestRpcClient extends RpcClientApi {
    public mockRequest = vi.fn();

    public request<ReqT, RespT>(request: ReqT): Promise<RespT> {
        return this.mockRequest(request);
    }
}

describe("RpcClientApi", () => {
    let client: TestRpcClient;

    beforeEach(() => {
        client = new TestRpcClient();
    });

    describe("ds.copyDataset", () => {
        it("should call request with correct command and parameters", async () => {
            const mockResponse: ds.CopyDatasetResponse = {
                success: true,
            };
            client.mockRequest.mockResolvedValue(mockResponse);

            const result = await client.ds.copyDataset({
                fromDataset: "SOURCE.DATA.SET",
                toDataset: "TARGET.DATA.SET",
            });

            expect(client.mockRequest).toHaveBeenCalledWith({
                command: "copyDataset",
                fromDataset: "SOURCE.DATA.SET",
                toDataset: "TARGET.DATA.SET",
            });
            expect(result).toEqual(mockResponse);
        });

        it("should pass replace option correctly", async () => {
            const mockResponse: ds.CopyDatasetResponse = {
                success: true,
            };
            client.mockRequest.mockResolvedValue(mockResponse);

            await client.ds.copyDataset({
                fromDataset: "SOURCE.PDS",
                toDataset: "TARGET.PDS",
                replace: true,
            });

            expect(client.mockRequest).toHaveBeenCalledWith({
                command: "copyDataset",
                fromDataset: "SOURCE.PDS",
                toDataset: "TARGET.PDS",
                replace: true,
            });
        });

        it("should pass overwrite option correctly", async () => {
            const mockResponse: ds.CopyDatasetResponse = {
                success: true,
            };
            client.mockRequest.mockResolvedValue(mockResponse);

            await client.ds.copyDataset({
                fromDataset: "SOURCE.PDS",
                toDataset: "TARGET.PDS",
                overwrite: true,
            });

            expect(client.mockRequest).toHaveBeenCalledWith({
                command: "copyDataset",
                fromDataset: "SOURCE.PDS",
                toDataset: "TARGET.PDS",
                overwrite: true,
            });
        });

        it("should handle member copies", async () => {
            const mockResponse: ds.CopyDatasetResponse = {
                success: true,
            };
            client.mockRequest.mockResolvedValue(mockResponse);

            await client.ds.copyDataset({
                fromDataset: "SOURCE.PDS(MEMBER)",
                toDataset: "TARGET.PDS(MEMBER)",
            });

            expect(client.mockRequest).toHaveBeenCalledWith({
                command: "copyDataset",
                fromDataset: "SOURCE.PDS(MEMBER)",
                toDataset: "TARGET.PDS(MEMBER)",
            });
        });

        it("should return targetCreated when target was created", async () => {
            const mockResponse: ds.CopyDatasetResponse = {
                success: true,
                targetCreated: true,
            };
            client.mockRequest.mockResolvedValue(mockResponse);

            const result = await client.ds.copyDataset({
                fromDataset: "SOURCE.DATA.SET",
                toDataset: "TARGET.DATA.SET",
            });

            expect(result.targetCreated).toBe(true);
        });

        it("should handle failure response", async () => {
            const mockResponse: ds.CopyDatasetResponse = {
                success: false,
            };
            client.mockRequest.mockResolvedValue(mockResponse);

            const result = await client.ds.copyDataset({
                fromDataset: "SOURCE.DATA.SET",
                toDataset: "TARGET.DATA.SET",
            });

            expect(result.success).toBe(false);
        });

        it("should propagate errors from request", async () => {
            const testError = new Error("Copy failed");
            client.mockRequest.mockRejectedValue(testError);

            await expect(
                client.ds.copyDataset({
                    fromDataset: "SOURCE.DATA.SET",
                    toDataset: "TARGET.DATA.SET",
                }),
            ).rejects.toThrow("Copy failed");
        });
    });
});
