#ifndef PLUGIN_HPP
#define PLUGIN_HPP

#include <vector>
#include "../factory.hpp"

class CommandProviderImpl
{
public:
  virtual void registerCommands() = 0;
};

typedef Factory<CommandProviderImpl> CommandProvider;

class PluginManager
{
  std::vector<CommandProvider *> m_commandProviders;

public:
  void registerCommandProvider(CommandProvider *provider)
  {
    m_commandProviders.push_back(provider);
  }
};

#endif