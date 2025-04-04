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

import {
    type ICommandHandler,
    type IConfigLayer,
    type IHandlerParameters,
    type IHandlerResponseApi,
    type IProfileTypeConfiguration,
    ImperativeConfig,
    ImperativeError,
    ProfileInfo,
    TextUtils,
} from "@zowe/imperative";
import * as termkit from "terminal-kit";
import {
    AbstractConfigManager,
    MESSAGE_TYPE,
    type ProgressCallback,
    type inputBoxOpts,
    type qpItem,
    type qpOpts,
} from "zowe-native-proto-sdk";

export default class ServerInstallHandler implements ICommandHandler {
    public async process(params: IHandlerParameters): Promise<void> {
        const profInfo = new ProfileInfo("zowe");
        await profInfo.readProfilesFromDisk();
        const cliPromptApi = new CliPromptApi(profInfo, params.response);
        const profile = await cliPromptApi.promptForProfile();
        if (profile) {
            params.response.console.log(
                `SSH Profile Validated: ${(profInfo.getTeamConfig().api.layers.find(profile.name) as IConfigLayer).path}`,
            );
        }
    }
}

export class CliPromptApi extends AbstractConfigManager {
    constructor(
        mProfilesCache: ProfileInfo,
        private mResponseApi: IHandlerResponseApi,
    ) {
        super(mProfilesCache);
    }

    private term = termkit.terminal;
    protected showMessage(message: string, type: MESSAGE_TYPE): void {
        switch (type) {
            case MESSAGE_TYPE.INFORMATION:
                this.mResponseApi.console.log(message);
                break;
            case MESSAGE_TYPE.WARNING:
                this.mResponseApi.console.log(TextUtils.chalk.yellow(message));
                break;
            case MESSAGE_TYPE.ERROR:
                this.mResponseApi.console.error(TextUtils.chalk.red(message));
                break;
            default:
                throw new ImperativeError({ msg: "Unknown message type" });
        }
    }

    protected async showInputBox(opts: inputBoxOpts): Promise<string | undefined> {
        return await this.mResponseApi.console.prompt(`${opts.title}: `.replace("::", ":"), {
            hideText: opts.password,
        });
    }

    protected async withProgress<T>(message: string, task: (progress: ProgressCallback) => Promise<T>): Promise<T> {
        return await task(() => {});
    }

    protected async showMenu(opts: qpOpts): Promise<string | undefined> {
        return (await this.showCustomMenu(opts)).label;
    }

    protected async showCustomMenu(opts: qpOpts): Promise<qpItem | undefined> {
        return new Promise<qpItem | undefined>((resolve) => {
            this.term.grabInput(true);

            // Prepare menu items
            const menuItems = opts.items.map((item) =>
                item.separator
                    ? `${"─".repeat(10)}Migrate from SSH Config${"─".repeat(10)}`
                    : item.label.replace("$(plus)", "+"),
            );

            this.term.green(`\n${opts.placeholder.replace("enter user@host", "create a new SSH host")}\n`);

            let selectedIndex = 0;
            while (opts.items[selectedIndex]?.separator) {
                selectedIndex++;
            }

            this.term.getCursorLocation((error, x, y) => {
                if (error) {
                    console.error("Error getting cursor location:", error);
                    resolve(undefined);
                    return;
                }

                const menu = this.term.singleColumnMenu(menuItems, {
                    cancelable: true,
                    continueOnSubmit: false,
                    oneLineItem: true,
                    selectedIndex,
                    y: y + 1,
                    submittedStyle: this.term.bold.green,
                    selectedStyle: this.term.brightGreen,
                    leftPadding: "  ",
                    selectedLeftPadding: "> ",
                    submittedLeftPadding: "> ",
                    extraLines: 2,
                }) as unknown as {
                    // biome-ignore lint/suspicious/noExplicitAny: Required for callback
                    on: (event: string, handler: (response: any) => void) => void;
                    abort: () => void;
                    select: (index: number) => void;
                };

                const moveSelection = (direction: 1 | -1) => {
                    let newIndex = selectedIndex;
                    do {
                        newIndex += direction;
                        if (newIndex < 0) newIndex = opts.items.length - 1;
                        if (newIndex >= opts.items.length) newIndex = 0;
                    } while (opts.items[newIndex]?.separator && newIndex >= 0 && newIndex < opts.items.length);

                    selectedIndex = newIndex;
                    menu.select(selectedIndex);
                };

                // Key bindings for navigation
                const keyHandler = (key: string) => {
                    if (key === "UP" || key === "k") moveSelection(-1);
                    if (key === "DOWN" || key === "j") moveSelection(1);
                    if (key === "TAB") moveSelection(1);
                };
                this.term.on("key", keyHandler);

                // Handle menu submission
                // biome-ignore lint/suspicious/noExplicitAny: Required for callback
                menu.on("submit", (response: any) => {
                    const selected = opts.items[response.selectedIndex];

                    // Cleanup event listeners and input grabbing
                    this.term.removeListener("key", keyHandler);
                    this.term.grabInput(false);
                    resolve(selected?.separator ? undefined : selected);
                });

                // Handle cancellation or ESCAPE
                const cancelHandler = (key: string) => {
                    if (key === "ESCAPE" || key === "CTRL_C") {
                        // Cleanup event listeners and input grabbing
                        this.term.removeListener("key", keyHandler);
                        this.term.removeListener("key", cancelHandler);
                        menu.abort();
                        this.term.grabInput(false);

                        resolve(undefined);
                    }
                };
                this.term.on("key", cancelHandler);
            });
        });
    }

    protected getCurrentDir(): string | undefined {
        return process.cwd();
    }

    protected getProfileType(): IProfileTypeConfiguration[] {
        return ImperativeConfig.instance.loadedConfig.profiles;
    }
}
