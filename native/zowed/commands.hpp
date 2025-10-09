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

#ifndef COMMANDS_HPP
#define COMMANDS_HPP

// Forward declaration
class CommandDispatcher;
class MiddlewareContext;

/**
 * Register all commands with the dispatcher
 * @param dispatcher The CommandDispatcher instance to register commands with
 */
void register_all_commands(CommandDispatcher &dispatcher);

#endif
