# Change Log

All notable changes to the Client code for "zowe-native-proto-cli" are documented in this file.

Check [Keep a Changelog](http://keepachangelog.com/) for recommendations on how to structure this file.

## Recent Changes

- Added support for listing the long format for USS files with the `long` request option. When provided, the CLI plug-in prints additional file attributes, including permissions, file tag, file size, number of links, owner, and group.
- Added support for listing all USS files with the `all` request option. When provided, the CLI plug-in includes hidden files in the list response.

## `0.1.1`

- Fixed issue where a jobs list request returns unexpected results whenever a search query does not match any jobs. [#217](https://github.com/zowe/zowe-native-proto/pull/217)

## `0.1.0`

- Added `zssh restore dataset` command. [#38](https://github.com/zowe/zowe-native-proto/pull/38)
- Added `zssh view uss-file` command. [#38](https://github.com/zowe/zowe-native-proto/pull/38)
- Added `zssh list spool-files` command. [#38](https://github.com/zowe/zowe-native-proto/pull/38)
- Added support for cancelling jobs. [#138](https://github.com/zowe/zowe-native-proto/pull/138)
- Added support for holding and releasing jobs. [#182](https://github.com/zowe/zowe-native-proto/pull/182)
- Updated error handling for listing data sets. [#185](https://github.com/zowe/zowe-native-proto/pull/185)
- Added `zssh submit uss-file|local-file|data-set` commands. [#184](https://github.com/zowe/zowe-native-proto/pull/184)
- Added support for conflict detection through use of e-tags. When a data set or USS file is viewed, the e-tag is displayed to the user and can be passed for future write requests to prevent overwriting new changes on the target system. [#144](https://github.com/zowe/zowe-native-proto/issues/144)

## [Unreleased]

- Initial release
