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

import type { inputBoxOpts, MESSAGE_TYPE, qpItem, qpOpts } from "../src/doc";
import { AbstractConfigManager, type ProgressCallback } from "../src/AbstractConfigManager";
import type { IProfileTypeConfiguration, ProfileInfo } from "@zowe/imperative";
import { ZoweVsCodeExtension } from "@zowe/zowe-explorer-api";

vi.mock("ssh2");
vi.mock("vscode");
vi.mock("@zowe/zowe-explorer-api", () => ({
    ZoweVsCodeExtension: {
        getZoweExplorerApi: () => ({
            getExplorerExtenderApi: () => ({
                getProfilesCache: () => ({
                    getProfileInfo: () =>
                        Promise.resolve({
                            getAllProfiles: (): ProfileInfo => [] as any,
                        }),
                }),
            }),
        }),
    },
}));

export class TestAbstractConfigManager extends AbstractConfigManager {
    // All overridden methods are initialized as spies that do nothing
    protected showMessage = vi.fn<(message: string, type: MESSAGE_TYPE) => void>();

    protected showInputBox = vi.fn<(opts: inputBoxOpts) => Promise<string | undefined>>().mockResolvedValue(undefined);

    protected withProgress = vi
        .fn()
        .mockImplementation(
            <T>(_message: string, task: (progress: ProgressCallback) => Promise<T>): Promise<T> => task(() => {}),
        );

    public showMenu = vi.fn<(opts: qpOpts) => Promise<string | undefined>>().mockResolvedValue(undefined);

    protected showCustomMenu = vi.fn<(opts: qpOpts) => Promise<qpItem | undefined>>().mockResolvedValue(undefined);

    protected getCurrentDir = vi.fn<() => string | undefined>().mockReturnValue("/mock/dir");

    protected getProfileSchemas = vi.fn<() => IProfileTypeConfiguration[]>().mockReturnValue([]);
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
    });
    describe.only("promptForProfile", async () => {
        it.only("should return merged attributes if a profile name is provided", async () => {
            const mockMergeAttrs = { attrs: "fake" };
            vi.spyOn(testManager, "getMergedAttrs" as any).mockReturnValueOnce(mockMergeAttrs);
            expect(await testManager.promptForProfile("testProfile")).toEqual({
                profile: mockMergeAttrs,
                message: "",
                failNotFound: false,
                type: "ssh",
            });
        });
    });
});
