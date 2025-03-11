# Change Log

All notable changes to the Client code for "zowe-native-proto-cli" are documented in this file.

Check [Keep a Changelog](http://keepachangelog.com/) for recommendations on how to structure this file.

## Recent Changes

- Added `zssh restore dataset` command. [#38](https://github.com/zowe/zowe-native-proto/pull/38)
- Added `zssh view uss-file` command. [#38](https://github.com/zowe/zowe-native-proto/pull/38)
- Added `zssh list spool-files` command. [#38](https://github.com/zowe/zowe-native-proto/pull/38)
- Added support for cancelling jobs. [#138](https://github.com/zowe/zowe-native-proto/pull/138)
- Added support for holding and releasing jobs. [#182](https://github.com/zowe/zowe-native-proto/pull/182)
- Updated error handling for listing data sets. [#185](https://github.com/zowe/zowe-native-proto/pull/185)
- Added support for conflict detection through use of e-tags. When a data set or USS file is viewed, the e-tag is displayed to the user and can be passed for future write requests to prevent overwriting new changes on the target system. [#144](https://github.com/zowe/zowe-native-proto/issues/144)

## [Unreleased]

- Initial release
