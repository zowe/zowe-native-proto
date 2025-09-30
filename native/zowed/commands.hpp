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
 * Register all data set related commands with the dispatcher
 * @param dispatcher The CommandDispatcher instance to register commands with
 */
void register_ds_commands(CommandDispatcher &dispatcher);

/**
 * Register all job related commands with the dispatcher
 * @param dispatcher The CommandDispatcher instance to register commands with
 */
void register_job_commands(CommandDispatcher &dispatcher);

/**
 * Register all USS (Unix System Services) related commands with the dispatcher
 * @param dispatcher The CommandDispatcher instance to register commands with
 */
void register_uss_commands(CommandDispatcher &dispatcher);

/**
 * Internal helper method to process base64 encoded data from arguments
 * Decodes base64 data and moves it to the input stream, removing the "data" argument
 * @param context The MiddlewareContext to process
 */
void write_data_to_stdin(MiddlewareContext &context);

#endif
