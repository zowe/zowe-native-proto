# Zowe Native Proto — AI Agent Guide

An open-source, native protocol for z/OS mainframe operations via SSH with minimal server-side configuration.

## Architecture Overview

```txt
Client (local machine)              z/OS Server (remote)
┌──────────────────────┐            ┌───────────────────────────┐
│  VS Code Extension   │            │  zowex CLI binary (C++)   │
│  Zowe CLI Plug-in    │──── SSH ──▶│  I/O Server (zowex server)│
│  Node.js SDK         │            │  C++ backend libraries    │
└──────────────────────┘            └───────────────────────────┘
```

The SDK sends JSON-RPC requests over SSH to the `zowex server` process running on z/OS. The server dispatches commands to native C++ handlers that interact with mainframe resources (data sets, USS files, jobs, TSO, system log).

## Repository Structure

| Path | Description |
|------|-------------|
| `packages/sdk/` | Node.js SDK — TypeScript client that communicates with the z/OS server over SSH |
| `packages/cli/` | Zowe CLI plug-in — wraps the SDK for command-line use |
| `packages/vsce/` | VS Code extension — integrates with Zowe Explorer |
| `native/c/` | z/OS native code — C/C++ compiled and run on z/OS USS |
| `native/c/commands/` | Command handlers organized by domain (ds, uss, job, tso, system, etc.) |
| `native/c/server/` | RPC server and command dispatcher |
| `native/c/test/` | Native test suite (runs on z/OS) |
| `scripts/` | Build tooling (TypeScript) for z/OS upload, compile, test |
| `config.yaml` | z/OS connection profile and deploy directory |

## Key Concepts

- **RPC dispatch**: `native/c/server/rpc_commands.cpp` maps JSON-RPC method names to native handler functions
- **SDK bindings**: `packages/sdk/src/RpcClientApi.ts` maps client API methods to RPC method names
- **SDK types**: `packages/sdk/src/doc/rpc/` contains TypeScript interfaces for each RPC request/response
- **Command domains**: ds (data sets), uss (USS files), job (JES jobs), tso (TSO commands), system (system log, symbols, etc.), tool (utilities)

## SDK API Groups

The SDK client (`ZSshClient`) exposes API groups as properties:

| Property | Domain | Example method |
|----------|--------|----------------|
| `client.ds` | Data sets | `client.ds.listDataSets(req)` |
| `client.uss` | USS files | `client.uss.listFiles(req)` |
| `client.jobs` | JES jobs | `client.jobs.listJobs(req)` |
| `client.tso` | TSO commands | `client.tso.issueCommand(req)` |
| `client.system` | System operations | `client.system.viewSyslog(req)` |

## System Log (Syslog) API

`client.system.viewSyslog(req)` reads the z/OS system log via the VSAM logical syslog special interface.

### Request (`ViewSyslogRequest`)

| Field | Type | Description |
|-------|------|-------------|
| `date` | `string?` | Start date in `yyyy-mm-dd` format |
| `time` | `string?` | Start time in `hh:mm:ss.th` format |
| `secondsAgo` | `number?` | Start N seconds before current z/OS time; mutually exclusive with `date`/`time` |
| `maxLines` | `number?` | Maximum lines to return (1–10000). Default: 100 for absolute time, 10000 for `secondsAgo` |

When no arguments are given, defaults to the last 30 seconds (`--seconds-ago 30`).

### Response (`ViewSyslogResponse`)

| Field | Type | Description |
|-------|------|-------------|
| `data` | `string?` | Syslog text content (EBCDIC-decoded to ASCII/UTF-8) |
| `startDate` | `string?` | Actual start date of the returned window (`yyyy-mm-dd`) |
| `startTime` | `string?` | Actual start time of the returned window (`hh:mm:ss.th`) |
| `endDate` | `string?` | Date of the last returned syslog record (`yyyy-mm-dd`) |
| `endTime` | `string?` | Time of the last returned syslog record (`hh:mm:ss.th`) |
| `returnedLines` | `number?` | Number of lines returned |
| `hasMore` | `boolean?` | Whether more records are available after the returned window |

### Pagination

When `hasMore` is true, use `endDate`/`endTime` as the `--date`/`--time` for the next call to continue from where the previous call ended.

### Metadata display

Both the native CLI (`zowex`) and the TypeScript CLI (`zowe zssh`) print a one-line summary to stderr after the syslog text:

```
Start: 2026-03-24 00:00:00.00 | End: 2026-03-24 00:01:23.45 | Lines: 100 | Has more: yes
To see more, run:
  zowe zssh system view-syslog --date 2026-03-24 --time 00:01:23.45 --max-lines 500
```

## Build & Test

### Client packages (local)

```sh
npm install          # install dependencies
npm run build        # build SDK, CLI, and extension
npm run test         # run SDK/CLI/extension tests (vitest)
```

### SDK packaging

The SDK package (`packages/sdk/`) contains both TypeScript client code and the native z/OS server binary (`bin/server.pax.Z`). How to rebuild depends on what changed:

- **TypeScript only**: `npm run build --workspace=packages/sdk` (runs `tsc -b`)
- **Native only** (testing via `serverPath`): `npm run z:rebuild` — no SDK repackage needed, just point your SSH profile's `serverPath` to the remote `<deployDir>/c/build-out`
- **Full SDK package** (`.tgz` with bundled binaries): `npm run z:rebuild && npm run z:artifacts && npm run build --workspace=packages/sdk && mkdir -p dist && npm run package --workspace=packages/sdk`

### Native code (remote z/OS)

All `npm run z:*` commands target the z/OS system in `config.yaml`:

```sh
npm run z:upload     # upload native source to z/OS
npm run z:build      # compile on z/OS
npm run z:rebuild    # upload + build
npm run z:test       # build + run native tests on z/OS
```

For native-specific development patterns, see [native/AGENTS.md](native/AGENTS.md).

### Deploy a rebuilt server to z/OS

After changing native C++ code, the full cycle to get it live is:

```sh
npm run z:rebuild              # upload source and compile on z/OS
npm run z:artifacts            # download fresh zowex binary into packages/sdk/bin/
npx zowe zssh server install   # deploy the new binary via SSH
```

`z:test` builds the test binary but does **not** update the main `zowex` binary used by the SDK. Always run `z:artifacts` → `server install` to deploy a new server.

## PR Build Artifacts

PR builds run via the **Build** workflow. Find the run ID with:

```sh
gh run list --branch <branch-name> --limit 5
```

The Build workflow produces three artifacts:

- `zowe-native-proto-vsce` — the VS Code extension VSIX
- `zowe-native-proto-cli` — the CLI package
- `zowe-native-proto-sdk` — the SDK package (npm tarball)

### Downloading artifacts

List artifacts for a specific run:

```sh
gh api repos/zowe/zowe-native-proto/actions/runs/<RUN_ID>/artifacts --jq '.artifacts[] | "\(.name) \(.id)"'
```

Download by artifact name:

```sh
gh run download <RUN_ID> --name zowe-native-proto-vsce --dir /tmp/vsce-artifact
gh run download <RUN_ID> --name zowe-native-proto-cli --dir /tmp/cli-artifact
gh run download <RUN_ID> --name zowe-native-proto-sdk --dir /tmp/sdk-artifact
```

### Finding the PR build run

If you know the PR number, find the Build run via:

```sh
gh pr checks <PR_NUMBER>
```

The Build workflow URL contains the run ID (the numeric segment after `/runs/`).

## SonarCloud PR Quality Gate

After creating or pushing to a pull request, check SonarCloud for issues:

1. Wait ~1 minute for the analysis to complete
2. Fetch issues via the SonarCloud API:

    ```
    https://sonarcloud.io/api/issues/search?pullRequest={PR_NUMBER}&projects=zowe_zowe-native-proto&statuses=OPEN,CONFIRMED
    ```

3. If issues are found, fix them, commit, and push
4. Re-check until the quality gate passes (zero new issues)

### Common SonarCloud rules for this project

- `typescript:S7785` — Prefer async IIFE or top-level await over `.then()`/`.catch()` promise chains

## Changelog & Versioning

### Before updating a changelog

1. **Check released versions first** — run `gh release list --repo zowe/zowe-native-proto --limit 5` to see what has already been released.
2. **Never add entries to an already-released version** — if the version in the changelog has a corresponding GitHub release, it is frozen.
3. **Compare with `package.json`** — the `version` field in `package.json` may still show the last released version; this does not mean it is unreleased.

### Adding new entries

- Unreleased changes **must** go under a `## Recent Changes` header (not a version number). The CI changelog check (`awharn/check_changelog_action`) requires this exact header.
- If a `## Recent Changes` section already exists at the top, add the entry there.
- If the top section is an already-released version, create a new `## Recent Changes` section above it.
- Follow the existing bullet-point format: `- Description of change. [#issue](url)`.
- The `## Recent Changes` header is replaced with a version number at release time by the version bump process.

### Version bumps

Version bumps across `package.json` files are handled by a separate CI step (`Bump version to x.y.z [ci skip]`). Do not manually change `version` fields in `package.json` unless explicitly asked.

## z/OS SSH File Transfer & Encoding

The SDK deploys the server binary (`server.pax.Z`) to z/OS over SFTP (`sftp.fastPut`), which transfers binary data without EBCDIC conversion.

### SFTP vs SCP on z/OS

| Method | Binary-safe by default? | Notes |
|--------|------------------------|-------|
| **SFTP** (`sftp.fastPut`) | Yes | Transfers binary by default. Supports progress callbacks. Used by `installServer`. |
| **SCP** | **No** | Performs automatic EBCDIC-to-ASCII conversion on z/OS. Will corrupt binary files like `.pax.Z`. Never use for binary transfers. |

### `_BPXK_AUTOCVT` environment variable

Controls automatic EBCDIC conversion for tagged files on z/OS USS:

- `OFF` — no automatic conversion (safe for binary)
- `ON` — converts tagged files during I/O
- `ALL` — converts all files including untagged ones

### Native binary: build vs. run z/OS level

The native `zowex` binary is built with **Open XL C/C++** (`ibm-clang++64`, C++17) and links against the C++ runtime (e.g. CRTEQCXE) on the **build** system. If the binary is built on a newer z/OS level and run on an older one (e.g. z/OS 2.5), the loader may fail with **CEE3561S** (e.g. `External function _ZNSt5__1_e13__hash_memoryEPKvm was not found in DLL CRTEQCXE`) because the older Language Environment runtime does not export that symbol.

**Ways to make it work across z/OS release levels:**

1. **Build on the minimum supported z/OS** — Produce the shipped binary (or a "compatibility" build) on a system that matches the oldest supported level. The link step will then only reference symbols present in that LE runtime. No compiler flag; the build host's LE determines what gets linked.
2. **Link against older LE on the build system** — If Language Environment libraries from the older release are available on the build system, point the linker at them when building the "portable" binary (e.g. via `LDFLAGS` / library path / environment used by `ld`). Requires access to those datasets and the exact mechanism (LIBPATH, `-L`, or batch DD overrides) depends on your setup.
3. **Upgrade runtime on the older system** — Apply the latest PTFs for Language Environment and Open XL C/C++ runtime on the target system (e.g. PH45536 and any CRTE updates). Newer runtimes may add the missing symbol so the existing binary runs without a rebuild.

There is no single documented Open XL option (e.g. "target z/OS 2.5") that forces the binary to use only older runtime symbols; compatibility is achieved by build environment or runtime level on the target.

### Expired password detection

When an SSH session connects with an expired password, z/OS emits `FOTS1668`/`FOTS1669` on stderr. This can masquerade as SFTP failures or generic command errors. `ZSshUtils` checks for these codes after every `execCommand` call and throws an `ImperativeError` with `errorCode: "EPASSWD_EXPIRED"` so the client gets a clear, actionable message instead of a cryptic failure.

## DSLEVEL Pattern (List Data Sets)

The dataset list pattern is called DSLEVEL. It is not the same as grep regex or Windows filename masks; qualifiers are separated by dots and wildcards apply per qualifier or across qualifiers.

Rules:

- Pattern must not begin with a wildcard (first qualifier must be literal, e.g. `USER` or `MY.HIGH.LEVEL`).
- Maximum length 44 characters.

Wildcards:

- `%` — any single character in that position (e.g. `USER.T%ST` matches `USER.TEST`).
- `*` — any characters within that one qualifier only (e.g. `USER.J*.OLD` matches `USER.JCL.OLD` but not `USER.JCL.VERY.OLD`).
- `**` — any characters across any number of qualifiers (e.g. `USER.**.OLD` matches both `USER.JCL.OLD` and `USER.JCL.VERY.OLD`).

## Maintaining This Guide

When making changes that involve **important learnings or architectural decisions**, update this `AGENTS.md` (or the relevant sub-project `AGENTS.md` such as `native/AGENTS.md`) as part of the same change. Examples of what to capture:

- Platform-specific gotchas (e.g. z/OS encoding, SSH subsystem availability)
- Architectural patterns and their rationale (e.g. fallback strategies, retry logic)
- Non-obvious constraints discovered during implementation
- API quirks in dependencies (e.g. `node-ssh` exposing `connection` as a public property)

Keep entries concise and factual. Group related knowledge under a descriptive `##` heading. This guide is consumed by AI agents and human developers alike — accuracy matters more than prose.
