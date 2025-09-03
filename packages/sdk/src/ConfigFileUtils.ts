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

import type { Config } from "@zowe/imperative";
import type { CommentToken } from "comment-json";
import * as _ from "es-toolkit/compat";

export interface CommentedProperty {
    layerPath: string;
    propertyPath: string;
    originalValue: unknown;
    commentText?: string;
}

/**
 * Utility class for commenting out properties in JSON configuration files using Imperative Config API and comment-json
 */
export class ConfigFileUtils {
    private static instance: ConfigFileUtils;
    private static AFTER_PROPERTIES_SYMBOL = Symbol.for("after:properties");
    private constructor() {}

    public static getInstance(): ConfigFileUtils {
        if (!ConfigFileUtils.instance) {
            ConfigFileUtils.instance = new ConfigFileUtils();
        }

        return ConfigFileUtils.instance;
    }

    /**
     * Comment out a property in a team configuration using Config API and comment-json symbol
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
            const layerFromProfName = teamConfig.api.layers.find(profileName);
            if (!layerFromProfName) {
                console.warn("Could not determine active config layer path");
                return undefined;
            }
            // biome-ignore lint/suspicious/noExplicitAny: The type cast is necessary to access an internal function in the Config API.
            const layerJson = (teamConfig as any).findLayer(layerFromProfName.user, layerFromProfName.global);

            const profilePath = `properties.${teamConfig.api.profiles.getProfilePathFromName(profileName)}`;

            // Get the current value of the property
            const propertyPath = `properties.${propertyName}`;
            const profile = teamConfig.api.profiles.get(profileName, false);

            // Check if the property exists in the JSON structure
            if (!profile?.[propertyName]) {
                return undefined;
            }

            const currentValue = profile[propertyName];

            // Add a comment explaining why the property was commented out
            const commentText = `${propertyName} was moved to a comment as the value is invalid. Original value: ${JSON.stringify(currentValue)}`;

            const profileInJson = _.get(layerJson, profilePath);
            // Add comment before where the property would be
            const commentToken = {
                type: "LineComment",
                value: ` ${commentText}`,
                inline: false,
            } as CommentToken;
            const comments = profileInJson[ConfigFileUtils.AFTER_PROPERTIES_SYMBOL];
            if (!comments) {
                // No comments exist after `properties` object
                profileInJson[ConfigFileUtils.AFTER_PROPERTIES_SYMBOL] = [commentToken];
            } else {
                // Add comment above pre-existing comments to keep close to `properties` object
                comments.unshift(commentToken);
            }

            // Remove the property from the configuration
            delete profileInJson.properties[propertyName];

            // Write the modified content back to the file
            teamConfig.api.layers.write(layerJson);

            return {
                layerPath: layerJson.path,
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
     * Removes comments from the given object that match the given text and within the given comment-json symbol.
     * @param obj The object to remove comments from
     * @param text The text to compare against each comment; matches are removed
     * @param commentSymbol The comment-json symbol (`after-all, after:<propertyName>, before-all, before:<propertyName>`) where the comments are located
     */
    private removeCommentsInObject(
        obj: { [key: string | symbol]: unknown },
        text: string,
        commentSymbol: symbol,
    ): void {
        const comments = obj[commentSymbol] as CommentToken[] | undefined;
        if (comments == null) {
            return;
        }

        const filteredComments = comments.filter((comment) => comment.value?.trim() !== text);
        if (filteredComments.length === 0) {
            delete obj[commentSymbol];
        } else {
            obj[commentSymbol] = filteredComments;
        }
    }

    /**
     * Uncomment a previously commented property by restoring it to the configuration
     * @param teamConfig The team configuration object from Imperative
     * @param commentInfo Information about the commented property
     * @returns true if successful, false otherwise
     */
    public uncommentProperty(teamConfig: Config, profileName: string, commentInfo: CommentedProperty): boolean {
        try {
            const layerFromProfName = teamConfig.api.layers.find(profileName);
            // biome-ignore lint/suspicious/noExplicitAny: The type cast is necessary to access an internal function in the Config API.
            const layerJson = (teamConfig as any).findLayer(layerFromProfName.user, layerFromProfName.global);

            // Restore the property in the properties section under the given profile
            const profileJson = _.get(
                layerJson,
                `properties.${teamConfig.api.profiles.getProfilePathFromName(profileName)}`,
            );
            _.set(profileJson, commentInfo.propertyPath, commentInfo.originalValue);

            // Filter out only the specific comment that matches our comment text
            if (profileJson?.[ConfigFileUtils.AFTER_PROPERTIES_SYMBOL] && commentInfo.commentText) {
                // The presence of the comment symbol indicates at least one comment after the `properties` section
                this.removeCommentsInObject(
                    profileJson,
                    commentInfo.commentText,
                    ConfigFileUtils.AFTER_PROPERTIES_SYMBOL,
                );

                // Write the modified content back to the file
                teamConfig.api.layers.write(layerJson);
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
    public deleteCommentedLine(teamConfig: Config, profileName: string, commentInfo: CommentedProperty): boolean {
        try {
            const layerFromProfName = teamConfig.api.layers.find(profileName);
            // biome-ignore lint/suspicious/noExplicitAny: The type cast is necessary to access an internal function in the Config API.
            const layerJson = (teamConfig as any).findLayer(layerFromProfName.user, layerFromProfName.global);

            // Parse the property path to get profile and property name
            const profileJson = _.get(
                layerJson,
                `properties.${teamConfig.api.profiles.getProfilePathFromName(profileName)}`,
            );
            if (!profileJson) {
                return false;
            }

            // Filter out only the specific comment that matches our comment text
            if (profileJson[ConfigFileUtils.AFTER_PROPERTIES_SYMBOL] && commentInfo.commentText) {
                // The presence of the comment symbol indicates at least one comment after the `properties` section
                this.removeCommentsInObject(
                    profileJson,
                    commentInfo.commentText,
                    ConfigFileUtils.AFTER_PROPERTIES_SYMBOL,
                );

                // Write the modified content back to the file
                teamConfig.api.layers.write(layerJson);
            }
            return true;
        } catch (error) {
            console.error("Error deleting comment lines:", error);
            return false;
        }
    }
}
