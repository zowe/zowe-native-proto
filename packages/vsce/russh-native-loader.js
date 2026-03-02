// rspack loader for russh/lib/native.js
// Patches the .node require to use __non_webpack_require__ and load from prebuilds/.
module.exports = function (source) {
    return source.replace(
        /require\(`\.\.\/russh\.\$\{nativeModule\}\.node`\)/,
        '__non_webpack_require__(require("path").join(__dirname, "..", "prebuilds", `russh.${nativeModule}.node`))'
    );
};
