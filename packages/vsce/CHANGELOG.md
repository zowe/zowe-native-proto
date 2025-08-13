# Change Log

All notable changes to the "zowe-native-proto-vsce" extension will be documented in this file.

Check [Keep a Changelog](http://keepachangelog.com/) for recommendations on how to structure this file.

## `0.1.7`

- Fixed regression in performance where opening data sets and USS files in the editor could be slow. [#488](https://github.com/zowe/zowe-native-proto/pull/488)

## `0.1.6`

- Fixed regression where USS download operations would fail because of a missing callback function. [#482](https://github.com/zowe/zowe-native-proto/issues/482)

## `0.1.5`

- Implemented the `getTag` function in the `SshUssApi` class to return the tag of a UNIX file. Now, the VS Code extension automatically detects the file tag before the file is opened, and the detected encoding is shown in the "Open with Encoding" menu in Zowe Explorer. [#346](https://github.com/zowe/zowe-native-proto/issues/346)
- Adopted streaming for API methods that upload/download data sets and USS files. [#358](https://github.com/zowe/zowe-native-proto/pull/358)

## `0.1.2`

- Enhanced SSH profile validation to auto-deploy server when it is missing or out of date. [#44](https://github.com/zowe/zowe-native-proto/issues/44)
- Added keep-alive messages to keep SSH connection active. Their frequency can be controlled with the "Keep Alive Interval" option. [#260](https://github.com/zowe/zowe-native-proto/issues/260)

## `0.1.1`

- Fixed issue where a jobs list request returns unexpected results whenever a search query does not match any jobs. [#217](https://github.com/zowe/zowe-native-proto/pull/217)
- Fixed issue where data set save requests sometimes resulted in an unexpected conflict error. [#219](https://github.com/zowe/zowe-native-proto/pull/219)
- Fixed issue where Server Install Path setting did not work. [#220](https://github.com/zowe/zowe-native-proto/pull/220)

## `0.1.0`

- Fixed issue where `Open with Encoding: Binary` for MVS and USS files did not pass the correct encoding value to the server. [#61](https://github.com/zowe/zowe-native-proto/pull/61)
- Added support for cancelling jobs. [#138](https://github.com/zowe/zowe-native-proto/pull/138)
- Added support for running MVS commands. [#138](https://github.com/zowe/zowe-native-proto/pull/138)
- Updated error handling for listing data sets. [#185](https://github.com/zowe/zowe-native-proto/pull/185)
- Added support for conflict detection through use of e-tags. When a data set or USS file is opened, the e-tag is received by the VS Code extension and used in future write requests to prevent overwriting new changes on the target system. [#144](https://github.com/zowe/zowe-native-proto/issues/144)

## [Unreleased]

- Initial release
