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

import { type ZoweExplorerApiType, ZoweVsCodeExtension } from "@zowe/zowe-explorer-api";
import * as vscode from "vscode";

/**
 * Enhanced error handling utility for SSH operations using Zowe Explorer's ErrorCorrelator
 */
export class SshErrorHandler {
    private static instance: SshErrorHandler;
    private constructor() {}
    public static getInstance(): SshErrorHandler {
        if (!SshErrorHandler.instance) {
            SshErrorHandler.instance = new SshErrorHandler();
        }
        return SshErrorHandler.instance;
    }

    /**
     * Handles and displays an SSH error using error correlation if available
     * @param error The error that occurred
     * @param apiType The API type where the error occurred
     * @param context Additional context about the operation
     * @param allowRetry Whether to allow retrying the operation
     * @returns The user's response ("Retry", "Troubleshoot", etc.)
     */
    public async handleError(
        error: Error | string,
        apiType: ZoweExplorerApiType,
        context?: string,
        allowRetry: boolean = false,
    ): Promise<string | undefined> {
        const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
        const extenderApi = zoweExplorerApi?.getExplorerExtenderApi();
        const errorCorrelator = extenderApi?.getErrorCorrelator?.();

        // If error correlator is available, use it
        if (errorCorrelator) {
            const result = await errorCorrelator.displayError(apiType, error, {
                profileType: "ssh",
                additionalContext: context,
                allowRetry,
            });
            return result.userResponse;
        }

        // Fallback to basic error handling
        const errorMessage = error instanceof Error ? error.message : error;
        const contextPrefix = context ? `${context}: ` : "";

        const actions = [allowRetry ? "Retry" : undefined, "Show Details"].filter(Boolean) as string[];

        const response = await vscode.window.showErrorMessage(`${contextPrefix}${errorMessage}`, ...actions);

        if (response === "Show Details") {
            const outputChannel = vscode.window.createOutputChannel("Zowe SSH");
            outputChannel.appendLine(`Error: ${errorMessage}`);
            if (error instanceof Error && error.stack) {
                outputChannel.appendLine(`Stack: ${error.stack}`);
            }
            outputChannel.show();
        }

        return response;
    }

    /**
     * Checks if an error is a fatal SSH error that should terminate the connection
     * @param error The error to check
     * @returns True if the error is fatal
     */
    public isFatalError(error: Error | string): boolean {
        const errorMessage = error instanceof Error ? error.message : error;

        // Check for fatal OpenSSH error codes
        const fatalErrorCodes = [
            "FOTS4240", // kex_prop2buf: error (internal error, program ends)
            "FOTS4241", // Authentication failed
            "FOTS4134", // Client version uses unsafe key agreement
            "FOTS4231", // Server version uses unsafe key agreement
            "FOTS4203", // Server failed to confirm ownership of private host keys
            "FOTS4314", // xreallocarray: out of memory
            "FOTS4315", // xrecallocarray: out of memory
            "FOTS4216", // Couldn't allocate session state
            "FOTS4311", // could not allocate state
            "FOTS4151", // openpty failed
            "FOTS4154", // ssh_packet_set_connection failed
            "FOTS4150", // kex_setup failed
            "FOTS4312", // cipher_init failed
            "FSUM6260", // write error on file
        ];

        return fatalErrorCodes.some((code) => errorMessage.includes(code));
    }

    /**
     * Extracts the OpenSSH error code from an error message
     * @param error The error message
     * @returns The error code if found, undefined otherwise
     */
    public extractErrorCode(error: Error | string): string | undefined {
        const errorMessage = error instanceof Error ? error.message : error;
        const match = errorMessage.match(/FOTS\d{4}|FSUM\d{4}/);
        return match?.[0];
    }

    /**
     * Creates an error callback for use with SDK functions like installServer
     * @param apiType The API type for error correlation
     * @param context Additional context for the operation
     * @returns An error callback function that handles errors with correlation
     */
    public createErrorCallback(
        apiType: ZoweExplorerApiType,
        context?: string,
    ): (error: Error, operationContext: string) => Promise<boolean> {
        return async (error: Error, operationContext: string): Promise<boolean> => {
            const fullContext = context ? `${context} (${operationContext})` : operationContext;
            const userResponse = await SshErrorHandler.getInstance().handleError(error, apiType, fullContext, true);

            // Return true if user chose to retry, false otherwise
            return userResponse === "Retry";
        };
    }
}
