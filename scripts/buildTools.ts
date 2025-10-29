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

import * as fs from "node:fs";
import * as path from "node:path";
import { PassThrough, pipeline, Transform, type TransformCallback } from "node:stream";
import { promisify } from "node:util";
import { DeferredPromise, DeferredPromiseStatus, type IProfile, ProfileInfo } from "@zowe/imperative";
import * as chokidar from "chokidar";
import * as yaml from "js-yaml";
import { Client, type ClientCallback, type SFTPWrapper } from "ssh2";

interface IConfig {
    sshProfile: string | IProfile;
    deployDir: string;
    preBuildCmd?: string;
}

type SftpError = Error & { code?: number };

const localDeployDir = "./../native";
const args = process.argv.slice(2);
let deployDirs: {
    root: string;
    cDir: string;
    asmchdrDir: string;
    cTestDir: string;
    pythonDir: string;
    pythonTestDir: string;
    zowedDir: string;
};

const asciiToEbcdicMap =
    // biome-ignore format: the array should not be formatted
    [
    /*       0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F */
    /* 0 */ 0x00, 0x01, 0x02, 0x03, 0x37, 0x2d, 0x2e, 0x2f, 0x16, 0x05, 0x15, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    /* 1 */ 0x10, 0x11, 0x12, 0x13, 0x3c, 0x3d, 0x32, 0x26, 0x18, 0x19, 0x3f, 0x27, 0x1c, 0x1d, 0x1e, 0x1f,
    /* 2 */ 0x40, 0x5a, 0x7f, 0x7b, 0x5b, 0x6c, 0x50, 0x7d, 0x4d, 0x5d, 0x5c, 0x4e, 0x6b, 0x60, 0x4b, 0x61,
    /* 3 */ 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0x7a, 0x5e, 0x4c, 0x7e, 0x6e, 0x6f,
    /* 4 */ 0x7c, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    /* 5 */ 0xd7, 0xd8, 0xd9, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xad, 0xe0, 0xbd, 0x5f, 0x6d,
    /* 6 */ 0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
    /* 7 */ 0x97, 0x98, 0x99, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xc0, 0x4f, 0xd0, 0xa1, 0x07,
    /* 8 */ 0x20, 0x01, 0x02, 0x03, 0x37, 0x2d, 0x2e, 0x2f, 0x16, 0x05, 0x25, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    /* 9 */ 0x10, 0x11, 0x12, 0x13, 0x3c, 0x3d, 0x32, 0x26, 0x18, 0x19, 0x3f, 0x27, 0x1c, 0x1d, 0x1e, 0x1f,
    /* A */ 0x40, 0x5a, 0x4a, 0xb1, 0x5b, 0xb2, 0x6a, 0x7d, 0x4d, 0x5d, 0x5c, 0x4e, 0x6b, 0x60, 0x4b, 0x61,
    /* B */ 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0x7a, 0x5e, 0x4c, 0x7e, 0x6e, 0x6f,
    /* C */ 0x7c, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    /* D */ 0xd7, 0xd8, 0xd9, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xad, 0xe0, 0xbd, 0x5f, 0x6d,
    /* E */ 0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
    /* F */ 0x97, 0x98, 0x99, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xc0, 0x4f, 0xd0, 0xa1, 0x07,
    ];

class AsciiToEbcdicTransform extends Transform {
    _transform(chunk: Buffer, _encoding: BufferEncoding, callback: TransformCallback) {
        const output = Buffer.allocUnsafe(chunk.length);
        for (let i = 0; i < chunk.length; i++) {
            output[i] = asciiToEbcdicMap[chunk[i]];
        }
        callback(null, output);
    }
}

class WatchUtils {
    public cache: Record<string, number>;
    private buildMutex: DeferredPromise<void>;
    private cacheDir = path.resolve(__dirname, "../.cache");
    private cacheFile: string;
    private readonly pendingChanges: Map<string, { kind: "+" | "~" | "-"; mtime: Date }> = new Map();
    private rootDir: string;
    private readonly spinnerFrames = ["-", "\\", "|", "/"];
    private watcher: chokidar.FSWatcher;

    constructor(
        private readonly connection: Client,
        sshProfile: IProfile,
    ) {
        this.cacheFile = path.resolve(this.cacheDir, `${sshProfile.user}_${sshProfile.host}.json`);
        this.rootDir = path.resolve(__dirname, localDeployDir);
        this.loadCache();
    }

    public loadCache() {
        if (fs.existsSync(this.cacheFile)) {
            this.cache = JSON.parse(fs.readFileSync(this.cacheFile, "utf8"));
        } else {
            this.cache = {};
        }
    }

    public saveCache() {
        fs.mkdirSync(this.cacheDir, { recursive: true });
        fs.writeFileSync(this.cacheFile, JSON.stringify(this.cache, null, 2));
    }

    public async start() {
        const changedFiles = getAllServerFiles().filter(
            (filePath) =>
                !path.basename(filePath).startsWith(".") &&
                fs.statSync(path.join(this.rootDir, filePath)).mtimeMs >
                    (this.cache[filePath.replaceAll(path.sep, path.posix.sep)] ?? 0),
        );
        for (const filePath of changedFiles) {
            const absLocalPath = path.resolve(__dirname, `${localDeployDir}/${filePath}`);
            const changeKind = this.cache[filePath.replaceAll(path.sep, path.posix.sep)] == null ? "+" : "~";
            this.pendingChanges.set(filePath, { kind: changeKind, mtime: fs.statSync(absLocalPath).mtime });
        }

        this.watcher = chokidar.watch(["**/*"], {
            cwd: this.rootDir,
            ignoreInitial: true,
            persistent: true,
        });
        this.connection.on("close", () => this.watcher.close());
        let debounceTimer: NodeJS.Timeout;
        const applyChangesDebounced = () => {
            if (debounceTimer) {
                clearTimeout(debounceTimer);
            }
            debounceTimer = setTimeout(() => void this.applyChanges(), 250);
        };
        this.watcher.on("add", (filePath, stats) => {
            this.pendingChanges.set(filePath, { kind: "+", mtime: stats?.mtime ?? new Date() });
            applyChangesDebounced();
        });
        this.watcher.on("change", (filePath, stats) => {
            this.pendingChanges.set(filePath, { kind: "~", mtime: stats?.mtime ?? new Date() });
            applyChangesDebounced();
        });
        this.watcher.on("unlink", (filePath) => {
            this.pendingChanges.set(filePath, { kind: "-", mtime: new Date() });
            applyChangesDebounced();
        });

        this.printReadyMessage();
        if (this.pendingChanges.size > 0) {
            void this.applyChanges();
        }
    }

    private async applyChanges() {
        if (this.buildMutex?.status === DeferredPromiseStatus.Pending) {
            await this.buildMutex.promise;
        }
        if (this.pendingChanges.size === 0) {
            return;
        }

        this.buildMutex = new DeferredPromise<void>();
        let sftp: SFTPWrapper;
        try {
            const toDelete: string[] = [];
            const toUpload: { localPath: string; remotePath: string }[] = [];
            const uniqueDirs = new Set<string>();
            for (const [filePath, { kind, mtime }] of this.pendingChanges.entries()) {
                const remotePath = `${deployDirs.root}/${filePath.replaceAll(path.sep, path.posix.sep)}`;
                if (kind === "-") {
                    console.log(`${mtime.toLocaleString()} [-] ${filePath}`);
                    toDelete.push(remotePath);
                } else {
                    console.log(`${mtime.toLocaleString()} [${kind}] ${filePath} -> ${remotePath}`);
                    toUpload.push({ localPath: filePath, remotePath });
                    // Check . and .. directories to ensure parent dirs get created
                    uniqueDirs.add(path.dirname(path.dirname(remotePath)));
                    uniqueDirs.add(path.dirname(remotePath));
                }
            }
            this.pendingChanges.clear();

            sftp = await promisify(this.connection.sftp.bind(this.connection))();
            await Promise.all(toDelete.map((remotePath) => this.deleteFile(sftp, remotePath)));
            for (const dirPath of uniqueDirs) {
                // Create directories sequentially since order may matter
                await this.createDir(sftp, dirPath);
            }
            await Promise.all(
                toUpload.map(({ localPath, remotePath }) => this.uploadFile(sftp, localPath, remotePath)),
            );
            this.saveCache();
            await this.executeBuild(...toUpload.map(({ localPath }) => localPath));
        } finally {
            sftp?.end();
            this.buildMutex.resolve();
            this.printReadyMessage();
        }
    }

    private printReadyMessage() {
        console.log(`${new Date().toLocaleString()} watching for changes...`);
    }

    private async deleteFile(sftp: SFTPWrapper, remotePath: string) {
        try {
            await promisify(sftp.unlink.bind(sftp))(remotePath);
        } catch (err) {
            if (err && (err as SftpError).code !== 2) {
                throw err; // Ignore if file does not exist
            }
        }
        delete this.cache[path.posix.relative(deployDirs.root, remotePath)];
    }

    private async createDir(sftp: SFTPWrapper, remotePath: string) {
        try {
            await promisify(sftp.mkdir.bind(sftp))(remotePath);
        } catch (err) {
            if (err && (err as SftpError).code !== 4) {
                throw err; // Ignore if directory already exists
            }
        }
    }

    private async uploadFile(sftp: SFTPWrapper, localPath: string, remotePath: string) {
        const absLocalPath = path.resolve(__dirname, `${localDeployDir}/${localPath}`);
        await uploadFile(sftp, absLocalPath, remotePath);
        this.cache[path.posix.relative(deployDirs.root, remotePath)] = fs.statSync(absLocalPath).mtimeMs;
    }

    private async executeBuild(...paths: string[]) {
        const cSourceChanged = paths.some((filePath) => filePath.split(path.sep)[0] === "c");
        const zowedSourceChanged = paths.some((filePath) => filePath.split(path.sep)[0] === "zowed");
        let result: number | string; // number for failing exit code or string for successful output
        if (cSourceChanged) {
            result = await this.makeTask();
        }
        if (zowedSourceChanged || (typeof result === "string" && result.length > 0)) {
            await this.makeTask(deployDirs.zowedDir);
        }
    }

    private makeTask(inDir?: string): Promise<number | string> {
        return new Promise<number | string>((resolve, reject) => {
            this.connection.shell(false, async (err, stream) => {
                if (err) {
                    reject(err);
                    return;
                }

                // Capture initial shell output like MOTD before sending commands
                stream.write("echo\n");
                await new Promise<void>((resolve) => stream.once("data", resolve));

                const cwd = inDir ?? deployDirs.cDir;
                const cmd = `cd ${cwd}\nmake\nexit $?\n`;
                stream.write(cmd);
                const prefix = `\t[tasks -> ${path.basename(cwd)}] make`;
                const spinner = this.startSpinner(prefix);

                let outText = "";
                let errText = "";
                stream
                    .on("exit", (code: number) => {
                        this.stopSpinner(spinner);
                        if (errText.length > 0) {
                            const status = code > 0 ? `failed (rc=${code}) ✘` : `succeeded with warnings !`;
                            console.log(`${prefix} ${status}\nerror: \n${errText}`);
                            resolve(code);
                        } else {
                            const status = outText.length > 0 ? "succeeded ✔" : "detected no changes —";
                            console.log(`${prefix} ${status}`);
                            resolve(outText);
                        }
                    })
                    .on("error", reject)
                    .stdout.on("data", (data: Buffer) => {
                        const str = data.toString().trim();
                        outText += str;
                    })
                    .stderr.on("data", (data: Buffer) => {
                        // Filter out INFO level messages and ones about compiler optimizations
                        const str = data.toString().trim();
                        if (/IGD\d{5}I /.test(str) || /WARNING CLC1145:/.test(str)) return;
                        errText += str;
                    });
            });
        });
    }

    private startSpinner(prefix: string) {
        let spinnerIndex = 0;
        return setInterval(() => {
            process.stdout.write(`\r${prefix} ${this.spinnerFrames[spinnerIndex]}`);
            spinnerIndex = (spinnerIndex + 1) % this.spinnerFrames.length;
        }, 100);
    }

    private stopSpinner(spinner: NodeJS.Timeout | null) {
        spinner && clearInterval(spinner);
        process.stdout.write("\r");
    }
}

function DEBUG_MODE() {
    return process.env.ZOWE_NATIVE_DEBUG?.toUpperCase() === "TRUE" || process.env.ZOWE_NATIVE_DEBUG === "1";
}

function startSpinner(text = "Loading...") {
    if (DEBUG_MODE() || process.env.CI != null) {
        console.log(text);
        return null;
    }
    console.log(text);
    let progressIndex = 0;

    const PROGRESS_BAR_FRAMES = ["▏", "▎", "▍", "▌", "▋", "▊", "▉", "▊", "▋", "▌", "▍", "▎"];
    return setInterval(() => {
        const progressBar = PROGRESS_BAR_FRAMES.map((_, i) => (i === progressIndex ? "█" : " ")).join("");
        process.stdout.write(`\rRunning... █${progressBar}`);
        progressIndex = (progressIndex + 1) % PROGRESS_BAR_FRAMES.length;
    }, 100);
}

function stopSpinner(spinner: NodeJS.Timeout | null, text = "Done!") {
    if (DEBUG_MODE() || process.env.CI != null) {
        return;
    }
    spinner && clearInterval(spinner);
    process.stdout.write(`\x1b[2K\r${text}\n`);
}

function getAllServerFiles() {
    const files: string[] = getServerFiles();

    // skip reading dirs if specific arguments were passed to upload
    if (args[1]) {
        return files;
    }

    const dirs = getDirs();

    for (const dir of dirs) {
        files.push(...getServerFiles(dir));
    }

    return files;
}

function getServerFiles(dir = "") {
    let argsFound = false;
    const fileList: string[] = [];

    if (args[1]) {
        argsFound = true;

        args.forEach((arg, index) => {
            if (0 === index) {
                // do nothing
            } else {
                let stats: fs.Stats;
                try {
                    stats = fs.statSync(path.resolve(__dirname, `${localDeployDir}/${arg}`));
                } catch {
                    console.log(`Error: input '${arg}' is not found`);
                    process.exit(1);
                }

                if (stats.isDirectory()) {
                    const files = fs.readdirSync(path.resolve(__dirname, `${localDeployDir}/${arg}`), {
                        withFileTypes: true,
                    });
                    for (const entry of files) {
                        if (!entry.isDirectory()) {
                            fileList.push(`${arg}/${entry.name}`);
                        }
                    }
                } else {
                    fileList.push(arg);
                }
            }
        });
    }

    if (argsFound) {
        return [...fileList];
    }

    const filesList: string[] = [];
    const files = fs.readdirSync(path.resolve(__dirname, `${localDeployDir}/${dir}`), {
        withFileTypes: true,
    });

    for (const file of files) {
        if (file.isDirectory()) {
        } else {
            filesList.push(`${dir}${file.name}`);
        }
    }
    return filesList;
}

function getDirs(next = "") {
    const dirs: string[] = [];

    const readDirs = fs.readdirSync(path.resolve(__dirname, `${localDeployDir}/${next}`), { withFileTypes: true });
    for (const dir of readDirs) {
        if (dir.isDirectory()) {
            const newDir = `${dir.name}/`;
            dirs.push(`${next}${newDir}`);
            dirs.push(...getDirs(`${next}${newDir}`));
        }
    }
    return dirs;
}

async function artifacts(connection: Client, packageAll: boolean) {
    const artifactPaths = ["zowed/build-out/libzowed.so", "zowed/build-out/zowed"];
    if (packageAll) {
        artifactPaths.push("c/build-out/zoweax", "c/build-out/zowex");
    }
    const artifactNames = artifactPaths.map((file) => path.basename(file)).sort();
    const localDirs = packageAll ? ["dist"] : ["packages/cli/bin", "packages/vsce/bin"];
    const localFiles = ["server.pax.Z", "checksums.asc"];
    const [paxFile, checksumFile] = localFiles;
    const prePaxCmds = artifactPaths.map(
        (file) => `cp ../${file} ${path.basename(file)} && chmod 700 ${path.basename(file)}`,
    );
    const postPaxCmd = `rm ${artifactNames.join(" ")} && rmdir ../bin`;
    const e2aPipe = (file: string) => `iconv -f IBM-1047 -t ISO8859-1 > ${file} && chtag -tc ISO8859-1 ${file}`;
    await runCommandInShell(
        connection,
        [
            `cd ${deployDirs.root} && mkdir -p bin dist && cd bin`,
            ...prePaxCmds,
            `_BPXK_AUTOCVT=OFF sha256 -r ${artifactNames.join(" ")} | ${e2aPipe(checksumFile)}`,
            `pax -wvz -o saveext -f ../dist/${paxFile} ${artifactNames.join(" ")} ${checksumFile}`,
            `mv ${checksumFile} ../dist/${checksumFile}`,
            postPaxCmd,
        ].join("\n"),
    );
    for (const localDir of localDirs) {
        fs.mkdirSync(path.resolve(__dirname, `./../${localDir}`), { recursive: true });
        for (const localFile of localFiles) {
            if (localDirs.indexOf(localDir) === 0) {
                await retrieve(connection, [`dist/${localFile}`], localDir, true);
            } else {
                fs.cpSync(
                    path.resolve(__dirname, `./../${localDirs[0]}/${localFile}`),
                    path.resolve(__dirname, `./../${localDir}/${localFile}`),
                );
            }
        }
    }
}

async function runCommandInShell(connection: Client, command: string) {
    const spinner = startSpinner(`Running: ${command.trim()}`);
    return new Promise<string>((resolve, reject) => {
        let data = "";
        let error = "";
        const cb: ClientCallback = (err, stream) => {
            if (err) {
                stopSpinner(spinner, `Error: runCommand connection.exec error ${err}`);
                reject(err);
            }
            stream.on("close", () => {
                stopSpinner(spinner);
                resolve(data);
            });
            stream.on("data", (part: Buffer) => {
                data += part.toString();
            });
            stream.stderr.on("data", (err: Buffer) => {
                error += err.toString();
                DEBUG_MODE() && console.log(error);
            });
            stream.on("exit", (exitCode: number) => {
                if (exitCode !== 0) {
                    const fullError = `\nError: runCommand connection.exec error - stream.on exit: \n ${error || data}`;
                    stopSpinner(spinner, fullError);
                    process.exitCode = exitCode;
                    reject(fullError);
                }
            });
            stream.end(`${command}\nexit $?\n`);
        };
        connection.shell(false, cb);
    });
}

async function retrieve(connection: Client, files: string[], targetDir: string, useBasename = false) {
    return new Promise<void>((finish) => {
        console.log("Retrieving files...");

        connection.sftp(async (err, sftpcon) => {
            if (err) {
                console.log("Retrieve err");
                throw err;
            }

            for (let i = 0; i < files.length; i++) {
                const absTargetDir = path.resolve(__dirname, `./../${targetDir}`);
                if (!fs.existsSync(`${absTargetDir}`)) fs.mkdirSync(`${absTargetDir}`);
                const to = `${absTargetDir}/${useBasename ? path.basename(files[i]) : files[i]}`;
                const from = `${deployDirs.root}/${files[i]}`;
                await download(sftpcon, from, to);
            }
            console.log("Get complete!");
            finish();
        });
    });
}

async function upload(connection: Client, sshProfile: IProfile) {
    return new Promise<void>((finish) => {
        const spinner = startSpinner("Deploying files...");

        const dirs = getDirs();
        const files = getAllServerFiles();

        connection.sftp(async (err, sftpcon) => {
            if (err) {
                stopSpinner(spinner, `Deploy error!\n${err}`);
                throw err;
            }

            const filteredDirs = args[1] ? dirs.filter((dir) => args.some((arg) => `${arg}/`.startsWith(dir))) : dirs;
            for (const dir of ["", ...filteredDirs]) {
                await new Promise<void>((resolve, reject) => {
                    sftpcon.mkdir(`${deployDirs.root}/${dir}`, (err) => {
                        if (err && (err as SftpError).code !== 4) {
                            reject(err); // Ignore if directory already exists
                        } else {
                            resolve();
                        }
                    });
                });
            }

            const pendingUploads = [];
            const watcher = new WatchUtils(connection, sshProfile);
            if (args[1] == null) {
                pendingUploads.push(
                    uploadFile(sftpcon, path.resolve(__dirname, "../package.json"), `${deployDirs.root}/package.json`),
                );
            }
            for (let i = 0; i < files.length; i++) {
                const from = path.resolve(__dirname, `${localDeployDir}/${files[i]}`);
                const to = `${deployDirs.root}/${files[i]}`;
                pendingUploads.push(uploadFile(sftpcon, from, to));
                watcher.cache[files[i].replaceAll(path.sep, path.posix.sep)] = fs.statSync(from).mtimeMs;
            }
            await Promise.all(pendingUploads);

            stopSpinner(spinner, "Deploy complete!");
            watcher.saveCache();
            finish();
        });
    });
}

async function build(connection: Client, { preBuildCmd }: IConfig) {
    preBuildCmd = preBuildCmd ? `${preBuildCmd} && ` : "";
    console.log("Building native/c ...");
    let response = await runCommandInShell(
        connection,
        `${preBuildCmd}cd ${deployDirs.cDir} && make ${DEBUG_MODE() ? "-DBuildType=DEBUG" : ""}\n`,
    );
    DEBUG_MODE() && console.log(response);
    console.log("Building native/zowed ...");
    response = await runCommandInShell(
        connection,
        `${preBuildCmd}cd ${deployDirs.zowedDir} && make ${DEBUG_MODE() ? "-DBuildType=DEBUG" : ""}\n`,
    );
    DEBUG_MODE() && console.log(response);
    console.log("Build complete!");
}

async function make(connection: Client, inDir?: string) {
    const pwd = inDir ?? deployDirs.cDir;
    const targets = args.filter((arg, idx) => idx > 0 && !arg.startsWith("--")).join(" ");
    console.log(`Running "make ${targets || "all"}"${inDir ? ` in ${pwd}` : ""}...`);
    const response = await runCommandInShell(
        connection,
        `cd ${pwd} && make ${targets} ${DEBUG_MODE() ? "-DBuildType=DEBUG" : ""}\n`,
    );
    console.log(response);
}

async function test(connection: Client) {
    console.log("Testing native/c ...");
    const response = await runCommandInShell(
        connection,
        `cd ${deployDirs.cTestDir} && _CEE_RUNOPTS="TRAP(ON,NOSPIE)" ./build-out/ztest_runner ${args[1] ?? ""} \n`,
    );
    console.log(response);
    console.log("Testing complete!");
}

async function chdsect(connection: Client) {
    return new Promise<void>((finish, reject) => {
        if (args[1] == null) {
            console.log("Usage: npm run z:chdsect <target_name>");
            console.log("  example: npm run z:chdsect cvt.s");
            reject(new Error("Usage: npm run z:chdsect <target_name>"));
        }

        connection.sftp(async (err, sftpcon) => {
            if (err) {
                console.log("Chdsect err");
                reject(err);
            }
            await uploadFile(
                sftpcon,
                path.resolve(__dirname, `${localDeployDir}/asmchdr/${args[1]}`),
                `${deployDirs.asmchdrDir}/${args[1]}`,
            );
            let response = "";
            try {
                response = await runCommandInShell(
                    connection,
                    `cd ${deployDirs.asmchdrDir} && make build-${args[1]} 2>&1 \n`,
                );
            } catch (err) {
                console.log("Chdsect err");
                reject(err);
            }
            console.log(response);
            console.log("Chdsect complete!");

            const from = `${deployDirs.asmchdrDir}/build-out/${args[1].replace(".s", ".h")}`;
            const to = path.resolve(__dirname, `${localDeployDir}/c/chdsect/${args[1].replace(".s", ".h")}`);
            console.log(`Downloading file from '${from}' to '${to}'`);

            await download(sftpcon, from, to);
            sftpcon.end();
            finish();
        });
    });

    // console.log("Running chdsect ...");
    // const response = await runCommandInShell(connection, `cd ${deployDirs.asmchdrDir} && make build-${args[1]}\n`);
    // console.log(response);
    // console.log("Chdsect complete!");
}

async function clean(connection: Client) {
    console.log("Cleaning native/c ...");
    console.log(await runCommandInShell(connection, `cd ${deployDirs.cDir} && make clean\n`));
    console.log("Cleaning native/c/test ...");
    console.log(await runCommandInShell(connection, `cd ${deployDirs.cTestDir} && make clean\n`));
    console.log("Cleaning native/python ...");
    console.log(await runCommandInShell(connection, `cd ${deployDirs.pythonDir} && make clean\n`));
    console.log("Cleaning native/python/test ...");
    console.log(await runCommandInShell(connection, `cd ${deployDirs.pythonTestDir} && make clean\n`));
    console.log("Clean complete");
}

async function rmdir(connection: Client, sshProfile: IProfile) {
    console.log("Removing ROOT directory ...");
    console.log(await runCommandInShell(connection, `rm -rf ${deployDirs.root}\n`));
    console.log("Removal complete");
    const watcher = new WatchUtils(connection, sshProfile);
    watcher.cache = {};
    watcher.saveCache();
}

async function watch(connection: Client, sshProfile: IProfile) {
    await new WatchUtils(connection, sshProfile).start();
    return new Promise<void>((resolve) => connection.on("close", () => resolve()));
}

async function uploadFile(sftpcon: SFTPWrapper, from: string, to: string, convertEbcdic = true) {
    await new Promise<void>((finish) => {
        DEBUG_MODE() && console.log(`Uploading '${from}' to ${to}`);
        pipeline(
            fs.createReadStream(from),
            convertEbcdic ? new AsciiToEbcdicTransform() : new PassThrough(),
            sftpcon.createWriteStream(to),
            (err) => {
                if (err) {
                    console.log("Upload err");
                    console.log(from, to);
                    throw err;
                }
                finish();
            },
        );
    });
}

async function download(sftpcon: SFTPWrapper, from: string, to: string) {
    return new Promise<void>((finish) => {
        console.log(`Downloading '${from}' to ${to}`);
        sftpcon.fastGet(from, to, (err) => {
            if (err) {
                console.log("Get err");
                console.log(from, to);
                throw err;
            }
            finish();
        });
    });
}

async function loadConfig(): Promise<IConfig> {
    const configPath = path.join(__dirname, "..", "config.yaml");
    if (!fs.existsSync(configPath)) {
        console.error("Could not find config.yaml. See the README for instructions.");
        process.exit(1);
    }

    // biome-ignore lint/suspicious/noExplicitAny: Config file is not strongly typed
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
        config.sshProfile = Object.fromEntries(mergedArgs.knownArgs.map((arg) => [arg.argName, arg.argValue]));
    }
    config.deployDir = config.deployDir.replace(/^~/, ".");
    return config;
}

async function buildSshClient(sshProfile: IProfile): Promise<Client> {
    const client = new Client();
    return new Promise((resolve, reject) => {
        client.on("close", () => {
            console.log("Client connection is closed");
        });
        client.on("error", (err) => {
            console.error("Client connection errored");
            process.exitCode = 1;
            reject(err);
        });
        client.on("ready", () => resolve(client));
        client.connect({
            ...sshProfile,
            username: sshProfile.user,
            privateKey: sshProfile.privateKey ? fs.readFileSync(sshProfile.privateKey) : undefined,
            keepaliveInterval: 30e3,
        });
    });
}

async function main() {
    const config = await loadConfig();
    deployDirs = {
        root: config.deployDir,
        cDir: `${config.deployDir}/c`,
        asmchdrDir: `${config.deployDir}/asmchdr`,
        cTestDir: `${config.deployDir}/c/test`,
        pythonDir: `${config.deployDir}/python/bindings`,
        pythonTestDir: `${config.deployDir}/python/bindings/test`,
        zowedDir: `${config.deployDir}/zowed`,
    };
    const sshClient = await buildSshClient(config.sshProfile as IProfile);

    try {
        switch (args[0]) {
            case "artifacts":
                await artifacts(sshClient, false);
                break;
            case "build":
                await build(sshClient, config);
                break;
            case "build:chdsect":
                await chdsect(sshClient);
                break;
            case "build:python":
                await make(sshClient, deployDirs.pythonDir);
                break;
            case "build:zowed":
                await make(sshClient, deployDirs.zowedDir);
                break;
            case "clean":
                await clean(sshClient);
                break;
            case "delete":
                await rmdir(sshClient, config.sshProfile as IProfile);
                break;
            case "make":
                await make(sshClient);
                break;
            case "package":
                await artifacts(sshClient, true);
                break;
            case "rebuild":
                await upload(sshClient, config.sshProfile as IProfile);
                await build(sshClient, config);
                break;
            case "test":
                await test(sshClient);
                break;
            case "test:python":
                await make(sshClient, deployDirs.pythonTestDir);
                break;
            case "upload":
                await upload(sshClient, config.sshProfile as IProfile);
                break;
            case "watch":
                await watch(sshClient, config.sshProfile as IProfile);
                break;
            default:
                console.error(`Unsupported command "${args[0]}". See README for instructions.`);
                break;
        }
    } finally {
        sshClient.end();
    }
}

main().catch((err) => {
    console.error(err);
    process.exit(process.exitCode ?? 1);
});
