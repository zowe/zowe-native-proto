/*
* This program and the accompanying materials are made available under the terms of the
* Eclipse Public License v2.0 which accompanies this distribution, and is available at
* https://www.eclipse.org/legal/epl-v20.html
*
* SPDX-License-Identifier: EPL-2.0
*
* Copyright Contributors to the Zowe Project.
*
*/

const childProcess = require("child_process");
const fs = require("fs");
const path = require("path");

// Workaround for https://github.com/npm/cli/issues/3466
process.chdir(__dirname + "/..");
const tempDir = path.resolve(fs.mkdtempSync("build-"));
const execCmd = (cmd) => childProcess.execSync(cmd, { cwd: tempDir, stdio: "inherit" });
fs.mkdirSync("dist", {recursive: true});
fs.cpSync(path.join("packages", "cli"), tempDir, { recursive: true });
fs.copyFileSync("package-lock.json", path.join(tempDir, "npm-shrinkwrap.json"));
execCmd("npm install --ignore-scripts");
const srcDir = path.join("node_modules", "zowe-native-proto-sdk");
const destDir = path.join(tempDir, srcDir);
fs.rmSync(destDir, { recursive: true, force: true });
fs.cpSync(fs.realpathSync(srcDir), destDir, { recursive: true });
execCmd("npm pack --pack-destination=../dist");
fs.rmSync(tempDir, { recursive: true, force: true });
