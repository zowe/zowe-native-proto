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

// Usage: npm run z:sync -- ./native lpar1_ssh:/u/users/ibmuser/zowe-native-proto

import * as fsWalk from "@nodelib/fs.walk";
import { ImperativeError, type IProfile, ProfileInfo } from "@zowe/imperative";
import * as fs from "node:fs";
import * as path from "node:path";
import type { Client } from "ssh2";
import { B64String, ZSshClient, ZSshUtils, type uss } from "zowe-native-proto-sdk";

const TOOLS_DIR = "/tmp/zowe-native-proto_tools";
let TIME_OFFSET = 0;
let SPINNER_INDEX = 0;
const SPINNER_FRAMES = ["-", "\\", "|", "/"];

function startSpinner() {
    return setInterval(() => {
        process.stdout.write(`\r${SPINNER_FRAMES[SPINNER_INDEX]}`);
        SPINNER_INDEX = (SPINNER_INDEX + 1) % SPINNER_FRAMES.length;
    }, 100);
}

function stopSpinner(spinner: NodeJS.Timeout | null) {
    spinner && clearInterval(spinner);
    process.stdout.write("\r\n");
}

async function loadSshProfile(profileName: string): Promise<IProfile> {
    const profInfo = new ProfileInfo("zowe");
    await profInfo.readProfilesFromDisk();
    const profAttrs = profInfo.getAllProfiles("ssh").find((prof) => prof.profName === profileName);
    const mergedArgs = profInfo.mergeArgsForProfile(profAttrs, { getSecureVals: true });
    return mergedArgs.knownArgs.reduce((prof, arg) => ({ ...prof, [arg.argName]: arg.argValue }), {});
}

async function getTimeOffset(sshClient: Client, hostname: string): Promise<number> {
    const zsyncJsonPath = path.join(__dirname, "..", ".zsync.json");
    let zsyncJson: Record<string, number> = {};
    if (fs.existsSync(zsyncJsonPath)) {
        zsyncJson = JSON.parse(fs.readFileSync(zsyncJsonPath, "utf-8"));
    }
    if (zsyncJson[hostname] == null) {
        const startTime = Date.now();
        const utcTimeStr = await new Promise<string>((resolve, reject) => {
            sshClient.exec("date -u '+%Y-%m-%dT%TZ'", (err, stream) => {
                if (err) reject(err);
                stream.on("data", (data: Buffer) => {
                    resolve(data.toString().trim());
                });
            });
        });
        const roundTripTime = Date.now() - startTime;
        const estimatedServerTime = startTime + roundTripTime / 2;
        zsyncJson[hostname] = Math.round(new Date(utcTimeStr).getTime() - estimatedServerTime);
        fs.writeFileSync(zsyncJsonPath, JSON.stringify(zsyncJson, null, 2));
    }
    return zsyncJson[hostname];
}

async function upload(client: ZSshClient, srcDir: string, destDir: string, shouldDelete: boolean): Promise<void> {
    if (!fs.existsSync(srcDir)) {
        throw new Error(`Source directory ${srcDir} does not exist`);
    }
    let listResponse: uss.ListFilesResponse = { items: [], returnedRows: 0, success: true };
    try {
        listResponse = await client.uss.listFiles({ fspath: destDir, depth: 10, long: true });
    } catch (error) {
        if (error instanceof ImperativeError && error.additionalDetails.includes("does not exist")) {
            await client.uss.createFile({ fspath: destDir, isDir: true });
        } else {
            throw error;
        }
    }
    const pathCache: string[] = [];
    const uploadTasks = [];
    let createCount = 0;
    let updateCount = 0;
    let deleteCount = 0;
    for (const entry of fsWalk.walkSync(srcDir)) {
        const relPath = path.relative(srcDir, entry.path);
        const localPath = entry.path;
        const remotePath = path.posix.join(destDir, relPath);
        pathCache.push(relPath);
        if (entry.dirent.isDirectory()) {
            const dirExists = listResponse.items.some((item) => item.name === relPath);
            if (!dirExists) {
                await client.uss.createFile({ fspath: remotePath, isDir: true });
                createCount++;
            }
        } else if (!path.basename(relPath).startsWith(".")) {
            const remoteStats = listResponse.items.find((item) => item.name === relPath);
            let fileOutdated = true;
            if (remoteStats != null) {
                const localMtime = fs.statSync(localPath).mtime.getTime();
                const remoteMtime = new Date(remoteStats.mtime).getTime() - TIME_OFFSET;
                fileOutdated = remoteMtime - 100 < localMtime;
            }
            if (fileOutdated) {
                uploadTasks.push(
                    client.uss.writeFile({
                        fspath: remotePath,
                        data: B64String.encode(fs.readFileSync(localPath)),
                        // encoding: "IBM-1047",
                    }),
                );
                if (remoteStats != null) {
                    updateCount++;
                } else {
                    createCount++;
                }
            }
        }
    }
    if (shouldDelete) {
        for (const item of listResponse.items.reverse()) {
            if (!pathCache.includes(item.name)) {
                await client.uss.deleteFile({ fspath: path.posix.join(destDir, item.name), recursive: false });
                deleteCount++;
            }
        }
    }
    await Promise.all(uploadTasks);
    console.log(`\r+ ${createCount}\t- ${deleteCount}\t* ${updateCount}`);
}

async function download(client: ZSshClient, srcDir: string, destDir: string, shouldDelete: boolean): Promise<void> {
    let listResponse: uss.ListFilesResponse;
    try {
        listResponse = await client.uss.listFiles({ fspath: srcDir, depth: 10, long: true });
    } catch (error) {
        if (error instanceof ImperativeError && error.additionalDetails.includes("does not exist")) {
            throw new Error(`Source directory ${srcDir} does not exist`);
        } else {
            throw error;
        }
    }
    fs.mkdirSync(destDir, { recursive: true });
    const pathCache: string[] = [];
    const downloadTasks = [];
    let createCount = 0;
    let updateCount = 0;
    let deleteCount = 0;
    for (const item of listResponse.items) {
        const relPath = item.name;
        const remotePath = path.posix.join(srcDir, relPath);
        const localPath = path.join(destDir, relPath);
        pathCache.push(relPath);
        if (item.mode.startsWith("d")) {
            const dirExists = fs.existsSync(localPath);
            if (!dirExists) {
                fs.mkdirSync(localPath);
                createCount++;
            }
        } else if (!path.basename(relPath).startsWith(".")) {
            const localStats = fs.existsSync(localPath) ? fs.statSync(localPath) : undefined;
            let fileOutdated = true;
            if (localStats != null) {
                const localMtime = localStats.mtime.getTime();
                const remoteMtime = new Date(item.mtime).getTime() - TIME_OFFSET;
                fileOutdated = localMtime - 100 < remoteMtime;
            }
            if (fileOutdated) {
                downloadTasks.push(
                    client.uss
                        .readFile({
                            fspath: remotePath,
                        })
                        .then((response) => {
                            fs.writeFileSync(localPath, B64String.decodeBytes(response.data));
                        }),
                );
                if (localStats != null) {
                    updateCount++;
                } else {
                    createCount++;
                }
            }
        }
    }
    if (shouldDelete) {
        for (const entry of fsWalk.walkSync(srcDir).reverse()) {
            if (!pathCache.includes(path.relative(srcDir, entry.path))) {
                fs.unlinkSync(entry.path);
                deleteCount++;
            }
        }
    }
    await Promise.all(downloadTasks);
    console.log(`\r+ ${createCount}\t- ${deleteCount}\t* ${updateCount}`);
}

async function remoteSync(argv: any): Promise<void> {
    let shouldUpload = true;
    let sshProfile: string;
    let srcDir = argv._[0];
    if (srcDir.includes(":")) {
        shouldUpload = false;
        [sshProfile, srcDir] = srcDir.split(":");
    }
    let destDir = argv._[1];
    if (destDir.includes(":")) {
        if (!shouldUpload) {
            throw new Error("Cannot specify remote folders for both source and destination");
        }
        [sshProfile, destDir] = destDir.split(":");
    }
    if (sshProfile == null) {
        throw new Error("Cannot specify local folders for both source and destination");
    }
    const sshSession = ZSshUtils.buildSession(await loadSshProfile(sshProfile));
    using client = await ZSshClient.create(sshSession, {
        serverPath: TOOLS_DIR,
        numWorkers: 3,
    });
    TIME_OFFSET = await getTimeOffset((client as any).mSshClient as Client, sshSession.ISshSession.hostname);
    const spinner = startSpinner();
    console.time("sync");
    if (shouldUpload) {
        await upload(client, srcDir, destDir, argv.delete);
    } else {
        await download(client, srcDir, destDir, argv.delete);
    }
    stopSpinner(spinner);
    console.timeEnd("sync");
}

async function main() {
    const argv = require("minimist")(process.argv.slice(2));
    await remoteSync(argv);
}

main().catch(console.error);
