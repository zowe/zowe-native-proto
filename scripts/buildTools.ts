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
import { basename, resolve } from "node:path";
import * as readline from "node:readline/promises";
import { Client, type ClientCallback, type SFTPWrapper } from "ssh2";

let config: Record<string, any>;

try {
    console.log(resolve(__dirname, "./../config.local.json"));
    config = JSON.parse(fs.readFileSync(resolve(__dirname, "./../config.local.json")).toString());
} catch (e) {
    console.log("You must create config.local.json (model from config.default.jsonc) in same directory");
}

const host = config.host;
const port = config.port ?? 22;
const username = config.username;
let privateKey: Buffer | undefined;
const password = config.password;
try {
    privateKey = fs.readFileSync(config.privateKey);
} catch (e) {}

const localDeployDir = "./../native"; // from here
const deployDirectory = config.deployDirectory; // to here
const cDeployDirectory = `${config.deployDirectory}/c`; // to here
const goDeployDirectory = `${config.deployDirectory}/golang`; // to here

const args = process.argv.slice(2);

const line = readline.createInterface({
    input: process.stdin,
    output: process.stdout,
});

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
    spinner && clearInterval(spinner);
    process.stdout.write(`\x1b[2K\r${text}\n`);
}

const connection = new Client();

connection.on("ready", async () => {
    try {
        switch (args[0]) {
            case "init":
                await init(connection);
                break;
            case "deploy":
                await deploy(connection);
                await convert(connection);
                break;
            case "deploy:build":
            case "deploy-build":
                await deploy(connection);
                await convert(connection);
                await build(connection);
                break;
            case "get-listings":
                await getListings(connection);
                break;
            case "get-dumps":
                await getDumps(connection);
                break;
            case "artifacts":
                await artifacts(connection);
                break;
            case "clean":
                await clean(connection);
                break;
            case "delete":
                await rmdir(connection);
                break;
            case "bin":
                await bin(connection);
                break;
            case "build":
                await build(connection);
                break;
            default:
                console.log("Unsupported command\nUsage init|deploy|deploy-build [<file1>,<file2>,...|dir]");
                break;
        }
    } catch (err) {
        console.error(err);
        process.exit(process.exitCode ?? 1);
    }

    line.close();
    connection.end();
});

connection.on("close", () => {
    console.log("Client connection is closed");
});

connection.on("error", (err) => {
    console.error("Client connection errored");
    console.log(err);
    process.exit(1);
});

if (!privateKey) {
    connection.connect({
        host,
        port,
        username,
        password,
    });
} else {
    connection.connect({
        host,
        port,
        username,
        privateKey,
    });
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

    return files.filter((file) => !file.startsWith("golang/zowed"));
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
                    stats = fs.statSync(resolve(__dirname, `${localDeployDir}/${arg}`));
                } catch (e) {
                    console.log(`Error: input '${arg}' is not found`);
                    process.exit(1);
                }

                if (stats.isDirectory()) {
                    const files = fs.readdirSync(resolve(__dirname, `${localDeployDir}/${arg}`), {
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
    const files = fs.readdirSync(resolve(__dirname, `${localDeployDir}/${dir}`), {
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

    const readDirs = fs.readdirSync(resolve(__dirname, `${localDeployDir}/${next}`), { withFileTypes: true });
    for (const dir of readDirs) {
        if (dir.isDirectory()) {
            const newDir = `${dir.name}/`;
            dirs.push(`${next}${newDir}`);
            dirs.push(...getDirs(`${next}${newDir}`));
        }
    }
    return dirs;
}

async function init(connection: Client) {
    const dirs = getDirs();

    return new Promise<void>((finish) => {
        console.log("Making directories...");
        const dirs = getDirs();

        connection.shell(false, (err, stream) => {
            if (err) {
                console.log(`Error: runCommand connection.exec error ${err}`);
                throw err;
            }

            stream.write(`mkdir -p ${deployDirectory}\n`);
            stream.write(`cd ${deployDirectory}\n`);
            for (let i = 0; i < dirs.length; i++) {
                console.log(`Creating ${dirs[i]}...`);
                stream.write(`mkdir -p ${dirs[i]}\n`);
            }
            stream.end("exit\n");

            stream.on("close", () => {
                console.log("Directories created!");
                finish();
            });
            stream.on("data", (part: Buffer) => {
                console.log(part.toString());
            });
            stream.stderr.on("data", (data: Buffer) => {
                console.log(data.toString());
            });
        });
    });
}

async function getListings(connection: Client) {
    if (args[1]) {
        await convert(connection, "IBM-1047", "utf8", [...args.slice(1)]);
        await retrieve(connection, [...args.slice(1)], "listings");
        return;
    }

    const resp = (await runCommandInShell(connection, `cd ${cDeployDirectory}\nls *.lst`)).trim().split("\n");

    await convert(connection, "IBM-1047", "utf8", resp);
    await retrieve(connection, resp, "listings");
}

async function getDumps(connection: Client) {
    const resp = (await runCommandInShell(connection, `cd ${cDeployDirectory}\nls CEEDUMP.*`)).trim().split("\n");

    await convert(connection, "IBM-1047", "utf8", resp);
    await retrieve(connection, resp, "dumps");
}

async function artifacts(connection: Client) {
    const artifactPaths = ["c/zowex", "golang/zowed"];
    const artifactNames = artifactPaths.map((file) => basename(file)).sort();
    const localDirs = ["packages/cli/bin", "packages/vsce/bin"];
    const localFiles = ["server.pax.Z", "checksums.asc"];
    const [paxFile, checksumFile] = localFiles;
    const prePaxCmds = artifactPaths.map((file) => `cp ${file} ${basename(file)} && chmod 700 ${basename(file)}`);
    const postPaxCmd = `rm ${artifactNames.join(" ")}`;
    const e2aPipe = (file: string) => `iconv -f IBM-1047 -t ISO8859-1 > ${file} && chtag -t ISO8859-1 ${file}`;
    await runCommandInShell(
        connection,
        [
            `cd ${deployDirectory}`,
            ...prePaxCmds,
            `_BPXK_AUTOCVT=OFF sha256 -r ${artifactNames.join(" ")} | ${e2aPipe(checksumFile)}`,
            `pax -wvz -o saveext -f ${paxFile} ${artifactNames.join(" ")} ${checksumFile}`,
            postPaxCmd,
        ].join("\n"),
    );
    for (const localDir of localDirs) {
        fs.mkdirSync(resolve(__dirname, `./../${localDir}`), { recursive: true });
        for (const localFile of localFiles) {
            if (localDirs.indexOf(localDir) === 0) {
                await retrieve(connection, [localFile], localDir);
            } else {
                fs.cpSync(
                    resolve(__dirname, `./../${localDirs[0]}/${localFile}`),
                    resolve(__dirname, `./../${localDir}/${localFile}`),
                );
            }
        }
    }
}

async function runCommandInShell(connection: Client, command: string, pty = false) {
    const spinner = startSpinner(`Command: ${command.trim()}`);
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
                    const fullError = `\nError: runCommand connection.exec error: \n ${error || data}`;
                    stopSpinner(spinner, fullError);
                    process.exitCode = fullError.includes("SIGSEGV: segmentation violation") ? 11 : exitCode;
                    reject(fullError);
                }
            });
            stream.end(`${command}\nexit $?\n`);
        };
        if (pty) {
            connection.shell(cb);
        } else {
            connection.shell(false, cb);
        }
    });
}

async function retrieve(connection: Client, files: string[], targetDir: string) {
    return new Promise<void>((finish) => {
        console.log("Retrieving files...");

        connection.sftp(async (err, sftpcon) => {
            if (err) {
                console.log("Retrieve err");
                throw err;
            }

            for (let i = 0; i < files.length; i++) {
                const absTargetDir = resolve(__dirname, `./../${targetDir}`);
                if (!fs.existsSync(`${absTargetDir}`)) fs.mkdirSync(`${absTargetDir}`);
                const to = `${absTargetDir}/${files[i]}`;
                const from = `${deployDirectory}/${files[i]}`;
                // console.log(`from '${from}' to'${to}'`)
                await download(sftpcon, from, to);
            }
            console.log("Get complete!");
            finish();
        });
    });
}

async function deploy(connection: Client) {
    return new Promise<void>((finish) => {
        const spinner = startSpinner("Deploying files...");
        const files = getAllServerFiles();

        connection.sftp(async (err, sftpcon) => {
            if (err) {
                stopSpinner(spinner, `Deploy error!\n${err}`);
                throw err;
            }

            for (let i = 0; i < files.length; i++) {
                const from = resolve(__dirname, `${localDeployDir}/${files[i]}`);
                const to = `${deployDirectory}/${files[i]}`;
                await uploadFile(sftpcon, from, to);
            }
            stopSpinner(spinner, "Deploy complete!");
            finish();
        });
    });
}

async function convert(connection: Client, fromType = "utf8", toType = "IBM-1047", convFiles?: string[]) {
    return new Promise<void>((finish) => {
        const spinner = startSpinner(`Converting files from '${fromType}' to '${toType}'...`);
        const files = convFiles ?? getAllServerFiles();

        connection.shell(false, (err, stream) => {
            if (err) {
                stopSpinner(spinner, `Error: runCommand connection.exec error ${err}`);
                throw err;
            }

            stream.write(`cd ${deployDirectory}\n`);
            for (let i = 0; i < files.length; i++) {
                stream.write(`mv ${files[i]} ${files[i]}.u\n`);
                stream.write(`iconv -f ${fromType} -t ${toType} ${files[i]}.u > ${files[i]}\n`);
                stream.write(`chtag -t -c ${toType} ${files[i]}\n`);
            }
            stream.end("exit\n");

            stream.on("close", () => {
                stopSpinner(spinner, "Convert complete!");
                finish();
            });
            stream.on("data", (part: Buffer) => {
                console.log(part.toString());
            });
            stream.stderr.on("data", (data: Buffer) => {
                console.log(data.toString());
            });
        });
    });
}

async function bin(connection: Client) {
    return new Promise<void>((finish) => {
        connection.shell(false, (err, stream) => {
            if (err) {
                console.log(`Error: runCommand connection.exec error ${err}`);
                throw err;
            }

            // const buffer = Buffer.from([0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x15]);
            // const greeting = "Hello";
            const buffer = Buffer.from([0x01, 0x02, 0x03, 0x04, 0x05]); // 'Hello' in hexadecimal
            // const bufferEncoded = Buffer.from("12345").toString("base64")
            const bufferEncoded = buffer.toString("base64");

            // Print the binary data as a string of hexadecimal values
            // console.log(buffer.toString("hex"));
            // console.log(buffer.toString("utf-8"));

            // Print the binary data as a string of UTF-8 characters
            // const enc = Buffer.from(greeting).toString("base64")
            // console.log(enc.toString("hex"))
            stream.write(`cd ${goDeployDirectory}\n`, "ascii");
            stream.write("./ping\n", "ascii");
            stream.write(bufferEncoded, "ascii");
            stream.end(); // "", "ascii");

            // stream.write("pwd\n", "ascii");
            // stream.write(`cd ${deployDirectory}\n`);
            // for (let i = 0; i < dirs.length; i++) {
            //   console.log(`Creating ${dirs[i]}...`);
            //   stream.write(`mkdir -p ${dirs[i]}\n`);
            // }
            // stream.end("exit\n");

            stream.on("close", () => {
                console.log("Directories created !");
                finish();
            });
            stream.on("data", (part: Buffer) => {
                console.log(part.toString());
            });
            stream.stderr.on("data", (data: Buffer) => {
                console.log(data.toString());
            });
        });
    });
}

async function build(connection: Client) {
    console.log("Building native/c ...");
    const response = await runCommandInShell(
        connection,
        `cd ${cDeployDirectory} && make ${DEBUG_MODE() ? "-DBuildType=DEBUG" : ""}\n`,
    );
    DEBUG_MODE() && console.log(response);
    console.log("Building native/golang ...");
    console.log(
        await runCommandInShell(
            connection,
            `cd ${goDeployDirectory} &&${config.goEnv ? ` ${config.goEnv}` : ""} go build${DEBUG_MODE() ? "" : ' -ldflags="-s -w"'}\n`,
            true,
        ),
    );
    console.log("Build complete!");
}

async function clean(connection: Client) {
    console.log("Cleaning dir ...");
    const resp = await runCommandInShell(connection, `cd ${cDeployDirectory} && make clean\n`);
    console.log(resp);
    console.log("Clean complete");
}

async function rmdir(connection: Client) {
    console.log("Removing dir ...");
    const resp = await runCommandInShell(connection, `rm -rf ${deployDirectory}\n`);
    console.log(resp);
    console.log("Removal complete");
}

async function uploadFile(sftpcon: SFTPWrapper, from: string, to: string) {
    await new Promise<void>((finish) => {
        DEBUG_MODE() && console.log(`Uploading '${from}' to ${to}`);
        sftpcon.fastPut(from, to, (err) => {
            if (err) {
                console.log("Put err");
                console.log(from, to);
                throw err;
            }
            finish();
        });
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
