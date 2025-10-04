# Adding a New Command to Zowe Native Protocol

This guide walks you through creating a new command for the Zowe Native Protocol stack, from the low-level C++ implementation through the middleware layer to the SDK. We'll use a `getTime` command as an example that takes a timezone argument and returns the current date and time.

## Table of Contents

1. [Adding a Command to the Native CLI (zowex)](#1-adding-a-command-to-the-native-cli-zowex)
2. [Testing Your CLI Command](#2-testing-your-cli-command)
3. [Adding the Command to the Middleware (zowed)](#3-adding-the-command-to-the-middleware-zowed)
4. [Testing Your Middleware Command](#4-testing-your-middleware-command)

---

## 1. Adding a Command to the Native CLI (zowex)

The `zowex` CLI is the lowest level of the stack, providing direct access to mainframe capabilities. Commands are defined in the `native/c/commands/` directory.

### Step 1.1: Create Command Files

Create two files in `native/c/commands/`:

- `time.hpp` - Header file with function declarations
- `time.cpp` - Implementation file with command logic

**`native/c/commands/time.hpp`:**

```cpp
#include "../parser.hpp"

namespace time_cmd
{
using namespace plugin;
int handle_get_time(InvocationContext &context);
void register_commands(parser::Command &root_command);
} // namespace time_cmd
```

**`native/c/commands/time.cpp`:**

```cpp
#include "time.hpp"
#include "common_args.hpp"
#include <string>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <iomanip>

using namespace ast;
using namespace parser;
using namespace std;
using namespace commands::common;

namespace time_cmd
{

// Library method that performs the actual time retrieval logic
int get_time_for_timezone(const string &timezone, string &date_str, string &time_str)
{
  // Save the current TZ environment variable
  const char *old_tz = getenv("TZ");
  string saved_tz = old_tz ? old_tz : "";

  // Set the requested timezone
  if (!timezone.empty())
  {
    setenv("TZ", timezone.c_str(), 1);
    tzset();
  }

  // Get current time
  time_t now = time(nullptr);
  struct tm *timeinfo = localtime(&now);

  if (timeinfo == nullptr)
  {
    // Restore original timezone
    if (!saved_tz.empty())
    {
      setenv("TZ", saved_tz.c_str(), 1);
    }
    else
    {
      unsetenv("TZ");
    }
    tzset();
    return RTNCD_FAILURE;
  }

  // Format date as YYYY-MM-DD
  ostringstream date_stream;
  date_stream << put_time(timeinfo, "%Y-%m-%d");
  date_str = date_stream.str();

  // Format time as HH:MM:SS
  ostringstream time_stream;
  time_stream << put_time(timeinfo, "%H:%M:%S");
  time_str = time_stream.str();

  // Restore original timezone
  if (!saved_tz.empty())
  {
    setenv("TZ", saved_tz.c_str(), 1);
  }
  else
  {
    unsetenv("TZ");
  }
  tzset();

  return RTNCD_SUCCESS;
}

// Command handler that parses args, invokes library method, and sets return object
int handle_get_time(InvocationContext &context)
{
  // Parse the timezone argument
  string timezone = context.get<string>("timezone", "");

  if (timezone.empty())
  {
    context.error_stream() << "Error: timezone argument is required" << endl;
    return RTNCD_FAILURE;
  }

  // Invoke the library method
  string date_str;
  string time_str;
  int rc = get_time_for_timezone(timezone, date_str, time_str);

  if (rc != RTNCD_SUCCESS)
  {
    context.error_stream() << "Error: could not retrieve time for timezone: '" << timezone << "'" << endl;
    return rc;
  }

  // Print to stdout (for CLI output)
  context.output_stream() << "Date: " << date_str << endl;
  context.output_stream() << "Time: " << time_str << endl;
  context.output_stream() << "Timezone: " << timezone << endl;

  // Set return object (for programmatic access)
  const auto result = obj();
  result->set("date", str(date_str));
  result->set("time", str(time_str));
  result->set("timezone", str(timezone));
  context.set_object(result);

  return RTNCD_SUCCESS;
}

// Register the command with the parser
void register_commands(parser::Command &root_command)
{
  // Create the get-time command
  auto get_time_cmd = command_ptr(new Command("get-time", "get current date and time for a timezone"));

  // Add required timezone positional argument
  get_time_cmd->add_positional_arg(
      "timezone",
      "timezone identifier (e.g., 'America/New_York', 'UTC', 'Europe/London')",
      ArgType_Single,
      true  // required
  );

  // Set the handler function
  get_time_cmd->set_handler(handle_get_time);

  // Add an example
  get_time_cmd->add_example("Get time in UTC", "zowex get-time UTC");
  get_time_cmd->add_example("Get time in New York", "zowex get-time America/New_York");

  // Register with the root command
  root_command.add_command(get_time_cmd);
}

} // namespace time_cmd
```

### Step 1.2: Register Your Command

Edit `native/c/zowex.cpp` to include and register your new command:

```cpp
// Add include at the top with other command includes
#include "commands/time.hpp"

// In the main() function, add registration call with other commands
int main(int argc, char *argv[])
{
  // ... existing code ...

  // Register commands
  ds::register_commands(arg_parser.get_root_command());
  job::register_commands(arg_parser.get_root_command());
  uss::register_commands(arg_parser.get_root_command());
  time_cmd::register_commands(arg_parser.get_root_command());  // Add this line

  // ... rest of main ...
}
```

### Step 1.3: Update the Makefile

Edit `native/c/makefile` to include your new source file in the build:

```makefile
# Find the line with command object files and add time.o
COMMAND_OBJS = commands/ds.o commands/job.o commands/uss.o \
               commands/tso.o commands/console.o commands/tool.o \
               commands/core.o commands/time.o
```

---

## 2. Testing Your CLI Command

### Step 2.1: Build zowex

Upload your new source code:

```bash
npm run z:upload
```

Build the zowex C++ binary:

```bash
npm run z:build
```

**Note:** This command is equivalent to running `cd native/c && make` on z/OS.

### Step 2.2: Test the Command

Once compiled, you can test the command directly on z/OS:

```bash
# Test with UTC timezone
./zowex get-time UTC

# Expected output:
# Date: 2025-10-04
# Time: 18:23:45
# Timezone: UTC

# Test with a different timezone
./zowex get-time America/New_York

# Test with invalid timezone
./zowex get-time InvalidTimezone
# Should show an error message
```

---

## 3. Adding the Command to the Middleware (zowed)

The middleware (`zowed`) is a C++ application that provides remote access to zowex commands via JSON-RPC over SSH. To expose your command through the middleware, you need to:

1. Register the command with the C++ dispatcher using `CommandBuilder`
2. Define TypeScript types for the SDK

### Step 3.1: Register the Command in zowed

Edit `native/zowed/commands.cpp` to register your command with the dispatcher:

```cpp
#include "commands.hpp"
#include "dispatcher.hpp"
#include "../c/commands/ds.hpp"
#include "../c/commands/job.hpp"
#include "../c/commands/uss.hpp"
#include "../c/commands/time.hpp"  // Add this include

// ... existing registration functions ...

void register_time_commands(CommandDispatcher &dispatcher)
{
  dispatcher.register_command("getTime",
                              CommandBuilder(time_cmd::handle_get_time));
}

// In the main registration function, call register_time_commands:
void register_all_commands(CommandDispatcher &dispatcher)
{
  register_ds_commands(dispatcher);
  register_job_commands(dispatcher);
  register_uss_commands(dispatcher);
  register_cmd_commands(dispatcher);
  register_time_commands(dispatcher);  // Add this line
}
```

The `CommandBuilder` provides a fluent API for mapping RPC parameters to command arguments:

- `.rename_arg(from, to)` - Rename an RPC parameter to match the command's expected argument
- `.set_default(name, value)` - Set a default value for an argument
- `.flatten_obj(name)` - Flatten a nested JSON object into top-level arguments
- `.write_stdin(name, base64)` - Write an RPC parameter to the command's stdin
- `.read_stdout(name, base64)` - Read the command's stdout into the RPC response
- `.handle_fifo(rpcId, argName, mode)` - Create a FIFO pipe for streaming data

### Step 3.2: Build the Middleware

Upload your new source code:

```bash
npm run z:upload
```

Build the zowed C++ binary:

```bash
npm run z:build:zowed
```

**Note:** This command is equivalent to running `cd native/zowed && make` on z/OS.

---

## 4. Testing Your Middleware Command

### Step 4.1: Define SDK Types

Create TypeScript type definitions for your new command in `packages/sdk/src/doc/gen/time.ts`:

**`packages/sdk/src/doc/gen/time.ts`:**

```typescript
import type * as common from "./common";

export interface GetTimeRequest extends common.CommandRequest {
  command: "getTime";
  /**
   * Timezone identifier (e.g., 'America/New_York', 'UTC')
   */
  timezone: string;
}

export interface GetTimeResponse extends common.CommandResponse {
  /**
   * Current date in YYYY-MM-DD format
   */
  date: string;
  /**
   * Current time in HH:MM:SS format
   */
  time: string;
  /**
   * Timezone used
   */
  timezone: string;
}
```

You'll also need to export your new types from `packages/sdk/src/doc/index.ts`:

```typescript
export * as time from "./gen/time";
```

### Step 4.2: Add SDK Method

Edit `packages/sdk/src/AbstractRpcClient.ts` to add a method for your new command:

```typescript
import type {
  CommandRequest,
  CommandResponse,
  cmds,
  ds,
  jobs,
  uss,
  time,
} from "./doc";

export abstract class AbstractRpcClient {
  // ... existing methods (request, ds, jobs, uss, cmds) ...

  public get time() {
    return {
      getTime: (
        request: Omit<time.GetTimeRequest, "command">
      ): Promise<time.GetTimeResponse> =>
        this.request({ command: "getTime", ...request }),
    };
  }
}
```

### Step 4.3: Create a Test Script

Create a test script to verify your command works end-to-end:

**`test-gettime.ts`:**

```typescript
import { ZSshClient } from "zowe-native-proto-sdk/src/ZSshClient";
import { ProfileInfo } from "@zowe/imperative";

async function main() {
  // Load Zowe configuration
  const profileInfo = new ProfileInfo("zowe");
  await profileInfo.readProfilesFromDisk();

  // Get SSH profile
  const sshProfile = profileInfo.getDefaultProfile("ssh");
  if (!sshProfile) {
    throw new Error("No default SSH profile found");
  }

  // Create SSH client
  const client = new ZSshClient(sshProfile.profile);

  try {
    // Connect to the server
    await client.connect();
    console.log("Connected to server");

    // Test 1: Get time in UTC
    console.log("\nTest 1: UTC Timezone");
    const utcResponse = await client.time.getTime({ timezone: "UTC" });
    console.log("  Date:", utcResponse.date);
    console.log("  Time:", utcResponse.time);
    console.log("  Timezone:", utcResponse.timezone);

    // Test 2: Get time in New York
    console.log("\nTest 2: America/New_York Timezone");
    const nyResponse = await client.time.getTime({
      timezone: "America/New_York",
    });
    console.log("  Date:", nyResponse.date);
    console.log("  Time:", nyResponse.time);
    console.log("  Timezone:", nyResponse.timezone);

    // Test 3: Get time in Tokyo
    console.log("\nTest 3: Asia/Tokyo Timezone");
    const tokyoResponse = await client.time.getTime({ timezone: "Asia/Tokyo" });
    console.log("  Date:", tokyoResponse.date);
    console.log("  Time:", tokyoResponse.time);
    console.log("  Timezone:", tokyoResponse.timezone);

    console.log("\n✅ All tests passed!");
  } catch (error) {
    console.error("❌ Test failed:", error);
    process.exit(1);
  } finally {
    // Disconnect
    await client.disconnect();
    console.log("Disconnected from server");
  }
}

main().catch(console.error);
```

### Step 4.4: Run the Test

Make sure you have a Zowe SSH profile configured, then run the test:

```bash
# Build the SDK first
cd packages/sdk
npm run build

# Run the test script
npx tsx test-gettime.ts
```

Expected output:

```
Connected to server

Test 1: UTC Timezone
  Date: 2025-10-04
  Time: 18:23:45
  Timezone: UTC

Test 2: America/New_York Timezone
  Date: 2025-10-04
  Time: 14:23:45
  Timezone: America/New_York

Test 3: Asia/Tokyo Timezone
  Date: 2025-10-05
  Time: 03:23:45
  Timezone: Asia/Tokyo

✅ All tests passed!
Disconnected from server
```

---

## Summary

You've successfully added a new command to the Zowe Native Protocol stack! Here's what you did:

1. **Created a low-level C++ command** in `native/c/commands/` that can be invoked directly via `zowex`
2. **Tested it locally** on z/OS using the zowex CLI
3. **Registered it with zowed** using the `CommandBuilder` API in `native/zowed/commands.cpp`
4. **Created TypeScript types** in `packages/sdk/src/doc/gen/` for the SDK
5. **Added SDK methods** in TypeScript for easy client-side access
6. **Tested end-to-end** with a sample script

## Key Patterns

- **Separation of concerns**: Library logic is separate from command handling
- **Return objects**: Always set a return object using `context.set_object()` for programmatic access
- **Error handling**: Use proper return codes (`RTNCD_SUCCESS`, `RTNCD_FAILURE`) and error messages
- **CommandBuilder**: Use the fluent API to map RPC parameters to command arguments
- **Type safety**: Manually define TypeScript types to ensure consistency between middleware and SDK

## Next Steps

- Add the command to the CLI package (`packages/cli/`)
- Add the command to the VS Code extension (`packages/vsce/`)
- Write unit tests for your command
- Update documentation and examples
