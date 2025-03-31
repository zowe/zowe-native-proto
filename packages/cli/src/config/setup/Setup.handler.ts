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

import { CommandResponse, type ICommandHandler, type IHandlerParameters, ImperativeError } from "@zowe/imperative";
import { ProfileConstants } from "@zowe/core-for-zowe-sdk";
import { type IProfileTypeConfiguration, ProfileInfo } from "@zowe/imperative";
import {
    AbstractConfigManager,
    MESSAGE_TYPE,
    type ProgressCallback,
    type inputBoxOpts,
    type qpItem,
    type qpOpts,
} from "zowe-native-proto-sdk";
import * as termkit from "terminal-kit";

export default class ServerInstallHandler implements ICommandHandler {
    public async process(params: IHandlerParameters): Promise<void> {
        const profInfo = new ProfileInfo("zowe");
        await profInfo.readProfilesFromDisk();
        const cliPromptApi = new CliPromptApi(profInfo);
        const profile = await cliPromptApi.promptForProfile();
        console.debug("Profile", profile);
    }
}

export class CliPromptApi extends AbstractConfigManager {
    private term = termkit.terminal;
    protected showMessage(message: string, type: MESSAGE_TYPE): void {
        switch (type) {
            case MESSAGE_TYPE.INFORMATION:
                console.log(`INFO: ${message}`);
                break;
            case MESSAGE_TYPE.WARNING:
                console.warn(`WARNING: ${message}`);
                break;
            case MESSAGE_TYPE.ERROR:
                console.error(`ERROR: ${message}`);
                break;
            default:
                throw new ImperativeError({ msg: "Unknown message type" });
        }
    }

    protected async showInputBox(opts: inputBoxOpts): Promise<string | undefined> {
        this.term(`${opts.title || "Input"}: `);
        this.term.grabInput(true);
        return new Promise<string | undefined>((resolve) => {
            this.term.inputField(
                {
                    echo: !opts.password,
                },
                (error: Error, input: string) => {
                    this.term("\n\n");
                    this.term.grabInput(false);
                    if (error) {
                        console.error("An error occurred while getting input:", error);
                        resolve(undefined);
                    } else {
                        resolve(input);
                    }
                },
            );
        });
    }

    protected async withProgress<T>(message: string, task: (progress: ProgressCallback) => Promise<T>): Promise<T> {
        const commandResponse = new CommandResponse();

        commandResponse.console.log(message);

        let result: T;
        try {
            result = await task((percent) => {
                commandResponse.console.log(`Progress: ${percent}%`);
            });
            commandResponse.console.log("Task completed successfully.");
        } catch (error) {
            commandResponse.console.error(`Task failed: ${error}`);
            throw error;
        }

        return result;
    }

    protected async showQuickPick(opts: qpOpts): Promise<string | undefined> {
        this.term.grabInput(true);

        // Handle cancellation
        this.term.on("key", (key: string) => {
            if (key === "ESCAPE" || key === "CTRL_C") {
                this.term.grabInput(false);
                this.term.clear();
                return Promise<undefined>;
            }
        });
        return new Promise<string | undefined>((resolve) => {
            if (opts.placeholder) this.term.green(`\n${opts.placeholder}\n`);

            const menuItems = opts.items.map((item) => item.label);

            // Create menu with proper type assertion
            const menu = this.term.singleColumnMenu(menuItems, {
                cancelable: true,
                continueOnSubmit: false,
                // biome-ignore lint/suspicious/noExplicitAny: Required for callback
            } as any) as unknown as {
                on: (event: string, handler: (key: string) => void) => void;
                abort: () => void;
            };

            // biome-ignore lint/suspicious/noExplicitAny: Required for callback
            menu.on("submit", (response: any) => {
                const selected = opts.items[response.selectedIndex];
                this.term("\n\n");
                this.term.grabInput(false);
                resolve(selected.label);
            });
        });
    }

    protected async showCustomQuickPick(opts: qpOpts): Promise<qpItem | undefined> {
        this.term.grabInput(true);

        // Handle cancellation
        this.term.on("key", (key: string) => {
            if (key === "ESCAPE" || key === "CTRL_C") {
                this.term.grabInput(false);
                this.term.clear();
                return Promise<undefined>;
            }
        });
        return new Promise<qpItem | undefined>((resolve) => {
            // Map items for Terminal Kit with separator handling
            const menuItems = opts.items.map((item) =>
                item.separator ? `${"─".repeat(10)}Migrate from SSH Config${"─".repeat(10)}` : item.label,
            );

            this.term.green(`\n${opts.placeholder}\n`);

            // Create menu with proper type assertion
            const menu = this.term.singleColumnMenu(menuItems, {
                cancelable: true,
                continueOnSubmit: false,
                // biome-ignore lint/suspicious/noExplicitAny: Required for callback
            } as any) as unknown as {
                on: (event: string, handler: (key: string) => void) => void;
                abort: () => void;
            };

            // biome-ignore lint/suspicious/noExplicitAny: Required for callback
            menu.on("submit", (response: any) => {
                const selected = opts.items[response.selectedIndex];
                this.term.grabInput(false);
                this.term("\n\n");
                resolve(selected?.separator ? undefined : selected);
            });
        });
    }

    protected getCurrentDir(): string | undefined {
        return process.cwd();
    }

    protected getProfileType(): IProfileTypeConfiguration[] {
        return [
            // biome-ignore lint/suspicious/noExplicitAny: Accessing protected method
            ...(ProfileInfo as any).getCoreProfileTypes(),
            ProfileConstants.BaseProfile,
        ];
    }
}
