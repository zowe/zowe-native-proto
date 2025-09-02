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

import { readFileSync, unlinkSync, writeFileSync } from "node:fs";
import * as os from "node:os";
import * as path from "node:path";
import type { Config } from "@zowe/imperative";
import * as commentJson from "comment-json";
import { ConfigFileUtils } from "../src/ConfigFileUtils";

describe("ConfigFileUtils", () => {
    let tempFilePath: string;
    let mockTeamConfig: Config;

    beforeEach(() => {
        // Create a temporary file for testing
        tempFilePath = path.join(os.tmpdir(), `test-config-${Date.now()}.json`);
    });

    const createMockConfigWithPrivateKey = () => {
        const testConfig = {
            user: false,
            global: false,
            path: tempFilePath,
            properties: {
                profiles: {
                    testprofile: {
                        type: "ssh",
                        properties: {
                            host: "example.com",
                            user: "testuser",
                            privateKey: "/u/users/example/.ssh/id_XXX",
                        },
                    },
                },
            },
        };
        writeFileSync(tempFilePath, commentJson.stringify(testConfig, null, 4), "utf-8");
        return testConfig;
    };

    const createMockConfigWithCommentedKey = () => {
        const testConfig = {
            user: false,
            global: false,
            path: tempFilePath,
            properties: {
                profiles: {
                    testprofile: {
                        type: "ssh",
                        properties: {
                            host: "example.com",
                            user: "testuser",
                        },
                    },
                },
            },
        };
        // Add comment for private key that was removed
        (testConfig.properties.profiles as any).testprofile[Symbol.for("after:properties")] = [
            {
                type: "LineComment",
                value: ' privateKey was invalid and commented out to avoid re-use. Original value: "/u/users/example/.ssh/id_XXX"',
                inline: false,
            } as commentJson.CommentToken,
        ];
        writeFileSync(tempFilePath, commentJson.stringify(testConfig, null, 4), "utf-8");
        return testConfig;
    };

    const createMockTeamConfig = (testConfig: any) => {
        return {
            findLayer: (user: boolean, global: boolean) => testConfig,
            api: {
                profiles: {
                    get: (profileName: string, _failNotFound = true) => {
                        const profile = testConfig.properties?.profiles?.[profileName];
                        return profile ? {
                            host: profile.properties.host,
                            user: profile.properties.user,
                            privateKey: profile.properties.privateKey,
                        } : undefined;
                    },
                    getProfilePathFromName: (profileName: string) => {
                        return `profiles.${profileName}`;
                    }
                },
                layers: {
                    find: (profileName: string) => ({
                        user: testConfig.user,
                        global: testConfig.global,
                    }),
                    write: (layerJson: any) => {
                        writeFileSync(tempFilePath, commentJson.stringify(layerJson, null, 4), "utf-8");
                    }
                },
            },
        } as any;
    };

    afterEach(() => {
        // Clean up temporary file
        try {
            unlinkSync(tempFilePath);
        } catch {
            // Ignore if file doesn't exist
        }
    });

    describe("commentOutProperty", () => {
        it("should comment out a property in JSON file using Config API and comment-json", () => {
            // Start with a config that has the privateKey property
            const testConfig = createMockConfigWithPrivateKey();
            mockTeamConfig = createMockTeamConfig(testConfig);

            const result = ConfigFileUtils.getInstance().commentOutProperty(
                mockTeamConfig,
                "testprofile",
                "privateKey",
            );

            expect(result).toBeDefined();
            expect(result?.propertyPath).toBe("properties.privateKey");
            expect(result?.originalValue).toBe("/u/users/example/.ssh/id_XXX");
            expect(result?.layerPath).toBe(tempFilePath);
            expect(result?.commentText).toContain("privateKey was invalid and commented out");

            const content = readFileSync(tempFilePath, "utf-8");
            expect(content).toContain("privateKey was invalid and commented out");
            expect(content).not.toContain('"privateKey": "/u/users/example/.ssh/id_XXX"');
        });

        it("should return undefined for non-existent property", () => {
            // Start with a config that doesn't have the privateKey property
            const testConfig = {
                user: false,
                global: false,
                path: tempFilePath,
                properties: {
                    profiles: {
                        testprofile: {
                            type: "ssh",
                            properties: {
                                host: "example.com",
                                user: "testuser",
                                // No privateKey property
                            },
                        },
                    },
                },
            };
            writeFileSync(tempFilePath, commentJson.stringify(testConfig, null, 4), "utf-8");
            mockTeamConfig = createMockTeamConfig(testConfig);

            const result = ConfigFileUtils.getInstance().commentOutProperty(
                mockTeamConfig,
                "testprofile",
                "privateKey",
            );
            expect(result).toBeUndefined();
        });
    });

    describe("uncommentProperty", () => {
        it("should uncomment a previously commented property", () => {
            // Start with a config that has a commented privateKey
            const testConfig = createMockConfigWithCommentedKey();
            mockTeamConfig = createMockTeamConfig(testConfig);

            const success = ConfigFileUtils.getInstance().uncommentProperty(mockTeamConfig, "testprofile", {
                layerPath: tempFilePath,
                propertyPath: "properties.privateKey",
                originalValue: "/u/users/example/.ssh/id_XXX",
            });
            expect(success).toBe(true);

            const content = readFileSync(tempFilePath, "utf-8");
            expect(content).toContain('"privateKey": "/u/users/example/.ssh/id_XXX"');
        });
    });

    describe("deleteCommentedLine", () => {
        it("should delete comment lines", () => {
            // Start with a config that has a commented privateKey
            const testConfig = createMockConfigWithCommentedKey();
            mockTeamConfig = createMockTeamConfig(testConfig);

            const success = ConfigFileUtils.getInstance().deleteCommentedLine(mockTeamConfig, "testprofile", {
                layerPath: tempFilePath,
                propertyPath: "properties.privateKey",
                originalValue: "/u/users/example/.ssh/id_XXX",
            });
            expect(success).toBe(true);

            const content = readFileSync(tempFilePath, "utf-8");
            expect(content).not.toContain("privateKey was invalid");
            expect(content).not.toContain("privateKey");
        });

        it("should return false for invalid property path", () => {
            // Start with a basic config without the profile we're trying to access
            const testConfig = {
                user: false,
                global: false,
                path: tempFilePath,
                properties: {
                    profiles: {
                        // No testprofile here
                    },
                },
            };
            writeFileSync(tempFilePath, commentJson.stringify(testConfig, null, 4), "utf-8");
            mockTeamConfig = createMockTeamConfig(testConfig);

            const invalidCommentInfo = {
                layerPath: tempFilePath,
                propertyPath: "invalid.path",
                originalValue: "test",
            };

            const success = ConfigFileUtils.getInstance().deleteCommentedLine(mockTeamConfig, "invalidprofile", invalidCommentInfo);
            expect(success).toBe(false);
        });
    });
});
