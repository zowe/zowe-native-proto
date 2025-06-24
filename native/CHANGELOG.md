# Change Log

All notable changes to the native code for "zowe-native-proto" are documented in this file.

Check [Keep a Changelog](http://keepachangelog.com/) for recommendations on how to structure this file.

## Recent Changes

- `c`: Fixed issue where canceled jobs displayed as "CC nnnn" instead of the string "CANCELED". [#169](https://github.com/zowe/zowe-native-proto/issues/169)
- `c`: Fixed issue where input job SYSOUT files could not be listed or displayed". [#196](https://github.com/zowe/zowe-native-proto/issues/196)
- `native`: Fixed issue where `zowed` failed to process RPC requests larger than 64 KB. [#364](https://github.com/zowe/zowe-native-proto/pull/364)
- `golang`: `zowed` now returns the UNIX permissions for each item's `mode` property in a USS list response. [#341](https://github.com/zowe/zowe-native-proto/pull/341)
- `c`: Added `--long` option to the `zowex uss list` command to return the long format for list output, containing additional file metadata. [#341](https://github.com/zowe/zowe-native-proto/pull/341)
- `c`: Added `--all` option to the `zowex uss list` command to show hidden files when listing a UNIX directory. [#341](https://github.com/zowe/zowe-native-proto/pull/341)
- `c`: Added `ListOptions` parameter to the `zusf_list_uss_file_path` function to support listing the long format and hidden files. [#341](https://github.com/zowe/zowe-native-proto/pull/341)
- `c`: Added CLI parser and lexer library for use in `zowex`. For an example of how to use the new CLI parser library, refer to the sample CLI code in `examples/native-cli/testcli.cpp`.
- `c`: Fixed an issue where the zowex `ds list` command always printed data set attributes when passing the argument `--response-format-csv`, even if the attributes argument was `false`.

## `0.1.2`

- `golang`: `zowed` now prints a ready message once it can accept input over stdin. [#221](https://github.com/zowe/zowe-native-proto/pull/221)
- `golang`: Reduced startup time for `zowed` by initializing workers in the background. [#237](https://github.com/zowe/zowe-native-proto/pull/237)
- `golang`: Added verbose option to enable debug logging. [#237](https://github.com/zowe/zowe-native-proto/pull/237)
- `golang`: Added SHA256 checksums to the ready message to allow checks for outdated server. [#236](https://github.com/zowe/zowe-native-proto/pull/236)
- `c,golang`: Added support for streaming USS file contents for read and write operations (`zusf_read_from_uss_file_streamed`, `zusf_write_to_uss_file_streamed`). [#311](https://github.com/zowe/zowe-native-proto/pull/311)
- `c,golang`: Added support for streaming data set contents for read and write operations (`zds_read_from_dsn_streamed`, `zds_write_to_dsn_streamed`). [#326](https://github.com/zowe/zowe-native-proto/pull/326)
- `c`: Added support for `recfm` (record format) attribute when listing data sets.
- `c`: Fixed issue where data sets were not opened with the right `recfm` for reading/writing.

## `0.1.1`

- `c`: Fixed issue where running the `zowex uss write` or `zowex ds write` commands without the `--etag-only` parameter resulted in a S0C4 and abrupt termination. [#216](https://github.com/zowe/zowe-native-proto/pull/216)
- `c`: Fixed issue where running the `zowex uss write` or `zowex ds write` commands without the `--encoding` parameter resulted in a no-op. [#216](https://github.com/zowe/zowe-native-proto/pull/216)
- `golang`: Fixed issue where a jobs list request returns unexpected results whenever a search query does not match any jobs. [#217](https://github.com/zowe/zowe-native-proto/pull/217)
- `c`: Fixed issue where e-tag calculation did not match when a data set was saved with the `--encoding` parameter provided. [#219](https://github.com/zowe/zowe-native-proto/pull/219)

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

## [Unreleased]

- Initial release
