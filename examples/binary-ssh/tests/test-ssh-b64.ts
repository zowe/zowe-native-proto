import { createReadStream, createWriteStream, unlinkSync } from "node:fs";
import { dirname, parse } from "node:path";
import { Transform } from "node:stream";
import { pipeline } from "node:stream/promises";
import { Client } from "ssh2";
import * as utils from "./utils";
const userConfig = require("./config.json");
const testPrefix = parse(__filename).name;

async function main() {
    // Set up
    const sshConfig = utils.getSshConfig();
    const sshClient = new Client();
    const { localFile, remoteFile, tempFile } = utils.getFilenames(userConfig);
    sshClient.connect({ ...sshConfig, debug: userConfig.verboseSsh ? console.debug : undefined });
    await new Promise<void>((resolve, reject) => {
        sshClient.on("ready", () => {
            resolve();
        });
        sshClient.on("error", (err) => {
            reject(err);
        });
    });
    const b64Encoder = () =>
        new Transform({
            transform(chunk, encoding, callback) {
                this.push(chunk.toString("base64"));
                this.push("\n");
                callback();
            },
        });
    const b64Decoder = () =>
        new Transform({
            transform(chunk, encoding, callback) {
                const lines = ((this.lastChunk || "") + chunk.toString()).split("\n");
                for (let i = 0; i < lines.length - 1; i++) {
                    if (lines[i].length > 0) {
                        this.push(Buffer.from(lines[i], "base64"));
                    }
                }
                this.lastChunk = lines.pop();
                callback();
            },
        });

    for (let i = 0; i < userConfig.testCount; i++) {
        // 1. Upload
        console.time(`${testPrefix}:upload`);
        await new Promise<void>((resolve, reject) => {
            sshClient.exec(
                `${dirname(remoteFile)}/testb64 upload ${remoteFile} ${userConfig.chunkSize}`,
                async (err, stream) => {
                    if (err) {
                        reject(err);
                        return;
                    }
                    const srcStream = createReadStream(localFile, { highWaterMark: userConfig.chunkSize });
                    await pipeline(srcStream, b64Encoder(), stream.stdin);
                    resolve();
                },
            );
        });
        console.timeEnd(`${testPrefix}:upload`);

        // 2. Download
        console.time(`${testPrefix}:download`);
        await new Promise<void>((resolve, reject) => {
            sshClient.exec(
                `${dirname(remoteFile)}/testb64 download ${remoteFile} ${userConfig.chunkSize}`,
                async (err, stream) => {
                    if (err) {
                        reject(err);
                        return;
                    }
                    const destStream = createWriteStream(tempFile);
                    await pipeline(stream.stdout, b64Decoder(), destStream);
                    resolve();
                },
            );
        });
        console.timeEnd(`${testPrefix}:download`);

        // 3. Verify
        const success = await utils.compareChecksums(localFile, tempFile);
        if (success) {
            console.log(`✅ Checksums match (${i + 1}/${userConfig.testCount})`);
        } else {
            console.error(`❌ Checksums do not match (${i + 1}/${userConfig.testCount})`);
        }
    }

    // Tear down
    sshClient.end();
    unlinkSync(tempFile);
}

main();
