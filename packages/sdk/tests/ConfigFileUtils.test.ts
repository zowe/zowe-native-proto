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

import { readFileSync, writeFileSync, unlinkSync } from "node:fs";
import * as path from "node:path";
import * as os from "node:os";
import * as commentJson from "comment-json";
import { Config } from "@zowe/imperative";
import { ConfigFileUtils } from "../src/ConfigFileUtils";

describe("ConfigFileUtils", () => {
    let tempFilePath: string;
    let mockTeamConfig: Config;

    beforeEach(() => {
        // Create a temporary file for testing
        tempFilePath = path.join(os.tmpdir(), `test-config-${Date.now()}.json`);
        const testConfig = {
            profiles: {
                testprofile: {
                    type: "ssh",
                    properties: {
                        host: "example.com",
                        user: "testuser",
                    },
                },
            },
        };
        // Private key is already commented out
        (testConfig.profiles as any).testprofile[Symbol.for("after:properties")] = [
            {
                type: "LineComment",
                value: " Private key was invalid and commented out to avoid re-use. Original value: \"/u/users/example/.ssh/id_XXX\"",
                inline: false
            } as commentJson.CommentToken
        ];
        writeFileSync(tempFilePath, commentJson.stringify(testConfig, null, 4), "utf-8");

        // Mock the Config object
        mockTeamConfig = {
            layerActive: () => ({ path: tempFilePath }),
            api: {
                profiles: {
                    get: (profileName: string) => ({
                        host: "example.com",
                        user: "testuser",
                        privateKey: "/u/users/example/.ssh/id_XXX",
                    }),
                },
                layers: {
                    read: () => {},
                },
            },
        } as any;
    });

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
            const uncommentResult = ConfigFileUtils.getInstance().uncommentProperty(mockTeamConfig, {
                filePath: tempFilePath,
                propertyPath: "profiles.testprofile.properties.privateKey",
                originalValue: "/u/users/example/.ssh/id_XXX"
            });
            // Expect private key to be uncommented
            expect(uncommentResult).toBe(true);

            // Now add it back as a comment
            const result = ConfigFileUtils.getInstance().commentOutProperty(mockTeamConfig, "testprofile", "privateKey");

            expect(result).toBeDefined();
            expect(result?.propertyPath).toBe("profiles.testprofile.properties.privateKey");
            expect(result?.originalValue).toBe("/u/users/example/.ssh/id_XXX");

            const content = readFileSync(tempFilePath, "utf-8");
            expect(content).toContain("Private key was invalid and commented out");
            expect(content).not.toContain('"privateKey": "/u/users/example/.ssh/id_XXX"');
        });

        it("should return undefined for non-existent property", () => {
            // Mock profile that doesn't have privateKey
            mockTeamConfig.api.profiles.get = () => ({
                properties: {
                    host: "example.com",
                    user: "testuser",
                },
            });

            const result = ConfigFileUtils.getInstance().commentOutProperty(mockTeamConfig, "testprofile", "privateKey");
            expect(result).toBeUndefined();
        });
    });

    describe("uncommentProperty", () => {
        it("should uncomment a previously commented property", () => {
            // Then uncomment it
            const success = ConfigFileUtils.getInstance().uncommentProperty(mockTeamConfig, {
                filePath: tempFilePath,
                propertyPath: "profiles.testprofile.properties.privateKey",
                originalValue: "/u/users/example/.ssh/id_XXX"
            });
            expect(success).toBe(true);

            const content = readFileSync(tempFilePath, "utf-8");
            expect(content).toContain('"privateKey": "/u/users/example/.ssh/id_XXX"');
        });
    });

    describe("deleteCommentedLine", () => {
        it("should delete comment lines", () => {
            const success = ConfigFileUtils.getInstance().deleteCommentedLine({
                filePath: tempFilePath,
                propertyPath: "profiles.testprofile.properties.privateKey",
                originalValue: "/u/users/example/.ssh/id_XXX"
            });
            expect(success).toBe(true);

            const content = readFileSync(tempFilePath, "utf-8");
            expect(content).not.toContain("Private key was invalid");
            expect(content).not.toContain("privateKey");
        });

        it("should return false for invalid property path", () => {
            const invalidCommentInfo = {
                filePath: tempFilePath,
                propertyPath: "invalid.path",
                originalValue: "test",
            };

            const success = ConfigFileUtils.getInstance().deleteCommentedLine(invalidCommentInfo);
            expect(success).toBe(false);
        });
    });
});
