#ifndef SAMPLE_H
#define SAMPLE_H

#include "../parser.hpp"

namespace sample
{
int handle_ping(plugin::InvocationContext &context);
void register_commands(parser::Command &root_command);
} // namespace sample

#endif
