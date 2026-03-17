# Native Development — AI Agent Guide

C/C++ code that compiles and runs on z/OS USS. The `zowex` binary serves as both a CLI tool and an RPC server (`zowex server`).

## z/OS Process Creation

On z/OS USS, `fork()` creates a **new address space** — expensive and slow. Prefer `spawn()` from `<spawn.h>` with `_BPX_SHAREAS=YES` to run child processes as a new TCB in the **same address space**.

- Use `zut_spawn_shell_command()` for shell commands (uses `spawn()` + `_BPX_SHAREAS=YES`)
- `zut_run_program()` exists but uses `fork()`/`execvp()` — avoid for new command execution paths

## Adding a New Native Command

1. **Handler**: Implement in `c/commands/<domain>.cpp`, declare in the matching `.hpp`
2. **CLI registration**: Add subcommand in the `register_commands()` function of the domain's `.cpp`
3. **RPC registration**: Register in `c/server/rpc_commands.cpp` using `CommandBuilder` with `.validate<Req, Resp>()`, `.rename_arg()`, and `.read_stdout()` as needed
4. **SDK types**: Define request/response interfaces in `packages/sdk/src/doc/rpc/<domain>.ts`
5. **SDK binding**: Wire up in `packages/sdk/src/RpcClientApi.ts`

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

## Test Framework

- CLI tests: `c/test/zowex.<domain>.test.cpp` — use `execute_command_with_output()`
- RPC server tests: `c/test/zowex.server.test.cpp` — use `start_server()`, `write_to_server()`, `read_line_from_server()`
- SDK tests: `packages/sdk/tests/` — use `vitest` with mocked SSH layer
