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
import { type IProfile, ProfileInfo } from "@zowe/imperative";
import * as fs from "node:fs";
import * as path from "node:path";
import type { Client } from "ssh2";
import { B64String, ZSshClient, ZSshUtils } from "zowe-native-proto-sdk";

const TOOLS_DIR = "/tmp/zowe-native-proto_tools";
let TIME_OFFSET = 0;

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
    const listResponse = await client.uss.listFiles({ fspath: destDir, depth: 10, long: true });
    // TODO Create root dir if it doesn't exist
    const pathCache: string[] = [];
    const uploadTasks = [];
    let createCount = 0;
    let updateCount = 0;
    let deleteCount = 0;
    for (const entry of fsWalk.walkSync(srcDir)) {
        const relPath = path.relative(srcDir, entry.path);
        const remotePath = path.join(destDir, relPath);
        pathCache.push(relPath);
        if (entry.dirent.isDirectory()) {
            const dirExists = listResponse.items.some((item) => item.name === relPath);
            if (!dirExists) {
                console.log(`Creating ${relPath}`);
                await client.uss.createFile({ fspath: remotePath, isDir: true });
                createCount++;
            }
        } else if (!path.basename(relPath).startsWith(".")) {
            const remoteStats = listResponse.items.find((item) => item.name === relPath);
            let fileOutdated = true;
            if (remoteStats != null) {
                const localMtime = fs.statSync(entry.path).mtime.getTime();
                const remoteMtime = new Date(remoteStats.mtime).getTime() - TIME_OFFSET;
                fileOutdated = remoteMtime - 100 < localMtime;
            }
            if (fileOutdated) {
                uploadTasks.push(
                    client.uss.writeFile({
                        fspath: remotePath,
                        data: B64String.encode(fs.readFileSync(entry.path)),
                        encoding: "IBM-1047",
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
                await client.uss.deleteFile({ fspath: item.name, recursive: false });
                deleteCount++;
            }
        }
    }
    await Promise.all(uploadTasks);
    console.log(`+ ${createCount}\t- ${deleteCount}\t* ${updateCount}`);
}

async function download(
    _client: ZSshClient,
    _srcDir: string,
    _destDir: string,
    _shouldDelete: boolean,
): Promise<void> {}

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
    console.time("sync");
    if (shouldUpload) {
        await upload(client, srcDir, destDir, argv.delete);
    } else {
        await download(client, srcDir, destDir, argv.delete);
    }
    console.timeEnd("sync");
}

async function main() {
    const argv = require("minimist")(process.argv.slice(2));
    await remoteSync(argv);
}

main().catch(console.error);
