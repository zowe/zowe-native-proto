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

#ifndef PLUGIN_BRIDGE_HPP
#define PLUGIN_BRIDGE_HPP

#include "../extend/plugin.hpp"
#include "../parser.hpp"

// Forward declarations
class CommandDispatcher;

namespace plugin
{

/**
 * @brief Register all plugin commands to the middleware dispatcher
 *
 * This function takes a PluginManager that has already loaded plugins,
 * and registers all their commands to the middleware CommandDispatcher
 * as RPC methods using dot notation (e.g., "sample.hello").
 *
 * @param pm The PluginManager containing loaded plugins
 * @param dispatcher The CommandDispatcher to register commands to
 */
void register_commands_with_server(plugin::PluginManager &pm, CommandDispatcher &dispatcher);

} // namespace plugin

#endif
