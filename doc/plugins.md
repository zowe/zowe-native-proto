# Creating a `zowex` Plug-in

The native plug-in infrastructure lets extenders add new commands and behaviors to `zowex`. The entry points exposed in
`native/c/extend/plugin.hpp` describe the full contract between a plug-in and `zowex`.

## Runtime entry point

Each shared library is expected to export a single function that `zowex` discovers via dynamic loading:

```c
extern "C" void registerPlugin(PluginManager &manager);
```

Inside `registerPlugin`, call `manager.registerCommandProvider(...)` for every command set your plug-in offers. The
`PluginManager` takes ownership of the pointer you pass in, so allocate the provider factory on the heap:

```cpp
#include "native/c/extend/plugin.hpp"

class MyCommandProviderFactory : public CommandProvider {
public:
  CommandProviderImpl *create() override { return new MyCommandProviderImpl(); }
};

extern "C" void registerPlugin(PluginManager &manager)
{
  manager.registerCommandProvider(new MyCommandProviderFactory());
}
```

## Implementing a command provider

`CommandProvider` is a `Factory<CommandProviderImpl>`. When `zowex` is ready to populate the command tree it calls
`create()`, expects a `CommandProviderImpl`, and immediately invokes `registerCommands(...)` on the instance.

```cpp
class MyCommandProviderImpl : public CommandProviderImpl {
public:
  void registerCommands(CommandRegistrationContext &context) override;
};
```

The `CommandRegistrationContext` supplied to `registerCommands` is your builder for commands, arguments, and handlers. It
offers the following capabilities:

- `createCommand(name, help)` – create a child command and hold on to the returned handle.
- `getRootCommand()` – fetch a handle to the root so you can attach top-level commands.
- `addAlias(command, alias)` – register alternative names.
- `addKeywordArg(...)` and `addPositionalArg(...)` – add options/positional parameters with the specified
  `ArgumentType` (`Flag`, `Single`, `Multiple`, or `Positional`), whether they are required, and an optional default.
- `setHandler(command, handler)` – wire in an `int handler(const parser::ParseResult &)` that executes when the command runs.
- `addSubcommand(parent, child)` – stitch commands into the hierarchy. Pass `getRootCommand()` as the parent to create
  top-level commands.

### Providing default argument values

`CommandDefaultValue` supplies constructors for every supported kind (bool, integer, double, and string). Construct one
on the stack and pass its pointer when you want a default:

```cpp
CommandDefaultValue defaultTimeout(30LL);
context.addKeywordArg(cmd,
                      "timeout",
                      nullptr,
                      0,
                      "Number of seconds to wait",
                      CommandRegistrationContext::ArgumentType_Single,
                      /* required */ 0,
                      &defaultTimeout);
```

If you do not supply a default or pass `nullptr`, the argument inherits the parser's standard empty value.

### Complete example

```cpp
void MyCommandProviderImpl::registerCommands(CommandRegistrationContext &context)
{
  auto root = context.getRootCommand();
  auto hello = context.createCommand("hello", "Print a greeting");

  CommandDefaultValue defaultName("World");
  const char *aliases[] = {"-n", "--name"};
  context.addKeywordArg(hello,
                        "name",
                        aliases,
                        2,
                        "Person to greet",
                        CommandRegistrationContext::ArgumentType_Single,
                        /* required */ 0,
                        &defaultName);

  context.setHandler(hello, &handleHello);
  context.addSubcommand(root, hello);
}
```
