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

export interface ISshErrorDefinition {
    summary: string;
    matches: (string | RegExp)[];
    tips?: string[];
    resources?: { href: string; title: string }[];
}

export const SshErrors: Record<string, ISshErrorDefinition> = {
    // Connection Failures
    FOTS4241: {
        summary:
            "SSH authentication failed. The provided credentials are invalid or the authentication method is not supported.",
        matches: ["Authentication failed.", /FOTS4241.*Authentication failed/],
        tips: [
            "Verify that your username and password are correct.",
            "Check if your account is locked or expired on the mainframe system.",
            "Ensure the SSH profile is configured with the correct authentication method (password, key, or certificate).",
            "Contact your system administrator if the issue persists.",
        ],
        resources: [
            {
                href: "https://www.ibm.com/docs/en/zos/2.5.0?topic=messages-fots4241",
                title: "IBM z/OS OpenSSH Messages - FOTS4241",
            },
            {
                href: "https://www.ibm.com/docs/en/zos/2.5.0?topic=daemon-format-authorized-keys-file",
                title: "IBM z/OS - Format of authorized_keys file",
            },
            {
                href: "https://www.ibm.com/docs/en/zos/2.5.0?topic=openssh-setting-up-user-authentication",
                title: "IBM z/OS OpenSSH - Setting up user authentication",
            },
        ],
    },
    FOTS4134: {
        summary: "The SSH client version uses an unsafe key agreement protocol that is not supported by the server.",
        matches: [/Client version ".*" uses unsafe key agreement; refusing connection/, "FOTS4134"],
        tips: [
            "Update your SSH client to use a more secure key exchange algorithm.",
            "Contact your system administrator to check the server's supported key exchange methods.",
            "Consider using a newer version of the SSH protocol if available.",
        ],
        resources: [
            {
                href: "https://www.ibm.com/docs/en/zos/2.5.0?topic=messages-fots4134",
                title: "IBM z/OS OpenSSH Messages - FOTS4134",
            },
            {
                href: "https://www.ibm.com/docs/en/zos/3.1.0?topic=ibmssh-zos31mig-sshd-config",
                title: "IBM z/OS ZOS31MIG_SSHD_CONFIG migration (for z/OS v3.1.0)",
            },
            {
                href: "https://www.ibm.com/docs/en/zos/2.5.0?topic=troubleshooting-frequently-asked-questions",
                title: "IBM z/OS OpenSSH - Frequently Asked Questions",
            },
        ],
    },
    FOTS4231: {
        summary: "The SSH server version uses an unsafe key agreement protocol that is not supported by the client.",
        matches: [/Server version ".*" uses unsafe key agreement; refusing connection/, "FOTS4231"],
        tips: [
            "Contact your system administrator to upgrade the SSH server configuration.",
            "Ask your administrator to enable more secure key exchange algorithms on the server.",
            "Verify that your client supports the server's configured key exchange methods.",
        ],
        resources: [
            {
                href: "https://www.ibm.com/docs/en/zos/2.5.0?topic=messages-fots4231",
                title: "IBM z/OS OpenSSH Messages - FOTS4231",
            },
        ],
    },
    FOTS4203: {
        summary:
            "The SSH server could not verify ownership of its private host keys, indicating a potential security issue.",
        matches: ["Server failed to confirm ownership of private host keys", "FOTS4203"],
        tips: [
            "Contact your system administrator immediately as this may indicate a security compromise.",
            "Verify the server's host key fingerprint before continuing.",
            "Do not proceed with the connection until the server's identity is confirmed.",
        ],
        resources: [
            {
                href: "https://www.ibm.com/docs/en/zos/2.5.0?topic=messages-fots4203",
                title: "IBM z/OS OpenSSH Messages - FOTS4203",
            },
            {
                href: "https://www.ibm.com/docs/en/zos/2.5.0?topic=troubleshooting-frequently-asked-questions",
                title: "IBM z/OS OpenSSH - Frequently Asked Questions",
            },
        ],
    },
    FOTS4240: {
        summary:
            "SSH key exchange internal error occurred. This is typically caused by incompatible encryption parameters or corrupted key exchange data.",
        matches: ["kex_prop2buf: error", "FOTS4240"],
        tips: [
            "Try reconnecting to establish a fresh SSH session.",
            "Check that the SSH client and server support compatible key exchange algorithms.",
            "Contact your system administrator to review SSH server configuration and logs.",
            "If the problem persists, this may indicate a system-level issue requiring IBM support.",
        ],
        resources: [
            {
                href: "https://www.ibm.com/docs/en/zos/2.5.0?topic=messages-fots4240",
                title: "IBM z/OS OpenSSH Messages - FOTS4240",
            },
            {
                href: "https://www.ibm.com/docs/en/zos/2.5.0?topic=troubleshooting-frequently-asked-questions",
                title: "IBM z/OS OpenSSH - Frequently Asked Questions",
            },
        ],
    },

    // Memory Failures
    FOTS4314: {
        summary:
            "SSH client ran out of memory during array reallocation. The operation requires more memory than available.",
        matches: [/xreallocarray: out of memory \(elements \d+ of \d+ bytes\)/, "FOTS4314"],
        tips: [
            "Close other applications to free up system memory.",
            "Restart the SSH client to clear any memory leaks.",
            "Contact your system administrator if the issue persists.",
            "Consider reducing the size of data being transferred or processed.",
            "Ensure that your system has enough storage allocated for the operation.",
        ],
        resources: [
            {
                href: "https://www.ibm.com/docs/en/zos/2.5.0?topic=messages-fots4314",
                title: "IBM z/OS OpenSSH Messages - FOTS4314",
            },
        ],
    },
    FOTS4315: {
        summary:
            "SSH client ran out of memory when requesting storage. Insufficient memory available for the operation.",
        matches: [/xrecallocarray: out of memory \(elements \d+ of \d+ bytes\)/, "FOTS4315"],
        tips: [
            "Close unnecessary applications to free up memory.",
            "Restart the SSH session to clear memory usage.",
            "Try breaking large operations into smaller chunks.",
            "Contact your system administrator if memory issues persist.",
            "Ensure that your system has enough storage allocated for the operation.",
        ],
        resources: [
            {
                href: "https://www.ibm.com/docs/en/zos/2.5.0?topic=messages-fots4315",
                title: "IBM z/OS OpenSSH Messages - FOTS4315",
            },
        ],
    },
    FOTS4216: {
        summary:
            "SSH client failed to allocate memory for session state. The system is likely low on available memory.",
        matches: ["Couldn't allocate session state", "FOTS4216"],
        tips: [
            "Free up system memory by closing other applications.",
            "Restart the SSH client application.",
            "Check available system memory and disk space.",
            "Contact your system administrator if the problem continues.",
        ],
        resources: [
            {
                href: "https://www.ibm.com/docs/en/zos/2.5.0?topic=messages-fots4216",
                title: "IBM z/OS OpenSSH Messages - FOTS4216",
            },
        ],
    },
    FOTS4311: {
        summary: "SSH client could not allocate memory for internal state management.",
        matches: ["could not allocate state", "FOTS4311"],
        tips: [
            "Restart the SSH client to clear memory usage.",
            "Close other memory-intensive applications.",
            "Check that your system has sufficient available memory.",
            "Try the operation again after freeing up system resources.",
        ],
        resources: [
            {
                href: "https://www.ibm.com/docs/en/zos/2.5.0?topic=messages-fots4311",
                title: "IBM z/OS OpenSSH Messages - FOTS4311",
            },
            {
                href: "https://www.ibm.com/docs/en/zos/2.5.0?topic=troubleshooting-frequently-asked-questions",
                title: "IBM z/OS OpenSSH - Frequently Asked Questions",
            },
        ],
    },

    // File System Errors
    FSUM6260: {
        summary:
            "Failed to write to file. The file may be read-only, the disk may be full, or there may be permission issues.",
        matches: [
            /write error on file ".*"/,
            "FSUM6260",
            "Failed to upload server PAX file with RC 4: Error: Failure",
            "RC 4: Error: Failure",
        ],
        tips: [
            "Check that you have write permissions for the target file and directory.",
            "Verify that the disk has sufficient free space.",
            "Ensure the file is not locked by another process.",
            "Check if the file system is mounted as read-only.",
            "Try deploying to a different location to isolate the issue.",
        ],
        resources: [
            {
                href: "https://www.ibm.com/docs/en/zos/2.5.0?topic=fsum-fsum6260",
                title: "IBM z/OS UNIX System Services Messages - FSUM6260",
            },
            {
                href: "https://www.ibm.com/docs/en/zos/2.5.0?topic=scd-df-display-amount-free-space-in-file-system",
                title: "df (command) - Display amount of free space in file system",
            },
        ],
    },
    EDC5133I: {
        summary:
            "The operation failed because there is no space left on the device. Please check the available disk space on the z/OS system.",
        matches: ["EDC5133I"],
    },
    FOTS4152: {
        summary: "Failed to open a pseudo-terminal (pty). This is required for SSH shell sessions.",
        matches: [/openpty returns device for which (.+?) fails/, "FOTS4152"],
        tips: [
            "Check that your system supports pseudo-terminals.",
            "Contact your system administrator to check pty configuration.",
            "Try restarting the SSH service if you have administrative access.",
            "Ensure that the pseudo-terminal device returned by openpty is valid and accessible.",
        ],
        resources: [
            {
                href: "https://www.ibm.com/docs/en/zos/2.5.0?topic=messages-fots4152",
                title: "IBM z/OS OpenSSH Messages - FOTS4152",
            },
        ],
    },
    FOTS4154: {
        summary: "SSH client failed to establish packet connection. This indicates a low-level connection failure.",
        matches: ["ssh_packet_set_connection failed", "FOTS4154"],
        tips: [
            "Check your network connectivity to the SSH server.",
            "Verify that the SSH server is running and accessible.",
            "Try connecting again after a few minutes.",
            "Contact your network administrator if network issues persist.",
        ],
        resources: [
            {
                href: "https://www.ibm.com/docs/en/zos/2.5.0?topic=messages-fots4154",
                title: "IBM z/OS OpenSSH Messages - FOTS4154",
            },
        ],
    },
    FOTS4150: {
        summary: "SSH key exchange setup failed. The client and server could not agree on encryption parameters.",
        matches: [/kex_setup: .*/, "FOTS4150"],
        tips: [
            "Check that the SSH client and server support compatible encryption algorithms.",
            "Verify the SSH configuration allows the necessary key exchange methods.",
            "Contact your system administrator to review SSH server configuration.",
            "Try using a different SSH client or version if possible.",
        ],
        resources: [
            {
                href: "https://www.ibm.com/docs/en/zos/2.5.0?topic=messages-fots4150",
                title: "IBM z/OS OpenSSH Messages - FOTS4150",
            },
        ],
    },
    FOTS4312: {
        summary: "SSH cipher initialization failed. The client could not initialize the encryption cipher.",
        matches: ["cipher_init failed:", "FOTS4312"],
        tips: [
            "Check that the SSH client and server support compatible cipher algorithms.",
            "Verify that the SSH configuration allows the necessary encryption methods.",
            "Contact your system administrator to review cipher configuration.",
            "Try using a different encryption algorithm if supported.",
        ],
        resources: [
            {
                href: "https://www.ibm.com/docs/en/zos/2.5.0?topic=messages-fots4312",
                title: "IBM z/OS OpenSSH Messages - FOTS4312",
            },
        ],
    },
};

/**
 * Common patterns that indicate SSH private key authentication failures
 */
export const PrivateKeyFailurePatterns = [
    "All configured authentication methods failed",
    "Cannot parse privateKey: Malformed OpenSSH private key",
    "but no passphrase given",
    "integrity check failed",
    "Permission denied (publickey,password)",
    "Permission denied (publickey)",
    "Authentication failed",
    "Invalid private key",
    "privateKey value does not contain a (valid) private key",
    "Cannot parse privateKey",
];
