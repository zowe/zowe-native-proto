# Change Log

All notable changes to the Client code for "zowe-native-proto-sdk" are documented in this file.

Check [Keep a Changelog](http://keepachangelog.com/) for recommendations on how to structure this file.

## Recent Changes

- Improved error handling when SSH client connects to throw `ENOTFOUND` if server binary is missing. [#44](https://github.com/zowe/zowe-native-proto/issues/44)
- Added `ZSshUtils.checkIfOutdated` method to check if deployed server is out of date. [#44](https://github.com/zowe/zowe-native-proto/issues/44)

## `0.1.1`

- Improved unclear error message when uploading PAX file fails. [#220](https://github.com/zowe/zowe-native-proto/pull/220)

## `0.1.0`

- Added `ds.restoreDataset` function. [#38](https://github.com/zowe/zowe-native-proto/pull/38)
- Added support for cancelling jobs. [#138](https://github.com/zowe/zowe-native-proto/pull/138)
- Added support for holding and releasing jobs. [#182](https://github.com/zowe/zowe-native-proto/pull/182)
- Added `jobs.submitUss` function. [#184](https://github.com/zowe/zowe-native-proto/pull/184)

## [Unreleased]

- Initial release
