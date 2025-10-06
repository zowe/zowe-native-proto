# Creating a `zowex` Plug-in

The native plug-in infrastructure lets extenders add new commands and behaviors to `zowex`. The entry points exposed in
`native/c/extend/plugin.hpp` describe the full contract between a plug-in and `zowex`.

## Runtime entry point

Each shared library is expected to export a single function that `zowex` discovers via dynamic loading:

```c
extern "C" void register_plugin(PluginManager &manager);
```

Inside `register_plugin`, call `manager.register_plugin_metadata(...)` once to describe your plug-in and then `manager.register_command_provider(...)` for every command set the plug-in contributes. The `PluginManager` takes ownership of the pointer you pass in, so allocate the provider factory on the heap. The manager automatically records the underlying shared library filename for debugging, so you only supply a display name and version:

```cpp
#include "native/c/extend/plugin.hpp"

class MyCommandProviderFactory : public CommandProvider {
public:
  CommandProviderImpl *create() override { return new MyCommandProviderImpl(); }
};

extern "C" void register_plugin(PluginManager &manager)
{
  manager.register_plugin_metadata("Sample Plug-in", "1.0.0");
  manager.register_command_provider(new MyCommandProviderFactory());
}
```

## Implementing a command provider

`CommandProvider` is a `Factory<CommandProviderImpl>`. When `zowex` is ready to populate the command tree it calls
`create()`, expects a `CommandProviderImpl`, and immediately invokes `register_commands(...)` on the instance.

```cpp
class MyCommandProviderImpl : public CommandProviderImpl {
public:
  void register_commands(CommandRegistrationContext &context) override;
};
```

The `CommandRegistrationContext` supplied to `register_commands` is your builder for commands, arguments, and handlers. It offers the following capabilities:

- `create_command(name, help)` – create a child command and hold on to the returned handle.
- `get_root_command()` – fetch a handle to the root so you can attach top-level commands.
- `add_alias(command, alias)` – register alternative names.
- `add_keyword_arg(...)` and `add_positional_arg(...)` – add options/positional parameters with the specified `ArgumentType` (`Flag`, `Single`, `Multiple`, or `Positional`), whether they are required, and an optional default.
- `set_handler(command, handler)` – wire in an `int handler(const parser::ParseResult &)` that executes when the command runs.
- `add_subcommand(parent, child)` – stitch commands into the hierarchy. Pass `get_root_command()` as the parent to create top-level commands.

### Providing default argument values

`CommandDefaultValue` supplies constructors for every supported kind (bool, integer, double, and string). Construct one on the stack and pass its pointer when you want a default:

```cpp
CommandDefaultValue default_timeout(30LL);
context.add_keyword_arg(cmd,
                      "timeout",
                      nullptr,
                      0,
                      "Number of seconds to wait",
                      CommandRegistrationContext::ArgumentType_Single,
                      /* required */ 0,
                      &default_timeout);
```

If you do not supply a default or pass `nullptr`, the argument inherits the parser's standard empty value.

### Complete example

```cpp
int hello_cmd_handler(const InvocationContext& context) {
  const auto name = context.get<std::string>("name", "");
  if (!name.empty()) {
    context.println("hello, " + name + "!");
  } else {
    context.println("hello");
  }

  return 0;
}

void MyCommandProviderImpl::register_commands(CommandRegistrationContext &context)
{
  auto root = context.get_root_command();
  auto hello = context.create_command("hello", "Print a greeting");

  CommandDefaultValue default_name("World");
  const char *aliases[] = {"-n", "--name"};
  context.add_keyword_arg(hello,
                        "name",
                        aliases,
                        2,
                        "Person to greet",
                        CommandRegistrationContext::ArgumentType_Single,
                        /* required */ 0,
                        &defaultName);

  context.set_handler(hello, &hello_cmd_handler);
  context.add_subcommand(root, hello);
}
```
