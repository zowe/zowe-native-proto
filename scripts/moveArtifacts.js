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
