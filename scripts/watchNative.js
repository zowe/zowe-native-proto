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
const USER_TASKS_DEFINED = fs.existsSync(p.join("tools", "build", "tasks.local.json"));
const TASKS = JSON.parse(fs.readFileSync(p.join("tools", "build", USER_TASKS_DEFINED ? "tasks.local.json" : "tasks.default.json")));

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
  cmd += `${TASKS[task]} 1> /dev/null && exit\n`;
  return cmd;
}

/**
 * Invokes the given task and prints the status
 * @param {string} task - The task to run
 * @param {string} cmd - The command to run
 * @param {Stream} stream - The stream to write the command to
 * @param {Function} resolve - The function to call when the command is finished
 */
function invokeTaskAndPrintStatus(task, cmd, stream, resolve) {
  const taskCmd = TASKS[task];
  let errText = "";
  stream
    .on("close", () => {
      if (errText.length == 0) {
        console.log(`\n\t[tasks -> ${task}] ${taskCmd} succeeded ✔`);
      } else {
        console.log(
          `\t[tasks -> ${task}] ${taskCmd} failed ✘\nerror: \n`,
          errText
        );
      }
      resolve();
    })
    .on("data", (data) => {
      if (data.trim() === "$" || data.trim() === "exit") {
        return;
      }
      errText += data;
    });
  stream.write(cmd);
}

/**
 * Runs the given task to rebuild after changes were made to the given file
 * @param {string} remotePath - The remote path of the file
 * @param {string} task - The task to run
 * @param {Stream} stream - The stream to write the command to
 * @param {Function} resolve - The function to call when the command is finished
 */
function runTask(remotePath, task, stream, resolve) {
  if (err) {
    console.error("✘ Failed to start shell:", err);
    resolve();
    return;
  }

  let cmd = buildCommonCommand(remotePath, task);
  invokeTaskAndPrintStatus(task, cmd, stream, resolve);
}

/**
 * Delete a file from the remote server and run the appropriate task
 * @param {string} localPath - The local path of the file
 * @param {string} remotePath - The remote path of the file
 * @returns {Promise<void>} A promise that resolves when the file is deleted
 */
async function deleteFile(localPath, remotePath) {
  return new Promise((resolve, reject) => {
    if (!sshReady) {
      return reject(new Error("SSH connection not ready"));
    }

    conn.sftp((err, sftp) => {
      if (err) {
        return reject(err);
      }
      sftp.unlink(remotePath);
      if (localPath.split(p.sep)[0] === "c") {
        runTask(remotePath, "c", stream, resolve);
      } else if (localPath.split(p.sep)[0] == "golang") {
        runTask(remotePath, "golang", stream, resolve);
      } else {
        resolve();
      }
    });
  });
}

/**
 * Upload a file to the remote server and run the appropriate task
 * @param {string} localPath - The local path of the file
 * @param {string} remotePath - The remote path of the file
 * @returns {Promise<void>} A promise that resolves when the file is uploaded
 */
async function uploadFile(localPath, remotePath) {
  return new Promise((resolve, reject) => {
    if (!sshReady) {
      return reject(new Error("SSH connection not ready"));
    }

    conn.sftp((err, sftp) => {
      if (err) {
        return reject(err);
      }

      process.stdout.write(` -> ${remotePath} `);
      // Set up streams to upload the file using SFTP
      const readStream = fs.createReadStream(
        p.join("native", localPath),
        "utf8"
      );
      const writeStream = sftp.createWriteStream(remotePath, {
        encoding: "utf8",
      });

      writeStream.on("close", () => {
        conn.shell({ modes: { ECHO: 0, ECHONL: 0 }, env: { PS1: "$" } }, (err, stream) => {
          if (err) {
            reject(err);
            return;
          }

          // Run the appropriate build task based on the file type
          if (localPath.split(p.sep)[0] === "c") {
            runTask(remotePath, "c", stream, resolve);
          } else if (localPath.split(p.sep)[0] == "golang") {
            runTask(remotePath, "golang", stream, resolve);
          } else {
            resolve();
          }
        });
      });

      writeStream.on("error", (err) => {
        reject(err);
      });

      // Stream the file to the remote path
      readStream.pipe(writeStream);
    });
  });
}

watcher.on("add", async (path, stats) => {
  process.stdout.write(`${new Date().toLocaleString()} [+] ${path}`);
  try {
    await uploadFile(
      path,
      p.posix.join(CONFIG.deployDirectory, path.replace(p.sep, p.posix.sep))
    );
  } catch (err) {
    console.error("✘", err);
  }
});

watcher.on("change", async (path, stats) => {
  process.stdout.write(`${new Date().toLocaleString()} [~] ${path}`);
  try {
    await uploadFile(
      path,
      p.posix.join(CONFIG.deployDirectory, path.replace(p.sep, p.posix.sep))
    );
  } catch (err) {
    console.error("✘", err);
  }
});

watcher.on("unlink", async (path, stats) => {
  process.stdout.write(`${new Date().toLocaleString()} [-] ${path}`);
  try {
    await deleteFile(
      path,
      p.posix.join(CONFIG.deployDirectory, path.replace(p.sep, p.posix.sep))
    );
  } catch (err) {
    console.error("✘", err);
  }
});

console.log("watching for changes...");
// Keep the script running indefinitely so the watcher can detect changes
setInterval(() => {}, 1 << 30);
