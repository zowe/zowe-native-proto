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

/**
 * Converts a file path to use POSIX separators (forward slashes).
 * This ensures consistent cache keys across Windows and Unix systems.
 */
function toPosixPath(filePath: string): string {
    return filePath.split(path.sep).join("/");
}

/**
 * Compares two strings for sorting in alphabetical order.
 */
function localeCompare(a: string, b: string): number {
    return a.localeCompare(b);
}

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
    zowedTestDir: string;
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

// EBCDIC to ASCII conversion table (inverse of ASCII to EBCDIC)
// This table maps EBCDIC byte values to their ASCII equivalents
const ebcdicToAsciiMap =
    // biome-ignore format: the array should not be formatted
    [
    /*       0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F */
    /* 0 */ 0x00, 0x01, 0x02, 0x03, 0x00, 0x09, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    /* 1 */ 0x10, 0x11, 0x12, 0x13, 0x00, 0x0a, 0x08, 0x00, 0x18, 0x19, 0x00, 0x00, 0x1c, 0x1d, 0x1e, 0x1f,
    /* 2 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x17, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x06, 0x07,
    /* 3 */ 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x14, 0x15, 0x00, 0x1a,
    /* 4 */ 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa2, 0x2e, 0x3c, 0x28, 0x2b, 0x7c,
    /* 5 */ 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x24, 0x2a, 0x29, 0x3b, 0x5e,
    /* 6 */ 0x2d, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa6, 0x2c, 0x25, 0x5f, 0x3e, 0x3f,
    /* 7 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x3a, 0x23, 0x40, 0x27, 0x3d, 0x22,
    /* 8 */ 0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 9 */ 0x00, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* A */ 0x00, 0x7e, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* B */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* C */ 0x7b, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* D */ 0x7d, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* E */ 0x5c, 0x00, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* F */ 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    ];

class EbcdicToAsciiTransform extends Transform {
    _transform(chunk: Buffer, _encoding: BufferEncoding, callback: TransformCallback) {
        const output = Buffer.allocUnsafe(chunk.length);
        for (let i = 0; i < chunk.length; i++) {
            output[i] = ebcdicToAsciiMap[chunk[i]];
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
    private sftp: SFTPWrapper | null = null;
    private readonly ui: WatchUI = new WatchUI();
    private watcher: chokidar.FSWatcher;

    constructor(
        private readonly connection: Client,
        sshProfile: IProfile,
    ) {
        this.cacheFile = path.resolve(this.cacheDir, `${sshProfile.user}_${sshProfile.host}.json`);
        this.rootDir = path.resolve(__dirname, localDeployDir);
        this.loadCache();
    }

    private async getSftp(): Promise<SFTPWrapper> {
        if (this.sftp == null) {
            this.sftp = await promisify(this.connection.sftp.bind(this.connection))();
            this.sftp.on("close", () => {
                this.sftp = null;
            });
        }
        return this.sftp;
    }

    private closeSftp() {
        if (this.sftp != null) {
            this.sftp.end();
            this.sftp = null;
        }
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
                fs.statSync(path.join(this.rootDir, filePath)).mtimeMs > (this.cache[toPosixPath(filePath)] ?? 0),
        );
        for (const filePath of changedFiles) {
            const absLocalPath = path.resolve(__dirname, `${localDeployDir}/${filePath}`);
            const changeKind = this.cache[toPosixPath(filePath)] == null ? "+" : "~";
            this.pendingChanges.set(filePath, { kind: changeKind, mtime: fs.statSync(absLocalPath).mtime });
        }

        this.watcher = chokidar.watch(["**/*"], {
            cwd: this.rootDir,
            ignoreInitial: true,
            persistent: true,
        });
        let debounceTimer: NodeJS.Timeout;
        this.connection.on("close", () => {
            if (debounceTimer) {
                clearTimeout(debounceTimer);
            }
            this.closeSftp();
            this.watcher.close();
        });
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

        if (this.pendingChanges.size > 0) {
            void this.applyChanges();
        } else {
            // Show initial "watching" message when no changes pending
            console.log(`${new Date().toLocaleString()} watching for changes...`);
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
        try {
            // Update UI with pending file changes
            this.ui.setFiles(this.pendingChanges);
            this.ui.setFooter("syncing files...");
            this.ui.render();

            const toDelete: string[] = [];
            const toUpload: { localPath: string; remotePath: string }[] = [];
            const uniqueDirs = new Set<string>();
            for (const [filePath, { kind }] of this.pendingChanges.entries()) {
                const remotePath = `${deployDirs.root}/${toPosixPath(filePath)}`;
                if (kind === "-") {
                    toDelete.push(remotePath);
                } else {
                    toUpload.push({ localPath: filePath, remotePath });
                    // Ensure all parent directories exist on the remote side
                    this.collectParentDirs(remotePath, uniqueDirs);
                }
            }
            this.pendingChanges.clear();

            const sftp = await this.getSftp();
            await Promise.all(toDelete.map((remotePath) => this.deleteFile(sftp, remotePath)));
            const orderedDirs = Array.from(uniqueDirs).sort((left, right) => {
                return left.split("/").length - right.split("/").length;
            });
            for (const dirPath of orderedDirs) {
                // Create directories sequentially since order may matter
                await this.createDir(sftp, dirPath);
            }
            await Promise.all(
                toUpload.map(({ localPath, remotePath }) => this.uploadFile(sftp, localPath, remotePath)),
            );
            this.saveCache();
            await this.executeBuild(...toUpload.map(({ localPath }) => localPath));
        } finally {
            this.buildMutex.resolve();
            this.printReadyMessage();
        }
    }

    private printReadyMessage() {
        this.ui.setFooter("watching for changes...");
        this.ui.render();
        // Reset UI state for next change batch (keeps last render visible)
        this.ui.reset();
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

    private collectParentDirs(remotePath: string, uniqueDirs: Set<string>) {
        let dirPath = path.posix.dirname(remotePath);
        while (dirPath && dirPath !== "." && dirPath !== "/" && dirPath !== deployDirs.root) {
            uniqueDirs.add(dirPath);
            const nextDir = path.posix.dirname(dirPath);
            if (nextDir === dirPath) {
                break;
            }
            dirPath = nextDir;
        }
        if (deployDirs.root && deployDirs.root !== "/" && deployDirs.root !== ".") {
            uniqueDirs.add(deployDirs.root);
        }
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
        const cSourceChanged = paths.some((filePath) => toPosixPath(filePath).split("/")[0] === "c");
        const zowedSourceChanged = paths.some((filePath) => toPosixPath(filePath).split("/")[0] === "zowed");

        // Determine which tasks need to run
        const tasksToRun = [
            ...(cSourceChanged ? ["c"] : []),
            ...(zowedSourceChanged || cSourceChanged ? ["zowed"] : []),
        ];

        // Add tasks to UI and start spinner
        for (const task of tasksToRun) {
            this.ui.addTask(task);
        }
        this.ui.setFooter("building...");
        this.ui.startSpinner();

        let result: number | string; // number for failing exit code or string for successful output
        if (cSourceChanged) {
            result = await this.makeTask("c");
        }
        if (zowedSourceChanged || typeof result === "string" || result === 0) {
            await this.makeTask("zowed", deployDirs.zowedDir);
        }

        this.ui.stopSpinner();
    }

    private makeTask(taskName: string, inDir?: string): Promise<number | string> {
        this.ui.updateTask(taskName, "running");
        this.ui.render();

        return new Promise<number | string>((resolve, reject) => {
            this.connection.shell(false, async (err, stream) => {
                if (err) {
                    this.ui.updateTask(taskName, "error", err.message);
                    this.ui.render();
                    reject(err);
                    return;
                }

                // Capture initial shell output like MOTD before sending commands
                stream.write("echo\n");
                await new Promise<void>((resolve) => stream.once("data", resolve));

                const cwd = inDir ?? deployDirs.cDir;
                const cmd = `cd ${cwd}\nmake\nexit $?\n`;
                stream.write(cmd);

                let outText = "";
                let errText = "";
                stream
                    .on("exit", (code: number) => {
                        if (errText.length > 0) {
                            const status: TaskStatus = code > 0 ? "error" : "warning";
                            this.ui.updateTask(taskName, status, errText);
                            this.ui.render();
                            resolve(code);
                        } else {
                            const status: TaskStatus = outText.length > 0 ? "success" : "no_changes";
                            this.ui.updateTask(taskName, status);
                            this.ui.render();
                            resolve(outText);
                        }
                    })
                    .on("error", (err: Error) => {
                        this.ui.updateTask(taskName, "error", err.message);
                        this.ui.render();
                        reject(err);
                    })
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
}

function DEBUG_MODE() {
    return process.env.ZOWE_NATIVE_DEBUG?.toUpperCase() === "TRUE" || process.env.ZOWE_NATIVE_DEBUG === "1";
}

// ANSI escape codes for terminal formatting
const ANSI = {
    HIDE_CURSOR: "\x1b[?25l",
    SHOW_CURSOR: "\x1b[?25h",
    CLEAR_LINE: "\x1b[2K",
    MOVE_UP: (n: number) => `\x1b[${n}A`,
    GREEN: "\x1b[32m",
    RED: "\x1b[31m",
    YELLOW: "\x1b[33m",
    DIM: "\x1b[2m",
    RESET: "\x1b[0m",
};

// Box drawing characters (rounded)
const BOX = {
    TOP_LEFT: "╭",
    TOP_RIGHT: "╮",
    BOTTOM_LEFT: "╰",
    BOTTOM_RIGHT: "╯",
    HORIZONTAL: "─",
    VERTICAL: "│",
    LEFT_T: "├",
    RIGHT_T: "┤",
    DASHED: "╌",
};

const BRAILLE_FRAMES = ["⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"];

type FileChangeKind = "+" | "~" | "-";
type TaskStatus = "pending" | "running" | "success" | "warning" | "error" | "no_changes";

interface TaskState {
    name: string;
    status: TaskStatus;
    error?: string;
    errorLogPath?: string;
}

class WatchUI {
    private readonly files: Map<string, FileChangeKind> = new Map();
    private readonly tasks: Map<string, TaskState> = new Map();
    private timestamp: Date = new Date();
    private lineCount = 0;
    private spinnerInterval: NodeJS.Timeout | null = null;
    private spinnerFrame = 0;
    private footerMessage = "watching for changes...";
    private lastFallbackState = "";
    private readonly printedFallbackItems: Set<string> = new Set();

    private isInteractive(): boolean {
        return process.stdout.isTTY === true && !DEBUG_MODE() && process.env.CI == null;
    }

    private get boxWidth(): number {
        return process.stdout.columns || 80;
    }

    setFiles(files: Map<string, { kind: FileChangeKind; mtime: Date }>) {
        this.files.clear();
        this.timestamp = new Date();
        for (const [filePath, { kind }] of files.entries()) {
            this.files.set(filePath, kind);
        }
    }

    addTask(name: string) {
        this.tasks.set(name, { name, status: "pending" });
    }

    updateTask(name: string, status: TaskStatus, error?: string) {
        const task = this.tasks.get(name);
        if (task) {
            task.status = status;
            task.error = error;
            // Write full error to log file if present
            if (error) {
                const errorLogDir = path.resolve(__dirname, "../.cache/build-errors");
                fs.mkdirSync(errorLogDir, { recursive: true });
                const logPath = path.join(errorLogDir, `${name}.log`);
                const logContent = `Build error log for [${name}]\nTimestamp: ${this.timestamp.toLocaleString()}\n${"─".repeat(50)}\n\n${error}`;
                fs.writeFileSync(logPath, logContent);
                task.errorLogPath = `.cache/build-errors/${name}.log`;
            }
        }
    }

    setFooter(message: string) {
        this.footerMessage = message;
    }

    private getStatusIcon(status: TaskStatus): string {
        switch (status) {
            case "running":
                return BRAILLE_FRAMES[this.spinnerFrame];
            case "success":
                return `${ANSI.GREEN}✔${ANSI.RESET}`;
            case "warning":
                return `${ANSI.YELLOW}!${ANSI.RESET}`;
            case "error":
                return `${ANSI.RED}✘${ANSI.RESET}`;
            case "no_changes":
                return `${ANSI.DIM}—${ANSI.RESET}`;
            default:
                return " ";
        }
    }

    private getFileIcon(kind: FileChangeKind): string {
        switch (kind) {
            case "+":
                return `${ANSI.GREEN}+${ANSI.RESET}`;
            case "-":
                return `${ANSI.RED}-${ANSI.RESET}`;
            case "~":
                return `${ANSI.YELLOW}~${ANSI.RESET}`;
        }
    }

    private horizontalLine(left: string, right: string, char = BOX.HORIZONTAL): string {
        return `${left}${char.repeat(this.boxWidth - 2)}${right}`;
    }

    private dashedLine(): string {
        return `${BOX.LEFT_T}${BOX.DASHED.repeat(this.boxWidth - 2)}${BOX.RIGHT_T}`;
    }

    private padLine(content: string, rawLength: number): string {
        const padding = this.boxWidth - 4 - rawLength;
        return `${BOX.VERTICAL} ${content}${" ".repeat(Math.max(0, padding))} ${BOX.VERTICAL}`;
    }

    private buildLines(): string[] {
        const timeStr = this.timestamp.toLocaleString();

        // Top border with timestamp and separator
        const lines: string[] = [
            this.horizontalLine(BOX.TOP_LEFT, BOX.TOP_RIGHT),
            this.padLine(timeStr, timeStr.length),
            this.horizontalLine(BOX.LEFT_T, BOX.RIGHT_T),
        ];

        // File changes
        for (const [filePath, kind] of this.files.entries()) {
            const icon = this.getFileIcon(kind);
            const display = `${icon} ${filePath}`;
            lines.push(this.padLine(display, filePath.length + 2));
        }

        // Dashed separator before tasks
        if (this.tasks.size > 0) {
            lines.push(this.dashedLine(), this.padLine("tasks:", 6));

            for (const [, task] of this.tasks.entries()) {
                const icon = this.getStatusIcon(task.status);
                const taskLine = `  [${task.name}] make ${icon}`;
                const rawLen = `  [${task.name}] make X`.length;
                lines.push(this.padLine(taskLine, rawLen));

                // Show error inline if present
                if (task.error) {
                    const errorHeader = `${ANSI.RED}error (${task.name}):${ANSI.RESET}`;
                    lines.push(this.padLine("", 0), this.padLine(`  ${errorHeader}`, `  error (${task.name}):`.length));

                    // Split error into lines and show first few
                    const allErrorLines = task.error.split("\n").filter((line) => line.trim().length > 0);
                    const maxLines = 10;
                    const visibleLines = allErrorLines.slice(0, maxLines);
                    const remainingCount = allErrorLines.length - maxLines;

                    for (const errLine of visibleLines) {
                        const truncated =
                            errLine.length > this.boxWidth - 10
                                ? `${errLine.slice(0, this.boxWidth - 13)}...`
                                : errLine;
                        lines.push(this.padLine(`    ${ANSI.DIM}${truncated}${ANSI.RESET}`, truncated.length + 4));
                    }

                    // Show "N more lines" indicator if truncated
                    if (remainingCount > 0) {
                        const moreText = `↓ ${remainingCount} more line${remainingCount > 1 ? "s" : ""}`;
                        const padding = this.boxWidth - 6 - moreText.length;
                        lines.push(
                            this.padLine(
                                `${" ".repeat(padding)}${ANSI.DIM}${moreText}${ANSI.RESET}`,
                                this.boxWidth - 6,
                            ),
                        );
                    }

                    // Show log file path
                    if (task.errorLogPath) {
                        const logText = `full log: ${task.errorLogPath}`;
                        lines.push(
                            this.padLine("", 0),
                            this.padLine(`  ${ANSI.DIM}${logText}${ANSI.RESET}`, logText.length + 2),
                        );
                    }
                }
            }
        }

        // Footer separator, message, and bottom border
        lines.push(
            this.horizontalLine(BOX.LEFT_T, BOX.RIGHT_T),
            this.padLine(this.footerMessage, this.footerMessage.length),
            this.horizontalLine(BOX.BOTTOM_LEFT, BOX.BOTTOM_RIGHT),
        );

        return lines;
    }

    clear() {
        if (!this.isInteractive()) return;

        if (this.lineCount > 0) {
            // Move cursor up and clear each line
            process.stdout.write(ANSI.MOVE_UP(this.lineCount));
            for (let i = 0; i < this.lineCount; i++) {
                process.stdout.write(`${ANSI.CLEAR_LINE}\n`);
            }
            process.stdout.write(ANSI.MOVE_UP(this.lineCount));
        }
    }

    render() {
        if (!this.isInteractive()) {
            // Fallback: simple console output
            this.renderFallback();
            return;
        }

        this.clear();
        const lines = this.buildLines();
        process.stdout.write(lines.join("\n") + "\n");
        this.lineCount = lines.length;
    }

    private renderFallback() {
        const fileKey = Array.from(this.files.keys()).sort(localeCompare).join(",");
        const filesChanged = fileKey !== this.lastFallbackState;

        // Print file changes header once
        if (filesChanged && this.files.size > 0) {
            console.log(`\n${this.timestamp.toLocaleString()}`);
            for (const [filePath, kind] of this.files.entries()) {
                console.log(`[${kind}] ${filePath}`);
            }
            this.lastFallbackState = fileKey;
        }

        for (const [, task] of this.tasks.entries()) {
            const runningKey = `task:${task.name}:running`;
            const doneKey = `task:${task.name}:done`;

            // Print "running" once when task starts
            if (task.status === "running" && !this.printedFallbackItems.has(runningKey)) {
                console.log(`  [${task.name}] make running`);
                this.printedFallbackItems.add(runningKey);
            }

            // Print final result once when task completes
            if (!this.printedFallbackItems.has(doneKey)) {
                if (
                    task.status === "success" ||
                    task.status === "error" ||
                    task.status === "warning" ||
                    task.status === "no_changes"
                ) {
                    const statusText =
                        task.status === "success"
                            ? "✔"
                            : task.status === "error"
                              ? "✘"
                              : task.status === "warning"
                                ? "!"
                                : "—";
                    console.log(`  [${task.name}] make ${statusText}`);
                    if (task.error) {
                        console.log(`    error: ${task.error}`);
                    }
                    this.printedFallbackItems.add(doneKey);
                }
            }
        }

        // Print footer only when transitioning to "watching" state
        if (this.footerMessage === "watching for changes..." && !this.printedFallbackItems.has("footer:watching")) {
            console.log(this.footerMessage);
            this.printedFallbackItems.add("footer:watching");
        }
    }

    startSpinner() {
        if (!this.isInteractive()) return;

        process.stdout.write(ANSI.HIDE_CURSOR);
        this.spinnerInterval = setInterval(() => {
            this.spinnerFrame = (this.spinnerFrame + 1) % BRAILLE_FRAMES.length;
            this.render();
        }, 80);
    }

    stopSpinner() {
        if (this.spinnerInterval) {
            clearInterval(this.spinnerInterval);
            this.spinnerInterval = null;
        }
        if (this.isInteractive()) {
            process.stdout.write(ANSI.SHOW_CURSOR);
        }
    }

    reset() {
        this.stopSpinner();
        this.files.clear();
        this.tasks.clear();
        this.lineCount = 0;
        this.lastFallbackState = "";
        this.printedFallbackItems.clear();
    }
}

function startSpinner(text = "Loading...") {
    if (DEBUG_MODE() || process.env.CI != null) {
        console.log(text);
        return null;
    }

    const BRAILLE_FRAMES = ["⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"];
    let frameIndex = 0;

    // Truncate text to fit terminal width (leave room for spinner and padding)
    const maxWidth = (process.stdout.columns || 80) - 4;
    const displayText = text.length > maxWidth ? `${text.slice(0, maxWidth - 3)}...` : text;

    // Hide cursor
    process.stdout.write("\x1b[?25l");
    return setInterval(() => {
        process.stdout.write(`\r${BRAILLE_FRAMES[frameIndex]} ${displayText}`);
        frameIndex = (frameIndex + 1) % BRAILLE_FRAMES.length;
    }, 80);
}

function stopSpinner(spinner: NodeJS.Timeout | null, text = "Done!") {
    if (DEBUG_MODE() || process.env.CI != null) {
        return;
    }
    spinner && clearInterval(spinner);
    // Restore cursor
    process.stdout.write(`\x1b[?25h`);
    process.stdout.write(`\x1b[2K\r${text}\n`);
}

function getAllServerFiles() {
    const files: string[] = getServerFiles();

    // skip reading dirs if specific arguments were passed to upload
    if (args[1]) {
        return files;
    }

    const dirs = getDirs();
    const dirFiles = dirs.map((dir) => getServerFiles(dir));
    files.push(...dirFiles.flat());

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
            dirs.push(`${next}${newDir}`, ...getDirs(`${next}${newDir}`));
        }
    }
    return dirs;
}

async function artifacts(connection: Client, packageAll: boolean) {
    const artifactPaths = ["zowed/build-out/libzowed.so", "zowed/build-out/zowed"];
    if (packageAll) {
        artifactPaths.push("c/build-out/zoweax", "c/build-out/zowex");
    }
    const artifactNames = artifactPaths.map((file) => path.basename(file)).sort(localeCompare);
    const localDir = packageAll ? "dist" : "packages/sdk/bin";
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
    fs.mkdirSync(path.resolve(__dirname, `./../${localDir}`), { recursive: true });
    for (const localFile of localFiles) {
        await retrieve(connection, [`dist/${localFile}`], localDir, true);
    }
}

async function runCommandInShell(connection: Client, command: string, streamOutput = false) {
    const spinner = streamOutput ? null : startSpinner(`$ ${command.trim()}`);
    return new Promise<string>((resolve, reject) => {
        let data = "";
        let error = "";
        const cb: ClientCallback = (err, stream) => {
            if (err) {
                if (spinner) stopSpinner(spinner, `Error: runCommand connection.exec error ${err}`);
                reject(err);
            }
            stream.on("close", () => {
                if (spinner) stopSpinner(spinner);
                resolve(data);
            });
            stream.on("data", (part: Buffer) => {
                const output = part.toString();
                data += output;
                if (streamOutput) {
                    process.stdout.write(output);
                }
            });
            stream.stderr.on("data", (err: Buffer) => {
                const errOutput = err.toString();
                error += errOutput;
                if (streamOutput || DEBUG_MODE()) {
                    process.stderr.write(errOutput);
                }
            });
            stream.on("exit", (exitCode: number) => {
                if (exitCode !== 0) {
                    const fullError = `\nError: runCommand connection.exec error - stream.on exit: \n ${error || data}`;
                    if (spinner) stopSpinner(spinner, fullError);
                    process.exitCode = exitCode;
                    reject(fullError);
                }
            });
            stream.end(`${command}\nexit $?\n`);
        };
        connection.shell(false, cb);
    });
}

async function retrieve(
    connection: Client,
    files: string[],
    targetDir: string,
    useBasename = false,
    convertAscii = false,
) {
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
                await download(sftpcon, from, to, convertAscii);
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
                const to = `${deployDirs.root}/${toPosixPath(files[i])}`;
                pendingUploads.push(uploadFile(sftpcon, from, to));
                watcher.cache[toPosixPath(files[i])] = fs.statSync(from).mtimeMs;
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
    const testEnv = '_CEE_RUNOPTS="TRAP(ON,NOSPIE)"';
    const cTestCmd = `cd ${deployDirs.cTestDir} && ${testEnv} ./build-out/ztest_runner ${args[1] ?? ""}`;
    const zowedTestCmd = `cd ${path.posix.relative(deployDirs.cTestDir, deployDirs.zowedTestDir)} && ${testEnv} ./build-out/zowed_test_runner ${args[1] ?? ""}`;
    const exitMaxRc = `[ "$rc1" -gt "$rc2" ] && exit "$rc1" || exit "$rc2"`;
    await runCommandInShell(connection, `${cTestCmd}; rc1=$?; ${zowedTestCmd}; rc2=$?; ${exitMaxRc}\n`, true);
    console.log("\nTesting complete!");
    await retrieve(connection, [`c/test/test-results.xml`, `zowed/test/test-results.xml`], "native", false, true);
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

async function download(sftpcon: SFTPWrapper, from: string, to: string, convertAscii = false) {
    return new Promise<void>((finish) => {
        console.log(`Downloading '${from}' to ${to}`);
        const callback = (err: Error | null) => {
            if (err) {
                console.log("Get err");
                console.log(from, to);
                throw err;
            }
            finish();
        };
        if (convertAscii) {
            pipeline(sftpcon.createReadStream(from), new EbcdicToAsciiTransform(), fs.createWriteStream(to), callback);
        } else {
            sftpcon.fastGet(from, to, callback);
        }
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

async function testConnection(client: Client): Promise<void> {
    return new Promise((resolve, reject) => {
        client.exec('echo "SSH connection established successfully"', (err, stream) => {
            if (err) {
                return reject(err);
            }
            stream
                .on("close", (code: number) => {
                    if (code === 0) {
                        resolve();
                    } else {
                        reject(new Error(`SSH test failed with exit code ${code}`));
                    }
                })
                .on("data", () => {})
                .stderr.on("data", (data: Buffer) => {
                    console.error(`STDERR: ${data}`);
                });
        });
    });
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
        zowedTestDir: `${config.deployDir}/zowed/test`,
    };
    const sshClient = await buildSshClient(config.sshProfile as IProfile);
    await testConnection(sshClient);
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
