# Change Log

All notable changes to the Client code for "zowe-native-proto-sdk" are documented in this file.

Check [Keep a Changelog](http://keepachangelog.com/) for recommendations on how to structure this file.

## `0.1.9`

- Added a `promptForDeployDirectory` function to prompt the users to choose a deploy directory aside from the default.[#527] (https://github.com/zowe/zowe-native-proto/issues/527)
- Updated the `ZSshUtils.uninstallServer` function to remove the deploy directory and all of its contents.[#484] (https://github.com/zowe/zowe-native-proto/issues/484)
- Changed default number of `zowex` worker threads from 10 to 3 to reduce resource usage on z/OS. [#514](https://github.com/zowe/zowe-native-proto/pull/514)
- Added support for `localEncoding` option in data set, USS file, and job file operations to specify the source encoding of content (defaults to UTF-8). [#511](https://github.com/zowe/zowe-native-proto/issues/511)
- Added support for `volume` option when reading/writing data sets. [#439](https://github.com/zowe/zowe-native-proto/issues/439)
- Added a new `ConfigFileUtils` class with helper functions for commenting out properties, removing comments after the `properties` section of a profile, and restoring properties from comments to their original values. [#534](https://github.com/zowe/zowe-native-proto/issues/534)

## `0.1.7`

- Fixed inconsistent type of the `data` property between the `ReadDatasetResponse` and `ReadFileResponse` types. [#488](https://github.com/zowe/zowe-native-proto/pull/488)

## `0.1.6`

- Fixed a TypeError in `ZSshClient.request` that caused stream operations to fail if a callback was not provided for progress updates. [#482](https://github.com/zowe/zowe-native-proto/issues/482)

## `0.1.5`

- Added support for progress messages for USS files downloaded and uploaded via the CLI plug-in. [#426](https://github.com/zowe/zowe-native-proto/pull/426)
- Added support for listing the long format for USS files with the `long` request option. When provided, the SDK returns additional file attributes, including permissions, file tag, file size, number of links, owner, and group. [#346](https://github.com/zowe/zowe-native-proto/issues/346)
- Added support for listing all USS files with the `all` request option. When provided, the SDK includes hidden files in the list response. [#421](https://github.com/zowe/zowe-native-proto/pull/421)
- Refactored handling of RPC notifications to be managed by a separate class `RpcNotificationManager`. [#358](https://github.com/zowe/zowe-native-proto/pull/358)
- Added content length validation for streamed requests. [#358](https://github.com/zowe/zowe-native-proto/pull/358)
- Removed unnecessary prompt to unlock keyring on MacOS when connecting to a new host with a private key. [#453](https://github.com/zowe/zowe-native-proto/issues/453)

## `0.1.3`

- The `mode` property for a list files response now contains the UNIX permissions of the corresponding file/folder. [#341](https://github.com/zowe/zowe-native-proto/pull/341)
- Fixed an issue where a non-fatal `chdir` error (OpenSSH code `FOTS1681`) prevented the clients from handling the middleware's ready message. Now, the SSH client waits for the ready message from the `zowed` middleware before returning the new client connection.

## `0.1.2`

- Improved error handling when SSH client connects to throw `ENOTFOUND` if server binary is missing. [#44](https://github.com/zowe/zowe-native-proto/issues/44)
- Added `ZSshUtils.checkIfOutdated` method to check if deployed server is out of date. [#44](https://github.com/zowe/zowe-native-proto/issues/44)
- Added `keepAliveInterval` option when creating an instance of the `ZSshClient` class that defaults to 30 seconds. [#260](https://github.com/zowe/zowe-native-proto/issues/260)
- Added support for streaming data in chunks to handle large requests and responses in the `ZSshClient` class. This allows streaming USS file contents for read and write operations (`ReadDatasetRequest`, `ReadFileRequest`, `WriteDatasetRequest`, `WriteFileRequest`). [#311](https://github.com/zowe/zowe-native-proto/pull/311)

## `0.1.1`

- Improved unclear error message when uploading PAX file fails. [#220](https://github.com/zowe/zowe-native-proto/pull/220)

## `0.1.0`

- Added `ds.restoreDataset` function. [#38](https://github.com/zowe/zowe-native-proto/pull/38)
- Added support for cancelling jobs. [#138](https://github.com/zowe/zowe-native-proto/pull/138)
- Added support for holding and releasing jobs. [#182](https://github.com/zowe/zowe-native-proto/pull/182)
- Added `jobs.submitUss` function. [#184](https://github.com/zowe/zowe-native-proto/pull/184)

## [Unreleased]

- Initial release
