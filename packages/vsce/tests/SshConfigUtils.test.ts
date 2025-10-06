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

import { afterEach, beforeEach, describe, expect, it, type Mock, vi } from "vitest";
import {
    type AbstractConfigManager,
    MESSAGE_TYPE,
    type PrivateKeyWarningOptions,
    ZSshClient,
} from "zowe-native-proto-sdk";
import { SshConfigUtils, VscePromptApi } from "../src/SshConfigUtils";
import { getVsceConfig } from "../src/Utilities";
import { ZoweVsCodeExtension } from "@zowe/zowe-explorer-api";
import * as vscode from "vscode";

vi.mock("vscode", () => ({
    Disposable: vi.fn(),
    window: {
        createQuickPick: vi.fn(),
    },
    workspace: {
        getConfiguration: vi.fn(() => ({
            get: vi.fn(),
            update: vi.fn(),
        })),
    },
    ConfigurationTarget: {
        Global: 1,
        Workspace: 2,
        WorkspaceFolder: 3,
    },
}));

vi.mock("@zowe/zowe-explorer-api", () => ({
    Gui: {},
    ZoweExplorerApiType: {
        All: "all",
        Mvs: "mvs",
        Uss: "uss",
        Jobs: "jobs",
        Command: "command",
    },
    ZoweVsCodeExtension: {
        getZoweExplorerApi: vi.fn(),
    },
    imperative: {
        DeferredPromise: class {
            promise: Promise<unknown>;
            resolve!: (v?: unknown) => void;
            reject!: (r?: unknown) => void;
            constructor() {
                this.promise = new Promise((res, rej) => {
                    this.resolve = res;
                    this.reject = rej;
                });
            }
        },
    },
}));
vi.mock("../src/Utilities", async (importOriginal) => {
    const actual = await importOriginal<typeof import("../src/Utilities")>();
    return {
        ...actual,
        getVsceConfig: vi.fn(),
    };
});

describe("SshConfigUtils", () => {
    const defaultPath = "~/.zowe-server";
    afterEach(() => {
        vi.clearAllMocks();
    });

    describe("getServerPath", () => {
        beforeEach(async () => {
            delete process.env.ZOWE_OPT_SERVER_PATH;
            vi.spyOn(ZSshClient, "DEFAULT_SERVER_PATH", "get").mockReturnValue(defaultPath);
        });
        afterEach(() => {
            vi.restoreAllMocks();
        });
        it("returns path from VS Code config when host is mapped", () => {
            const profile: IProfile = { host: "testHost" } as IProfile;
            (getVsceConfig as Mock).mockReturnValue({
                get: vi.fn().mockReturnValue({ testHost: "/mapped/path" }),
            });

            const result = SshConfigUtils.getServerPath(profile);
            expect(result).toBe("/mapped/path");
        });
        it("falls back to env var if no config", () => {
            const profile: IProfile = { host: "testHost" } as IProfile;
            (getVsceConfig as Mock).mockReturnValue({ get: vi.fn().mockReturnValue({}) });
            process.env.ZOWE_OPT_SERVER_PATH = "/env/path";

            const result = SshConfigUtils.getServerPath(profile);
            expect(result).toBe("/env/path");
        });
        it("returns default path if nothing else is set", () => {
            const profile: IProfile = { host: "testHost" } as IProfile;
            (getVsceConfig as Mock).mockReturnValue({ get: vi.fn().mockReturnValue({}) });

            const result = SshConfigUtils.getServerPath(profile);
            expect(result).toBe(defaultPath);
        });
    });
    describe("showSessionInTree", () => {
        let mockApi: any;
        let mockProvider: any;
        let mockLocalStorage: any;
        beforeEach(() => {
            mockLocalStorage = {
                getValue: vi.fn().mockReturnValue({ sessions: [] }),
                setValue: vi.fn(),
            };

            mockProvider = {
                mSessionNodes: [],
                addSession: vi.fn(),
                deleteSession: vi.fn(),
                getTreeType: vi.fn().mockReturnValue("datasetProvider"),
            };

            mockApi = {
                datasetProvider: mockProvider,
                ussFileProvider: { ...mockProvider },
                jobsProvider: { ...mockProvider },
                getLocalStorage: vi.fn().mockReturnValue(mockLocalStorage),
            };
            vi.spyOn(ZoweVsCodeExtension, "getZoweExplorerApi").mockReturnValue({
                getExplorerExtenderApi: () => mockApi,
            } as any);
        });
        afterEach(() => {
            vi.restoreAllMocks();
        });
        it("should add a session when visible is true and session is not present", async () => {
            await SshConfigUtils.showSessionInTree("testProfile", true);

            expect(mockProvider.addSession).toHaveBeenCalledWith(
                expect.objectContaining({ sessionName: "testProfile", profileType: "ssh" }),
            );
            expect(mockLocalStorage.setValue).toHaveBeenCalledWith(
                "datasetProvider",
                expect.objectContaining({ sessions: ["testProfile"] }),
            );
        });
        it("should delete a session when visible is false and session is present", async () => {
            const fakeNode = { getProfileName: () => "testProfile" };
            mockProvider.mSessionNodes = [fakeNode];

            await SshConfigUtils.showSessionInTree("testProfile", false);

            expect(mockProvider.deleteSession).toHaveBeenCalledWith(fakeNode);
            expect(mockLocalStorage.setValue).toHaveBeenCalledWith(
                "datasetProvider",
                expect.objectContaining({ sessions: [] }),
            );
        });
    });
    describe("VscePromptApi", () => {
        let mockQuickPick: any;
        const mockProfilesCache = {} as unknown as AbstractConfigManager["mProfilesCache"];
        const instance = new VscePromptApi(mockProfilesCache);

        beforeEach(() => {
            vi.resetAllMocks();
            mockQuickPick = {
                items: [],
                selectedItems: [],
                show: vi.fn(),
                hide: vi.fn(),
                onDidAccept: vi.fn(),
                onDidHide: vi.fn(),
                onDidChangeValue: vi.fn(),
            };
        });
        describe("showMessage", () => {
            beforeEach(() => {
                vscode.window.showInformationMessage = vi.fn();
                vscode.window.showWarningMessage = vi.fn();
                vscode.window.showErrorMessage = vi.fn();
            });

            afterEach(() => {
                vi.restoreAllMocks();
            });
            it("returns an information message", () => {
                (instance as any).showMessage("test info message", MESSAGE_TYPE.INFORMATION);
                expect(vscode.window.showInformationMessage).toHaveBeenCalledWith("test info message");
            });
            it("returns a warning message", () => {
                (instance as any).showMessage("test warning message", MESSAGE_TYPE.WARNING);
                expect(vscode.window.showWarningMessage).toHaveBeenCalledWith("test warning message");
            });
            it("returns an error message", () => {
                (instance as any).showMessage("test error message", MESSAGE_TYPE.ERROR);
                expect(vscode.window.showErrorMessage).toHaveBeenCalledWith("test error message");
            });
        });
        describe("showMenu", () => {
            beforeEach(() => {
                vi.spyOn(vscode.window, "createQuickPick").mockReturnValue(mockQuickPick);
            });
            afterEach(() => {
                vi.restoreAllMocks();
            });
            it("resolves with the selected item when user selects an item", async () => {
                const opts = {
                    items: [{ label: "Option 1" }, { label: "Option 2" }],
                    title: "Select an option",
                    placeholder: "Choose...",
                };
                mockQuickPick.selectedItems = [opts.items[0]];
                mockQuickPick.onDidAccept.mockImplementation((cb: Function) => cb());
                const result = await (instance as any).showMenu(opts);

                expect(vscode.window.createQuickPick).toHaveBeenCalled();
                expect(mockQuickPick.show).toHaveBeenCalled();
                expect(mockQuickPick.hide).toHaveBeenCalled();
                expect(result).toBe("Option 1");
            });
        });
        describe("showCustomMenu", () => {
            beforeEach(() => {
                vi.spyOn(vscode.window, "createQuickPick").mockReturnValue(mockQuickPick);
            });
            afterEach(() => {
                vi.restoreAllMocks();
            });
            it("shows mapped items in quick pick", async () => {
                const opts = {
                    title: "Select SSH",
                    placeholder: "Pick one",
                    items: [{ label: "host1", description: "host" }],
                };
                const promise = (instance as any).showCustomMenu(opts);
                expect(mockQuickPick.items).toEqual([{ label: "host1", description: "host" }]);
                expect(mockQuickPick.title).toBe("Select SSH");
                expect(mockQuickPick.placeholder).toBe("Pick one");
                expect(mockQuickPick.ignoreFocusOut).toBe(true);

                const onDidHide = (mockQuickPick.onDidHide as any).mock.calls[0][0];
                onDidHide();

                const result = await promise;
                expect(result).toBeUndefined();
            });
            it("returns custom item when input starts with >", async () => {
                const opts = { items: [{ label: "host1", description: "host" }] };
                const promise = (instance as any).showCustomMenu(opts);
                const onDidChangeValue = (mockQuickPick.onDidChangeValue as any).mock.calls[0][0];
                onDidChangeValue("customHost");

                expect(mockQuickPick.items[0]).toEqual({
                    label: "> customHost",
                    description: "Custom SSH Host",
                });
                mockQuickPick.selectedItems = [mockQuickPick.items[0]];
                const onDidAccept = (mockQuickPick.onDidAccept as any).mock.calls[0][0];
                onDidAccept();

                const result = await promise;
                expect(result).toEqual({
                    label: "customHost",
                    description: "Custom SSH Host",
                });
            });

            it("returns undefined when quick pick is hidden", async () => {
                const opts = { items: [] };
                const promise = (instance as any).showCustomMenu(opts);
                const onDidHide = (mockQuickPick.onDidHide as any).mock.calls[0][0];
                onDidHide();
                const result = await promise;
                expect(result).toBeUndefined();
            });
        });
        describe("getCurrentDir", () => {
            it("returns the fsPath when workspaceRoot is defined", () => {
                const testPath = "/user/project";
                (ZoweVsCodeExtension as any).workspaceRoot = {
                    uri: { fsPath: testPath },
                };
                const result = (instance as any).getCurrentDir();
                expect(result).toBe(testPath);
            });
        });
        describe("showPrivateKeyWarning", () => {
            beforeEach(() => {
                vi.spyOn(vscode.window, "createQuickPick").mockReturnValue(mockQuickPick);
            });
            afterEach(() => {
                vi.restoreAllMocks();
            });
            it("resolves true when 'continue' is selected", async () => {
                const opts: PrivateKeyWarningOptions = {
                    profileName: "test",
                    privateKeyPath: "",
                };
                const promise = (instance as any).showPrivateKeyWarning(opts);

                const accept = mockQuickPick.onDidAccept.mock.calls[0][0];
                mockQuickPick.selectedItems = [{ action: "continue" }];
                accept();

                const result = await promise;
                expect(result).toBe(true);
            });
        });
        describe("storeServerPath", () => {
            let mockGet: any;
            let mockUpdate: any;
            beforeEach(() => {
                mockGet = vi.fn();
                mockUpdate = vi.fn();
                (getVsceConfig as Mock).mockReturnValue({ get: mockGet, update: mockUpdate });
            });
            afterEach(() => {
                vi.resetAllMocks();
            });
            it("updates an existing serverPathMap with new host and path", () => {
                mockGet.mockReturnValue({ oldHost: "/old/path" });
                (instance as any).storeServerPath("newHost", "/new/path");
                expect(mockUpdate).toHaveBeenCalledWith(
                    "serverInstallPath",
                    {
                        oldHost: "/old/path",
                        newHost: "/new/path",
                    },
                    vscode.ConfigurationTarget.Global,
                );
            });
            it("adds a new host and path entry when serverPathMap is empty", () => {
                mockGet.mockReturnValue(undefined);
                (instance as any).storeServerPath("newHost", "/new/path");
                expect(mockUpdate).toHaveBeenCalledWith(
                    "serverInstallPath",
                    { newHost: "/new/path" },
                    vscode.ConfigurationTarget.Global,
                );
            });
        });
    });
});
