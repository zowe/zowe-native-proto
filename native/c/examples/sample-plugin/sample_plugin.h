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

#ifndef SAMPLE_PLUGIN_H
#define SAMPLE_PLUGIN_H

#include "../../extend/plugin.hpp"

class BasicCommandRegistry : public plugin::CommandProviderImpl
{
public:
  void registerCommands(CommandRegistrationContext &context);
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