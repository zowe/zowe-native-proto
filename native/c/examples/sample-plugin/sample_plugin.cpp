#include "sample_plugin.h"

int hello_command(plugin::InvocationContext &context)
{
}

void BasicCommandRegistry::registerCommands(CommandProviderImpl::CommandRegistrationContext &ctx)
{
  auto sample_group = ctx.createCommand("sample", "Sample command group for plug-in operations");
  auto hello = ctx.createCommand("hello", "says hi to the caller");
  ctx.addAlias(hello, "hi");
  ctx.addPositionalArg(hello, "name", "the name of the person calling the command", CommandProviderImpl::CommandRegistrationContext::ArgumentType_Positional, 0, nullptr);
  ctx.setHandler(hello, hello_command);
  ctx.addSubcommand(sample_group, hello);
}

void registerPlugin(plugin::PluginManager &pm)
{
  auto commandProvider = BasicCommandProvider();
  pm.registerCommandProvider(&commandProvider);
}
