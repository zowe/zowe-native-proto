#ifndef PING_H
#define PING_H

#include "../parser.hpp"

namespace ping
{
int handle_ping(plugin::InvocationContext &context);
void register_commands(parser::Command &root_command);
} // namespace ping

#endif