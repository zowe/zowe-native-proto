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

const fs = require("fs");
const { ProfileInfo } = require("@zowe/imperative");
const { SshSession } = require("@zowe/zos-uss-for-zowe-sdk");
const { ZSshClient } = require("./packages/sdk");
let client;

async function main(localFile, remoteFile, encoding) {
    const profInfo = new ProfileInfo("zowe");
    await profInfo.readProfilesFromDisk();
    const sshProfAttrs = profInfo.getDefaultProfile("ssh");
    // console.log(sshProfAttrs);
    const sshMergedArgs = profInfo.mergeArgsForProfile(sshProfAttrs, { getSecureVals: true });
    const session = new SshSession(ProfileInfo.initSessCfg(sshMergedArgs.knownArgs));
    const serverPathArg = sshMergedArgs.knownArgs.find((arg) => arg.argName === "serverPath");
    client = await ZSshClient.create(session, { serverPath: serverPathArg?.argValue });

    /* Test uss files */
    await client.uss.createFile({ fspath: remoteFile });
    console.time("uploadTime");
    await client.uss.writeFile({
        fspath: remoteFile,
        stream: fs.createReadStream(localFile),
        encoding,
    });
    console.timeEnd("uploadTime");
    console.log("uploadSize:", fs.statSync(localFile).size.toLocaleString());
    console.time("downloadTime");
    await client.uss.readFile({
        fspath: remoteFile,
        stream: fs.createWriteStream(`${localFile}.tmp`),
        encoding,
    });
    console.timeEnd("downloadTime");
    console.log("downloadSize:", fs.statSync(`${localFile}.tmp`).size.toLocaleString());
    // await client.uss.deleteFile({ fspath: remoteFile });

    /* Test data sets */
    // console.time("uploadTime");
    // await client.ds.writeDataset({
    //     dsname: remoteFile,
    //     stream: fs.createReadStream(localFile),
    //     encoding,
    // });
    // console.timeEnd("uploadTime");
    // console.log("uploadSize:", fs.statSync(localFile).size.toLocaleString());
    // console.time("downloadTime");
    // await client.ds.readDataset({
    //     dsname: remoteFile,
    //     stream: fs.createWriteStream(`${localFile}.tmp`),
    //     encoding,
    // });
    // console.timeEnd("downloadTime");
    // console.log("downloadSize:", fs.statSync(`${localFile}.tmp`).size.toLocaleString());

    fs.unlinkSync(`${localFile}.tmp`);
}

main(...process.argv.slice(2)).catch(console.error).finally(() => client?.dispose());