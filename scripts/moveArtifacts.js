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
const path = require("path");
const srcDir = path.resolve(__dirname, "..");
const srcFiles = ["checksums.asc", "server.pax.Z"];
const destDirs = [path.join(srcDir, "packages/cli/bin"), path.join(srcDir, "packages/vsce/bin")];
for (const destDir of destDirs) {
    fs.mkdirSync(destDir, { recursive: true });
    for (const srcFile of srcFiles) {
        fs.copyFileSync(path.join(srcDir, srcFile), path.join(destDir, srcFile));
    }
}
srcFiles.forEach(fs.unlinkSync);
