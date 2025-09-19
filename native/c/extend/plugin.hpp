/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

#ifndef PLUGIN_HPP
#define PLUGIN_HPP

#include <vector>

#include "../factory.hpp"
#include "io.hpp"

namespace parser
{
class Command;
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
    typedef int (*CommandHandler)(InvocationContext &context);

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
  PluginManager() = default;
  ~PluginManager();

  // Takes ownership of the provider pointer; it will be deleted with the manager.
  void registerCommandProvider(CommandProvider *provider);

  // Register commands from all providers, attaching them under the supplied root command.
  void registerCommands(parser::Command &rootCommand);

  PluginManager(const PluginManager &) = delete;
  PluginManager &operator=(const PluginManager &) = delete;

private:
  std::vector<CommandProvider *> m_commandProviders;
};

inline PluginManager::~PluginManager()
{
  for (std::vector<CommandProvider *>::iterator it =
           m_commandProviders.begin();
       it != m_commandProviders.end(); ++it)
  {
    delete *it;
  }
}

inline void PluginManager::registerCommandProvider(CommandProvider *provider)
{
  // Take ownership of the pointer for the plugin manager lifetime.
  m_commandProviders.push_back(provider);
}
} // namespace plugin

#endif
