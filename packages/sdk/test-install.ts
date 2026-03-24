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

/**
 * Manual integration test for installServer with SFTP fallback.
 *
 * Usage:
 *   npx tsx test-install.ts <host> <user> <password> [serverPath]
 *
 * Example:
 *   npx tsx test-install.ts r152.msd.labs.broadcom.net PROD002 mypass
 *   npx tsx test-install.ts r152.msd.labs.broadcom.net PROD002 mypass ~/.zowe-server-test
 */

import { Logger, LoggingConfigurer } from "@zowe/imperative";
import { SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { ZSshClient } from "./src/ZSshClient";
import { ZSshUtils } from "./src/ZSshUtils";

const [host, user, password, serverPath] = process.argv.slice(2);

if (!host || !user || !password) {
    console.error("Usage: npx tsx test-install.ts <host> <user> <password> [serverPath]");
    process.exit(1);
}

const targetPath = serverPath ?? ZSshClient.DEFAULT_SERVER_PATH;

Logger.initLogger(LoggingConfigurer.configureLogger(process.cwd(), { name: "test" }));

const session = new SshSession({
    hostname: host,
    port: 22,
    user,
    password,
});

console.log(`\n=== installServer integration test ===`);
console.log(`Host:       ${host}`);
console.log(`User:       ${user}`);
console.log(`ServerPath: ${targetPath}`);
console.log();

async function run(): Promise<void> {
    console.log("--- Step: Uninstall (clean slate) ---");
    try {
        await ZSshUtils.uninstallServer(session, targetPath);
        console.log("Uninstall OK\n");
    } catch (err) {
        console.log(`Uninstall skipped (may not exist): ${err}\n`);
    }

    console.log("--- Step: Install ---");
    const startTime = Date.now();
    let totalProgress = 0;
    const result = await ZSshUtils.installServer(session, targetPath, {
        onProgress: (increment) => {
            totalProgress += increment;
            process.stdout.clearLine(0);
            process.stdout.cursorTo(0);
            process.stdout.write(`  upload: ${totalProgress}%`);
        },
        onError: async (error, context) => {
            console.error(`\n  ERROR in "${context}": ${error.message}`);
            return false;
        },
    });
    process.stdout.write("\n");
    const elapsed = ((Date.now() - startTime) / 1000).toFixed(1);
    console.log(`\ninstallServer returned: ${result} (${elapsed}s)\n`);

    if (!result) {
        console.error("INSTALL FAILED — check logs above for [ZSshUtils] messages");
        process.exit(1);
    }

    console.log("--- Step: Verify (connect to server) ---");
    try {
        using client = await ZSshClient.create(session, { serverPath: targetPath });
        const resp = await client.uss.listFiles({ fspath: "/tmp" });
        console.log(`Connected OK — /tmp has ${resp.items.length} entries`);
    } catch (err: unknown) {
        const e = err as { message?: string; additionalDetails?: string };
        console.error(`Connection failed: ${e.message ?? String(err)}`);
        if (e.additionalDetails) {
            console.error(`Details:\n${e.additionalDetails}`);
        }
        process.exit(1);
    }

    console.log("\n=== ALL PASSED ===\n");
}

void (async () => {
    // NOSONAR: top-level await not available in CommonJS module target; this file is run via tsx only
    try {
        await run();
    } catch (err) {
        console.error(`\nFATAL: ${err}`);
        process.exit(1);
    }
})();
