const chokidar = require("chokidar");
const child_process = require("node:child_process");
const p = require("path");

const watcher = chokidar.watch(["c/**/*.{c,cpp,h,s}", "golang/**"], {
  cwd: "native/",
  ignoreInitial: true,
  persistent: true,
});

watcher.on("add", (path, stats) => {
  process.stdout.write(`${Date.now().toLocaleString()} [+] ${path}`);
  child_process.execSync(
    ["npm", "run", "tools:deploy", p.posix.normalize(path)].join(" ")
  );
  console.log(" ✔");
});
watcher.on("change", (path, stats) => {
  process.stdout.write(`${Date.now().toLocaleString()} [~] ${path}`);
  child_process.execSync(
    ["npm", "run", "tools:deploy", p.posix.normalize(path)].join(" ")
  );
  console.log(" ✔");
});
watcher.on("unlink", (path, stats) => {
  process.stdout.write(`${Date.now().toLocaleString()} [-] ${path}`);
  console.log(" ✔");
});

console.log("watching for changes...");
setInterval(() => {}, 1_000_000);
