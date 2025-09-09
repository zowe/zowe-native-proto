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
import * as path from "node:path";
import {
    type Config,
    ConfigBuilder,
    ConfigSchema,
    ConfigUtils,
    type IConfig,
    type IConfigBuilderOpts,
    type IConfigProfile,
    type IImperativeConfig,
    type IProfAttrs,
    type IProfile,
    type IProfileLoaded,
    type IProfileTypeConfiguration,
    type ProfileInfo,
} from "@zowe/imperative";
import { NodeSSH } from "node-ssh";
import { ConfigFileUtils } from "./ConfigFileUtils";
import { type inputBoxOpts, MESSAGE_TYPE, type PrivateKeyWarningOptions, type qpItem, type qpOpts } from "./doc";
import { type ISshConfigExt, ZClientUtils } from "./ZClientUtils";

export type ProgressCallback = (percent: number) => void;
export abstract class AbstractConfigManager {
    public constructor(private mProfilesCache: ProfileInfo) {}

    protected abstract showMessage(message: string, type: MESSAGE_TYPE): void;
    protected abstract showInputBox(opts: inputBoxOpts): Promise<string | undefined>;
    protected abstract withProgress<T>(message: string, task: (progress: ProgressCallback) => Promise<T>): Promise<T>;
    protected abstract showMenu(opts: qpOpts): Promise<string | undefined>;
    protected abstract showCustomMenu(opts: qpOpts): Promise<qpItem | undefined>;
    protected abstract getCurrentDir(): string | undefined;
    protected abstract getProfileSchemas(): IProfileTypeConfiguration[];
    protected abstract showPrivateKeyWarning(opts: PrivateKeyWarningOptions): Promise<boolean>;

    private migratedConfigs: ISshConfigExt[];
    private filteredMigratedConfigs: ISshConfigExt[];
    private validationResult: ISshConfigExt | undefined;
    private selectedProfile: ISshConfigExt | undefined;
    private sshProfiles: IProfileLoaded[];
    private sshRegex = /^ssh\s+(?:([a-zA-Z0-9_-]+)@)?([a-zA-Z0-9.-]+)/;
    private flagRegex = /-(\w+)(?:\s+("[^"]+"|'[^']+'|\S+))?/g;
    /**/
    public async promptForProfile(
        profileName?: string,
        setExistingProfile = true,
    ): Promise<IProfileLoaded | undefined> {
        this.validationResult = undefined;
        if (profileName) {
            return { profile: this.getMergedAttrs(profileName), message: "", failNotFound: false, type: "ssh" };
        }

        this.sshProfiles = this.fetchAllSshProfiles().filter(({ name, profile }) => name && profile?.host);

        // Get configs from ~/.ssh/config
        this.migratedConfigs = await ZClientUtils.migrateSshConfig();

        // Parse to remove migratable configs that already exist on the team config
        this.filteredMigratedConfigs = this.migratedConfigs.filter(
            (migratedConfig) =>
                !this.sshProfiles.some((sshProfile) => sshProfile.profile?.host === migratedConfig.hostname),
        );

        // Prompt user for ssh (new config, existing, migrating)
        const result = await this.showCustomMenu({
            items: [
                { label: "$(plus) Add New SSH Host..." },
                ...this.sshProfiles.map(({ name, profile }) => ({
                    label: name!,
                    description: profile!.host!,
                })),
                {
                    label: "Migrate From SSH Config",
                    separator: true,
                },
                ...this.filteredMigratedConfigs.map(({ name, hostname }) => ({
                    label: name!,
                    description: hostname,
                })),
            ],
            placeholder: "Select configured SSH host or enter user@host",
        });

        // If nothing selected, return
        if (!result) return;

        // If result is add new SSH host then create a new config, if not use migrated configs
        this.selectedProfile = this.filteredMigratedConfigs.find(
            ({ name, hostname }) => result?.label === name && result?.description === hostname,
        );

        if (result.description === "Custom SSH Host") {
            const createNewConfig = await this.createNewProfile(result.label);
            if (!createNewConfig) return undefined;
            this.selectedProfile = createNewConfig;
        } else if (result.label === "$(plus) Add New SSH Host...") {
            const createNewConfig = await this.createNewProfile();
            if (!createNewConfig) return undefined;
            this.selectedProfile = createNewConfig;
        }

        // If an existing team config profile was selected
        if (!this.selectedProfile) {
            const foundProfile = this.sshProfiles.find(({ name }) => name === result.label);
            if (foundProfile) {
                const validConfig = await this.validateConfig({
                    name: foundProfile.name,
                    hostname: foundProfile?.profile?.host,
                    port: foundProfile?.profile?.port,
                    privateKey: foundProfile?.profile?.privateKey,
                    keyPassphrase: foundProfile?.profile?.keyPassphrase,
                    user: foundProfile?.profile?.user,
                    password: foundProfile?.profile?.password,
                });
                if (validConfig === undefined) return;

                if (setExistingProfile || Object.keys(validConfig).length > 0) {
                    if (validConfig.password) {
                        foundProfile.profile.privateKey = foundProfile.profile.keyPassphrase = undefined;
                    }
                    await this.setProfile(validConfig, foundProfile.name);
                }
                return { ...foundProfile, profile: { ...foundProfile.profile, ...validConfig } };
            }
        }

        // Current directory open in vscode window
        const workspaceDir = this.getCurrentDir();

        // Prioritize creating a team config in the local workspace if it exists even if a global config exists
        // TODO: This behavior is only for the POC phase
        ////////////////////////////////////////////////////////////////////////////////////////////////////////
        const useProject = workspaceDir !== undefined && !this.mProfilesCache.getTeamConfig().layerExists(workspaceDir);
        await this.createZoweSchema(!useProject);
        ////////////////////////////////////////////////////////////////////////////////////////////////////////

        // Prompt for a new profile name with the hostname (for adding a new config) or host value (for migrating from a config)
        this.selectedProfile = await this.getNewProfileName(this.selectedProfile!, this.mProfilesCache.getTeamConfig());

        if (!this.selectedProfile?.name) {
            this.showMessage("SSH setup cancelled.", MESSAGE_TYPE.WARNING);
            return;
        }

        // Attempt connection if private key was provided and it has not been validated
        if (this.validationResult === undefined && this.selectedProfile.privateKey) {
            this.validationResult = await this.validateConfig(this.selectedProfile, false);
        }

        if (this.validationResult === undefined) {
            await this.validateFoundPrivateKeys();
        }

        if (this.validationResult === undefined) {
            // Attempt to validate with given URL/creds
            this.validationResult = await this.validateConfig(this.selectedProfile);
        }

        // If validateConfig returns a string, that string is the correct keyPassphrase
        if (this.validationResult && Object.keys(this.validationResult).length >= 1) {
            this.selectedProfile.privateKey = this.selectedProfile.keyPassphrase = undefined;
            this.selectedProfile = { ...this.selectedProfile, ...this.validationResult };
        }

        // If no private key or password is on the profile then there is no possible validation combination, thus return
        if (!this.selectedProfile?.privateKey && !this.selectedProfile?.password) {
            this.showMessage("SSH setup cancelled.", MESSAGE_TYPE.WARNING);
            return;
        }

        await this.setProfile(this.selectedProfile);

        return {
            name: this.selectedProfile.name,
            message: "",
            failNotFound: false,
            type: "ssh",
            profile: {
                host: this.selectedProfile.hostname,
                name: this.selectedProfile.name,
                password: this.selectedProfile.password,
                user: this.selectedProfile.user,
                privateKey: this.selectedProfile.privateKey,
                handshakeTimeout: this.selectedProfile.handshakeTimeout,
                port: this.selectedProfile.port,
                keyPassphrase: this.selectedProfile.keyPassphrase,
            },
        };
    }

    private async createNewProfile(knownConfigOpts?: string): Promise<ISshConfigExt | undefined> {
        const SshProfile: ISshConfigExt = {};

        let sshResponse: string | undefined;

        // KnownConfigOpts is defined if a custom option is selected via the first quickpick (ex: user@host is entered in search bar)
        if (!knownConfigOpts) {
            sshResponse = await this.showInputBox({
                title: "Enter SSH connection command",
                placeHolder: "E.g. ssh user@example.com",
            });
        } else {
            sshResponse = `ssh ${knownConfigOpts}`;
            const match = sshResponse.match(this.sshRegex);
            if (!match || match[0].length < sshResponse.length) {
                this.showMessage(
                    "Invalid custom connection format. Ensure it matches the expected pattern.",
                    MESSAGE_TYPE.ERROR,
                );
                return undefined;
            }
        }

        if (sshResponse === undefined) {
            this.showMessage("SSH setup cancelled.", MESSAGE_TYPE.WARNING);
            return undefined;
        }

        const sshMatch = sshResponse.match(this.sshRegex);

        if (!sshMatch) {
            this.showMessage("Invalid SSH command format. Ensure it matches the expected pattern.", MESSAGE_TYPE.ERROR);
            return undefined;
        }

        SshProfile.user = sshMatch[1] || require("node:os").userInfo().username;
        SshProfile.hostname = sshMatch[2];

        let flagMatch: RegExpExecArray | null;

        if (!knownConfigOpts) {
            // biome-ignore lint/suspicious/noAssignInExpressions: We just want to use the regex array in the loop
            while ((flagMatch = this.flagRegex.exec(sshResponse)) !== null) {
                const [, flag, value] = flagMatch;
                // Check for missing value
                if (!value) {
                    this.showMessage(`Missing value for flag -${flag}.`, MESSAGE_TYPE.ERROR);
                    return undefined;
                }

                const unquotedValue = value.replace(/^["']|["']$/g, ""); // Remove surrounding quotes

                // Map aliases to consistent keys
                if (flag === "p" || flag.toLowerCase() === "port") {
                    const portNumber = Number.parseInt(unquotedValue, 10);
                    if (Number.isNaN(portNumber)) {
                        this.showMessage(
                            `Invalid value for flag ${flag.length > 1 ? "-" : ""}-${flag}. Port must be a valid number.`,
                            MESSAGE_TYPE.ERROR,
                        );
                        return undefined;
                    }
                    SshProfile.port = portNumber;
                } else if (flag === "i" || flag.toLowerCase() === "identity_file") {
                    SshProfile.privateKey = unquotedValue;
                }

                // Validate if quotes are required
                if (/\s/.test(unquotedValue) && !/^["'].*["']$/.test(value)) {
                    this.showMessage(
                        `Invalid value for flag ${flag.length > 1 ? "-" : ""}-${flag}. Values with spaces must be quoted.`,
                        MESSAGE_TYPE.ERROR,
                    );
                    return undefined;
                }
            }
        }
        return SshProfile;
    }
    // Cloned method
    private async createZoweSchema(global: boolean): Promise<void> {
        try {
            const homeDir = ConfigUtils.getZoweDir();

            const user = false;
            const workspaceDir = this.getCurrentDir();

            const config = this.mProfilesCache.getTeamConfig();

            if (config.layerExists(global ? homeDir : workspaceDir)) return;

            config.api.layers.activate(user, global);

            const profSchemas = this.getProfileSchemas();
            config.setSchema(ConfigSchema.buildSchema(profSchemas));

            // Note: IConfigBuilderOpts not exported
            // const opts: IConfigBuilderOpts = {
            const opts: IConfigBuilderOpts = {
                // getSecureValue: this.promptForProp.bind(this),
                populateProperties: true,
            };
            // Build new config and merge with existing layer
            const baseProfSchema = profSchemas.find((schema) => schema.type === "base");
            const impConfig: Partial<IImperativeConfig> = {
                profiles: [baseProfSchema],
                baseProfile: baseProfSchema,
            };
            const newConfig: IConfig = await ConfigBuilder.build(impConfig, global, opts);
            config.api.layers.merge(newConfig);
            // Use api.layers.write() instead of save() to avoid keyring prompts on MacOS
            config.api.layers.write();
        } catch {}
    }

    private async validateConfig(newConfig: ISshConfigExt, askForPassword = true): Promise<ISshConfigExt | undefined> {
        const configModifications: ISshConfigExt | undefined = {};
        try {
            const privateKeyPath = newConfig.privateKey;

            if (!newConfig.user) {
                const userModification = await this.showInputBox({
                    title: `Enter user for host: '${newConfig.hostname}'`,
                    placeHolder: "Enter the user for the target host",
                });
                configModifications.user = userModification;
            }

            if ((!privateKeyPath || !readFileSync(path.normalize(privateKeyPath), "utf-8")) && !newConfig.password) {
                const passwordPrompt = askForPassword && (await this.promptForPassword(newConfig, configModifications));
                return passwordPrompt ? { ...configModifications, ...passwordPrompt } : undefined;
            }

            await this.attemptConnection({ ...newConfig, ...configModifications });
        } catch (err) {
            const errorMessage = `${err}`;
            if (
                newConfig.privateKey &&
                !newConfig.password &&
                errorMessage.includes("All configured authentication methods failed")
            ) {
                if (!(await this.handleInvalidPrivateKey(newConfig))) {
                    return undefined;
                }
                newConfig.privateKey = undefined;
            }

            if (errorMessage.includes("Invalid username")) {
                const testUser = await this.showInputBox({
                    title: `Enter user for host: '${newConfig.hostname}'`,
                    placeHolder: "Enter the user for the target host",
                });

                if (!testUser) return undefined;
                try {
                    await this.attemptConnection({ ...newConfig, user: testUser });
                    return { user: testUser };
                } catch {
                    return undefined;
                }
            }

            if (errorMessage.includes("but no passphrase given") || errorMessage.includes("integrity check failed")) {
                const privateKeyPath = newConfig.privateKey;
                for (let attempts = 0; attempts < 3; attempts++) {
                    const testKeyPassphrase = await this.showInputBox({
                        title: `Enter passphrase for key '${privateKeyPath}'`,
                        password: true,
                        placeHolder: "Enter passphrase for key",
                    });

                    try {
                        await this.attemptConnection({
                            ...newConfig,
                            ...configModifications,
                            keyPassphrase: testKeyPassphrase,
                        });
                        return { ...configModifications, keyPassphrase: testKeyPassphrase };
                    } catch (error) {
                        if (!`${error}`.includes("integrity check failed")) break;
                        this.showMessage(`Passphrase Authentication Failed (${attempts + 1}/3)`, MESSAGE_TYPE.ERROR);
                    }
                }
                if (!(await this.handleInvalidPrivateKey(newConfig))) {
                    return undefined;
                }
                newConfig.privateKey = undefined;
                newConfig.keyPassphrase = undefined;
                return undefined;
            }

            if (errorMessage.includes("All configured authentication methods failed")) {
                const passwordPrompt = askForPassword
                    ? await this.promptForPassword(newConfig, configModifications)
                    : undefined;

                // If password authentication succeeded and we had a private key that failed,
                // comment out the private key in the configuration file
                if (passwordPrompt && newConfig.privateKey) {
                    if (!(await this.handleInvalidPrivateKey(newConfig))) {
                        return undefined;
                    }
                }

                return passwordPrompt ? { ...configModifications, ...passwordPrompt } : undefined;
            }

            if (errorMessage.includes("Timed out while waiting for handshake")) {
                this.showMessage("Timed out while waiting for handshake", MESSAGE_TYPE.ERROR);
                return undefined;
            }

            if (errorMessage.includes("Cannot parse privateKey: Malformed OpenSSH private key")) {
                if (!(await this.handleInvalidPrivateKey(newConfig))) {
                    return undefined;
                }
                return undefined;
            }
        }
        return configModifications;
    }

    private async attemptConnection(config: ISshConfigExt): Promise<void> {
        const ssh = new NodeSSH();

        try {
            // Prepare connection configuration
            const connectionConfig = {
                host: config.hostname,
                port: config.port || 22,
                username: config.user,
                password: config.password,
                privateKey: config.privateKey ? readFileSync(path.normalize(config.privateKey), "utf8") : undefined,
                passphrase: config.keyPassphrase,
                readyTimeout: config.handshakeTimeout || 30000,
            };

            // Attempt connection
            await ssh.connect(connectionConfig);
            if (!ssh.isConnected()) {
                throw new Error("Failed to connect to SSH: All configured authentication methods failed");
            }
            // Test the connection by executing a simple command
            const result = await ssh.execCommand("exit");
            if (result.stderr?.startsWith("FOTS1668")) {
                throw new Error(result.stderr);
            }
        } finally {
            ssh.dispose();
        }
    }

    private async promptForPassword(
        config: ISshConfigExt,
        configModifications: ISshConfigExt,
    ): Promise<ISshConfigExt | undefined> {
        for (let attempts = 0; attempts < 3; attempts++) {
            const testPassword = await this.showInputBox({
                title: `${configModifications.user ?? config.user}@${config.hostname}'s password:`,
                password: true,
                placeHolder: "Enter your password",
            });

            if (!testPassword) return undefined;

            try {
                await this.attemptConnection({ ...config, ...configModifications, password: testPassword });
                return { password: testPassword };
            } catch (error) {
                if (`${error}`.includes("FOTS1668")) {
                    this.showMessage("Password Expired on Target System", MESSAGE_TYPE.ERROR);
                    return undefined;
                }
                this.showMessage(`Password Authentication Failed (${attempts + 1}/3)`, MESSAGE_TYPE.ERROR);
            }
        }
        return undefined;
    }

    private async validateFoundPrivateKeys() {
        // Create a progress bar using the custom Gui.withProgress
        await this.withProgress("Validating Private Keys...", async (progress) => {
            // Find private keys located at ~/.ssh/ and attempt to connect with them
            if (!this.validationResult) {
                const foundPrivateKeys = await ZClientUtils.findPrivateKeys();
                for (const privateKey of foundPrivateKeys) {
                    const testValidation: ISshConfigExt = { ...this.selectedProfile };
                    testValidation.privateKey = privateKey;

                    const result = await this.validateConfig(testValidation, false);

                    progress(100 / foundPrivateKeys.length);

                    if (result) {
                        this.validationResult = {};
                        this.selectedProfile = { ...this.selectedProfile, ...result, privateKey };
                        return;
                    }
                }
            }

            // Match hostname to configurations from ~/.ssh/config file
            let validationAttempts = this.migratedConfigs.filter(
                (config) => config.hostname === this.selectedProfile?.hostname,
            );

            // If multiple matches exist, narrow down by user
            if (validationAttempts.length > 1 && this.selectedProfile?.user) {
                validationAttempts = validationAttempts.filter((config) => config.user === this.selectedProfile?.user);
            } else {
                // If no user is specified, allow all configs where the hostname matches
                validationAttempts = validationAttempts.filter(
                    (config) => !this.selectedProfile?.user || config.user === this.selectedProfile?.user,
                );
            }

            for (const profile of validationAttempts) {
                const testValidation: ISshConfigExt = profile;
                const result = await this.validateConfig(testValidation, false);
                progress(100 / validationAttempts.length);
                if (result !== undefined) {
                    this.validationResult = {};
                    this.selectedProfile = {
                        ...this.selectedProfile,
                        privateKey: testValidation.privateKey,
                    };
                    if (Object.keys(result).length >= 1) {
                        this.selectedProfile = {
                            ...this.selectedProfile,
                            ...result,
                        };
                    }
                    return;
                }
            }
        });
    }

    private async setProfile(selectedConfig: ISshConfigExt, updatedProfile?: string): Promise<void> {
        const configApi = this.mProfilesCache.getTeamConfig().api;
        // Create the base config object
        const config: IConfigProfile = {
            type: "ssh",
            properties: {
                user: selectedConfig.user,
                host: selectedConfig.hostname,
                privateKey: selectedConfig.privateKey,
                port: selectedConfig.port || 22,
                keyPassphrase: selectedConfig.keyPassphrase,
                password: selectedConfig.password,
            },
            secure: [],
        };
        //if password or KP is defined, make them secure
        if (selectedConfig.password) config.secure.push("password" as never);
        if (selectedConfig.keyPassphrase) config.secure.push("keyPassphrase" as never);

        if (updatedProfile) {
            for (const key of Object.keys(selectedConfig)) {
                const validKey = key as keyof ISshConfigExt;

                // Get the location of the property being modified

                const propertyLocation = this.mProfilesCache
                    .mergeArgsForProfile({
                        profName: updatedProfile,
                        profType: "ssh",
                        isDefaultProfile: this.fetchDefaultProfile().name === updatedProfile,
                        profLoc: { locType: 1 },
                    })
                    .knownArgs.find((obj) => obj.argName === key)?.argLoc.jsonLoc;

                let allowBaseModification: string | undefined;

                if (propertyLocation) {
                    const profileName = configApi.profiles.getProfileNameFromPath(propertyLocation);

                    // Check to see if the property being modified comes from the service profile to handle possibly breaking configuration changes
                    const doesPropComeFromProfile = profileName === updatedProfile;

                    if (!doesPropComeFromProfile) {
                        const qpOpts: qpOpts = {
                            items: [
                                { label: "Yes", description: "Proceed with modification" },
                                { label: "No", description: "Modify SSH profile instead" },
                            ],
                            title: `Property: "${key}" found in a possibly shared configuration and may break others, continue?`,
                            placeholder: "Select an option",
                        };
                        allowBaseModification = await this.showMenu(qpOpts);
                    }
                }
                this.mProfilesCache.updateProperty({
                    profileName: updatedProfile,
                    profileType: "ssh",
                    property: validKey,
                    value: selectedConfig[validKey],
                    forceUpdate: allowBaseModification !== "Yes",
                    setSecure: this.mProfilesCache.isSecured(),
                });
            }
        } else {
            if (!configApi.profiles.defaultGet("ssh") || !configApi.layers.get().properties.defaults.ssh)
                configApi.profiles.defaultSet("ssh", selectedConfig.name!);
            configApi.profiles.set(selectedConfig.name!, config);
        }

        if (config.secure.length > 0) {
            await this.mProfilesCache.getTeamConfig().save();
        } else {
            this.mProfilesCache.getTeamConfig().api.layers.write();
        }
    }

    private async getNewProfileName(
        selectedProfile: ISshConfigExt,
        configApi: Config,
    ): Promise<ISshConfigExt | undefined> {
        let isUniqueName = false;

        // If no name option set then use hostname with all "." replaced with "_"
        if (!selectedProfile.name) selectedProfile.name = selectedProfile.hostname!.replace(/\./g, "_");

        // If selectedProfile already has a name, return it unless an existing profile is found
        if (selectedProfile.name) {
            const existingProfile = configApi.layerActive().properties.profiles[selectedProfile.name];
            if (existingProfile) {
                const overwriteOpts: qpOpts = {
                    items: [{ label: "Yes" }, { label: "No" }],
                    placeholder: `A profile with the name "${selectedProfile.name}" already exists. Do you want to overwrite it?`,
                };

                const overwriteResponse = await this.showMenu(overwriteOpts);

                if (overwriteResponse === "Yes") return selectedProfile;
            } else return selectedProfile;
        }

        // If no name set or overwriting, proceed with the input loop
        while (!isUniqueName) {
            selectedProfile.name = await this.showInputBox({
                title: "Enter a name for the SSH config",
                value: selectedProfile.name!.replace(/\./g, "_"),
                validateInput: (input) => (input.includes(".") ? "Name cannot contain '.'" : null),
            });

            if (!selectedProfile.name) return;
            const existingProfile = configApi.layerActive().properties.profiles[selectedProfile.name];
            if (existingProfile) {
                const overwriteResponse = await this.showMenu({
                    items: [{ label: "Yes" }, { label: "No" }],
                    placeholder: `A profile with the name "${selectedProfile.name}" already exists. Do you want to overwrite it?`,
                });
                if (overwriteResponse === "Yes") {
                    isUniqueName = true;
                }
            } else {
                isUniqueName = true;
            }
        }
        return selectedProfile;
    }

    // Taken from ZE Api and tweaked for usage
    private getMergedAttrs(prof: string | IProfAttrs): IProfile {
        const profile: IProfile = {};
        if (prof !== null) {
            const mergedArgs = this.mProfilesCache.mergeArgsForProfile(
                typeof prof === "string"
                    ? this.mProfilesCache.getAllProfiles("ssh").find((p) => p.profName === prof)
                    : prof,
                { getSecureVals: true },
            );
            for (const arg of mergedArgs.knownArgs) {
                profile[arg.argName] = arg.argValue;
            }
        }
        return profile;
    }

    // Taken from ZE Api and tweaked for usage
    private fetchAllSshProfiles(): IProfileLoaded[] {
        const profByType: IProfileLoaded[] = [];
        const profilesForType = this.mProfilesCache.getAllProfiles("ssh");
        for (const prof of profilesForType) {
            profByType.push({
                message: "",
                name: prof.profName,
                type: "ssh",
                profile: this.getMergedAttrs(prof),
                failNotFound: false,
            });
        }
        return profByType;
    }

    // Taken from ZE Api and tweaked for usage
    private fetchDefaultProfile(): IProfileLoaded {
        const defaultProfile = this.mProfilesCache.getDefaultProfile("ssh");
        return {
            message: "",
            name: defaultProfile.profName,
            type: "ssh",
            profile: this.getMergedAttrs(defaultProfile),
            failNotFound: false,
        };
    }

    /**
     * Handle an invalid private key by commenting it out in the configuration file
     * and showing a warning to the user
     */
    private async handleInvalidPrivateKey(config: ISshConfigExt): Promise<boolean> {
        if (!config.privateKey || !config.name) {
            // Private key is not invalid if its missing
            return true;
        }

        try {
            // Get the team configuration object
            const teamConfig = this.mProfilesCache.getTeamConfig();

            if (teamConfig.properties.autoStore === false) {
                return true;
            }

            // Comment out the private key property using Config API and comment-json
            const commentedProperty = ConfigFileUtils.getInstance().commentOutProperty(
                teamConfig,
                config.name,
                "privateKey",
            );

            if (commentedProperty) {
                // Show warning to user with undo/delete options
                const shouldContinue = await this.showPrivateKeyWarning({
                    profileName: config.name,
                    privateKeyPath: config.privateKey,
                    onUndo: () => {
                        const success = ConfigFileUtils.getInstance().uncommentProperty(
                            teamConfig,
                            config.name,
                            commentedProperty,
                        );
                        if (success) {
                            this.showMessage(
                                `Private key has been restored for profile "${config.name}".`,
                                MESSAGE_TYPE.INFORMATION,
                            );
                        } else {
                            this.showMessage(
                                "Failed to restore private key. You may need to manually edit the configuration file.",
                                MESSAGE_TYPE.ERROR,
                            );
                        }
                    },
                    onDelete: () => {
                        const success = ConfigFileUtils.getInstance().deleteCommentedLine(
                            teamConfig,
                            config.name,
                            commentedProperty,
                        );
                        if (success) {
                            this.showMessage(
                                `Private key comment lines have been deleted from profile "${config.name}".`,
                                MESSAGE_TYPE.INFORMATION,
                            );
                        } else {
                            this.showMessage(
                                "Failed to delete private key comment lines. You may need to manually edit the configuration file.",
                                MESSAGE_TYPE.ERROR,
                            );
                        }
                    },
                });
                return shouldContinue;
            }
        } catch (error) {
            console.error("Error handling invalid private key:", error);
            this.showMessage(
                "Failed to comment out invalid private key. You may need to manually edit the configuration file.",
                MESSAGE_TYPE.WARNING,
            );
        }

        return false;
    }
}
