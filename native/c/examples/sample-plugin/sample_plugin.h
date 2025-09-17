#ifndef SAMPLE_PLUGIN_H
#define SAMPLE_PLUGIN_H

#include "../../extend/plugin.hpp"

class BasicCommandRegistry : public plugin::CommandProviderImpl
{
public:
  void registerCommands(CommandRegistrationContext &context) override;
};

class BasicCommandProvider : public Factory<plugin::CommandProviderImpl>
{
  static plugin::CommandProviderImpl *m_providerImpl;

public:
  plugin::CommandProviderImpl *create()
  {
    if (m_providerImpl == nullptr)
    {
      m_providerImpl = new BasicCommandRegistry();
    }

    return m_providerImpl;
  }
};

extern "C" void registerPlugin(plugin::PluginManager &pm);

#endif