# Change Log

All notable changes to the Client code for "zowe-native-proto-cli" are documented in this file.

Check [Keep a Changelog](http://keepachangelog.com/) for recommendations on how to structure this file.

## Recent Changes

- Added support for the `local-encoding` option to data set, USS file, and job file commands (view, download, upload) to specify the source encoding of content (defaults to UTF-8). [#511](https://github.com/zowe/zowe-native-proto/issues/511)
- Added support for `--volume-serial` option when uploading/downloading data sets. [#439](https://github.com/zowe/zowe-native-proto/issues/439)
- Fixed an issue where the `zssh config setup` command did not prompt the user for a password if the given private key was not recognized by the host. [#524](https://github.com/zowe/zowe-native-proto/issues/524)
- Added a new prompt that shows if the user has an invalid private key on an existing profile when running the `zssh config setup` command. Now, if an invalid private key is detected, it is moved to a new comment in the JSON file and the user is given options to proceed. They can undo the comment action, delete the comment entirely, or preserve the comment and succeed with setup. [#524](https://github.com/zowe/zowe-native-proto/issues/524)

## `0.1.5`

- Added support for progress messages for USS files downloaded and uploaded via the CLI plug-in. [#426](https://github.com/zowe/zowe-native-proto/pull/426)
- Added support for listing the long format for USS files with the `long` request option. When provided, the CLI plug-in prints additional file attributes, including permissions, file tag, file size, number of links, owner, and group. [#346](https://github.com/zowe/zowe-native-proto/issues/346)
- Added support for listing all USS files with the `all` request option. When provided, the CLI plug-in includes hidden files in the list response. [#421](https://github.com/zowe/zowe-native-proto/pull/421)
- Added support for the `encoding` option to the `zssh upload uss` and the `zssh upload ds` commands. When the `encoding` option is provided, the command uses the option value as the target encoding for the uploaded content. [#427](https://github.com/zowe/zowe-native-proto/issues/427)
- Added support for the `file` option to the `zssh download uss` and `zssh download ds` commands. When the `file` option is provided, the command uses the option value as the destination file path for the downloaded content. [#428](https://github.com/zowe/zowe-native-proto/issues/428)
- Adopted streaming for commands that upload/download data sets and USS files. [#358](https://github.com/zowe/zowe-native-proto/pull/358)

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
