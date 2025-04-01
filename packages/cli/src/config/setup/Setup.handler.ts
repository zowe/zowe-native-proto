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
        await cliPromptApi.promptForProfile();
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
                this.mResponseApi.console.log(TextUtils.chalk.red(message));
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

    protected async showQuickPick(opts: qpOpts): Promise<string | undefined> {
        return (await this.showCustomQuickPick(opts)).label;
    }

    protected async showCustomQuickPick(opts: qpOpts): Promise<qpItem | undefined> {
        this.term.grabInput(true);

        return new Promise<qpItem | undefined>((resolve) => {
            // Handle cancellation
            this.term.on("key", (key: string) => {
                if (key === "ESCAPE" || key === "CTRL_C") {
                    this.term.grabInput(false);
                    resolve(undefined);
                }
            });

            // Map items for Terminal Kit with separator handling
            const menuItems = opts.items.map((item) =>
                item.separator
                    ? `${"─".repeat(10)}Migrate from SSH Config${"─".repeat(10)}`
                    : item.label.replace("$(plus)", "+"),
            );

            this.term.green(`\n${opts.placeholder.replace("enter user@host", "create a new SSH host")}\n`);

            // Create menu with proper type assertion
            const menu = this.term.singleColumnMenu(menuItems, {
                cancelable: true,
                continueOnSubmit: false,
                leftPadding: "",
                oneLineItem: true,
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
        return ImperativeConfig.instance.loadedConfig.profiles;
    }
}
