const chokidar = require("chokidar");
const p = require("path");

const watcher = chokidar.watch(["c/**/*.{c,cpp,h,s}", "golang/**"], {
  cwd: "native/",
  ignoreInitial: true,
  persistent: true,
});

const fs = require('fs');
const { Client } = require('ssh2');
const config = JSON.parse(fs.readFileSync(p.join("tools", "build", "config.local.json")));
let sshReady = false;

const conn = new Client()
.on('ready', () => {
  sshReady = true;
  console.log('SSH connection established');
}).connect({
  host: config.host,
  port: config.port || 22,
  username: config.user,
  privateKey: config.privateKey ? fs.readFileSync(config.privateKey) : undefined,
  password: config.password
});

async function uploadFile(localPath, remotePath) {
  return new Promise((resolve, reject) => {
    if (!sshReady) {
      reject(new Error('SSH connection not ready'));
      return;
    }
    
    conn.sftp((err, sftp) => {
      if (err) {
        reject(err);
        return;
      }
      const readStream = fs.createReadStream(p.join('native', localPath));
      const writeStream = sftp.createWriteStream(remotePath);
      
      writeStream.on('close', () => {
        // If the uploaded file is in the `c` directory, run gmake
        if (localPath.startsWith('c/')) {
          conn.exec(`cd ${p.posix.join(config.deployDirectory, 'c')} && gmake`, (err, stream) => {
            if (err) {
              console.error('Failed to run gmake:', err);
              resolve();
              return;
            }
            
            stream.on('close', () => {
              resolve();
            }).on('data', (data) => {
              process.stdout.write(data);
            }).stderr.on('data', (data) => {
              process.stderr.write(data);
            });
          });
        } else if (localPath.startsWith('golang/')) {
          // Run `go build` for golang files
          conn.shell((err, stream) => {
            if (err) {
              console.error('Failed to start shell:', err);
              resolve();
              return;
            }

            let buffer = '';
            stream.on('close', () => {
              resolve();
            }).on('data', (data) => {
              buffer += data;
              process.stdout.write(data);
            }).stderr.on('data', (data) => {
              process.stderr.write(data);
            });

            stream.end(`cd ${p.posix.join(config.deployDirectory, 'golang')} && go build\nexit\n`);
          });
        } else {
          resolve();
        }
      });
      
      writeStream.on('error', (err) => {
        reject(err);
      });
      
      readStream.pipe(writeStream);
    });
  });
}

watcher.on("add", async (path, stats) => {
  process.stdout.write(`${Date.now().toLocaleString()} [+] ${path}`);
  try {
    await uploadFile(path, p.posix.join(config.deployDirectory, path));
    console.log(" ✔");
  } catch (err) {
    console.error(" ✘", err);
  }
});

watcher.on("change", async (path, stats) => {
  process.stdout.write(`${Date.now().toLocaleString()} [~] ${path}`);
  try {
    await uploadFile(path, p.posix.join(config.deployDirectory, path));
    console.log(" ✔");
  } catch (err) {
    console.error(" ✘", err);
  }
});

watcher.on("unlink", (path, stats) => {
  process.stdout.write(`${Date.now().toLocaleString()} [-] ${path}`);
  console.log(" ✔");
});

console.log("watching for changes...");
setInterval(() => {}, 1_000_000);
