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

import { statSync, unlinkSync } from "node:fs";
import { parse } from "node:path";
import { ProfileInfo } from "@zowe/imperative";
import { Download, Upload } from "@zowe/zos-files-for-zowe-sdk";
import * as utils from "./utils";
const userConfig = require("./config.json");
const testPrefix = parse(__filename).name;

async function main() {
    // Set up
    const profInfo = new ProfileInfo("zowe");
    await profInfo.readProfilesFromDisk();
    const zosmfProfAttrs = profInfo.getAllProfiles("zosmf").find((prof) => prof.profName === userConfig.zosmfProfile);
    const zosmfMergedArgs = profInfo.mergeArgsForProfile(zosmfProfAttrs, { getSecureVals: true });
    const session = ProfileInfo.createSession(zosmfMergedArgs.knownArgs);
    const { localFile, remoteFile, tempFile } = utils.getFilenames(userConfig);

    for (let i = 0; i < userConfig.testCount; i++) {
        // 1. Upload
        console.time(`${testPrefix}:upload`);
        await Upload.fileToUssFile(session, localFile, remoteFile, { binary: true });
        console.timeEnd(`${testPrefix}:upload`);

        // 2. Download
        console.time(`${testPrefix}:download`);
        await Download.ussFile(session, remoteFile, { binary: true, file: tempFile });
        console.timeEnd(`${testPrefix}:download`);

        // 3. Verify
        const success = await utils.compareChecksums(localFile, tempFile);
        if (success) {
            console.log(`✅ Checksums match (${i + 1}/${userConfig.testCount})`);
        } else {
            console.error(`❌ Checksums do not match (${i + 1}/${userConfig.testCount}): ${statSync(tempFile).size}`);
        }
    }

    // Tear down
    unlinkSync(tempFile);
}

main();
