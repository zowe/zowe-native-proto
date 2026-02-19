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

import type { IHandlerParameters } from "@zowe/imperative";
import { beforeEach, describe, expect, it, vi } from "vitest";
import type { ds, ZSshClient } from "zowe-native-proto-sdk";
import CopyDataSetHandler from "../../src/copy/data-set/DataSet.handler";

describe("CopyDataSetHandler", () => {
    let handler: CopyDataSetHandler;
    let mockClient: ZSshClient;
    let mockParams: IHandlerParameters;
    let mockCopyDataset: ReturnType<typeof vi.fn>;

    beforeEach(() => {
        handler = new CopyDataSetHandler();

        mockCopyDataset = vi.fn();
        mockClient = {
            ds: {
                copyDataset: mockCopyDataset,
            },
        } as unknown as ZSshClient;

        mockParams = {
            arguments: {
                fromDataset: "SOURCE.DATA.SET",
                toDataset: "TARGET.DATA.SET",
            },
            response: {
                data: {
                    setMessage: vi.fn(),
                    setObj: vi.fn(),
                },
                console: {
                    log: vi.fn(),
                },
            },
        } as unknown as IHandlerParameters;
    });

    describe("processWithClient", () => {
        it("should copy a data set successfully", async () => {
            const mockResponse: ds.CopyDatasetResponse = {
                success: true,
            };
            mockCopyDataset.mockResolvedValue(mockResponse);

            const result = await handler.processWithClient(mockParams, mockClient);

            expect(mockCopyDataset).toHaveBeenCalledWith({
                fromDataset: "SOURCE.DATA.SET",
                toDataset: "TARGET.DATA.SET",
                replace: false,
                deleteTargetMembers: false,
            });
            expect(mockParams.response.data.setMessage).toHaveBeenCalledWith(
                'Data set "SOURCE.DATA.SET" copied to "TARGET.DATA.SET"',
            );
            expect(mockParams.response.console.log).toHaveBeenCalledWith(
                'Data set "SOURCE.DATA.SET" copied to "TARGET.DATA.SET"',
            );
            expect(result).toEqual(mockResponse);
        });

        it("should display message when target is created", async () => {
            const mockResponse: ds.CopyDatasetResponse = {
                success: true,
                targetCreated: true,
            };
            mockCopyDataset.mockResolvedValue(mockResponse);

            await handler.processWithClient(mockParams, mockClient);

            expect(mockParams.response.data.setMessage).toHaveBeenCalledWith(
                'Data set "TARGET.DATA.SET" created and copied from "SOURCE.DATA.SET"',
            );
        });

        it("should display message when deleteTargetMembers is used", async () => {
            mockParams.arguments.deleteTargetMembers = true;
            const mockResponse: ds.CopyDatasetResponse = {
                success: true,
            };
            mockCopyDataset.mockResolvedValue(mockResponse);

            await handler.processWithClient(mockParams, mockClient);

            expect(mockCopyDataset).toHaveBeenCalledWith({
                fromDataset: "SOURCE.DATA.SET",
                toDataset: "TARGET.DATA.SET",
                replace: false,
                deleteTargetMembers: true,
            });
            expect(mockParams.response.data.setMessage).toHaveBeenCalledWith(
                'Target members deleted and data set "TARGET.DATA.SET" replaced with contents of "SOURCE.DATA.SET"',
            );
        });

        it("should display message when replace is used", async () => {
            mockParams.arguments.replace = true;
            const mockResponse: ds.CopyDatasetResponse = {
                success: true,
            };
            mockCopyDataset.mockResolvedValue(mockResponse);

            await handler.processWithClient(mockParams, mockClient);

            expect(mockCopyDataset).toHaveBeenCalledWith({
                fromDataset: "SOURCE.DATA.SET",
                toDataset: "TARGET.DATA.SET",
                replace: true,
                deleteTargetMembers: false,
            });
            expect(mockParams.response.data.setMessage).toHaveBeenCalledWith(
                'Data set "TARGET.DATA.SET" updated with contents of "SOURCE.DATA.SET"',
            );
        });

        it("should prioritize targetCreated message over deleteTargetMembers", async () => {
            mockParams.arguments.deleteTargetMembers = true;
            const mockResponse: ds.CopyDatasetResponse = {
                success: true,
                targetCreated: true,
            };
            mockCopyDataset.mockResolvedValue(mockResponse);

            await handler.processWithClient(mockParams, mockClient);

            expect(mockParams.response.data.setMessage).toHaveBeenCalledWith(
                'Data set "TARGET.DATA.SET" created and copied from "SOURCE.DATA.SET"',
            );
        });

        it("should copy a member to another member", async () => {
            mockParams.arguments.fromDataset = "SOURCE.PDS(MEMBER1)";
            mockParams.arguments.toDataset = "TARGET.PDS(MEMBER2)";
            const mockResponse: ds.CopyDatasetResponse = {
                success: true,
            };
            mockCopyDataset.mockResolvedValue(mockResponse);

            await handler.processWithClient(mockParams, mockClient);

            expect(mockCopyDataset).toHaveBeenCalledWith({
                fromDataset: "SOURCE.PDS(MEMBER1)",
                toDataset: "TARGET.PDS(MEMBER2)",
                replace: false,
                deleteTargetMembers: false,
            });
        });

        it("should not log to console when response is unsuccessful", async () => {
            const mockResponse: ds.CopyDatasetResponse = {
                success: false,
            };
            mockCopyDataset.mockResolvedValue(mockResponse);

            await handler.processWithClient(mockParams, mockClient);

            expect(mockParams.response.data.setMessage).toHaveBeenCalled();
            expect(mockParams.response.console.log).not.toHaveBeenCalled();
        });

        it("should handle both replace and deleteTargetMembers options", async () => {
            mockParams.arguments.replace = true;
            mockParams.arguments.deleteTargetMembers = true;
            const mockResponse: ds.CopyDatasetResponse = {
                success: true,
            };
            mockCopyDataset.mockResolvedValue(mockResponse);

            await handler.processWithClient(mockParams, mockClient);

            expect(mockCopyDataset).toHaveBeenCalledWith({
                fromDataset: "SOURCE.DATA.SET",
                toDataset: "TARGET.DATA.SET",
                replace: true,
                deleteTargetMembers: true,
            });
            // deleteTargetMembers takes precedence in the message
            expect(mockParams.response.data.setMessage).toHaveBeenCalledWith(
                'Target members deleted and data set "TARGET.DATA.SET" replaced with contents of "SOURCE.DATA.SET"',
            );
        });
    });
});
