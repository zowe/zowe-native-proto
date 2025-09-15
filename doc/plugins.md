# Creating a `zowex` Plug-in

Extenders can contribute their own native, backend code to `zowex` starting with version `0.y.z` (TBD). The plug-in
system allows developers to add commands and run custom logic during `zowex` command operations.

## Managing plug-ins

## Plug-in prerequisites

The plug-in needs to export a function named `registerPlugin` so that the `PluginManager` can start execution of the plug-in library's logic at runtime:

```c
extern "C" void registerPlugin(PluginManager& manager);
```

## Contributing commands

Extenders can contribute new commands by registering a `CommandProvider` instance with the `PluginManager`:

```cpp
template <typename Interface>
class Factory {
public:
    virtual Interface* create() = 0;
};

class CommandProviderImpl {
public:
    virtual void registerCommands() = 0;
};

typedef Factory<CommandProviderImpl> CommandProvider;
```

```cpp
class PluginManager {
public:
    void registerCommandProvider(CommandProvider& provider) { /* ... */ }
}
```
