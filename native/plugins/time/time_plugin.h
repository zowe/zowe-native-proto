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

#ifndef TIME_PLUGIN_H
#define TIME_PLUGIN_H

#include "../../c/extend/plugin.hpp"

class BasicCommandRegistry : public plugin::CommandProviderImpl
{
public:
  void register_commands(CommandRegistrationContext &context);
};

class BasicCommandProvider : public Factory<plugin::CommandProviderImpl>
{
public:
  std::unique_ptr<plugin::CommandProviderImpl> create() override
  {
    return std::make_unique<BasicCommandRegistry>();
  }
};

extern "C" void register_plugin(plugin::PluginManager &pm);

#endif
