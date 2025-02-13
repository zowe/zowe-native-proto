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

const chokidar = require("chokidar");
const p = require("path");
const fs = require("fs");
const { Client } = require("ssh2");
const { promisify } = require("util");

// Initialize file watcher to detect changes to native code
const watcher = chokidar.watch(["c/**/*.{c,cpp,h,hpp,s}", "golang/**"], {
  cwd: "native/",
  ignoreInitial: true,
  persistent: true,
});

// Read user configuration and tasks
const CONFIG = JSON.parse(
  fs.readFileSync(p.join("tools", "build", "config.local.json"))
);
const USER_TASKS_DEFINED = fs.existsSync(
  p.join("tools", "build", "tasks.local.json")
);
const TASKS = JSON.parse(
  fs.readFileSync(
    p.join(
      "tools",
      "build",
      USER_TASKS_DEFINED ? "tasks.local.json" : "tasks.default.json"
    )
  )
);

let sshReady = false;

// Initialize SSH connection
const conn = new Client()
  .on("ready", () => {
    sshReady = true;
    console.log("✔ SSH connection established");
  })
  .connect({
    host: CONFIG.host,
    port: CONFIG.port || 22,
    username: CONFIG.username,
    privateKey: CONFIG.privateKey
      ? fs.readFileSync(CONFIG.privateKey)
      : undefined,
    password: CONFIG.password,
  });

/**
 * Builds a series of commands to convert the file and invoke the build task
 * @param {string} remotePath - The remote path of the file
 * @param {string} task - The task to run
 * @returns {string} A series of commands to convert the file and invoke the build task
 */
function buildCommonCommand(remotePath, task) {
  let cmd = `mv ${remotePath} ${remotePath}.u && `;
  cmd += `iconv -f utf8 -t IBM-1047 ${remotePath}.u > ${remotePath} && `;
  cmd += `chtag -t -c IBM-1047 ${remotePath} && `;
  cmd += `cd ${p.posix.join(CONFIG.deployDirectory, task)} && `;
  cmd += `${TASKS[task]} 1> /dev/null`;
  return cmd;
}

/**
 * Invokes the given task and prints the status
 * @param {string} task - The task to run
 * @param {string} cmd - The command to run
 * @param {Function} resolve - The resolve function to call once the command has finished
 */
function invokeTaskAndPrintStatus(task, cmd, resolve) {
  const taskCmd = TASKS[task];
  let errText = "";

  conn.exec(cmd, (err, stream) => {
    if (err) {
      console.error(err);
      resolve();
    }

    stream
      .on("end", () => {
        if (errText.length === 0) {
          console.log(`\n  [tasks -> ${task}] ${taskCmd} succeeded ✔`);
        } else {
          console.error(
            `\n  [tasks -> ${task}] ${taskCmd} failed ✘\n`,
            errText
          );
        }
        resolve();
      })
      .stderr.on("data", (data) => {
        let str = data.toString().trim();
        if (
          str === "$" ||
          str === "exit" ||
          /CCN[0-9]{4}\(W\)/.test(str) ||
          str.startsWith("IGD")
        ) {
          return;
        }
        errText += str;
      });
  });
}

/**
 * Runs the given task to rebuild after changes were made to the given file
 * @param {string} remotePath - The remote path of the file
 * @param {string} task - The task to run
 * @returns {Promise<void>} A promise that resolves once the task is complete
 */
async function runTask(remotePath, task) {
  let cmd = buildCommonCommand(remotePath, task);
  return new Promise((resolve) => invokeTaskAndPrintStatus(task, cmd, resolve));
}

/**
 * Performs an action on the remote server for the given file and then runs the appropriate task
 * @param {string} localPath - The local path of the file
 * @param {string} remotePath - The remote path of the file
 * @param {string} - Whether to "upload" a file (addition/change) or "delete" it
 * @returns {Promise<void>} A promise that resolves when the file is deleted
 */
async function performActionOnFile(localPath, remotePath, action) {
  return new Promise((resolve, reject) => {
    if (!sshReady) {
      return reject(new Error("SSH connection not ready"));
    }

    conn.sftp(async (err, sftp) => {
      if (err) {
        return reject(err);
      }
      switch (action) {
        case "upload":
          console.log(` -> ${remotePath}`);
          // Set up streams to upload the file using SFTP
          const readStream = fs.createReadStream(
            p.join("native", localPath),
            "utf8"
          );
          const writeStream = sftp.createWriteStream(remotePath, {
            encoding: "utf8",
          });

          writeStream.on("error", (err) => {
            reject(err);
          });

          writeStream.on("close", () => {
            resolve();
          });

          // Stream the file to the remote path
          readStream.pipe(writeStream);
          break;
        case "delete":
          // Delete the file at the remote path
          sftp.unlink(remotePath);
          resolve();
          break;
        default:
          break;
      }
    });
  }).then(
    // Run tasks after performing the action
    async (val) => {
      return runTask(remotePath, localPath.split(p.sep)[0]);
    }
  );
}

watcher.on("add", async (path, stats) => {
  process.stdout.write(`${new Date().toLocaleString()} [+] ${path}`);
  try {
    await performActionOnFile(
      path,
      p.posix.join(CONFIG.deployDirectory, path.replace(p.sep, p.posix.sep)),
      "upload"
    );
  } catch (err) {
    console.error("✘", err);
  }
});

watcher.on("change", async (path, stats) => {
  process.stdout.write(`${new Date().toLocaleString()} [~] ${path}`);
  try {
    await performActionOnFile(
      path,
      p.posix.join(CONFIG.deployDirectory, path.replace(p.sep, p.posix.sep)),
      "upload"
    );
  } catch (err) {
    console.error("✘", err);
  }
});

watcher.on("unlink", async (path, stats) => {
  process.stdout.write(`${new Date().toLocaleString()} [-] ${path}`);
  try {
    await performActionOnFile(
      path,
      p.posix.join(CONFIG.deployDirectory, path.replace(p.sep, p.posix.sep)),
      "delete"
    );
  } catch (err) {
    console.error("✘", err);
  }
});

console.log("watching for changes...");
// Keep the script running indefinitely so the watcher can detect changes
setInterval(() => {}, 1 << 30);
