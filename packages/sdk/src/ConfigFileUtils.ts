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

import { readFileSync, writeFileSync } from "node:fs";
import type { Config } from "@zowe/imperative";
import type { CommentToken } from "comment-json";
import * as commentJson from "comment-json";
import * as _ from "es-toolkit/compat";

export interface CommentedProperty {
    filePath: string;
    propertyPath: string;
    originalValue: unknown;
    commentText?: string;
}

/**
 * Utility class for commenting out properties in JSON configuration files using Imperative Config API and comment-json
 */
export class ConfigFileUtils {
    private static instance: ConfigFileUtils;
    private constructor() {}

    public static getInstance(): ConfigFileUtils {
        if (!ConfigFileUtils.instance) {
            ConfigFileUtils.instance = new ConfigFileUtils();
        }

        return ConfigFileUtils.instance;
    }

    /**
     * Comment out a property in a team configuration using Config API and comment-json
     * @param teamConfig The team configuration object from Imperative
     * @param profileName The name of the profile
     * @param propertyName The property name to comment out (e.g., "privateKey")
     * @returns Information about the commented property for potential undo operation
     */
    public commentOutProperty(
        teamConfig: Config,
        profileName: string,
        propertyName: string,
    ): CommentedProperty | undefined {
        try {
            // Get the active layer that contains the profile
            const activeLayer = teamConfig.layerActive();
            if (!activeLayer?.path) {
                console.warn("Could not determine active config layer path");
                return undefined;
            }

            const profilePath = profileName.split(".").reduce((all, seg, i, arr) => {
                // Last segment in split
                if (arr.length === 1 || i === arr.length - 1) {
                    return `${all}${seg}`;
                }

                return `${all}${seg}.profiles.`;
            }, "profiles.");

            const filePath = activeLayer.path;
            const content = readFileSync(filePath, "utf-8");

            // Get the current value of the property
            const propertyPath = `${profilePath}.properties.${propertyName}`;
            const profile = teamConfig.api.profiles.get(profileName, false);
            const currentValue = profile?.privateKey;

            if (currentValue === undefined) {
                return undefined; // Property doesn't exist
            }

            // Parse the JSON content with comment-json to preserve comments and formatting
            const jsonWithComments = commentJson.parse(content) as object;

            const profileInJson = _.get(jsonWithComments, profilePath);
            // Check if the property exists in the JSON structure
            if (!profileInJson?.properties?.[propertyName]) {
                return undefined;
            }

            // Add a comment explaining why the property was commented out
            const commentText = `Private key was invalid and commented out to avoid re-use. Original value: ${JSON.stringify(currentValue)}`;

            // Add comment before where the property would be
            profileInJson[Symbol.for("after:properties")] = [
                {
                    type: "LineComment",
                    value: ` ${commentText}`,
                    inline: false,
                } as CommentToken,
            ];

            // Remove the property from the configuration
            delete profileInJson.properties[propertyName];

            // Write the modified content back to the file
            const modifiedContent = commentJson.stringify(jsonWithComments, null, 4);
            writeFileSync(filePath, modifiedContent, "utf-8");

            // Reload the team config to reflect changes
            teamConfig.api.layers.read();

            return {
                filePath,
                propertyPath,
                originalValue: currentValue,
                commentText,
            };
        } catch (error) {
            console.error(`Error commenting out property ${propertyName} for profile ${profileName}:`, error);
            return undefined;
        }
    }

    /**
     * Uncomment a previously commented property by restoring it to the configuration
     * @param teamConfig The team configuration object from Imperative
     * @param commentInfo Information about the commented property
     * @returns true if successful, false otherwise
     */
    public uncommentProperty(teamConfig: Config, commentInfo: CommentedProperty): boolean {
        try {
            const content = readFileSync(commentInfo.filePath, "utf-8");

            // Parse the property path to get profile and property names
            const profilePath = commentInfo.propertyPath.substring(
                0,
                commentInfo.propertyPath.indexOf("properties.privateKey") - 1,
            );
            // Parse the JSON content with comment-json to preserve formatting
            const jsonWithComments = commentJson.parse(content) as object;
            _.set(jsonWithComments, commentInfo.propertyPath, commentInfo.originalValue);

            // Remove any comments associated with this property
            const commentSymbol = Symbol.for(`after:properties`);
            const profileObject = _.get(jsonWithComments, profilePath);
            if (profileObject?.[commentSymbol]) {
                delete profileObject[commentSymbol];
                // Write the modified content back to the file
                const modifiedContent = commentJson.stringify(jsonWithComments, null, 4);
                writeFileSync(commentInfo.filePath, modifiedContent, "utf-8");

                // Reload the team config to reflect changes
                teamConfig.api.layers.read();
                return true;
            }

            return false;
        } catch (error) {
            console.error("Error uncommenting property:", error);
            return false;
        }
    }

    /**
     * Delete comment lines related to a commented property
     * @param commentInfo Information about the commented property
     * @returns true if the comment was removed or no longer in file, false otherwise
     */
    public deleteCommentedLine(commentInfo: CommentedProperty): boolean {
        try {
            const content = readFileSync(commentInfo.filePath, "utf-8");

            // Parse the JSON content with comment-json
            const jsonWithComments = commentJson.parse(content) as object;

            // Parse the property path to get profile and property names
            // Strip off "properties" and "privateKey" to access comment in profile object
            const profilePath = commentInfo.propertyPath.split(".").slice(0, -2).join(".");

            const profileObject = _.get(jsonWithComments, profilePath);
            if (!profileObject) {
                return false;
            }

            // Remove comment placed after `properties` object under profile/group
            const commentSymbol = Symbol.for(`after:properties`);
            if (profileObject[commentSymbol]) {
                delete profileObject[commentSymbol];
                // Write the modified content back to the file
                writeFileSync(commentInfo.filePath, commentJson.stringify(jsonWithComments, null, 4), "utf-8");
            }
            return true;
        } catch (error) {
            console.error("Error deleting comment lines:", error);
            return false;
        }
    }
}
