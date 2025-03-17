# Change Log

All notable changes to the native code for "zowe-native-proto" are documented in this file.

Check [Keep a Changelog](http://keepachangelog.com/) for recommendations on how to structure this file.

## `0.1.0`

- `c`: Enable `langlvl(extended0x)` for C++ code to support C++0x features (`auto`, `nullptr`, etc.) [#35](https://github.com/zowe/zowe-native-proto/pull/35)
- `c`: Replace all applicable `NULL` references with `nullptr` in C++ code. [#36](https://github.com/zowe/zowe-native-proto/pull/36)
- `c,golang`: Add support for writing data sets and USS files with the given encoding. [#37](https://github.com/zowe/zowe-native-proto/pull/37)
- `golang`: Added `restoreDataset` function in the middleware. [#38](https://github.com/zowe/zowe-native-proto/pull/38)
- `golang`: Added `procstep` and `dsname` to the list of fields when getting spool files
- `golang`: Added `tygo` to generate types based on the Go types for the TypeScript SDK. [#71](https://github.com/zowe/zowe-native-proto/pull/71)
- `c,golang`: Added `watch:native` npm script to detect and upload changes to files during development. [#100](https://github.com/zowe/zowe-native-proto/pull/100)
- `c,golang`: Added `chmod`, `chown`, `chtag` and `delete` functionality for USS files & folders. [#143](https://github.com/zowe/zowe-native-proto/pull/143)
- `c,golang`: Added support for recursive `chmod` functionality for USS folders. [#143](https://github.com/zowe/zowe-native-proto/pull/143)
- `c,golang`: Added capability to submit JCL from stdin. [#143](https://github.com/zowe/zowe-native-proto/pull/143)
- `c`: Add support for `mkdir -p` behavior when making a new USS directory. [#143](https://github.com/zowe/zowe-native-proto/pull/143)
- `golang`: Refactored error handling in Go layer to forward errors to client as JSON. [#143](https://github.com/zowe/zowe-native-proto/pull/143)
- `c`: Fixed dangling pointers in CLI code & refactored reading resource contents to avoid manual memory allocation. [#167](https://github.com/zowe/zowe-native-proto/pull/167)
- `golang`: Added `createDataset` function in the middleware. [#95](https://github.com/zowe/zowe-native-proto/pull/95)
- `c,golang`: Added `createMember` function. [#95](https://github.com/zowe/zowe-native-proto/pull/95)
- `c,golang`: Added `cancelJob` function. [#138](https://github.com/zowe/zowe-native-proto/pull/138)
- `c,golang`: Added `holdJob` and `releaseJob` functions. [#182](https://github.com/zowe/zowe-native-proto/pull/182)
- `c`: Fixed issue where data set search patterns did not return the same results as z/OSMF. [#74](https://github.com/zowe/zowe-native-proto/issues/74)
- `c`: Added check for maximum data set pattern length before making a list request. Now, requests with patterns longer than 44 characters are rejected. [#185](https://github.com/zowe/zowe-native-proto/pull/185)
- `c,golang`: Fixed issue where submit JCL handler did not convert input data from UTF-8 and did not support an `--encoding` option. [#198](https://github.com/zowe/zowe-native-proto/pull/198)
- `c`: Fixed issue where submit JCL handler did not support raw bytes from stdin when the binary is directly invoked through a shell. [#198](https://github.com/zowe/zowe-native-proto/pull/198)
- `c,golang`: Added `submitUss` function. [#184](https://github.com/zowe/zowe-native-proto/pull/184)
- `golang`: Fixed issue where listing a non-existent data set pattern resulted in a panic and abrupt termination of `zowed`. [#200](https://github.com/zowe/zowe-native-proto/issues/200)
- `golang`: Fixed issue where a newline was present in the job ID when returning a response for the "submitJcl" command. [#211](https://github.com/zowe/zowe-native-proto/pull/211)
- `c`: Added conflict detection for USS and Data Set write operations through use of the `--etag` option. [#144](https://github.com/zowe/zowe-native-proto/issues/144)
- `golang`: Added `Etag` property to request and response types for both USS and Data Set write operations. [#144](https://github.com/zowe/zowe-native-proto/issues/144)
- `c`: Fixed issue where running the `zowex uss write` or `zowex ds write` commands without the `--etag-only` parameter resulted in a S0C4 and abrupt termination. [#216](https://github.com/zowe/zowe-native-proto/pull/216)
- `c`: Fixed issue where running the `zowex uss write` or `zowex ds write` commands without the `--encoding` parameter resulted in a no-op. [#216](https://github.com/zowe/zowe-native-proto/pull/216)

## [Unreleased]

- Initial release
