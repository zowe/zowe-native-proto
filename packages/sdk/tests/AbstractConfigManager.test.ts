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

import { MESSAGE_TYPE, type inputBoxOpts, type qpItem, type qpOpts } from "../src/doc";
import { AbstractConfigManager, type ProgressCallback } from "../src/AbstractConfigManager";
import {
    ConfigBuilder,
    ConfigSchema,
    ConfigUtils,
    type IProfileLoaded,
    type IProfileTypeConfiguration,
    type ProfileInfo,
} from "@zowe/imperative";
import { ZoweVsCodeExtension } from "@zowe/zowe-explorer-api";
import { type ISshConfigExt, ZClientUtils } from "../src/ZClientUtils";
import type { Config } from "node-ssh";
import { normalize } from "path";

vi.mock("path", () => ({
    normalize: vi.fn(() => (p: string) => p),
}));
vi.mock("ssh2");
vi.mock("node:fs", () => ({
    readFileSync: vi.fn(() => "mocked file content"),
}));
vi.mock("vscode");
vi.mock("@zowe/zowe-explorer-api", () => ({
    ZoweVsCodeExtension: {
        getZoweExplorerApi: () => ({
            getExplorerExtenderApi: () => ({
                getProfilesCache: () => ({
                    getProfileInfo: () =>
                        Promise.resolve({
                            getAllProfiles: (): ProfileInfo => [] as any,
                            getTeamConfig: (): Config => undefined,
                        }),
                }),
            }),
        }),
    },
}));

const sshProfiles: IProfileLoaded[] = [
    {
        message: "",
        name: "ssh1",
        type: "ssh",
        failNotFound: false,
        profile: { host: "lpar1.com", port: 22, user: "zoweUser1", privateKey: "/path/to/id_rsa" },
    },
    {
        message: "",
        name: "ssh2",
        type: "ssh",
        failNotFound: false,
        profile: { host: "lpar2.com", port: 22, user: "zoweUser2", password: "password" },
    },
];

const migratedSshConfigs: ISshConfigExt[] = [
    {
        hostname: "lpar3.com",
        name: "SSHlpar3",
        port: 22,
        privateKey: "/Users/users/.ssh/id_rsa",
        user: "zoweUser3",
    },
    {
        hostname: "lpar4.com",
        name: "SSHlpar4",
        port: 22,
        privateKey: "/Users/users/.ssh/id_rsa",
        user: "zoweUser4",
    },
];

export class TestAbstractConfigManager extends AbstractConfigManager {
    // All overridden methods are initialized as spies that do nothing
    public showMessage = vi.fn<(message: string, type: MESSAGE_TYPE) => void>();

    public showInputBox = vi.fn<(opts: inputBoxOpts) => Promise<string | undefined>>().mockResolvedValue(undefined);

    public withProgress = vi
        .fn()
        .mockImplementation(
            <T>(_message: string, task: (progress: ProgressCallback) => Promise<T>): Promise<T> => task(() => {}),
        );

    public showMenu = vi.fn<(opts: qpOpts) => Promise<string | undefined>>().mockResolvedValue(undefined);

    public showCustomMenu = vi.fn<(opts: qpOpts) => Promise<qpItem | undefined>>().mockResolvedValue(undefined);

    public getCurrentDir = vi.fn<() => string | undefined>().mockReturnValue("/mock/dir");

    public getProfileSchemas = vi.fn<() => IProfileTypeConfiguration[]>().mockReturnValue([]);
}

describe("AbstractConfigManager", async () => {
    let profCache: ProfileInfo;
    let testManager: TestAbstractConfigManager;
    beforeAll(async () => {
        profCache = await ZoweVsCodeExtension.getZoweExplorerApi()
            .getExplorerExtenderApi()
            .getProfilesCache()
            .getProfileInfo();
    });
    beforeEach(async () => {
        testManager = new TestAbstractConfigManager(profCache);
        vi.spyOn(testManager, "fetchAllSshProfiles" as any).mockReturnValueOnce(sshProfiles);
        vi.spyOn(ZClientUtils, "migrateSshConfig").mockResolvedValueOnce(migratedSshConfigs);
    });
    describe("promptForProfile", async () => {
        describe("profile selection and input", () => {
            // Common test data
            const mockProfile = { attrs: "fake" };
            const mockMenuItems = [
                { label: "$(plus) Add New SSH Host..." },
                { description: "lpar1.com", label: "ssh1" },
                { description: "lpar2.com", label: "ssh2" },
                { label: "Migrate From SSH Config", separator: true },
                { description: "lpar3.com", label: "SSHlpar3" },
                { description: "lpar4.com", label: "SSHlpar4" },
            ];

            const mockValidConfig = { privateKey: "/path/to/id_rsa", keyPassphrase: "testKP" };
            const mockNewProfile = { name: "sshProfile", port: 22 };

            // Helper function to setup common mocks
            const setupProfileCreationMocks = () => {
                vi.spyOn(testManager as any, "createZoweSchema").mockImplementation(() => {});
                vi.spyOn(testManager as any, "getNewProfileName").mockReturnValue(undefined);
                testManager.getCurrentDir.mockReturnValue(undefined);
            };

            beforeEach(() => {
                vi.clearAllMocks();
            });

            describe("when profile name is provided", () => {
                it("should return merged attributes", async () => {
                    vi.spyOn(testManager as any, "getMergedAttrs").mockReturnValueOnce(mockProfile);

                    const result = await testManager.promptForProfile("testProfile");

                    expect(result).toEqual({
                        profile: mockProfile,
                        message: "",
                        failNotFound: false,
                        type: "ssh",
                    });
                });
            });

            describe("when no profile name is provided", () => {
                it("should display custom menu with correct options", async () => {
                    const showCustomMenuSpy = vi.spyOn(testManager, "showCustomMenu").mockResolvedValueOnce(undefined);

                    await testManager.promptForProfile();

                    expect(showCustomMenuSpy).toHaveBeenCalledWith({
                        items: mockMenuItems,
                        placeholder: "Select configured SSH host or enter user@host",
                    });
                });

                describe("profile validation", () => {
                    const mockSelection = { description: "lpar1.com", label: "ssh1" };

                    it("should set profile when validation passes with empty config", async () => {
                        vi.spyOn(testManager, "showCustomMenu").mockResolvedValueOnce(mockSelection);
                        const validateSpy = vi.spyOn(testManager as any, "validateConfig").mockReturnValue({});
                        const setProfileSpy = vi.spyOn(testManager as any, "setProfile").mockImplementation(() => {});

                        await testManager.promptForProfile();

                        expect(validateSpy).toHaveBeenCalled();
                        expect(setProfileSpy).toHaveBeenCalledWith({}, "ssh1");
                    });

                    it("should set profile when validation returns modified config", async () => {
                        vi.spyOn(testManager, "showCustomMenu").mockResolvedValueOnce(mockSelection);
                        const validateSpy = vi
                            .spyOn(testManager as any, "validateConfig")
                            .mockReturnValue(mockValidConfig);
                        const setProfileSpy = vi.spyOn(testManager as any, "setProfile").mockImplementation(() => {});

                        const result = await testManager.promptForProfile();

                        expect(result).toBeDefined();
                        expect(validateSpy).toHaveBeenCalled();
                        expect(setProfileSpy).toHaveBeenCalledWith(mockValidConfig, "ssh1");
                    });

                    it("should return undefined when validation fails", async () => {
                        vi.spyOn(testManager, "showCustomMenu").mockResolvedValueOnce(mockSelection);
                        const validateSpy = vi.spyOn(testManager as any, "validateConfig").mockReturnValue(undefined);
                        const setProfileSpy = vi.spyOn(testManager as any, "setProfile");

                        const result = await testManager.promptForProfile();

                        expect(result).toBeUndefined();
                        expect(validateSpy).toHaveBeenCalled();
                        expect(setProfileSpy).not.toHaveBeenCalled();
                    });
                });

                describe("new profile creation", () => {
                    beforeEach(() => {
                        setupProfileCreationMocks();
                    });

                    it("should handle creating new profile with valid input", async () => {
                        vi.spyOn(testManager, "showCustomMenu").mockResolvedValueOnce({
                            label: "$(plus) Add New SSH Host...",
                        });
                        vi.spyOn(testManager as any, "createNewProfile").mockReturnValueOnce(mockNewProfile);

                        await testManager.promptForProfile();

                        // Test passes if no errors are thrown during profile creation
                    });

                    it("should return undefined when profile creation fails", async () => {
                        vi.spyOn(testManager, "showCustomMenu").mockResolvedValueOnce({
                            label: "$(plus) Add New SSH Host...",
                        });
                        vi.spyOn(testManager as any, "createNewProfile").mockReturnValueOnce(undefined);

                        const result = await testManager.promptForProfile();

                        expect(result).toBeUndefined();
                    });
                });

                describe("custom SSH host input", () => {
                    it("should handle custom SSH host entry", async () => {
                        const customHostSelection = {
                            description: "Custom SSH Host",
                            label: "ssh user@example.com",
                        };

                        vi.spyOn(testManager, "showCustomMenu").mockResolvedValueOnce(customHostSelection);
                        const createNewProfileSpy = vi
                            .spyOn(testManager as any, "createNewProfile")
                            .mockReturnValueOnce(mockNewProfile);
                        setupProfileCreationMocks();

                        await testManager.promptForProfile();

                        expect(createNewProfileSpy).toHaveBeenCalledWith("ssh user@example.com");
                    });
                });
            });
        });
        describe("profile validation sequence", async () => {
            // Common mock data
            const mockProfileData = {
                port: 22,
                host: "lpar1.com",
                user: "zoweUser",
                privateKey: "/path/to/id_rsa",
            };

            const mockProfileWithoutKey = {
                port: 22,
                host: "lpar1.com",
                user: "zoweUser",
            };

            // Helper function to setup common mocks
            const setupCommonMocks = (
                profileData: typeof mockProfileData | typeof mockProfileWithoutKey = mockProfileData,
            ) => {
                vi.spyOn(testManager, "showCustomMenu").mockResolvedValueOnce({
                    label: "$(plus) Add New SSH Host...",
                });
                vi.spyOn(testManager as any, "createNewProfile").mockReturnValueOnce(profileData);
                vi.spyOn(testManager as any, "getNewProfileName").mockReturnValueOnce({
                    name: "newProfileName",
                    ...profileData,
                });
                testManager.getCurrentDir.mockReturnValue(undefined);
            };

            beforeEach(async () => {
                vi.spyOn(testManager as any, "validateConfig").mockReturnValue({});
                vi.spyOn(testManager as any, "setProfile").mockImplementation(() => {});
            });

            describe("SSH config validation scenarios", () => {
                it("should validate migrated SSH config with valid privateKey", async () => {
                    setupCommonMocks();
                    vi.spyOn(testManager as any, "validateConfig").mockReturnValue({});

                    const result = await testManager.promptForProfile();

                    expect(result).toBeDefined();
                });

                it("should find local privateKey when attached key is invalid", async () => {
                    setupCommonMocks();
                    vi.spyOn(testManager as any, "validateConfig").mockReturnValue(undefined);

                    const validatePrivKeySpy = vi
                        .spyOn(testManager as any, "validateFoundPrivateKeys")
                        .mockImplementationOnce(() => {
                            (testManager as any).validationResult = {};
                        });

                    const result = await testManager.promptForProfile();

                    expect(result).toBeDefined();
                    expect(validatePrivKeySpy).toHaveBeenCalledOnce();
                });

                it("should handle invalid privateKey with available local key", async () => {
                    setupCommonMocks();
                    vi.spyOn(testManager as any, "validateConfig")
                        .mockReturnValueOnce(undefined)
                        .mockReturnValueOnce({ privateKey: "/path/to/id_rsa" });

                    const validatePrivKeySpy = vi
                        .spyOn(testManager as any, "validateFoundPrivateKeys")
                        .mockImplementationOnce(() => {});

                    const result = await testManager.promptForProfile();

                    expect(result).toBeDefined();
                    expect(validatePrivKeySpy).toHaveBeenCalledOnce();
                });

                it("should cancel SSH setup when no privateKey is available", async () => {
                    setupCommonMocks(mockProfileWithoutKey);
                    vi.spyOn(testManager as any, "validateConfig").mockReturnValue(undefined);

                    const validatePrivKeySpy = vi
                        .spyOn(testManager as any, "validateFoundPrivateKeys")
                        .mockImplementationOnce(() => {});

                    const result = await testManager.promptForProfile();

                    expect(result).toBeUndefined();
                    expect(validatePrivKeySpy).toHaveBeenCalledOnce();
                    expect(testManager.showMessage).toHaveBeenCalledWith("SSH setup cancelled.", MESSAGE_TYPE.WARNING);
                });
            });
        });
    });
    describe("createNewProfile", async () => {
        let showInputBoxSpy: any;

        beforeEach(() => {
            showInputBoxSpy = vi.spyOn(testManager, "showInputBox");
        });

        describe("valid inputs", () => {
            it("should parse basic SSH connection", async () => {
                showInputBoxSpy.mockResolvedValue("ssh user@example.com");

                const result = await (testManager as any).createNewProfile();

                expect(result).toStrictEqual({
                    user: "user",
                    hostname: "example.com",
                });
            });

            it("should parse SSH connection with port and identity file", async () => {
                showInputBoxSpy.mockResolvedValue("ssh user@example.com -p 22 -i /path/to/id_rsa");

                const result = await (testManager as any).createNewProfile();

                expect(result).toStrictEqual({
                    user: "user",
                    hostname: "example.com",
                    port: 22,
                    privateKey: "/path/to/id_rsa",
                });
            });

            it("should accept pre-configured connection string", async () => {
                const result = await (testManager as any).createNewProfile("user@example.com");

                expect(result).toBeDefined();
            });
        });

        describe("invalid inputs", () => {
            const testCases = [
                {
                    name: "should handle cancelled input",
                    input: undefined,
                    expectedMessage: "SSH setup cancelled.",
                    expectedType: MESSAGE_TYPE.WARNING,
                },
                {
                    name: "should reject malformed SSH command",
                    input: "bad ssh input",
                    expectedMessage: "Invalid SSH command format. Ensure it matches the expected pattern.",
                    expectedType: MESSAGE_TYPE.ERROR,
                },
                {
                    name: "should reject invalid port",
                    input: "ssh user@example.com -p badPort",
                    expectedMessage: "Invalid value for flag -p. Port must be a valid number.",
                    expectedType: MESSAGE_TYPE.ERROR,
                },
                {
                    name: "should reject missing port value",
                    input: "ssh user@example.com -p",
                    expectedMessage: "Missing value for flag -p.",
                    expectedType: MESSAGE_TYPE.ERROR,
                },
                {
                    name: "should reject unquoted paths with spaces",
                    input: "ssh user@example.com -i /path/with spaces/keyfile",
                    expectedMessage: "Invalid value for flag -i. Values with spaces must be quoted.",
                    expectedType: MESSAGE_TYPE.ERROR,
                    setup: () => {
                        vi.spyOn((testManager as any).flagRegex, "exec").mockReturnValueOnce([
                            "-i /path/with spaces/keyfile",
                            "i",
                            "/path/with spaces/keyfile",
                        ]);
                    },
                },
            ];

            testCases.forEach(({ name, input, expectedMessage, expectedType, setup }) => {
                it(name, async () => {
                    if (setup) setup();
                    if (input !== undefined) showInputBoxSpy.mockResolvedValue(input);

                    const result = await (testManager as any).createNewProfile();

                    expect(result).toBeUndefined();
                    expect(testManager.showMessage).toHaveBeenCalledWith(expectedMessage, expectedType);
                });
            });

            it("should reject invalid pre-configured connection string", async () => {
                const result = await (testManager as any).createNewProfile("bad config options");

                expect(result).toBeUndefined();
                expect(testManager.showMessage).toHaveBeenCalledWith(
                    "Invalid custom connection format. Ensure it matches the expected pattern.",
                    MESSAGE_TYPE.ERROR,
                );
            });
        });
    });
    describe("createZoweSchema", async () => {
        let testManager: TestAbstractConfigManager;
        let mockTeamConfig: any;
        let mockConfigApi: any;
        let mockLayers: any;

        const baseSchema = {
            type: "base",
            schema: { title: "Base Profile", description: "Base connection info", type: "object", properties: {} },
        };
        const zosmfSchema = {
            type: "zosmf",
            schema: { title: "z/OSMF Profile", description: "z/OSMF connection info", type: "object", properties: {} },
        };
        const defaultSchemas = [baseSchema, zosmfSchema];
        const homeDir = "/home/user/.zowe";
        const workspaceDir = "/mock/dir";

        beforeEach(async () => {
            mockLayers = { activate: vi.fn(), merge: vi.fn(), write: vi.fn() };
            mockConfigApi = { layers: mockLayers };
            mockTeamConfig = { layerExists: vi.fn(), api: mockConfigApi, setSchema: vi.fn() };

            const profCache = await ZoweVsCodeExtension.getZoweExplorerApi()
                .getExplorerExtenderApi()
                .getProfilesCache()
                .getProfileInfo();

            testManager = new TestAbstractConfigManager(profCache);
            vi.spyOn((testManager as any).mProfilesCache, "getTeamConfig").mockReturnValue(mockTeamConfig);
            testManager.getProfileSchemas.mockReturnValue(defaultSchemas);
        });

        async function runCreateSchema(globalFlag: boolean, layerExists: boolean, profileSchemas = defaultSchemas) {
            vi.spyOn(ConfigUtils, "getZoweDir").mockReturnValue(homeDir);
            if (!globalFlag) testManager.getCurrentDir.mockReturnValue(workspaceDir);
            testManager.getProfileSchemas.mockReturnValue(profileSchemas);
            mockTeamConfig.layerExists.mockReturnValue(layerExists);

            const mockNewConfig = { api: mockConfigApi, exists: true, layers: [] as any };
            vi.spyOn(ConfigSchema, "buildSchema").mockReturnValue({
                type: "object",
                description: "Test Schema",
                properties: {},
            } as any);
            vi.spyOn(ConfigBuilder, "build").mockResolvedValue(mockNewConfig as any);

            await (testManager as any).createZoweSchema(globalFlag);
            return { mockNewConfig };
        }

        describe.each`
            globalFlag | expectedDir
            ${true}    | ${homeDir}
            ${false}   | ${workspaceDir}
        `("layer exists (global=$globalFlag)", ({ globalFlag, expectedDir }) => {
            it("should return early if layer already exists", async () => {
                vi.spyOn(ConfigUtils, "getZoweDir").mockReturnValue(homeDir);
                if (!globalFlag) testManager.getCurrentDir.mockReturnValue(workspaceDir);
                mockTeamConfig.layerExists.mockReturnValue(true);

                await (testManager as any).createZoweSchema(globalFlag);

                expect(mockTeamConfig.layerExists).toHaveBeenCalledWith(expectedDir);
                expect(mockLayers.activate).not.toHaveBeenCalled();
                expect(mockTeamConfig.setSchema).not.toHaveBeenCalled();
            });
        });

        describe.each`
            globalFlag | expectedDesc
            ${true}    | ${"Test Schema"}
            ${false}   | ${"Test Schema"}
        `("schema creation success (global=$globalFlag)", ({ globalFlag, expectedDesc }) => {
            it("should create schema when layer doesn't exist", async () => {
                const { mockNewConfig } = await runCreateSchema(globalFlag, false);
                expect(mockLayers.activate).toHaveBeenCalledWith(false, globalFlag);
                expect(ConfigSchema.buildSchema).toHaveBeenCalledWith(defaultSchemas);
                expect(mockTeamConfig.setSchema).toHaveBeenCalledWith(
                    expect.objectContaining({ description: expectedDesc }),
                );
                expect(mockLayers.merge).toHaveBeenCalledWith(mockNewConfig);
                expect(mockLayers.write).toHaveBeenCalled();
            });
        });
        describe("validateConfig", () => {
            it("should return an empty object for a valid profile", async () => {
                vi.spyOn(testManager as any, "attemptConnection").mockResolvedValue(true);
                expect(
                    await (testManager as any).validateConfig(
                        { name: "ssh1", hostname: "lpar1.com", port: 22, user: "user1", privateKey: "/path/to/id_rsa" },
                        false,
                    ),
                ).toStrictEqual({});
            });

            it("should return a password for a partial profile", async () => {
                vi.spyOn(testManager, "showInputBox").mockResolvedValueOnce("user1").mockResolvedValueOnce("password1");
                vi.spyOn(testManager as any, "attemptConnection").mockResolvedValue(true);
                vi.spyOn(testManager as any, "promptForPassword").mockResolvedValue({ password: "password1" });

                expect(
                    await (testManager as any).validateConfig({ name: "ssh1", hostname: "lpar1.com", port: 22 }, true),
                ).toStrictEqual({ password: "password1", user: "user1" });
            });

            it("should return undefined for partial profile when no password prompt allowed", async () => {
                vi.spyOn(testManager, "showInputBox");
                vi.spyOn(testManager as any, "attemptConnection").mockResolvedValue(true);
                expect(
                    await (testManager as any).validateConfig(
                        { name: "ssh1", hostname: "lpar1.com", port: 22, user: "user1" },
                        false,
                    ),
                ).toBeUndefined();
            });

            it("should handle invalid username and return new user", async () => {
                vi.spyOn(testManager, "showInputBox").mockResolvedValueOnce("goodUser");
                vi.spyOn(testManager as any, "attemptConnection")
                    .mockRejectedValueOnce("Invalid username")
                    .mockResolvedValueOnce(true);

                expect(
                    await (testManager as any).validateConfig(
                        {
                            name: "ssh1",
                            hostname: "lpar1.com",
                            port: 22,
                            privateKey: "/path/to/id_rsa",
                            user: "badUser",
                        },
                        true,
                    ),
                ).toStrictEqual({ user: "goodUser" });
            });

            it("should return undefined if Invalid username and no new user provided", async () => {
                vi.spyOn(testManager, "showInputBox").mockResolvedValueOnce(undefined);
                vi.spyOn(testManager as any, "attemptConnection").mockRejectedValueOnce("Invalid username");

                expect(
                    await (testManager as any).validateConfig(
                        {
                            name: "ssh1",
                            hostname: "lpar1.com",
                            port: 22,
                            privateKey: "/path/to/id_rsa",
                            user: "badUser",
                        },
                        true,
                    ),
                ).toBeUndefined();
            });

            it("should handle passphrase errors and return keyPassphrase", async () => {
                vi.spyOn(testManager, "showInputBox").mockResolvedValueOnce("goodPass");
                vi.spyOn(testManager as any, "attemptConnection")
                    .mockRejectedValueOnce("but no passphrase given")
                    .mockResolvedValueOnce(true);

                expect(
                    await (testManager as any).validateConfig(
                        { name: "ssh1", hostname: "lpar1.com", port: 22, privateKey: "/path/to/id_rsa", user: "user1" },
                        true,
                    ),
                ).toStrictEqual({ keyPassphrase: "goodPass" });
            });

            it("should retry passphrase on integrity check failed", async () => {
                const msgSpy = vi.spyOn(testManager, "showMessage");
                vi.spyOn(testManager, "showInputBox")
                    .mockResolvedValueOnce("badPass")
                    .mockResolvedValueOnce("goodPass");
                vi.spyOn(testManager as any, "attemptConnection")
                    .mockRejectedValueOnce("integrity check failed")
                    .mockRejectedValueOnce("integrity check failed")
                    .mockResolvedValueOnce(true);

                expect(
                    await (testManager as any).validateConfig(
                        { name: "ssh1", hostname: "lpar1.com", port: 22, privateKey: "/path/to/id_rsa", user: "user1" },
                        true,
                    ),
                ).toStrictEqual({ keyPassphrase: "goodPass" });

                expect(msgSpy).toHaveBeenCalledWith(
                    expect.stringContaining("Passphrase Authentication Failed"),
                    expect.anything(),
                );
            });

            it("should handle All configured authentication methods failed with password prompt", async () => {
                vi.spyOn(testManager as any, "attemptConnection").mockRejectedValue(
                    "All configured authentication methods failed",
                );
                vi.spyOn(testManager as any, "promptForPassword").mockResolvedValue({ password: "pass123" });

                expect(
                    await (testManager as any).validateConfig(
                        { name: "ssh1", hostname: "lpar1.com", port: 22, user: "user1" },
                        true,
                    ),
                ).toStrictEqual({ password: "pass123" });
            });

            it("should return undefined for handshake timeout", async () => {
                const msgSpy = vi.spyOn(testManager, "showMessage");
                vi.spyOn(testManager as any, "attemptConnection").mockRejectedValue(
                    new Error("Timed out while waiting for handshake"),
                );

                expect(
                    await (testManager as any).validateConfig(
                        { name: "ssh1", hostname: "lpar1.com", port: 22, user: "user1", password: "badPassword" },
                        true,
                    ),
                ).toBeUndefined();
                expect(msgSpy).toHaveBeenCalledWith("Timed out while waiting for handshake", MESSAGE_TYPE.ERROR);
            });

            it("should return undefined for malformed private key", async () => {
                vi.spyOn(testManager as any, "attemptConnection").mockRejectedValue(
                    "Cannot parse privateKey: Malformed OpenSSH private key",
                );

                expect(
                    await (testManager as any).validateConfig(
                        { name: "ssh1", hostname: "lpar1.com", port: 22, user: "user1", privateKey: "/path/to/id_rsa" },
                        true,
                    ),
                ).toBeUndefined();
            });
            it("should return undefined for privateKey with no password if 'All configured authentication methods failed'", async () => {
                vi.spyOn(testManager as any, "attemptConnection").mockRejectedValue(
                    new Error("All configured authentication methods failed"),
                );

                const config = {
                    name: "ssh1",
                    hostname: "lpar1.com",
                    port: 22,
                    user: "user1",
                    privateKey: "/path/to/key",
                };
                const result = await (testManager as any).validateConfig(config, false);

                expect(result).toBeUndefined();
            });
            it("should return undefined if attemptConnection fails for new user", async () => {
                vi.spyOn(testManager, "showInputBox").mockResolvedValue("newUser");
                vi.spyOn(testManager as any, "attemptConnection").mockRejectedValue(new Error("Invalid username"));

                const result = await (testManager as any).validateConfig(
                    { name: "ssh1", hostname: "lpar1.com", port: 22, user: "badUser", privateKey: "/path/to/id_rsa" },
                    true,
                );

                expect(result).toBeUndefined();
            });

            it("should clear privateKey and keyPassphrase after 3 failed passphrase attempts", async () => {
                vi.spyOn(testManager, "showInputBox").mockResolvedValue("wrongPass"); // always fails
                vi.spyOn(testManager as any, "attemptConnection").mockRejectedValue(
                    new Error("integrity check failed"),
                );
                vi.spyOn(testManager, "showMessage").mockImplementation(() => {});

                const config = {
                    name: "ssh1",
                    hostname: "lpar1.com",
                    port: 22,
                    user: "user1",
                    privateKey: "/path/to/key",
                };
                const result = await (testManager as any).validateConfig(config, true);

                expect(result).toBeUndefined();
                expect(config.privateKey).toBeUndefined();
                expect((config as any).keyPassphrase).toBeUndefined();
            });

            it("should call promptForPassword when password missing and askForPassword is true", async () => {
                vi.spyOn(testManager as any, "attemptConnection").mockImplementation(() => {
                    throw new Error("All configured authentication methods failed");
                });
                vi.spyOn(testManager as any, "promptForPassword").mockResolvedValue({ password: "pw123" });

                const result = await (testManager as any).validateConfig(
                    {
                        name: "ssh1",
                        hostname: "lpar1.com",
                        port: 22,
                        user: "user1",
                        // Setting password to exit check
                        password: "badPw",
                        privateKey: undefined,
                    },
                    true,
                );

                expect(result).toStrictEqual({ password: "pw123" });
            });
            it("should call promptForPassword when password missing and askForPassword is false", async () => {
                vi.spyOn(testManager as any, "attemptConnection").mockImplementation(() => {
                    throw new Error("All configured authentication methods failed");
                });

                const result = await (testManager as any).validateConfig(
                    {
                        name: "ssh1",
                        hostname: "lpar1.com",
                        port: 22,
                        user: "user1",
                        // Setting password to exit check
                        password: "badPw",
                        privateKey: undefined,
                    },
                    false,
                );

                expect(result).toBeUndefined();
            });
        });
    });
});
