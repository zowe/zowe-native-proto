#ifndef PLUGIN_HPP
#define PLUGIN_HPP

#include "../factory.hpp"

namespace parser
{
class Command;
class ParseResult;
} // namespace parser

namespace plugin
{
class CommandProviderImpl
{
public:
  struct CommandDefaultValue
  {
    enum ValueKind
    {
      ValueKind_None = 0,
      ValueKind_Bool = 1,
      ValueKind_Int = 2,
      ValueKind_Double = 3,
      ValueKind_String = 4
    };

    ValueKind kind;
    union
    {
      int bool_value;
      long long int_value;
      double double_value;
      const char *string_value;
    } value;

    CommandDefaultValue()
    {
      kind = ValueKind_None;
      value.int_value = 0;
    }

    explicit CommandDefaultValue(bool v)
    {
      kind = ValueKind_Bool;
      value.bool_value = v ? 1 : 0;
    }

    explicit CommandDefaultValue(long long v)
    {
      kind = ValueKind_Int;
      value.int_value = v;
    }

    explicit CommandDefaultValue(double v)
    {
      kind = ValueKind_Double;
      value.double_value = v;
    }

    explicit CommandDefaultValue(const char *v)
    {
      kind = ValueKind_String;
      value.string_value = v;
    }
  };

  class CommandRegistrationContext
  {
  public:
    enum ArgumentType
    {
      ArgumentType_Flag = 0,
      ArgumentType_Single = 1,
      ArgumentType_Multiple = 2,
      ArgumentType_Positional = 3
    };

    typedef void *CommandHandle;
    typedef int (*CommandHandler)(const parser::ParseResult &result);

    virtual ~CommandRegistrationContext()
    {
    }

    virtual CommandHandle createCommand(const char *name, const char *help) = 0;
    virtual CommandHandle getRootCommand() = 0;
    virtual void addAlias(CommandHandle command, const char *alias) = 0;
    virtual void addKeywordArg(CommandHandle command,
                               const char *name,
                               const char **aliases,
                               unsigned int aliasCount,
                               const char *help,
                               ArgumentType type,
                               int required,
                               const CommandDefaultValue *defaultValue) = 0;
    virtual void addPositionalArg(CommandHandle command,
                                  const char *name,
                                  const char *help,
                                  ArgumentType type,
                                  int required,
                                  const CommandDefaultValue *defaultValue) = 0;
    virtual void setHandler(CommandHandle command, CommandHandler handler) = 0;
    virtual void addSubcommand(CommandHandle parent, CommandHandle child) = 0;
  };

  virtual ~CommandProviderImpl()
  {
  }

  virtual void registerCommands(CommandRegistrationContext &context) = 0;
};

typedef CommandProviderImpl::CommandDefaultValue DefaultValue;
typedef Factory<CommandProviderImpl> CommandProvider;

class PluginManager
{
public:
  PluginManager();
  ~PluginManager();

  // Takes ownership of the provider pointer; it will be deleted with the manager.
  void registerCommandProvider(CommandProvider *provider);

  // Register commands from all providers, attaching them under the supplied root command.
  void registerCommands(parser::Command &rootCommand);

  PluginManager(const PluginManager &) = delete;
  PluginManager &operator=(const PluginManager &) = delete;

private:
  class Impl;
  Impl *m_impl;
};
} // namespace plugin

#endif
