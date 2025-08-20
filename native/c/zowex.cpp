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

#include "ztype.h"
#pragma runopts("TRAP(ON,NOSPIE)")

#include <iostream>
#include <memory>
#include <vector>
#include <stdlib.h>
#include <string>
#include <cstdlib>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>
#include <limits.h>
#include "parser.hpp"
#include "zshmem.hpp"
#include "zcli.hpp"

// Version information
#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "unknown"
#endif

#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

using namespace parser;
using namespace std;

int handle_version(const ParseResult &result);
int handle_root_command(const ParseResult &result);

bool should_quit(const std::string &input);
int run_interactive_mode(const std::string &shm_file_path);

std::tr1::shared_ptr<ArgumentParser> arg_parser;
int main(int argc, char *argv[])
{
#ifdef ZLOG_ENABLE
  string args = "";
  for (int i = 0; i < argc; i++)
  {
    args += argv[i];
    if (i < argc - 1)
    {
      args += " ";
    }
  }
  ZLOG_TRACE("Starting zowex with args: %s", args.c_str());
#endif

  arg_parser = std::tr1::shared_ptr<ArgumentParser>(new ArgumentParser(argv[0], "Zowe Native Protocol CLI"));
  arg_parser->get_root_command().add_keyword_arg("interactive",
                                                 make_aliases("--interactive", "--it"),
                                                 "interactive (REPL) mode", ArgType_Flag, false,
                                                 ArgValue(false));
  arg_parser->get_root_command().add_keyword_arg("version",
                                                 make_aliases("--version", "-v"),
                                                 "display version information", ArgType_Flag, false,
                                                 ArgValue(false));
  arg_parser->get_root_command().add_keyword_arg("shm-file",
                                                 make_aliases("--shm-file"),
                                                 "shared memory file path", ArgType_Single, false);

  arg_parser->get_root_command().set_handler(handle_root_command);

  register_console_group(arg_parser->get_root_command());
  register_ds_group(arg_parser->get_root_command());
  register_job_group(arg_parser->get_root_command());
  register_tool_group(arg_parser->get_root_command());
  register_tso_group(arg_parser->get_root_command());
  register_uss_group(arg_parser->get_root_command());

  // Version command
  auto version_cmd = command_ptr(new Command("version", "display version information"));
  version_cmd->add_alias("--version");
  version_cmd->add_alias("-v");
  version_cmd->set_handler(handle_version);
  arg_parser->get_root_command().add_command(version_cmd);

  // Parse and execute through normal command handling
  ParseResult result = arg_parser->parse(argc, argv);
  return result.exit_code;
}

int handle_version(const ParseResult &result)
{
  cout << "Zowe Native Protocol CLI (zowex)" << endl;
  cout << "Version: " << PACKAGE_VERSION << endl;
  cout << "Build Date: " << BUILD_DATE << " " << BUILD_TIME << endl;
  cout << "Copyright Contributors to the Zowe Project." << endl;
  return 0;
}

int handle_root_command(const ParseResult &result)
{
  const auto is_interactive = result.find_kw_arg_bool("interactive");
  if (result.find_kw_arg_bool("version"))
  {
    const auto version_rc = handle_version(result);
    if (!is_interactive)
    {
      return version_rc;
    }
  }

  if (is_interactive)
  {
    return run_interactive_mode(result.find_kw_arg_string("shm-file"));
  }

  // If no interactive mode and no subcommands were invoked, show help
  result.m_command->generate_help(std::cout);
  return 0;
}

bool should_quit(const std::string &input)
{
  return (input == "quit" || input == "exit" ||
          input == "QUIT" || input == "EXIT");
}

int run_interactive_mode(const std::string &shm_file_path)
{
  // Initialize shared memory
  int shm_id;
  ZSharedRegion *shm_ptr = nullptr;

  if (!shm_file_path.empty())
  {
    // Create new shared memory for this process (each process gets its own)
    shm_id = init_shared_memory(&shm_ptr, shm_file_path.c_str());
    if (shm_id == -1)
    {
      cerr << "Failed to initialize shared memory" << endl;
      return RTNCD_FAILURE;
    }
  }

  std::cout << "Started, enter command or 'quit' to quit..." << std::endl;

  std::string command;
  int rc = 0;
  int is_tty = isatty(fileno(stdout));

  do
  {
    if (is_tty)
      std::cout << "\r> " << std::flush;

    std::getline(std::cin, command);

    if (should_quit(command))
      break;

    // Parse and execute the command
    ParseResult result = arg_parser->parse(command);
    rc = result.exit_code;

    if (!is_tty)
    {
      std::cout << "[" << rc << "]" << std::endl;
      // EBCDIC \x37 = ASCII \x04 = End of Transmission (Ctrl+D)
      std::cout << '\x37' << std::flush;
      std::cerr << '\x37' << std::flush;
    }

  } while (!should_quit(command));

  std::cout << "...terminated" << std::endl;

  // Clean up this process's shared memory
  if (!shm_file_path.empty())
  {
    cleanup_shared_memory(shm_id, shm_ptr, shm_file_path.c_str());
  }

  return rc;
}
