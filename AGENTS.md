# Zowe Native Protocol - AGENTS.md

## Project Overview

Zowe Native Protocol (ZNP) is an SSH-based stack for modernizing mainframe access through direct system calls. It provides a high-performance alternative to z/OSMF REST APIs by leveraging Metal C for low-level operations.

**Key principle:** Direct system calls via Metal C bypass TSO/REXX overhead - all mainframe operations use native z/OS APIs (VSAM, QSAM, JES).

## Architecture

### Four-Layer Design

```
TypeScript Client Layer (packages/sdk/, packages/cli/, packages/vsce/)
    ↓ SSH + JSON-RPC
C++ Middleware Layer (native/zowed/)
    ↓ Command invocation
C++ Backend Layer (native/c/)
    ↓ System calls
z/OS System Services (Data Sets, USS, JES, MVS Console)
```

**Call flow example:**

```
client.ds.listDatasets() → JSON-RPC over SSH → zowed dispatcher → zowex ds list → VSAM/QSAM APIs
```

## Critical Concepts

### Compiler and Language Features

Two different compilers with different C++ support levels:

- **`native/c/` (xlc)**: Partial C++11 support - `auto`, `long long`, some features via `tr1` namespace
- **`native/zowed/` (xlclang)**: Full C++17 support - modern C++ features available

**Critical:** Backend code is limited to C++11 subset; middleware can use modern C++17.

### Memory Management - Metal C vs LE

Two execution environments with different memory rules:

- **Language Environment (LE)**: Standard C++ with RAII, exceptions, STL
- **Metal C**: No LE services, manual memory management, no exceptions
- **31-bit vs 64-bit**: Some system services require 31-bit storage below the bar

**Critical:** Metal C code cannot use LE features - no exceptions, no STL, manual cleanup required.

👉 **Full details:** [native/README.md](native/README.md)

### Error Handling

- **TypeScript SDK validates** parameters before sending requests
- **zowed middleware** parses JSON-RPC, routes commands, handles encoding
- **C++ backend** uses return codes (`RTNCD_SUCCESS`, `RTNCD_FAILURE`)
- **Metal C** uses z/OS return codes and ABEND recovery

👉 **Full details:** [doc/add-new-command.md](doc/add-new-command.md)

### Encoding and Data Transfer

- **Text files**: Converted from EBCDIC (IBM-1047) to UTF-8 for clients
- **Binary data**: Base64 encoded for transmission over SSH
- **Large files**: Use FIFO pipes for streaming to avoid memory overhead
- **Codepage support**: Configurable source/target encodings per request

👉 **Full details:** [doc/server/zowed_architecture.md](doc/server/zowed_architecture.md)

## File Organization

### Naming Convention

```
C++ Backend: zds.cpp, zds.hpp → Commands: commands/ds.cpp → Middleware: register_ds_commands()
TypeScript: RpcClientApi.ds.* → CLI: zssh ds list → VS Code: SshMvsApi
```

### Key Directories

**Client Packages:**

- `packages/sdk/` - TypeScript SDK with ZSshClient and RPC types
- `packages/cli/` - Zowe CLI plug-in (`zssh` command group)
- `packages/vsce/` - VS Code extension for Zowe Explorer

**Server Components:**

- `native/c/` - C++ backend libraries (Metal C + LE)
- `native/zowed/` - C++ middleware (JSON-RPC dispatcher)
- `native/c/commands/` - CLI command handlers for `zowex`

**Documentation:**

- `doc/` - Architecture guides and examples
- `doc/add-new-command.md` - Step-by-step guide for adding commands

## Adding New Commands - Quick Steps

1. Create command handler in `native/c/commands/`
2. Register with `zowex` CLI parser
3. Define TypeScript RPC types in `packages/sdk/src/doc/rpc/`
4. Generate C++ schemas with `npm run build:types:zowed`
5. Register with `zowed` dispatcher using `CommandBuilder`
6. Add SDK method in `RpcClientApi`
7. Optionally add CLI command and VS Code integration

👉 **Full step-by-step guide:** [doc/add-new-command.md](doc/add-new-command.md)

## Common Pitfalls

❌ Using C++17 features in `native/c/` (xlc) → Compilation errors  
❌ Using LE features in Metal C code → ABEND  
❌ Forgetting Base64 encoding for binary data → Corruption  
❌ Not handling EBCDIC/UTF-8 conversion → Garbled text  
❌ Missing FIFO cleanup → File descriptor leaks  
❌ Wrong storage mode (31-bit vs 64-bit) → ABEND  
❌ Not setting return object in command handler → Empty responses  
❌ Forgetting to register command in dispatcher → Command not found errors

👉 **Troubleshooting:** [doc/troubleshooting.md](doc/troubleshooting.md)

## Code Generation

- **Hand-written:** C++ backend, middleware, command handlers
- **Generated:** C++ JSON schemas from TypeScript types
- **Tool:** `scripts/generateTypes.ts`

To regenerate schemas:

```bash
npm run build:types:zowed
```

## Testing

### Local Tests (Vitest)

Client-side TypeScript code uses Vitest for unit testing:

```bash
# Run all tests
npm test

# Run tests with coverage
npm test -- --coverage

# Run tests in watch mode
npm test -- --watch
```

Test files are located in `tests/**/*.test.ts` directories within each package.

### z/OS Native Tests (Custom Framework)

C++ backend code uses a custom Jest-like testing framework (`native/c/test/ztest.hpp`):

```bash
# Run native tests on z/OS
npm run z:test
```

👉 **Full testing guide:** [native/README.md#testing](native/README.md#testing)

### Testing Checklist

- [ ] Command works in `zowex` CLI on z/OS
- [ ] JSON-RPC request/response types defined
- [ ] Middleware registration includes validation
- [ ] SDK method added with proper types
- [ ] Encoding handled correctly (EBCDIC ↔ UTF-8)
- [ ] Binary data Base64 encoded/decoded
- [ ] Large files use FIFO streaming
- [ ] Error cases return proper error responses
- [ ] Memory cleaned up (no leaks in Metal C)
- [ ] Recovery established for ABENDs (if needed)
- [ ] Unit tests written for SDK/CLI functionality (Vitest)
- [ ] Native tests added for C++ backend (if applicable)

## Threading and Concurrency

**⚠️ Metal C is NOT thread-safe** - Use proper locking for shared resources.

| Component          | Concurrency Model | Notes                                       |
| ------------------ | ----------------- | ------------------------------------------- |
| **zowed**          | Worker pool       | Multiple concurrent requests handled        |
| **zowex**          | Single-threaded   | Each invocation is independent              |
| **SSH connection** | Multiplexed       | Single persistent connection, many requests |

👉 **Full details:** [doc/server/zowed_architecture.md](doc/server/zowed_architecture.md)

## Build Commands

```bash
# Upload and build everything on z/OS
npm run all

# Build only client packages locally
npm run build

# Upload source files to z/OS
npm run z:upload

# Build native binaries on z/OS
npm run z:build

# Run native tests on z/OS
npm run z:test

# Download artifacts from z/OS
npm run z:artifacts

# Package for distribution
npm run package

# Watch for changes and auto-rebuild
npm run watch:all
```

## Development Workflow

### Quick Iteration

1. **Edit C++ code** in `native/c/` or `native/zowed/`
2. **Upload and rebuild:** `npm run z:rebuild`
3. **Test via SDK:** Use `serverPath` in SSH profile to skip artifact download
4. **Build client code:** `npm run build` after editing TypeScript

### Adding Commands

1. **Start with backend:** Create command in `native/c/commands/`
2. **Test locally:** Build and run `zowex` on z/OS
3. **Add middleware:** Define RPC types, register with `zowed`
4. **Test end-to-end:** Create test script using SDK
5. **Add CLI/VS Code:** Implement user-facing commands

👉 **Complete example:** [examples/add-new-command/](examples/add-new-command/)

## Documentation

**Quick reference:** This file + inline code comments

**Getting started:** [README.md](README.md) - Setup and usage

**Detailed guides** in `doc/` folder:

- `add-new-command.md` - **Start here!** Complete tutorial with examples
- `architecture-comparisons.md` - ZNP vs z/OSMF comparison
- `server/zowed_architecture.md` - Middleware design and request flow
- `server/zowex_architecture.md` - Backend CLI architecture
- `client/architecture.md` - SDK, CLI, and VS Code extension design
- `troubleshooting.md` - Common build and runtime issues

## Quick Decision Trees

**"Which layer should this code go in?"**  
Direct z/OS API calls? → Metal C backend (`native/c/`, xlc, C++11 subset)  
Command parsing/routing? → Middleware (`native/zowed/`, xlclang, C++17)  
Network/SSH handling? → SDK (`packages/sdk/`)  
User interface? → CLI or VS Code extension

**"What encoding should I use?"**  
Text content? → Convert EBCDIC to UTF-8  
Binary content? → Base64 encode  
Large files? → Use FIFO pipes with streaming

**"How do I handle errors?"**  
Metal C → Return codes + ABEND recovery  
C++ backend → Return codes (`RTNCD_*`)  
Middleware → JSON error responses  
TypeScript → Throw typed errors

👉 **See also:** [doc/add-new-command.md#key-patterns](doc/add-new-command.md#key-patterns)

## Examples

### Basic Command Flow

```typescript
// Client (TypeScript)
const client = new ZSshClient(profile);
await client.connect();
const response = await client.ds.listDatasets({ pattern: "USER.*" });
```

```cpp
// Middleware (C++ - zowed)
dispatcher.register_command("listDatasets",
  CommandBuilder(ds::handle_list)
    .validate<ListDatasetsRequest, ListDatasetsResponse>()
    .rename_arg("pattern", "filter"));
```

```cpp
// Backend (C++ - zowex)
int handle_list(InvocationContext &context) {
  string pattern = context.get<string>("filter");
  // Call Metal C to list datasets
  ZDS_LIST_RESULT result = zds_list(pattern.c_str());
  // Return JSON response
  context.set_object(format_response(result));
  return RTNCD_SUCCESS;
}
```

### Streaming Large Files

```typescript
// Use FIFO for large file upload
const response = await client.uss.uploadFile({
  path: "/u/users/me/large.bin",
  data: largeBase64Data, // Streamed via FIFO
  binary: true,
});
```

```cpp
// Middleware handles FIFO creation
CommandBuilder(uss::handle_upload)
  .validate<UploadFileRequest, UploadFileResponse>()
  .handle_fifo("data", "input", FIFO_READ);  // Create FIFO for streaming
```

## Zowe Conformance

ZNP follows Zowe conformance criteria:

- ✅ VS Code extension declares Zowe Explorer dependency
- ✅ Registers SSH profile type via Zowe Explorer API
- ✅ Supports base profiles and team configuration
- ✅ Provides CLI plugin matching VS Code capabilities
- ✅ Uses Zowe error message formats
- ✅ Shares connection info via Zowe Team Config

## When In Doubt

1. Check existing similar commands for patterns
2. Read `doc/add-new-command.md` for step-by-step guidance
3. Verify encoding/decoding for text vs binary data
4. Test Metal C code carefully (no exceptions, manual cleanup)
5. Use logging (`ZLOG_*` macros) for debugging
6. Run tests on z/OS with `npm run z:test`
7. Check `doc/troubleshooting.md` for common issues

## Performance Considerations

**ZNP advantages over z/OSMF:**

- Single persistent SSH connection (vs stateless HTTP)
- Direct system calls (vs TSO/REXX interpretation)
- Efficient binary encoding (vs text parsing)
- Multiplexed requests (vs per-request overhead)
- Lower latency for repeated operations

**Optimization tips:**

- Use FIFO pipes for files > 1MB
- Reuse SSH connections via `SshClientCache`
- Enable Base64 encoding in Metal C for performance
- Batch multiple operations when possible
- Use streaming for large data transfers

👉 **Performance comparison:** [doc/architecture-comparisons.md](doc/architecture-comparisons.md)

---

**Remember:** Four layers (TypeScript → Middleware → Backend → z/OS), three environments (LE C++, Metal C, TypeScript), SSH is the transport, JSON-RPC is the protocol.
