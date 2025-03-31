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

import { type ICommandHandler, type IHandlerParameters, ImperativeError } from "@zowe/imperative";
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

        return new Promise<string | undefined>((resolve) => {
            this.term.inputField(
                {
                    echo: true,
                },
                (error: Error, input: string) => {
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
        throw new ImperativeError({ msg: "Not implemented yet" });
    }

    protected async showQuickPick(opts: qpOpts): Promise<string | undefined> {
        throw new ImperativeError({ msg: "Not implemented yet" });
    }

    protected async showCustomQuickPick(opts: qpOpts): Promise<qpItem | undefined> {
        const items: string[] = [];

        for (const index of opts.items) {
            items.push(index.label);
        }

        let selectedItem: qpItem;

        this.term.singleColumnMenu(items, function (error, response) {
            this.term("\n").eraseLineAfter.green(
                "#%s selected: %s (%s,%s)\n",
                response.selectedIndex,
                response.selectedText,
                response.x,
                response.y,
            );
            selectedItem.label = response.selectedText;
            process.exit();
        });
        return selectedItem;
    }

    protected getCurrentDir(): string | undefined {
        throw new ImperativeError({ msg: "Not implemented yet" });
    }

    protected getProfileType(): IProfileTypeConfiguration[] {
        throw new ImperativeError({ msg: "Not implemented yet" });
    }
}
