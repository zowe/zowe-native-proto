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

import {
    type ErrorCorrelation,
    type ErrorCorrelator,
    type ZoweExplorerApiType,
    ZoweVsCodeExtension,
} from "@zowe/zowe-explorer-api";

/**
 * Registers all SSH-specific error correlations with the Zowe Explorer ErrorCorrelator
 */
export function registerSshErrorCorrelations(): void {
    const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
    if (!zoweExplorerApi) {
        return;
    }

    const extenderApi = zoweExplorerApi.getExplorerExtenderApi();
    const errorCorrelator = extenderApi?.getErrorCorrelator?.();
    if (!errorCorrelator) {
        return;
    }

    registerConnectionFailures(errorCorrelator);
    registerMemoryFailures(errorCorrelator);
    registerFileSystemErrors(errorCorrelator);
}

function registerConnectionFailures(correlator: ErrorCorrelator): void {
    const connectionFailures: ErrorCorrelation[] = [
        {
            errorCode: "FOTS4241",
            matches: ["Authentication failed.", /FOTS4241.*Authentication failed/],
            summary:
                "SSH authentication failed. The provided credentials are invalid or the authentication method is not supported.",
            tips: [
                "Verify that your username and password are correct.",
                "Check if your account is locked or expired on the mainframe system.",
                "Ensure the SSH profile is configured with the correct authentication method (password, key, or certificate).",
                "Contact your system administrator if the issue persists.",
            ],
            resources: [
                {
                    href: "https://www.ibm.com/docs/en/zos/2.4.0?topic=guide-openssh-messages",
                    title: "IBM z/OS OpenSSH Messages and Codes",
                },
            ],
        },
        {
            errorCode: "FOTS4134",
            matches: [/Client version ".*" uses unsafe key agreement; refusing connection/, "FOTS4134"],
            summary:
                "The SSH client version uses an unsafe key agreement protocol that is not supported by the server.",
            tips: [
                "Update your SSH client to use a more secure key exchange algorithm.",
                "Contact your system administrator to check the server's supported key exchange methods.",
                "Consider using a newer version of the SSH protocol if available.",
            ],
        },
        {
            errorCode: "FOTS4231",
            matches: [/Server version ".*" uses unsafe key agreement; refusing connection/, "FOTS4231"],
            summary:
                "The SSH server version uses an unsafe key agreement protocol that is not supported by the client.",
            tips: [
                "Contact your system administrator to upgrade the SSH server configuration.",
                "Ask your administrator to enable more secure key exchange algorithms on the server.",
                "Verify that your client supports the server's configured key exchange methods.",
            ],
        },
        {
            errorCode: "FOTS4203",
            matches: ["Server failed to confirm ownership of private host keys", "FOTS4203"],
            summary:
                "The SSH server could not verify ownership of its private host keys, indicating a potential security issue.",
            tips: [
                "Contact your system administrator immediately as this may indicate a security compromise.",
                "Verify the server's host key fingerprint before continuing.",
                "Do not proceed with the connection until the server's identity is confirmed.",
            ],
        },
    ];

    connectionFailures.forEach((correlation) => {
        correlator.addCorrelation("All" as ZoweExplorerApiType, "ssh", correlation);
    });
}

function registerMemoryFailures(correlator: ErrorCorrelator): void {
    const memoryFailures: ErrorCorrelation[] = [
        {
            errorCode: "FOTS4314",
            matches: [/xreallocarray: out of memory \(elements \d+ of \d+ bytes\)/, "FOTS4314"],
            summary:
                "SSH client ran out of memory during array reallocation. The operation requires more memory than available.",
            tips: [
                "Close other applications to free up system memory.",
                "Restart the SSH client to clear any memory leaks.",
                "Contact your system administrator if the issue persists.",
                "Consider reducing the size of data being transferred or processed.",
            ],
        },
        {
            errorCode: "FOTS4315",
            matches: [/xrecallocarray: out of memory \(elements \d+ of \d+ bytes\)/, "FOTS4315"],
            summary:
                "SSH client ran out of memory during array recalculation. Insufficient memory available for the operation.",
            tips: [
                "Close unnecessary applications to free up memory.",
                "Restart the SSH session to clear memory usage.",
                "Try breaking large operations into smaller chunks.",
                "Contact your system administrator if memory issues persist.",
            ],
        },
        {
            errorCode: "FOTS4216",
            matches: ["Couldn't allocate session state", "FOTS4216"],
            summary:
                "SSH client failed to allocate memory for session state. The system is likely low on available memory.",
            tips: [
                "Free up system memory by closing other applications.",
                "Restart the SSH client application.",
                "Check available system memory and disk space.",
                "Contact your system administrator if the problem continues.",
            ],
        },
        {
            errorCode: "FOTS4311",
            matches: [/.*: cound not allocate state/, "FOTS4311"],
            summary: "SSH client could not allocate memory for internal state management.",
            tips: [
                "Restart the SSH client to clear memory usage.",
                "Close other memory-intensive applications.",
                "Check that your system has sufficient available memory.",
                "Try the operation again after freeing up system resources.",
            ],
        },
    ];

    memoryFailures.forEach((correlation) => {
        correlator.addCorrelation("All" as ZoweExplorerApiType, "ssh", correlation);
    });
}

function registerFileSystemErrors(correlator: ErrorCorrelator): void {
    const fileSystemErrors: ErrorCorrelation[] = [
        {
            errorCode: "FSUM6260",
            matches: [/write error on file ".*"/, "FSUM6260", "Failed to upload server PAX file with RC 4: Error: Failure"],
            summary:
                "Failed to write to file. The file may be read-only, the disk may be full, or there may be permission issues.",
            tips: [
                "Check that you have write permissions for the target file and directory.",
                "Verify that the disk has sufficient free space.",
                "Ensure the file is not locked by another process.",
                "Check if the file system is mounted as read-only.",
                "Try writing to a different location to isolate the issue.",
            ],
            resources: [
                {
                    href: "https://www.ibm.com/docs/en/zos/2.4.0?topic=messages-fsum6260",
                    title: "IBM z/OS UNIX System Services Messages - FSUM6260",
                },
            ],
        },
        {
            errorCode: "FOTS4151",
            matches: [/openpty: .*/, "FOTS4151"],
            summary: "Failed to open a pseudo-terminal (pty). This is required for SSH shell sessions.",
            tips: [
                "Check that your system supports pseudo-terminals.",
                "Verify that the /dev/pts file system is mounted and accessible.",
                "Contact your system administrator to check pty configuration.",
                "Try restarting the SSH service if you have administrative access.",
            ],
        },
        {
            errorCode: "FOTS4154",
            matches: ["ssh_packet_set_connection failed", "FOTS4154"],
            summary: "SSH client failed to establish packet connection. This indicates a low-level connection failure.",
            tips: [
                "Check your network connectivity to the SSH server.",
                "Verify that the SSH server is running and accessible.",
                "Try connecting again after a few minutes.",
                "Contact your network administrator if network issues persist.",
            ],
        },
        {
            errorCode: "FOTS4150",
            matches: [/kex_setup: .*/, "FOTS4150"],
            summary: "SSH key exchange setup failed. The client and server could not agree on encryption parameters.",
            tips: [
                "Check that the SSH client and server support compatible encryption algorithms.",
                "Verify the SSH configuration allows the necessary key exchange methods.",
                "Contact your system administrator to review SSH server configuration.",
                "Try using a different SSH client or version if possible.",
            ],
        },
        {
            errorCode: "FOTS4312",
            matches: [/.*: cipher_init failed: .*/, "FOTS4312"],
            summary: "SSH cipher initialization failed. The client could not initialize the encryption cipher.",
            tips: [
                "Check that the SSH client and server support compatible cipher algorithms.",
                "Verify that the SSH configuration allows the necessary encryption methods.",
                "Contact your system administrator to review cipher configuration.",
                "Try using a different encryption algorithm if supported.",
            ],
        },
    ];

    fileSystemErrors.forEach((correlation) => {
        correlator.addCorrelation("All" as ZoweExplorerApiType, "ssh", correlation);
    });
}
