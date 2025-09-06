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

import { type IProfile, ProfileInfo } from "@zowe/imperative";
import * as fs from "node:fs";
import * as path from "node:path";
import * as yaml from "js-yaml";
import { B64String, ZSshClient, ZSshUtils } from "zowe-native-proto-sdk";
import { Client } from "ssh2";
import * as fsWalk from "@nodelib/fs.walk";

const args = process.argv.slice(2);
const srcDir = path.resolve(__dirname, "../native");

interface IConfig {
    sshProfile: string | IProfile;
    deployDir: string;
    goBuildEnv?: string;
    preBuildCmd?: string;
}

async function loadConfig(): Promise<IConfig> {
    const configPath = path.join(__dirname, "..", "config.yaml");
    if (!fs.existsSync(configPath)) {
        console.error("Could not find config.yaml. See the README for instructions.");
        process.exit(1);
    }

    const configYaml: any = yaml.load(fs.readFileSync(configPath, "utf-8"));
    let activeProfile = configYaml.activeProfile;
    const profileArgIdx = args.findIndex((arg) => arg.startsWith("--profile="));
    if (profileArgIdx !== -1) {
        activeProfile = args.splice(profileArgIdx, 1)[0].split("=").pop();
    }
    if (configYaml.profiles?.[activeProfile] == null) {
        console.error(`Could not find profile "${activeProfile}" in config.yaml. See the README for instructions.`);
        process.exit(1);
    }

    const config: IConfig = configYaml.profiles[activeProfile];
    if (typeof config.sshProfile === "string") {
        const profInfo = new ProfileInfo("zowe");
        await profInfo.readProfilesFromDisk();
        const profAttrs = profInfo.getAllProfiles("ssh").find((prof) => prof.profName === config.sshProfile);
        const mergedArgs = profInfo.mergeArgsForProfile(profAttrs, { getSecureVals: true });
        config.sshProfile = mergedArgs.knownArgs.reduce((prof, arg) => ({ ...prof, [arg.argName]: arg.argValue }), {});
    }
    config.deployDir = config.deployDir.replace(/^~/, ".");
    return config;
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

async function remoteSync(config: IConfig): Promise<void> {
    // TODO respect gitignore to skip files like .DS_Store
    // TODO use new SSH client method once available to run commands
    // TODO add progress bar
    // TODO create root dir if it doesn't exist
    const sshSession = ZSshUtils.buildSession(config.sshProfile as IProfile);
    using client = await ZSshClient.create(sshSession, {
        serverPath: (config.sshProfile as IProfile).serverPath,
        numWorkers: 10,
    });
    const timeOffset = await getTimeOffset((client as any).mSshClient as Client, sshSession.ISshSession.hostname);
    console.time("sync");
    const listResponse = await client.uss.listFiles({ fspath: config.deployDir, depth: 10, long: true });
    const uploadTasks = [];
    for (const entry of fsWalk.walkSync(srcDir)) {
        const relPath = path.relative(srcDir, entry.path);
        const remotePath = path.join(config.deployDir, relPath);
        if (entry.dirent.isDirectory()) {
            const dirExists = listResponse.items.some((item) => item.name === relPath);
            if (!dirExists) {
                console.log(`Creating ${relPath}`);
                await client.uss.createFile({ fspath: remotePath, isDir: true });
            }
        } else if (!relPath.includes(".DS_Store")) {
            const remoteStats = listResponse.items.find((item) => item.name === relPath);
            let fileOutdated = true;
            if (remoteStats != null) {
                const localMtime = fs.statSync(entry.path).mtime.getTime();
                const remoteMtime = new Date(remoteStats.mtime).getTime() - timeOffset;
                fileOutdated = remoteMtime - 100 < localMtime;
            }
            if (fileOutdated) {
                console.log(`Uploading ${relPath}`);
                uploadTasks.push(
                    client.uss.writeFile({
                        fspath: remotePath,
                        data: B64String.encode(fs.readFileSync(entry.path)),
                        encoding: "IBM-1047",
                    }),
                );
            }
        }
    }
    await Promise.all(uploadTasks);
    console.timeEnd("sync");
}

async function main() {
    const config = await loadConfig();
    await remoteSync(config);
}

main().catch(console.error);
