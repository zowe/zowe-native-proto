import * as crypto from "node:crypto";
import * as fs from "node:fs";
import * as path from "node:path";

export function getFilenames(userConfig: any): Record<string, string> {
    const localFile = path.join(__dirname, userConfig.testFile);
    const remoteFile = path.join(userConfig.ussDir, path.basename(userConfig.testFile));
    const tempFile = `${localFile}.tmp`;
    return { localFile, remoteFile, tempFile };
}

export function getSshConfig(): Record<string, any> {
    const config = require("../../../config.local.json");
    const host = config.host;
    const port = config.port ?? 22;
    const user = config.username;
    let privateKey: Buffer | undefined;
    const password = config.password;
    try {
        privateKey = fs.readFileSync(config.privateKey);
    } catch (e) {}
    return { host, port, user, password, privateKey };
}

export async function compareChecksums(filePath1: string, filePath2: string): Promise<boolean> {
    const getSha256Hash = (filePath: string): Promise<string> =>
        new Promise((resolve, reject) => {
            const hash = crypto.createHash("sha256");
            const stream = fs.createReadStream(filePath);
            stream.on("error", reject);
            stream.on("data", (chunk) => hash.update(chunk));
            stream.on("end", () => resolve(hash.digest("hex")));
        });
    const [hash1, hash2] = await Promise.all([getSha256Hash(filePath1), getSha256Hash(filePath2)]);
    return hash1 === hash2;
}
