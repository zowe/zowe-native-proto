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

import { type SshSession, ZosUssProfile } from "@zowe/zos-uss-for-zowe-sdk";
import { imperative, type MainframeInteraction } from "@zowe/zowe-explorer-api";
import * as vscode from "vscode";
import { type ZSshClient, ZSshUtils } from "zowe-native-proto-sdk";
import { SshClientCache } from "../SshClientCache";

export class SshCommonApi implements MainframeInteraction.ICommon {
    public constructor(public profile?: imperative.IProfileLoaded) {}

    public getProfileTypeName(): string {
        return ZosUssProfile.type;
    }

    public getSession(profile?: imperative.IProfileLoaded): imperative.Session {
        return this.getSshSession(profile) as unknown as imperative.Session;
    }

    public async getStatus(profile: imperative.IProfileLoaded, profileType?: string): Promise<string> {
        if (profileType === ZosUssProfile.type) {
            try {
                await SshClientCache.inst.connect(profile);
                return "active";
            } catch (err) {
                const errorMessage = (err as Error).toString();
                
                // Check if this is a private key authentication failure
                if (this.isPrivateKeyAuthFailure(errorMessage, profile.profile)) {
                    try {
                        // Attempt to prompt for password and retry connection
                        const updatedProfile = await this.handlePrivateKeyFailure(profile);
                        if (updatedProfile) {
                            await SshClientCache.inst.connect(updatedProfile);
                            return "active";
                        }
                    } catch (retryErr) {
                        imperative.Logger.getAppLogger().warn(
                            `Password authentication also failed for profile ${profile.name}: ${retryErr}`
                        );
                        vscode.window.showErrorMessage(
                            `Authentication failed for profile ${profile.name}. Both private key and password authentication failed.`
                        );
                        return "inactive";
                    }
                }
                
                vscode.window.showErrorMessage(errorMessage);
                return "inactive";
            }
        }
        return "unverified";
    }

    public get client(): Promise<ZSshClient> {
        if (this.profile == null) {
            throw new Error("Failed to create SSH client: no profile found");
        }
        return SshClientCache.inst.connect(this.profile);
    }

    public getSshSession(profile?: imperative.IProfileLoaded): SshSession {
        return ZSshUtils.buildSession((profile ?? this.profile)?.profile!);
    }

    /**
     * Determines if the error indicates a private key authentication failure
     * @param errorMessage The error message from the SSH connection attempt
     * @param profile The profile being used for connection
     * @returns true if this appears to be a private key authentication failure
     */
    private isPrivateKeyAuthFailure(errorMessage: string, profile?: imperative.IProfile): boolean {
        if (!profile?.privateKey) {
            return false; // No private key configured, so this isn't a private key failure
        }

        // Check for common SSH authentication failure patterns that indicate private key issues
        const privateKeyFailurePatterns = [
            "All configured authentication methods failed",
            "Cannot parse privateKey: Malformed OpenSSH private key",
            "but no passphrase given",
            "integrity check failed",
            "Permission denied (publickey)",
            "Authentication failed"
        ];

        return privateKeyFailurePatterns.some(pattern => errorMessage.includes(pattern));
    }

    /**
     * Handles private key authentication failure by prompting for password
     * @param profile The profile that failed private key authentication
     * @returns Updated profile with password, or undefined if user cancelled
     */
    private async handlePrivateKeyFailure(profile: imperative.IProfileLoaded): Promise<imperative.IProfileLoaded | undefined> {
        try {
            // Prompt for password using VS Code's native input box
            const password = await vscode.window.showInputBox({
                title: `${profile.profile?.user}@${profile.profile?.host}'s password:`,
                password: true,
                placeHolder: "Enter your password",
                prompt: `Enter password for ${profile.profile?.user}@${profile.profile?.host}`,
                ignoreFocusOut: true,
            });

            if (!password) {
                return undefined;
            }

            // Create a new profile with password authentication (temporarily disabling private key)
            const updatedProfile: imperative.IProfileLoaded = {
                ...profile,
                profile: {
                    ...profile.profile!,
                    password,
                    // Temporarily disable private key for this connection attempt
                    privateKey: undefined,
                    keyPassphrase: undefined,
                },
            };

            return updatedProfile;
        } catch (error) {
            imperative.Logger.getAppLogger().error(`Failed to handle private key failure: ${error}`);
            return undefined;
        }
    }
}
