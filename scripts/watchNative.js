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

const watcher = chokidar.watch(
  ["c/makefile", "c/**/*.{c,cpp,h,hpp,s,sh}", "golang/**"],
  {
    cwd: "native/",
    ignoreInitial: true,
    persistent: true,
  }
);

const fs = require("fs");
const { Client } = require("ssh2");
const config = JSON.parse(
  fs.readFileSync(p.join(__dirname, "..", "config.local.json"))
);
let sshReady = false;

const conn = new Client()
  .on("ready", () => {
    sshReady = true;
    console.log("SSH connection established");
  })
  .connect({
    host: config.host,
    port: config.port || 22,
    username: config.username,
    privateKey: config.privateKey
      ? fs.readFileSync(config.privateKey)
      : undefined,
    password: config.password,
    keepaliveInterval: 30e3,
  });

function cTask(err, remotePath, stream, resolve) {
  if (err) {
    console.error("Failed to start shell:", err);
    resolve();
    return;
  }

  if (err) {
    console.error("Failed to run make:", err);
    resolve();
    return;
  }

  let cmd = `mv ${remotePath} ${remotePath}.u\n`;
  cmd += `iconv -f utf8 -t IBM-1047 ${remotePath}.u > ${remotePath}\n`;
  cmd += `chtag -t -c IBM-1047 ${remotePath}\n`;
  cmd += `\ncd ${p.posix.join(config.deployDirectory, "c")}\n`;
  cmd += `make 1> /dev/null\nexit\n`;

  stream.write(cmd);

  let errText = "";
  stream
    .on("end", () => {
      if (errText.length == 0) {
        console.log("\n\t[tasks -> c] make succeeded ✔");
      } else {
        console.log("\n\t[tasks -> c] make failed ✘\nerror: \n", errText);
      }
      resolve();
    })
    .on("data", (data) => { })
    .stderr.on("data", (data) => {
      let str = data.toString().trim();
      if (/IGD\d{5}I /.test(str)) return;
      errText += str;
    });
}

function golangTask(err, remotePath, stream, resolve) {
  if (err) {
    console.error("Failed to start shell:", err);
    resolve();
    return;
  }

  let errText = "";
  let cmd = `mv ${remotePath} ${remotePath}.u\n`;
  cmd += `iconv -f utf8 -t IBM-1047 ${remotePath}.u > ${remotePath}\n`;
  cmd += `chtag -t -c IBM-1047 ${remotePath}\n`;

  stream.write(cmd);
  stream
    .on("end", () => {
      if (errText.length == 0) {
        console.log("\n\t[tasks -> golang] go build succeeded ✔");
      } else {
        console.log(
          "\n\t[tasks -> golang] go build failed ✘\nerror: \n",
          errText
        );
      }
      resolve();
    })
    .on("data", (data) => { })
    .stderr.on("data", (data) => {
      errText += data;
    });

  stream.end(
    `cd ${p.posix.join(config.deployDirectory, "golang")} && go build -ldflags="-s -w"\nexit\n`
  );
}

async function deleteFile(remotePath) {
  return new Promise((resolve, reject) => {
    if (!sshReady) {
      reject(new Error("SSH connection not ready"));
      return;
    }

    conn.sftp((err, sftp) => {
      sftp.unlink(remotePath, (err) => {
        if (err) {
          return reject(err);
        }

        sftp.end();
        resolve();
      });
    });
  });
}

async function uploadFile(localPath, remotePath) {
  return new Promise((resolve, reject) => {
    if (!sshReady) {
      reject(new Error("SSH connection not ready"));
      return;
    }

    conn.sftp((err, sftp) => {
      if (err) {
        reject(err);
        return;
      }

      process.stdout.write(` -> ${remotePath} `);
      const readStream = fs.createReadStream(
        p.join("native", localPath),
        "utf8"
      );
      const writeStream = sftp.createWriteStream(remotePath, {
        encoding: "utf8",
      });

      writeStream.on("close", () => {
        conn.shell(false, (err, stream) => {
          // If the uploaded file is in the `c` directory, run `make`
          if (localPath.split(p.sep)[0] === "c") {
            cTask(err, remotePath, stream, resolve);
          } else if (localPath.split(p.sep)[0] == "golang") {
            // Run `go build` when a Golang file has changed
            golangTask(err, remotePath, stream, resolve);
          } else {
            console.log();
            resolve();
          }
        });
        sftp.end();
      });

      writeStream.on("error", (err) => {
        reject(err);
      });

      readStream.pipe(writeStream);
    });
  });
}

watcher.on("add", async (path, stats) => {
  process.stdout.write(`${new Date().toLocaleString()} [+] ${path}`);
  try {
    await uploadFile(
      path,
      p.posix.join(config.deployDirectory, path.replaceAll(p.sep, p.posix.sep))
    );
  } catch (err) {
    console.error(" ✘", err);
  }
});

watcher.on("change", async (path, stats) => {
  process.stdout.write(`${new Date().toLocaleString()} [~] ${path}`);
  try {
    await uploadFile(
      path,
      p.posix.join(config.deployDirectory, path.replaceAll(p.sep, p.posix.sep))
    );
  } catch (err) {
    console.error(" ✘", err);
  }
});

watcher.on("unlink", async (path, stats) => {
  process.stdout.write(`${new Date().toLocaleString()} [-] ${path}`);
  try {
    await deleteFile(
      p.posix.join(config.deployDirectory, path.replaceAll(p.sep, p.posix.sep))
    );
    console.log(" ✔");
  } catch (err) {
    console.error(" ✘", err);
  }
});

console.log("watching for changes...");
setInterval(() => { }, 1e6);
