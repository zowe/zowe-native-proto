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

#include "commands.hpp"
#include "dispatcher.hpp"
#include "schemas/requests.hpp"
#include "schemas/responses.hpp"
#include "../c/commands/ds.hpp"
#include "../c/commands/job.hpp"
#include "../c/commands/uss.hpp"
#include "../c/commands/sample.hpp"

// ... existing registration functions ...

void register_sample_commands(CommandDispatcher &dispatcher)
{
  dispatcher.register_command("ping", CommandBuilder(sample::handle_ping)
                                          .validate<PingRequest, PingResponse>()
                                          .set_default("message", "hello from zowed"));
}

void register_all_commands(CommandDispatcher &dispatcher)
{
  register_ds_commands(dispatcher);
  register_job_commands(dispatcher);
  register_uss_commands(dispatcher);
  register_cmd_commands(dispatcher);
  register_sample_commands(dispatcher);
}
