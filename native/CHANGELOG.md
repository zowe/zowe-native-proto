# Change Log

All notable changes to the native code for "zowe-native-proto" are documented in this file.

Check [Keep a Changelog](http://keepachangelog.com/) for recommendations on how to structure this file.

## Recent Changes

- Added the rename dataset functionality to the backend. [#376] (https://github.com/zowe/zowe-native-proto/issues/376)

## `0.2.2`

- `c`: Renable `zowex ds compress` and correct 0C4 abend. [#640](https://github.com/zowe/zowe-native-proto/issues/640)
- `c`: Fixed an issue where `--wait ACTIVE` on `zowex job` commands would wait indefinitely if the job was fast enough to reach the `OUTPUT` phase before polling its status. [#700](https://github.com/zowe/zowe-native-proto/pull/700)
- `zowed`: Fixed `message` property of `error` object in the JSON response to contain valuable details about errors thrown by `zowex`. [#712](https://github.com/zowe/zowe-native-proto/pull/712)
- `c`: You can now access dynamic arguments from a command handler through its `InvocationContext` parameter. [#715](https://github.com/zowe/zowe-native-proto/pull/715)
- `zowed`: Fixed an issue where the `submitJcl` command failed if an encoding was not specified for the JCL contents. [#724](https://github.com/zowe/zowe-native-proto/pull/724)
- `c`: Fixed an issue where multibyte codepages like IBM-939 were not always encoded correctly when writing data sets and USS files. [#718](https://github.com/zowe/zowe-native-proto/pull/718)

## `0.2.1`

- `c`: Disallow `zowex compress ds` to prevent lurking abend.
- `c`: Fixed `zowex uss create-file` and `create-dir` commands to respect the `mode` option when provided. [#626](https://github.com/zowe/zowe-native-proto/pull/626)
- `c`: Fixed issue where `zowex uss chown` (and `zusf_chown_uss_file_or_dir`) silently succeeded with exit code `0` when a non-existent user or group was supplied. The command now validates `user:group` input and returns a non-zero exit code with a clear error message when invalid. [#565](https://github.com/zowe/zowe-native-proto/pull/565)
- `c`: Fixed issue where uploading changes to a data set did not always flush to disk. [#643](https://github.com/zowe/zowe-native-proto/issues/643)
- `c`: Updated commands that read data from stdin to read literal text rather than parsing hex string. [#645](https://github.com/zowe/zowe-native-proto/pull/645)
- `c`: Reverted previous fix to preserve ISPF stats on PDS members since it could cause uploading changes to fail. [#669](https://github.com/zowe/zowe-native-proto/pull/669)
- `zowed`: Fixed issue where enabling verbose logging would cause a deadlock during initialization. [#652](https://github.com/zowe/zowe-native-proto/issues/652)
- `zowed`: Implemented support for automatic worker recovery. If a worker crashes or throws an exception, it is replaced with a new worker. If the maximum number of replacement attempts have been exceeded, the worker is disabled to prevent thrashing the CPU with replacement requests. [#410](https://github.com/zowe/zowe-native-proto/issues/410)
- `zowed`: Optimized the `get_ready_worker` function to avoid worst-case linear search for the next available worker. [#651](https://github.com/zowe/zowe-native-proto/pull/651)
- `zowed`: Replaced the options logic with the parser library to avoid code duplication and establish consistency with backend. [#655](https://github.com/zowe/zowe-native-proto/issues/655)
- `zowed`: Implemented support for server-side request timeouts. If the request timeout is exceeded for a single worker, the hanging worker is replaced and the ongoing request is discarded. [#416](https://github.com/zowe/zowe-native-proto/issues/416)
- `c`: De-duplicated makefile contents through `.INCLUDE` keyword and separate toolchain file. [#651](https://github.com/zowe/zowe-native-proto/pull/651)
- `zowed`: Fixed issue where opening a file larger than 10 MB could fail with "Invalid JSON" error. [#656](https://github.com/zowe/zowe-native-proto/issues/656)
- `c`: Fix PSW alignment issue. [#559](https://github.com/zowe/zowe-native-proto/issues/559)
- When listing data sets with attributes, a comprehensive set is now retrieved that is similar to what ISPF displays. [#629](https://github.com/zowe/zowe-native-proto/issues/629)

## `0.2.0`

- `c`: Fixed issue where uploading changes to a PDS member removed its ISPF stats. [#556](https://github.com/zowe/zowe-native-proto/issues/556)
- `c`: Streamlined argument checks and access within command handlers. Handlers can now use `get`, `get_if` and `find` to search for arguments and retrieve their values, regardless of whether the argument is a keyword or positional argument. [#574](https://github.com/zowe/zowe-native-proto/pull/574)
- `c`: Command handlers can now be called directly by providing an `InvocationContext`. The context provides helper functions for error, input, and output stream redirection. Handlers can use this context to print output and errors, as well as setting content length. [#574](https://github.com/zowe/zowe-native-proto/pull/574)
- `c`: Organized command handlers into separate source files and namespaces in the `commands` folder. Reduced size of main entrypoint function by moving command registration to a `register_commands` function contained in each namespace. [#409](https://github.com/zowe/zowe-native-proto/issues/409)
- `native`: Added a `--depth` option to the `zowex uss list` command for listing USS directories recursively. [#575](https://github.com/zowe/zowe-native-proto/pull/575)
- `native`: Added `zowex job list-proclib` to list proclib concatenation.
- `c`: Fixed issue where a directory handle was leaked when running `chmod`, `chown`, `chtag` or `delete` USS commands. [#577](https://github.com/zowe/zowe-native-proto/issues/577)
- `c`: Fixed issue where a file handle was leaked if the `fldata` function call failed when listing data set members. [#577](https://github.com/zowe/zowe-native-proto/issues/577)
- `c`: Fixed issue where a file handle was leaked if the `fldata` function call failed in the `handle_data_set_compress` command handler. [#577](https://github.com/zowe/zowe-native-proto/issues/577)
- `c`: Fixed issue in the `zut_substitute_symbol` function where passing patterns with a length greater than the input buffer would cause a buffer overflow. [#577](https://github.com/zowe/zowe-native-proto/issues/577)
- `c`: Fixed undefined behavior in the `zut_bpxwdyn` function by verifying that the allocated 31-bit region is valid before writing to it. [#577](https://github.com/zowe/zowe-native-proto/issues/577)
- `c`: The CLI parser now prints unexpected, trailing positional arguments when handling command input. [#594](https://github.com/zowe/zowe-native-proto/issues/594)
- Rewrote the middleware (`zowed`) in C++. This removes our dependency on Golang, improves performance, and allows parallel requests to be handled in a single address space. [#595](https://github.com/zowe/zowe-native-proto/pull/595)
- `c`: Fixed issue where automatic codepage conversion with `_BPXK_AUTOCVT` was not disabled for all USS read and write operations, which could interfere with the `--encoding` option. [#620](https://github.com/zowe/zowe-native-proto/pull/620)

## `0.1.10`

- `c`: Added `zowex tool list-parmlib` command to list parmlib concatenation data sets.
- Added plug-in support to the `zowex` backend. Plug-ins can contribute commands that users invoke through `zowex`. For more information on how to create and register a plug-in with `zowex`, please refer to the `plugins.md` file in the `doc/` root-level folder. [#148](https://github.com/zowe/zowe-native-proto/issues/148)

## `0.1.9`

- `golang`: Fixed an issue where an empty response on the `HandleReadFileRequest` function would result in a panic. [#550](https://github.com/zowe/zowe-native-proto/pull/550)
- `c`: Fixed issue where the `zowex ds lm` command always returned non-zero exit code for warnings and ignored the `--no-warn` flag. [#498](https://github.com/zowe/zowe-native-proto/issues/498)
- `c`: Fixed issue where the `zowex job submit-jcl` command submitted the given JCL contents twice, causing two jobs to be created. [#508](https://github.com/zowe/zowe-native-proto/issues/508)
- `c`: Implemented a logger for Metal C and C++ source code for diagnostics, debug information, and printing dumps. When enabled, log messages are written to a log file named `zowex.log` in a new `logs` folder, relative to the location of the `zowex` binary. [#107](https://github.com/zowe/zowe-native-proto/issues/107)
- `golang`: Moved location of log file inside "logs" directory to be consistent with `zowex`. [#514](https://github.com/zowe/zowe-native-proto/pull/514)
- `c`: Fixed issue where the `zowex ds write` command automatically created a data set when it did not exist. [#292](https://github.com/zowe/zowe-native-proto/issues/292)
- `native`: Fixed issue where the `zowex ds ls` command could hang when listing data sets that the system cannot open. [#496](https://github.com/zowe/zowe-native-proto/issues/496)
- `c`: Added `--local-encoding` option for read and write operations on data sets, USS files, and job files to specify the source encoding of content (defaults to UTF-8). [#511](https://github.com/zowe/zowe-native-proto/issues/511)
- `c`: Fixed issue where the `zowex ds create` command did not parse `--alcunit` and integer arguments (e.g., `--primary`). [#414](https://github.com/zowe/zowe-native-proto/issues/414)
- `c`: Fixed issue where listing data sets fails if the `OBTAIN` service fails while obtaining attributes for a data set in the list of matches. [#529](https://github.com/zowe/zowe-native-proto/issues/529)
- `native`: Added support for `volser` option when reading/writing data sets. [#439](https://github.com/zowe/zowe-native-proto/issues/439)
- `native`: Reduced number of memory allocations in vectors by reserving capacity before adding elements. [#522](https://github.com/zowe/zowe-native-proto/issues/522)
- `c`: Added wrappers for Web Enablement Toolkit to be invoked via Metal C as header only or from LE-C using a 64-bit wrapper.
- `c`: Added the `zstd::optional` class for handling optional values.
- `c`: Added the `zstd::unique_ptr` class and `zstd::make_unique` function for RAII-based automatic memory management.
- `c`: Added the `zstd::expected` class for error handling similar to Rust `Result` type.

## `0.1.8`

- `native`: Added default value for `--recfm` so that when no options are specified the data set will not contain errors. [#493](https://github.com/zowe/zowe-native-proto/issues/493)
- Fixed issue where special characters were detected as invalid characters when provided to `zowex` commands. [#491](https://github.com/zowe/zowe-native-proto/issues/491)
- `native`: Increase default max returned entries in `zowex ds list` from 100 to 5000. This helps with [#487](https://github.com/zowe/zowe-native-proto/issues/487) but does not fix it. In the future, users should be able to specify on the Zowe Clients the max number of entries.

## `0.1.7`

- Updated CLI parser `find_kw_arg_bool` function to take in an optional boolean `check_for_negation` that, when `true`, looks for a negated option value. [#485](https://github.com/zowe/zowe-native-proto/issues/485)
- Fixed issue where listing data set members did not check for the negated option value. Now, the command handler passes the `check_for_negation` option to the `find_kw_arg_bool` function to check the value of the negated, equivalent option. [#485](https://github.com/zowe/zowe-native-proto/issues/485)
- `golang`: Fixed inconsistent type of the `data` property between the `ReadDatasetResponse` and `ReadFileResponse` types. [#488](https://github.com/zowe/zowe-native-proto/pull/488)

## `0.1.6`

- `native`: Fixed regression where data set download operations would fail due to a content length mismatch, due to the content length being printed as hexadecimal rather than decimal. [#482](https://github.com/zowe/zowe-native-proto/issues/482)

## `0.1.5`

- `native`: Added completion code for `POST` so that users of the library code may determine if a timeout has occurred.
- `native`: Added `timeout` for `zowex console issue` to prevent indefinite hang when no messages are returned. [#470](https://github.com/zowe/zowe-native-proto/pull/470)
- `native`: Added `contentLen` property to RPC responses for reading/writing data sets and USS files. [#358](https://github.com/zowe/zowe-native-proto/pull/358)
- `native`: Fixed file tag being prioritized over user-specified codepage when reading/writing USS files. [#467](https://github.com/zowe/zowe-native-proto/pull/467)
- `native`: Fixed issue where `max-entries` argument was incorrectly parsed as a string rather than an integer. [#469](https://github.com/zowe/zowe-native-proto/issues/469)
- `native`: The `zowex` root command now has a command handler to make adding new options easier. [#468](https://github.com/zowe/zowe-native-proto/pull/468)

## `0.1.4`

- `c`: Fixed an issue where the CLI help text showed the `[options]` placeholder in the usage example before the positional arguments, which is not a supported syntax. Now, the usage text shows the `[options]` placeholder after the positional arguments for the given command.
- `c`: Added `zowex version` command to print the latest build information for the `zowex` executable. The version output contains the build date and the package version. [#366](https://github.com/zowe/zowe-native-proto/issues/366)
- `c`: Added `full_status` variable from job output to the CSV output for the `zowex job view-status --rfc` command. [#450](https://github.com/zowe/zowe-native-proto/pull/450)
- `golang`: If the `zowed` process abnormally terminates due to a SIGINT or SIGTERM signal, the worker processes are now gracefully terminated. [#372](https://github.com/zowe/zowe-native-proto/issues/372)
- `c`: Updated `zowex uss list` command to provide same attributes as output from the `ls -l` UNIX command when the `--long` flag is specified. [#346](https://github.com/zowe/zowe-native-proto/issues/346)
- `c`: Updated `zowex uss list` command to match format of the `ls -l` UNIX command. [#383](https://github.com/zowe/zowe-native-proto/issues/383)
- `c`: Added `response-format-csv` option to the `zowex uss list` command to print the file attributes in CSV format. [#346](https://github.com/zowe/zowe-native-proto/issues/346)
- `golang`: Added additional data points to the USS item response for the `HandleListFilesRequest` command handler. [#346](https://github.com/zowe/zowe-native-proto/issues/346)

## `0.1.3`

- `c`: Fixed S0C4 where supervisor state, key 4 caller invokes `zcn_put` while running in SUPERVISOR state. [#392](https://github.com/zowe/zowe-native-proto/issues/392)
- `c`: Fixed S0C4 where supervisor state, key 4 caller invokes `zcn` APIs and several miscellaneous issues. [#389](https://github.com/zowe/zowe-native-proto/issues/389)
- `c`: Fixed issue where canceled jobs displayed as "CC nnnn" instead of the string "CANCELED". [#169](https://github.com/zowe/zowe-native-proto/issues/169)
- `c`: Fixed issue where input job SYSOUT files could not be listed or displayed". [#196](https://github.com/zowe/zowe-native-proto/issues/196)
- `native`: Fixed issue where `zowed` failed to process RPC requests larger than 64 KB. [#364](https://github.com/zowe/zowe-native-proto/pull/364)
- `golang`: `zowed` now returns the UNIX permissions for each item's `mode` property in a USS list response. [#341](https://github.com/zowe/zowe-native-proto/pull/341)
- `c`: Added `--long` option to the `zowex uss list` command to return the long format for list output, containing additional file metadata. [#341](https://github.com/zowe/zowe-native-proto/pull/341)
- `c`: Added `--all` option to the `zowex uss list` command to show hidden files when listing a UNIX directory. [#341](https://github.com/zowe/zowe-native-proto/pull/341)
- `c`: Added `ListOptions` parameter to the `zusf_list_uss_file_path` function to support listing the long format and hidden files. [#341](https://github.com/zowe/zowe-native-proto/pull/341)
- `c`: Fixed issue where the record format (`recfm` attribute) was listed as unknown for a Partitioned Data Set (PDS) with no members. Now, the record format for all data sets is retrieved through the Volume Table of Contents. [#351](https://github.com/zowe/zowe-native-proto/pull/351)
- `c`: Added CLI parser and lexer library for use in `zowex`. For an example of how to use the new CLI parser library, refer to the sample CLI code in `examples/native-cli/testcli.cpp`.
- `c`: Fixed an issue where the zowex `ds list` command always printed data set attributes when passing the argument `--response-format-csv`, even if the attributes argument was `false`.
- `c`: Fixed an issue where the `zusf_chmod_uss_file_or_dir` function did not handle invalid input before passing the mode to the `chmod` C standard library function. [#399](https://github.com/zowe/zowe-native-proto/pull/399)
- `c`: Refactored the Base64 encoder and decoder to remove external dependency. [#385](https://github.com/zowe/zowe-native-proto/issues/385)

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
