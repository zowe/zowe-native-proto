# Native Development — AI Agent Guide

C/C++ code that compiles and runs on z/OS USS. The `zowex` binary serves as both a CLI tool and an RPC server (`zowex server`).

## z/OS Process Creation

On z/OS USS, `fork()` creates a **new address space** — expensive and slow. Prefer `spawn()` from `<spawn.h>` with `_BPX_SHAREAS=YES` to run child processes as a new TCB in the **same address space**.

- Use `zut_spawn_shell_command()` for shell commands (uses `spawn()` + `_BPX_SHAREAS=YES`)
- `zut_run_program()` exists but uses `fork()`/`execvp()` — avoid for new command execution paths

**Identity-changing commands (newgrp, su, sg):** Commands that change real/effective UID or GID are not supported in a shared address space ([IBM newgrp doc](https://www.ibm.com/docs/en/zos/3.2.0?topic=descriptions-newgrp-change-new-group)). `zut_spawn_shell_command()` detects when the user's command is one of `newgrp`, `su`, or `sg` (by parsing the first token) and in that case does **not** pass `_BPX_SHAREAS=YES` to the child, so the child runs in its own address space. To support additional such commands, add the command basename to `ZUT_NOSHAREAS_COMMANDS` in `zut.cpp`.

## Adding a New Native Command

1. **Handler**: Implement in `c/commands/<domain>.cpp`, declare in the matching `.hpp`
2. **CLI registration**: Add subcommand in the `register_commands()` function of the domain's `.cpp`
3. **RPC registration**: Register in `c/server/rpc_commands.cpp` using `CommandBuilder` with `.validate<Req, Resp>()`, `.rename_arg()`, and `.read_stdout()` as needed
4. **SDK types**: Define request/response interfaces in `packages/sdk/src/doc/rpc/<domain>.ts`
5. **SDK binding**: Wire up in `packages/sdk/src/RpcClientApi.ts`
6. **Schema regen**: Run `npm run generateTypes` (or `npx tsx scripts/generateTypes.ts`) to regenerate `native/c/server/schemas/requests.hpp` and `responses.hpp` from the TypeScript types

## Build & Test Workflow

All `npm run z:*` commands work from the repo root and target the remote z/OS system configured in `config.yaml`.

| Command | Purpose |
|---------|---------|
| `npm run z:upload` | Upload source files to z/OS |
| `npm run z:build` | Compile on z/OS |
| `npm run z:rebuild` | Clean + build |
| `npm run z:test` | Build all + run native tests |
| `npm run z:make all tests` | Build everything including test binaries |

After modifying native source or test files, run `npm run z:upload` before `npm run z:test` to ensure the latest code is compiled on z/OS.

The `zowex` binary on z/OS is at `<deployDir>/c/build-out/zowex` (deployDir is in `config.yaml`, typically `~/zowe-native-proto`).

### Deploying native changes end-to-end

`z:test` builds the test binary but does **not** update the main `zowex` binary used by the SDK. To fully deploy:

```sh
npm run z:rebuild              # upload + compile on z/OS
npm run z:artifacts            # download fresh zowex into packages/sdk/bin/
npx zowe zssh server install   # push new binary to z/OS and activate it
```

## Test Framework

- CLI tests: `c/test/zowex.<domain>.test.cpp` — use `execute_command_with_output()` or `execute_command()`
- RPC server tests: `c/test/zowex.server.test.cpp` — use `start_server()`, `write_to_server()`, `read_line_from_server()`
- SDK tests: `packages/sdk/tests/` — use `vitest` with mocked SSH layer

### `execute_command` vs `execute_command_with_output`

| Function | Stderr | Use when |
|----------|--------|----------|
| `execute_command_with_output(cmd, output)` | Merged into `output` via `2>&1` | Output is all on stdout; no need to distinguish |
| `execute_command(cmd, stdout, stderr)` | Captured separately | Command writes data to stdout and metadata/diagnostics to stderr (e.g. `system view-syslog`) |

Using `execute_command_with_output` for commands that write syslog text to stdout and metadata to stderr will mix both streams into one string, causing line-count assertions to fail.

### Test filter scope

`npm run z:test -- "<filter>"` matches against `it()` description strings only, not the enclosing `describe()` name. If a test suite has many `it()` calls without the filter word, use a word that appears in most of them:

```sh
npm run z:test -- "syslog"        # matches only it() descriptions containing "syslog"
npm run z:test -- "endDate"       # matches end-timestamp tests
npm run z:test -- "end timestamp" # matches chronological ordering tests
npm run z:test -- "pagination"    # matches pagination test
```

## EBCDIC Encoding in C++ (ibm-clang++64)

`ibm-clang++64` on z/OS USS compiles with **ASCII source encoding**. Character literals in source code (e.g. `'0'`, `':'`) are ASCII values (0x30, 0x3A). However, data read from VSAM datasets is raw **EBCDIC** bytes.

When parsing EBCDIC text buffers in C++, use explicit hex constants rather than character literals:

| Character | EBCDIC hex |
|-----------|-----------|
| Digits `0`–`9` | `0xF0`–`0xF9` |
| Space | `0x40` |
| Colon `:` | `0x7A` |
| Dot `.` | `0x4B` |

Newlines appended by C++ code (e.g. `response.append(1, '\n')`) are ASCII `0x0A` even though the surrounding data is EBCDIC — they are injected by the C++ runtime, not read from VSAM.

`sscanf` and `sprintf` operate in ASCII on `ibm-clang++64` z/OS binaries. They cannot directly parse EBCDIC digit strings; convert digits manually using `ch - 0xF0`.

## Metal C (`#pragma metal`)

Metal C files (compiled with `xlc -W"c,metal,..."`) can use:

- Inline z/OS assembler instructions
- z/OS system services like `stckconv` (TOD clock conversion)
- Headers like `ztime.h` which define `TIME_STRUCT`, `stckconv`, and related model macros

**Metal C headers (e.g. `ztime.h`) must not be included in regular C++ files.** They define assembly model variable declarations (`dsa_stckconv_model`, etc.) that only make sense in Metal C translation units. Including them in `ibm-clang++64` C++ sources causes `undeclared identifier` errors.

**Pattern:** If a function needs `stckconv` or `TIME_STRUCT`, put it in a Metal C file (e.g. `zdsm.c`) and expose it via a plain `extern "OS"` declaration in the corresponding `.h` header. Regular C++ code calls the Metal C wrapper.

## Struct Field Conventions — Boolean Flags

Boolean flags inside structs **must** use `unsigned int field : 1;` bitfields, not `int` or `bool`. This is required for structs passed between Language Environment (LE) and Metal C, where only primitive types are allowed, and is the established pattern throughout the codebase.

```cpp
// Correct
unsigned int dynalloc : 1; // indicates that the data set was dynamically allocated
unsigned int has_more  : 1;

// Incorrect — do not use int or bool for flags in structs
int has_more;
bool has_more;
```

Group related bitfields together under a comment block (see `zrecovery.h`, `zamtypes.h`, `zdstype.h` for examples).

## Function Parameter Count

Prefer a dedicated options/result struct when a function has more than ~5 parameters, or when it has multiple output-only reference parameters. Follow the existing patterns in the codebase:

| Pattern | Example | Use for |
|---------|---------|---------|
| Input options only | `CopyOptions`, `ListOptions` (`zusf.hpp`) | Grouping boolean/config inputs |
| Input + output combined | `ZDSCopyOptions` (`zds.hpp`) | Functions that both take options and return multiple results |

Structs replace scattered `bool &out_flag`, `std::string &out_val` parameters and make call sites easier to read and extend without signature changes.

```cpp
// Before — too many params, hard to extend
int zjb_read_syslog(ZJB *zjb, std::string &response, std::string &date,
    std::string &time, int max_lines, bool &has_more,
    std::string &end_date, std::string &end_time);

// After — clear, extensible
struct ZJBSyslogOptions {
  std::string date;     // input
  std::string time;     // input
  int max_lines = 0;    // input
  bool has_more = false; // output
  std::string end_date; // output
  std::string end_time; // output
  int returned_lines = 0; // output
};
int zjb_read_syslog(ZJB *zjb, std::string &response, ZJBSyslogOptions &opts);
```

## z/OS System Log (Syslog) — VSAM Special Interface

The z/OS system log is stored as a VSAM KSDS and accessed via the **logical syslog special interface** (documented at [IBM z/OS VSAM special processing](https://www.ibm.com/docs/en/zos/3.2.0?topic=interface-special-processing-logical-syslog-data-sets)).

### Key facts

- The special interface returns **formatted text records** (EBCDIC), not raw binary VSAM records. The ETOD key is stripped from the returned data; it is not in the record buffer.
- The RPL (`IFGRPL`) fields (`rplrbar.rplrbarx`, `rplarg`) are used to set the **POINT** key before reading, but they are **not reliably updated** with the current record's key after a sequential GET. Do not attempt to extract the timestamp from the RPL after a GET.
- To obtain the timestamp of the last returned record, parse the syslog text itself (see below).
- POINT uses `FIRST_OCCURRENCE` with an ETOD key built from a `TIME_STRUCT` via `convetod()`.

### Syslog record text format

Each message line in the syslog output follows this layout (all fields EBCDIC):

```
<type> <seqno>  <sysname>     <YYDDD> <HH:MM:SS.th> <rest of message...>
  1      7          4             5          11
  0      2          10            19         25
```

- **Position 0**: Record type (`M`, `N`, `R`, `I`, `D`, `S`, `E`, …)
- **Positions 2–8**: Sequence number (7 chars)
- **Positions 10–13**: System name (4 chars, blank-padded)
- **Positions 14–18**: Padding spaces (5 chars)
- **Positions 19–23**: Julian date `YYDDD` (2-digit year + 3-digit day-of-year)
- **Position 24**: Space
- **Positions 25–35**: Time of day `HH:MM:SS.th` (11 chars)

Continuation lines (types `D`, `S`, `E`) do **not** have a date/time at these positions — those columns are spaces.

### Adaptive timestamp parsing (do not use fixed columns)

Because the column layout can theoretically vary (or continuation lines can appear at the end of a batch), the correct approach is to **search within each line for the pattern `YYDDD HH:MM:SS.th`** rather than reading fixed columns. See `parse_syslog_last_timestamp()` in `native/c/zjb.cpp` for the reference implementation.

The pattern search:
1. Iterates backwards over lines (most-recent-last)
2. Within each line scans for: 5 EBCDIC digits + EBCDIC space + time whose structural characters (hour tens 0–2, colon at +2, minute tens 0–5, colon at +5, second tens 0–5, dot at +8) all match and the Julian day is in range 1–366
3. On a match, decodes the Julian date to `yyyy-mm-dd` using `mktime()` and converts the time digits from EBCDIC to ASCII

### Default `maxLines` behaviour

| Invocation | Default `maxLines` |
|------------|-------------------|
| `--seconds-ago N` or no arguments (implies 30 s) | 10 000 (system max) |
| Explicit `--date`/`--time` | 100 |

`maxLines` is capped at 10 000 (validated in the command handler).

## Output Stream Convention (`context.output_stream` vs `context.error_stream`)

For commands that return both structured data and human-readable metadata:

- Write the **data payload** (syslog text, file contents, etc.) to `context.output_stream()` — this is captured by `CommandBuilder::read_stdout()` and stored in the JSON-RPC `data` field.
- Write **diagnostic or summary lines** (e.g. the `Start: … | End: … | Lines: …` footer) to `context.error_stream()` — this goes to stderr and is not included in the JSON-RPC response body.
- The `context.is_redirecting_output()` guard prevents the summary line from being printed when the command is invoked in RPC mode (where only the JSON object matters).

## JSON Schema Auto-generation

`native/c/server/schemas/requests.hpp` and `responses.hpp` are **auto-generated** from the TypeScript type definitions in `packages/sdk/src/doc/rpc/`. After adding or changing fields in those TypeScript files, regenerate:

```sh
npx tsx scripts/generateTypes.ts
```

Never edit the generated `.hpp` files by hand — they will be overwritten.
