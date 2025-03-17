# Change Log

All notable changes to the "zowe-native-proto-vsce" extension will be documented in this file.

Check [Keep a Changelog](http://keepachangelog.com/) for recommendations on how to structure this file.

## TBD Release

- Fixed issue where a jobs list request returns unexpected results whenever a search query does not match any jobs. [#217](https://github.com/zowe/zowe-native-proto/pull/217)

## `0.1.0`

- Fixed issue where `Open with Encoding: Binary` for MVS and USS files did not pass the correct encoding value to the server. [#61](https://github.com/zowe/zowe-native-proto/pull/61)
- Added support for cancelling jobs. [#138](https://github.com/zowe/zowe-native-proto/pull/138)
- Added support for running MVS commands. [#138](https://github.com/zowe/zowe-native-proto/pull/138)
- Updated error handling for listing data sets. [#185](https://github.com/zowe/zowe-native-proto/pull/185)
- Added support for conflict detection through use of e-tags. When a data set or USS file is opened, the e-tag is received by the VS Code extension and used in future write requests to prevent overwriting new changes on the target system. [#144](https://github.com/zowe/zowe-native-proto/issues/144)

## [Unreleased]

- Initial release
