const chokidar = require("chokidar");
const p = require("path");

const watcher = chokidar.watch(["c/**/*.{c,cpp,h,hpp,s}", "golang/**"], {
  cwd: "native/",
  ignoreInitial: true,
  persistent: true,
});

const fs = require("fs");
const { Client } = require("ssh2");
const config = JSON.parse(
  fs.readFileSync(p.join("tools", "build", "config.local.json"))
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
  cmd += `make\nexit\n`;

  stream.write(cmd);

  let errText = "";
  stream
    .on("close", () => {
      if (errText.length == 0) {
        console.log("\n\t[tasks -> c] make succeeded ✔");
      } else {
        console.log("\t[tasks -> c] make failed ✘\nerror: \n", errText);
      }
      resolve();
    })
    .on("data", (data) => {})
    .stderr.on("data", (data) => {
      errText += data;
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
    .on("close", () => {
      if (errText.length == 0) {
        console.log("\n\t[tasks -> golang] go build succeeded ✔");
      } else {
        console.log(
          "\t[tasks -> golang] go build failed ✘\nerror: \n",
          errText
        );
      }
      resolve();
    })
    .on("data", (data) => {})
    .stderr.on("data", (data) => {
      errText += data;
    });

  stream.end(
    `cd ${p.posix.join(config.deployDirectory, "golang")} && go build\nexit\n`
  );
}

async function deleteFile(remotePath) {
  return new Promise((resolve, reject) => {
    if (!sshReady) {
      reject(new Error("SSH connection not ready"));
      return;
    }

    conn.sftp((err, sftp) => {
      sftp
        .deleteFile(remotePath)
        .then((res) => resolve())
        .catch((err) => console.log(err) && resolve());
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
        conn.shell((err, stream) => {
          // If the uploaded file is in the `c` directory, run `make`
          if (localPath.split(p.sep)[0] === "c") {
            cTask(err, remotePath, stream, resolve);
          } else if (localPath.split(p.sep)[0] == "golang") {
            // Run `go build` when a Golang file has changed
            golangTask(err, remotePath, stream, resolve);
          } else {
            resolve();
          }
        });
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
      p.posix.join(config.deployDirectory, path.replace(p.sep, p.posix.sep))
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
      p.posix.join(config.deployDirectory, path.replace(p.sep, p.posix.sep))
    );
  } catch (err) {
    console.error(" ✘", err);
  }
});

watcher.on("unlink", async (path, stats) => {
  process.stdout.write(`${Date.now().toLocaleString()} [-] ${path}`);
  try {
    await deleteFile(
      p.posix.join(config.deployDirectory, path.replace(p.sep, p.posix.sep))
    );
  } catch (err) {
    console.error(" ✘", err);
  }
});

console.log("watching for changes...");
setInterval(() => {}, 1e6);
