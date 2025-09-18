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

#include "plugin.hpp"
#include "../parser.hpp"

#include <string>
#include <vector>

namespace plugin
{
class RegistrationContextImpl
    : public CommandProviderImpl::CommandRegistrationContext
{

public:
  explicit RegistrationContextImpl(parser::Command &root)
      : m_root(root), m_rootRecord(root)
  {
  }

  ~RegistrationContextImpl()
  {
    for (std::vector<CommandRecord *>::iterator it = m_records.begin();
         it != m_records.end(); ++it)
    {
      delete *it;
    }
  }

  virtual CommandHandle createCommand(const char *name, const char *help)
  {
    std::string cmdName = name ? name : "";
    std::string cmdHelp = help ? help : "";

    parser::command_ptr command(new parser::Command(cmdName, cmdHelp));
    CommandRecord *record = new CommandRecord(command);
    m_records.push_back(record);
    return reinterpret_cast<CommandHandle>(record);
  }

  virtual CommandHandle getRootCommand()
  {
    return reinterpret_cast<CommandHandle>(&m_rootRecord);
  }

  virtual void addAlias(CommandHandle command, const char *alias)
  {
    CommandRecord *record = toRecord(command);
    if (!record || !alias)
      return;

    record->get().add_alias(alias);
  }

  virtual void addKeywordArg(CommandHandle command,
                             const char *name,
                             const char **aliases,
                             unsigned int aliasCount,
                             const char *help,
                             ArgumentType type,
                             int required,
                             const DefaultValue *defaultValue)
  {
    CommandRecord *record = toRecord(command);
    if (!record || !name)
      return;

    std::vector<std::string> aliasVector;
    if (aliases)
    {
      for (unsigned int i = 0; i < aliasCount; ++i)
      {
        if (aliases[i])
          aliasVector.push_back(aliases[i]);
      }
    }

    parser::ArgValue defaultArg = convertDefaultValue(defaultValue);

    record->get().add_keyword_arg(std::string(name), aliasVector,
                                  help ? std::string(help) : std::string(),
                                  convertArgType(type), required != 0,
                                  defaultArg);
  }

  virtual void addPositionalArg(CommandHandle command,
                                const char *name,
                                const char *help,
                                int required,
                                const DefaultValue *defaultValue)
  {
    CommandRecord *record = toRecord(command);
    if (!record || !name)
      return;

    parser::ArgValue defaultArg = convertDefaultValue(defaultValue);

    record->get().add_positional_arg(std::string(name),
                                     help ? std::string(help) : std::string(),
                                     parser::ArgType_Positional, required != 0,
                                     defaultArg);
  }

  virtual void setHandler(CommandHandle command, CommandHandler handler)
  {
    CommandRecord *record = toRecord(command);
    if (!record)
      return;

    record->get().set_handler(handler);
  }

  virtual void addSubcommand(CommandHandle parent, CommandHandle child)
  {
    CommandRecord *parentRecord = toRecord(parent);
    CommandRecord *childRecord = toRecord(child);
    if (!parentRecord || !childRecord)
      return;

    if (parentRecord->isRoot())
    {
      m_root.add_command(childRecord->getCommandPointer());
    }
    else
    {
      parentRecord->get()
          .add_command(childRecord->getCommandPointer());
    }
  }

private:
  struct CommandRecord
  {
    parser::command_ptr pointer;
    parser::Command *raw;
    bool root;

    explicit CommandRecord(parser::Command &existing)
        : raw(&existing), root(true)
    {
    }

    explicit CommandRecord(const parser::command_ptr &cmd)
        : pointer(cmd), raw(cmd.get()), root(false)
    {
    }

    parser::Command &get()
    {
      return *raw;
    }

    const parser::command_ptr &getCommandPointer() const
    {
      return pointer;
    }

    bool isRoot() const
    {
      return root;
    }
  };

  CommandRecord *toRecord(CommandHandle handle)
  {
    return reinterpret_cast<CommandRecord *>(handle);
  }

  parser::ArgValue convertDefaultValue(const DefaultValue *value)
  {
    if (!value || value->kind == DefaultValue::ValueKind_None)
    {
      return parser::ArgValue();
    }

    switch (value->kind)
    {
    case DefaultValue::ValueKind_Bool:
      return parser::ArgValue(value->value.bool_value != 0);
    case DefaultValue::ValueKind_Int:
      return parser::ArgValue(static_cast<long long>(value->value.int_value));
    case DefaultValue::ValueKind_Double:
      return parser::ArgValue(value->value.double_value);
    case DefaultValue::ValueKind_String:
      if (value->value.string_value)
        return parser::ArgValue(std::string(value->value.string_value));
      return parser::ArgValue();
    default:
      return parser::ArgValue();
    }
  }

  parser::ArgType convertArgType(ArgumentType type)
  {
    switch (type)
    {
    case ArgumentType_Flag:
      return parser::ArgType_Flag;
    case ArgumentType_Single:
      return parser::ArgType_Single;
    case ArgumentType_Multiple:
      return parser::ArgType_Multiple;
    case ArgumentType_Positional:
      return parser::ArgType_Positional;
    }
    return parser::ArgType_Flag;
  }

  parser::Command &m_root;
  CommandRecord m_rootRecord;
  std::vector<CommandRecord *> m_records;
};

class PluginManager::Impl
{
public:
  std::vector<CommandProvider *> commandProviders;

  ~Impl()
  {
    for (std::vector<CommandProvider *>::iterator it = commandProviders.begin();
         it != commandProviders.end(); ++it)
    {
      delete *it;
    }
  }
};

PluginManager::PluginManager()
    : m_impl(new PluginManager::Impl())
{
}

PluginManager::~PluginManager()
{
  delete m_impl;
}

void PluginManager::registerCommandProvider(CommandProvider *provider)
{
  // Take ownership of the pointer for the plugin manager lifetime.
  m_impl->commandProviders.push_back(provider);
}

void PluginManager::registerCommands(parser::Command &rootCommand)
{
  RegistrationContextImpl context(rootCommand);

  for (std::vector<CommandProvider *>::iterator it =
           m_impl->commandProviders.begin();
       it != m_impl->commandProviders.end(); ++it)
  {
    CommandProvider *factory = *it;
    if (!factory)
      continue;

    CommandProviderImpl *provider = factory->create();
    if (!provider)
      continue;

    provider->registerCommands(context);
    delete provider;
  }
}
} // namespace plugin
