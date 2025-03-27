import type { ISshConfigExt } from "./ZClientUtils";
import {
    FileManagement,
    Gui,
    type IZoweTree,
    type IZoweTreeNode,
    ZoweVsCodeExtension,
    imperative,
} from "@zowe/zowe-explorer-api";
import { ProfileConstants } from "@zowe/core-for-zowe-sdk";
import { Client, type ClientChannel } from "ssh2";
import { readFileSync } from "node:fs";
import * as path from "node:path";

export enum MESSAGE_TYPE {
    INFORMATION = 1,
    WARNING = 2,
    ERROR = 3,
}

export interface inputBoxOpts {
    title: string;
    placeHolder: string;
    ignoreFocusOut?: boolean;
    password?: boolean;
}
export type ProgressCallback = (percent: number) => void;
export abstract class AbstractConfigManager {
    protected abstract showMessage(message: string, type: MESSAGE_TYPE): void;
    protected abstract showInputBox(opts: inputBoxOpts): Promise<string | undefined>;
    protected abstract withProgress<T>(message: string, task: (progress: ProgressCallback) => Promise<T>): Promise<T>;

    public async doLongTask(max: number): Promise<void> {
        await this.withProgress("Working...", async (progress) => {
            for (let i = 0; i < max; i++) {
                // Invoke progress callback, then do long running task
                progress(1);
                await new Promise((resolve) => setTimeout(resolve, 100));
            }
        });
    }
    // protected abstract progressBar(): Promise<string>;

    public async createNewConfig(knownConfigOpts?: string): Promise<ISshConfigExt | undefined> {
        const sshRegex = /^ssh\s+(?:([a-zA-Z0-9_-]+)@)?([a-zA-Z0-9.-]+)/;

        const flagRegex = /-(\w+)(?:\s+("[^"]+"|'[^']+'|\S+))?/g;
        const SshProfile: ISshConfigExt = {};
        const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
        const profInfo = await zoweExplorerApi.getExplorerExtenderApi().getProfilesCache().getProfileInfo();

        //check if project layer exists, if it doesnt create one, but if no workspace then create it as global

        const workspaceDir = ZoweVsCodeExtension.workspaceRoot;
        if (workspaceDir?.uri.fsPath !== undefined && !profInfo.getTeamConfig().layerExists(workspaceDir!.uri.fsPath)) {
            await this.createZoweSchema(false);
        }

        let sshResponse: string | undefined;

        // KnownConfigOpts is defined if a custom option is selected via the first quickpick (ex: user@host is entered in search bar)
        if (!knownConfigOpts) {
            sshResponse = await this.showInputBox({
                title: "Enter SSH connection command",
                placeHolder: "E.g. ssh user@example.com",
                ignoreFocusOut: true,
            });
        } else {
            sshResponse = `ssh ${knownConfigOpts}`;
            const match = sshResponse.match(sshRegex);
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

        const sshMatch = sshResponse.match(sshRegex);

        if (!sshMatch) {
            this.showMessage("Invalid SSH command format. Ensure it matches the expected pattern.", MESSAGE_TYPE.ERROR);
            return undefined;
        }

        SshProfile.user = sshMatch[1] || require("node:os").userInfo().username;
        SshProfile.hostname = sshMatch[2];

        let flagMatch: RegExpExecArray | null;

        if (!knownConfigOpts) {
            // biome-ignore lint/suspicious/noAssignInExpressions: We just want to use the regex array in the loop
            while ((flagMatch = flagRegex.exec(sshResponse)) !== null) {
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
                            `Invalid value for flag --${flag}. Port must be a valid number.`,
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
                        `Invalid value for flag --${flag}. Values with spaces must be quoted.`,
                        MESSAGE_TYPE.ERROR,
                    );
                    return undefined;
                }
            }
        }
        return SshProfile;
    }
    // Cloned method
    public async createZoweSchema(global: boolean): Promise<void> {
        try {
            const user = false;
            const workspaceDir = ZoweVsCodeExtension.workspaceRoot;

            const config = await imperative.Config.load("zowe", {
                homeDir: FileManagement.getZoweDir(),
                projectDir: workspaceDir?.uri.fsPath,
            });

            config.api.layers.activate(user, global);

            const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
            const profCache = zoweExplorerApi.getExplorerExtenderApi().getProfilesCache();
            const knownCliConfig: imperative.ICommandProfileTypeConfiguration[] = [
                // biome-ignore lint/suspicious/noExplicitAny: Accessing protected method
                ...(profCache as any).getCoreProfileTypes(),
                ...profCache.getConfigArray(),
                ProfileConstants.BaseProfile,
            ];

            config.setSchema(imperative.ConfigSchema.buildSchema(knownCliConfig));

            // Note: IConfigBuilderOpts not exported
            // const opts: IConfigBuilderOpts = {
            const opts: imperative.IConfigBuilderOpts = {
                // getSecureValue: this.promptForProp.bind(this),
                populateProperties: true,
            };
            // Build new config and merge with existing layer
            const impConfig: Partial<imperative.IImperativeConfig> = {
                profiles: [ProfileConstants.BaseProfile],
                baseProfile: ProfileConstants.BaseProfile,
            };
            const newConfig: imperative.IConfig = await imperative.ConfigBuilder.build(impConfig, global, opts);
            config.api.layers.merge(newConfig);
            await config.save(false);
        } catch (err) {}
    }

    private async validateConfig(newConfig: ISshConfigExt): Promise<ISshConfigExt | undefined> {
        const configModifications: ISshConfigExt | undefined = {};
        const attemptConnection = async (config: ISshConfigExt): Promise<boolean> => {
            return new Promise((resolve, reject) => {
                const sshClient = new Client();
                const testConfig = { ...config };

                if (testConfig.privateKey && typeof testConfig.privateKey === "string") {
                    testConfig.privateKey = readFileSync(path.normalize(testConfig.privateKey), "utf8");
                }

                sshClient
                    .connect({ ...testConfig, passphrase: testConfig.keyPassphrase })
                    .on("error", reject)
                    .on("ready", () => {
                        sshClient.shell((err, stream: ClientChannel) => {
                            if (err) return reject(err);
                            stream.on("data", (data: Buffer | string) => {
                                if (data.toString().startsWith("FOTS1668")) reject(new Error(data.toString()));
                            });
                            stream.on("end", () => resolve(true));
                            sshClient.end();
                        });
                    });
            });
        };

        const promptForPassword = async (config: ISshConfigExt): Promise<ISshConfigExt | undefined> => {
            for (let attempts = 0; attempts < 3; attempts++) {
                const testPassword = await this.showInputBox({
                    title: `${configModifications.user ?? config.user}@${config.hostname}'s password:`,
                    password: true,
                    placeHolder: "Enter your password",
                    ignoreFocusOut: true,
                });

                if (!testPassword) return undefined;

                try {
                    await attemptConnection({ ...config, ...configModifications, password: testPassword });
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
        };

        try {
            const privateKeyPath = newConfig.privateKey;

            if (!newConfig.user) {
                const userModification = await this.showInputBox({
                    title: `Enter user for host: '${newConfig.hostname}'`,
                    placeHolder: "Enter the user for the target host",
                    ignoreFocusOut: true,
                });
                configModifications.user = userModification;
            }

            if ((!privateKeyPath || !readFileSync(path.normalize(privateKeyPath), "utf-8")) && !newConfig.password) {
                const passwordPrompt = await promptForPassword(newConfig);
                return passwordPrompt ? { ...configModifications, ...passwordPrompt } : undefined;
            }

            await attemptConnection({ ...newConfig, ...configModifications });
        } catch (err) {
            const errorMessage = `${err}`;

            if (
                newConfig.privateKey &&
                !newConfig.password &&
                errorMessage.includes("All configured authentication methods failed")
            ) {
                return undefined;
            }

            if (errorMessage.includes("Invalid username")) {
                const testUser = await this.showInputBox({
                    title: `Enter user for host: '${newConfig.hostname}'`,
                    placeHolder: "Enter the user for the target host",
                    ignoreFocusOut: true,
                });

                if (!testUser) return undefined;
                try {
                    await attemptConnection({ ...newConfig, user: testUser });
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
                        ignoreFocusOut: true,
                    });

                    try {
                        await attemptConnection({
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
                return undefined;
            }
            if (errorMessage.includes("All configured authentication methods failed")) {
                const passwordPrompt = await promptForPassword(newConfig);
                return passwordPrompt ? { ...configModifications, ...passwordPrompt } : undefined;
            }
        }
        return configModifications;
    }

    public async validateFoundPrivateKeys() {
        // Create a progress bar using the custom Gui.withProgress

        await this.withProgress("Working...", async (progress) => {
            // Find private keys located at ~/.ssh/ and attempt to connect with them
            if (!SshConfigUtils.validationResult) {
                const foundPrivateKeys = await ZClientUtils.findPrivateKeys();
                for (const privateKey of foundPrivateKeys) {
                    const testValidation: ISshConfigExt = SshConfigUtils.selectedProfile!;
                    testValidation.privateKey = privateKey;
                    const result = await SshConfigUtils.validateConfig(testValidation);
                    progress.report({ increment: 100 / foundPrivateKeys.length });

                    if (result) {
                        SshConfigUtils.validationResult = {};
                        if (Object.keys(result).length >= 1) {
                            SshConfigUtils.selectedProfile = { ...SshConfigUtils.selectedProfile, ...result };
                        }
                        return;
                    }
                }
            }

            // Match hostname to configurations from ~/.ssh/config file
            let validationAttempts = SshConfigUtils.migratedConfigs.filter(
                (config) => config.hostname === SshConfigUtils.selectedProfile?.hostname,
            );

            // If multiple matches exist, narrow down by user
            if (validationAttempts.length > 1 && SshConfigUtils.selectedProfile?.user) {
                validationAttempts = validationAttempts.filter(
                    (config) => config.user === SshConfigUtils.selectedProfile?.user,
                );
            } else {
                // If no user is specified, allow all configs where the hostname matches
                validationAttempts = validationAttempts.filter(
                    (config) =>
                        !SshConfigUtils.selectedProfile?.user || config.user === SshConfigUtils.selectedProfile?.user,
                );
            }

            for (const profile of validationAttempts) {
                const testValidation: ISshConfigExt = profile;
                const result = await SshConfigUtils.validateConfig(testValidation);
                progress.report({ increment: 100 / validationAttempts.length });
                if (result !== undefined) {
                    SshConfigUtils.validationResult = {};
                    if (Object.keys(result).length >= 1) {
                        SshConfigUtils.selectedProfile = {
                            ...SshConfigUtils.selectedProfile,
                            ...result,
                            privateKey: testValidation.privateKey,
                        };
                    }
                    return;
                }
            }
        });
    }
}
