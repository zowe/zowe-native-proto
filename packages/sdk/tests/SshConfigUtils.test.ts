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

import { readFileSync } from "node:fs";
import { join, normalize, resolve } from "node:path";
import { SshConfigUtils } from "../src/SshConfigUtils";

vi.mock("node:os", () => ({
    homedir: vi.fn(() => "/home/dir"),
}));

describe("findPrivateKeys", () => {
    it("should find private keys in home directory", async () => {
        const homeDir = "/home/dir";
        const expected = [
            resolve(join(homeDir, ".ssh", "id_ed25519")),
            resolve(join(homeDir, ".ssh", "id_rsa")),
            resolve(join(homeDir, ".ssh", "id_ecdsa")),
            resolve(join(homeDir, ".ssh", "id_dsa")),
        ];
        expect(await SshConfigUtils.findPrivateKeys()).toStrictEqual(expected);
    });
});

vi.mock("node:fs", () => ({
    accessSync: vi.fn(),
    readFileSync: vi.fn(),
    constants: {
        R_OK: vi.fn(),
    },
}));
vi.mock("node:os", () => ({
    homedir: vi.fn(() => "/home/dir"),
}));

describe("findPrivateKeys", () => {
    it("should find private keys in home directory", async () => {
        const homeDir = "/home/dir";
        const expected = [
            resolve(join(homeDir, ".ssh", "id_ed25519")),
            resolve(join(homeDir, ".ssh", "id_rsa")),
            resolve(join(homeDir, ".ssh", "id_ecdsa")),
            resolve(join(homeDir, ".ssh", "id_dsa")),
        ];
        expect(await SshConfigUtils.findPrivateKeys()).toStrictEqual(expected);
    });
});

describe("migrateSshConfig", () => {
    const mockReadFileSync = readFileSync as ReturnType<typeof vi.fn>;

    beforeEach(() => {
        vi.clearAllMocks();
    });

    it("should return empty array when config file does not exist", async () => {
        mockReadFileSync.mockImplementation(() => {
            throw new Error("File not found");
        });

        const result = await SshConfigUtils.migrateSshConfig();
        expect(result).toStrictEqual([]);
    });

    it("should parse a basic SSH config with hostname and user", async () => {
        const configContent = `
Host myserver
    HostName example.com
    User myuser
`;
        mockReadFileSync.mockReturnValue(configContent);

        const result = await SshConfigUtils.migrateSshConfig();
        expect(result).toHaveLength(1);
        expect(result[0]).toMatchObject({
            name: "myserver",
            hostname: "example.com",
            user: "myuser",
        });
    });

    it("should parse SSH config with all supported fields", async () => {
        const configContent = `
Host production
    HostName prod.example.com
    Port 2222
    User admin
    IdentityFile ~/.ssh/prod_key
    ConnectTimeout 30
`;
        mockReadFileSync.mockReturnValue(configContent);

        const result = await SshConfigUtils.migrateSshConfig();
        expect(result).toHaveLength(1);
        expect(result[0]).toMatchObject({
            name: "production",
            hostname: "prod.example.com",
            port: 2222,
            user: "admin",
            privateKey: normalize(join("/home/dir", ".ssh", "prod_key")),
            handshakeTimeout: 30000,
        });
    });

    it("should handle multiple host configurations", async () => {
        const configContent = `
Host server1
    HostName host1.example.com
    User user1

Host server2
    HostName host2.example.com
    User user2
    Port 22222
`;
        mockReadFileSync.mockReturnValue(configContent);

        const result = await SshConfigUtils.migrateSshConfig();
        expect(result).toHaveLength(2);
        expect(result[0].name).toBe("server1");
        expect(result[1].name).toBe("server2");
        expect(result[1].port).toBe(22222);
    });

    it("should take first name when Host has multiple values", async () => {
        const configContent = `
Host alias1 alias2 alias3
    HostName example.com
`;
        mockReadFileSync.mockReturnValue(configContent);

        const result = await SshConfigUtils.migrateSshConfig();
        expect(result[0].name).toBe("alias1");
    });

    it("should ignore unsupported SSH config parameters", async () => {
        const configContent = `
Host server
    HostName example.com
    ForwardAgent yes
    ProxyJump bastion
    ServerAliveInterval 60
`;
        mockReadFileSync.mockReturnValue(configContent);

        const result = await SshConfigUtils.migrateSshConfig();
        expect(result[0]).toMatchObject({
            name: "server",
            hostname: "example.com",
        });
        expect(result[0]).not.toHaveProperty("forwardAgent");
        expect(result[0]).not.toHaveProperty("proxyJump");
    });

    it("should handle config with only Host directive and no parameters", async () => {
        const configContent = `
Host emptyhost
`;
        mockReadFileSync.mockReturnValue(configContent);

        const result = await SshConfigUtils.migrateSshConfig();
        expect(result).toHaveLength(1);
        expect(result[0]).toMatchObject({
            name: "emptyhost",
        });
    });

    it("should handle empty config file", async () => {
        mockReadFileSync.mockReturnValue("");

        const result = await SshConfigUtils.migrateSshConfig();
        expect(result).toStrictEqual([]);
    });

    it("should handle config with comments and whitespace", async () => {
        const configContent = `
# This is a comment
Host myserver
    # Another comment
    HostName example.com
    User myuser
    # More comments
`;
        mockReadFileSync.mockReturnValue(configContent);

        const result = await SshConfigUtils.migrateSshConfig();
        expect(result).toHaveLength(1);
        expect(result[0]).toMatchObject({
            name: "myserver",
            hostname: "example.com",
            user: "myuser",
        });
    });

    it("should convert ConnectTimeout from seconds to milliseconds", async () => {
        const configContent = `
Host server
    HostName example.com
    ConnectTimeout 45
`;
        mockReadFileSync.mockReturnValue(configContent);

        const result = await SshConfigUtils.migrateSshConfig();
        expect(result[0].handshakeTimeout).toBe(45000);
    });
});
