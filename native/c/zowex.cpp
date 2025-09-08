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
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <sys/stat.h>
#include <limits.h>
#include "zcn.hpp"
#include "zut.hpp"
#include "parser.hpp"
#include "zds.hpp"
#include "ztso.hpp"
#include "zshmem.hpp"
#include "zuttype.h"
#include "zlogger.hpp"

#include "commands/ds.hpp"
#include "commands/job.hpp"
#include "commands/uss.hpp"

// Version information
#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "unknown"
#endif

#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

using namespace parser;
using namespace std;

int handle_console_issue(const ParseResult &result);

int handle_tso_issue(const ParseResult &result);

int handle_tool_convert_dsect(const ParseResult &result);
int handle_tool_dynalloc(const ParseResult &result);
int handle_tool_display_symbol(const ParseResult &result);
int handle_tool_search(const ParseResult &result);
int handle_tool_amblist(const ParseResult &result);
int handle_tool_run(const ParseResult &result);

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

  // Console command group
  auto console_cmd = command_ptr(new Command("console", "z/OS console operations"));
  console_cmd->add_alias("cn");

  // Console Issue subcommand
  auto issue_cmd = command_ptr(new Command("issue", "issue a console command"));
  issue_cmd->add_keyword_arg("console-name",
                             make_aliases("--cn", "--console-name"),
                             "extended console name", ArgType_Single, false,
                             ArgValue(std::string("zowex")));
  issue_cmd->add_keyword_arg("wait",
                             make_aliases("--wait"),
                             "wait for responses", ArgType_Flag, false,
                             ArgValue(true));
  issue_cmd->add_keyword_arg("timeout",
                             make_aliases("--timeout"),
                             "timeout in seconds", ArgType_Single, false);
  issue_cmd->add_positional_arg("command", "command to run, e.g. 'D IPLINFO'",
                                ArgType_Single, true);
  issue_cmd->set_handler(handle_console_issue);

  console_cmd->add_command(issue_cmd);
  arg_parser->get_root_command().add_command(console_cmd);

  // TSO command group
  auto tso_cmd = command_ptr(new Command("tso", "TSO operations"));

  // TSO issue subcommand
  auto tso_issue_cmd = command_ptr(new Command("issue", "issue TSO command"));
  tso_issue_cmd->add_positional_arg("command", "command to issue", ArgType_Single, true);
  tso_issue_cmd->set_handler(handle_tso_issue);

  tso_cmd->add_command(tso_issue_cmd);
  arg_parser->get_root_command().add_command(tso_cmd);

  auto encoding_option = make_aliases("--encoding", "--ec");
  auto source_encoding_option = make_aliases("--local-encoding", "--lec");
  auto response_format_csv_option = make_aliases("--response-format-csv", "--rfc");
  auto response_format_bytes_option = make_aliases("--response-format-bytes", "--rfb");

  ds::register_commands(arg_parser->get_root_command());
  uss::register_commands(arg_parser->get_root_command());

  // Tool command group
  auto tool_cmd = command_ptr(new Command("tool", "tool operations"));

  // Convert DSECT subcommand
  auto tool_convert_dsect_cmd = command_ptr(new Command("ccnedsct", "convert dsect to c struct"));
  tool_convert_dsect_cmd->add_keyword_arg("adata-dsn",
                                          make_aliases("--adata-dsn", "--ad"),
                                          "input adata dsn", ArgType_Single, true);
  tool_convert_dsect_cmd->add_keyword_arg("chdr-dsn",
                                          make_aliases("--chdr-dsn", "--cd"),
                                          "output chdr dsn", ArgType_Single, true);
  tool_convert_dsect_cmd->add_keyword_arg("sysprint",
                                          make_aliases("--sysprint", "--sp"),
                                          "sysprint output", ArgType_Single, false);
  tool_convert_dsect_cmd->add_keyword_arg("sysout",
                                          make_aliases("--sysout", "--so"),
                                          "sysout output", ArgType_Single, false);
  tool_convert_dsect_cmd->set_handler(handle_tool_convert_dsect);
  tool_cmd->add_command(tool_convert_dsect_cmd);

  // Dynalloc subcommand
  auto tool_dynalloc_cmd = command_ptr(new Command("bpxwdy2", "dynalloc command"));
  tool_dynalloc_cmd->add_positional_arg("parm", "dynalloc parm string", ArgType_Single, true);
  tool_dynalloc_cmd->set_handler(handle_tool_dynalloc);
  tool_cmd->add_command(tool_dynalloc_cmd);

  // Display symbol subcommand
  auto tool_display_symbol_cmd = command_ptr(new Command("display-symbol", "display system symbol"));
  tool_display_symbol_cmd->add_positional_arg("symbol", "symbol to display", ArgType_Single, true);
  tool_display_symbol_cmd->set_handler(handle_tool_display_symbol);
  tool_cmd->add_command(tool_display_symbol_cmd);

  // Search subcommand
  auto tool_search_cmd = command_ptr(new Command("search", "search members for string"));
  tool_search_cmd->add_positional_arg("dsn", "data set to search", ArgType_Single, true);
  tool_search_cmd->add_positional_arg("string", "string to search for", ArgType_Single, true);
  tool_search_cmd->add_keyword_arg("max-entries", make_aliases("--max-entries", "--me"), "max number of results to return before warning generated", ArgType_Single, false);
  tool_search_cmd->add_keyword_arg("warn", make_aliases("--warn"), "warn if truncated or not found", ArgType_Flag, false, ArgValue(true));
  tool_search_cmd->set_handler(handle_tool_search);
  tool_cmd->add_command(tool_search_cmd);

  // Amblist subcommand
  auto tool_amblist_cmd = command_ptr(new Command("amblist", "invoke amblist"));
  tool_amblist_cmd->add_positional_arg("dsn", "data containing input load modules", ArgType_Single, true);
  tool_amblist_cmd->add_keyword_arg("control-statements",
                                    make_aliases("--control-statements", "--cs"),
                                    "amblist control statements, e.g. listload output=map,member=testprog",
                                    ArgType_Single, true);
  tool_amblist_cmd->set_handler(handle_tool_amblist);
  tool_cmd->add_command(tool_amblist_cmd);

  // Run subcommand
  auto tool_run_cmd = command_ptr(new Command("run", "run a program"));
  tool_run_cmd->add_positional_arg("program", "name of program to run", ArgType_Single, true);
  tool_run_cmd->add_keyword_arg("dynalloc-pre",
                                make_aliases("--dynalloc-pre", "--dp"),
                                "dynalloc pre run statements", ArgType_Single, false);
  tool_run_cmd->add_keyword_arg("dynalloc-post",
                                make_aliases("--dynalloc-post", "--dt"),
                                "dynalloc post run statements", ArgType_Single, false);
  tool_run_cmd->add_keyword_arg("in-dd",
                                make_aliases("--in-dd", "--idd"),
                                "input ddname", ArgType_Single, false);
  tool_run_cmd->add_keyword_arg("input",
                                make_aliases("--input", "--in"),
                                "input", ArgType_Single, false);
  tool_run_cmd->add_keyword_arg("out-dd",
                                make_aliases("--out-dd", "--odd"),
                                "output ddname", ArgType_Single, false);
  tool_run_cmd->set_handler(handle_tool_run);
  tool_cmd->add_command(tool_run_cmd);

  arg_parser->get_root_command().add_command(tool_cmd);

  // Job commands
  job::register_commands(arg_parser->get_root_command());

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

int handle_console_issue(const ParseResult &result)
{
  int rc = 0;
  ZCN zcn = {0};

  string console_name = result.get_value<std::string>("console-name", "zowex");
  long long timeout = result.get_value<long long>("timeout", 0);

  string command = result.get_value<std::string>("command", "");
  bool wait = result.get_value<bool>("wait", true);

  if (timeout > 0)
  {
    zcn.timeout = timeout;
  }

  rc = zcn_activate(&zcn, console_name);
  if (0 != rc)
  {
    cerr << "Error: could not activate console: '" << console_name << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zcn.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  rc = zcn_put(&zcn, command);
  if (0 != rc)
  {
    cerr << "Error: could not write to console: '" << console_name << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zcn.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  if (wait)
  {
    string response = "";
    rc = zcn_get(&zcn, response);
    if (0 != rc)
    {
      cerr << "Error: could not get from console: '" << console_name << "' rc: '" << rc << "'" << endl;
      cerr << "  Details: " << zcn.diag.e_msg << endl;
      return RTNCD_FAILURE;
    }
    cout << response << endl;
  }

  rc = zcn_deactivate(&zcn);
  if (0 != rc)
  {
    cerr << "Error: could not deactivate console: '" << console_name << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zcn.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }
  return rc;
}

int handle_tso_issue(const ParseResult &result)
{
  int rc = 0;
  string command = result.get_value<std::string>("command", "");
  string response;

  rc = ztso_issue(command, response);

  if (0 != rc)
  {
    cerr << "Error running command, rc '" << rc << "'" << endl;
    cerr << "  Details: " << response << endl;
  }

  cout << response;

  return rc;
}

int handle_tool_convert_dsect(const ParseResult &result)
{
  int rc = 0;
  ZCN zcn = {0};
  unsigned int code = 0;
  string resp;

  string adata_dsn = result.get_value<std::string>("adata-dsn", "");
  string chdr_dsn = result.get_value<std::string>("chdr-dsn", "");
  string sysprint = result.get_value<std::string>("sysprint", "");
  string sysout = result.get_value<std::string>("sysout", "");

  const char *user = getlogin();
  string struser(user);
  transform(struser.begin(), struser.end(), struser.begin(), ::tolower);

  if (sysprint.empty())
    sysprint = "/tmp/" + struser + "_sysprint.txt";
  if (sysout.empty())
    sysout = "/tmp/" + struser + "_sysout.txt";

  cout << adata_dsn << " " << chdr_dsn << " " << sysprint << " " << sysout << endl;

  vector<string> dds;
  dds.reserve(4);
  dds.push_back("alloc fi(sysprint) path('" + sysprint + "') pathopts(owronly,ocreat,otrunc) pathmode(sirusr,siwusr,sirgrp) filedata(text) msg(2)");
  dds.push_back("alloc fi(sysout) path('" + sysout + "') pathopts(owronly,ocreat,otrunc) pathmode(sirusr,siwusr,sirgrp) filedata(text) msg(2)");
  dds.push_back("alloc fi(sysadata) da('" + adata_dsn + "') shr msg(2)");
  dds.push_back("alloc fi(edcdsect) da('" + chdr_dsn + "') shr msg(2)");

  rc = zut_loop_dynalloc(dds);
  if (0 != rc)
  {
    return RTNCD_FAILURE;
  }

  rc = zut_convert_dsect();
  if (0 != rc)
  {
    cerr << "Error: convert failed with rc: '" << rc << "'" << endl;
    cout << "  See '" << sysprint << "' and '" << sysout << "' for more details" << endl;
    return RTNCD_FAILURE;
  }

  cout << "DSECT converted to '" << chdr_dsn << "'" << endl;
  cout << "Copy it via `cp \"//'" + chdr_dsn + "'\" <member>.h`" << endl;

  // Free dynalloc dds
  zut_free_dynalloc_dds(dds);

  return rc;
}

int handle_tool_dynalloc(const ParseResult &result)
{
  int rc = 0;
  unsigned int code = 0;
  string resp;

  string parm = result.get_value<std::string>("parm", "");

  rc = zut_bpxwdyn(parm, &code, resp);
  if (0 != rc)
  {
    cerr << "Error: bpxwdyn with parm '" << parm << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << resp << endl;
    return RTNCD_FAILURE;
  }

  cout << resp << endl;

  return rc;
}

int handle_tool_display_symbol(const ParseResult &result)
{
  int rc = 0;
  string symbol = result.get_value<std::string>("symbol", "");
  transform(symbol.begin(), symbol.end(), symbol.begin(), ::toupper);
  symbol = "&" + symbol;
  string value;
  rc = zut_substitute_symbol(symbol, value);
  if (0 != rc)
  {
    cerr << "Error: asasymbf with parm '" << symbol << "' rc: '" << rc << "'" << endl;
    return RTNCD_FAILURE;
  }
  cout << value << endl;

  return RTNCD_SUCCESS;
}

int handle_tool_search(const ParseResult &result)
{
  int rc = 0;

  string pattern = result.get_value<std::string>("string", "");
  string warn = result.get_value<std::string>("warn", "");
  long long max_entries = result.get_value<long long>("max-entries", 0);
  string dsn = result.get_value<std::string>("dsn", "");

  ZDS zds = {0};
  bool results_truncated = false;

  if (max_entries > 0)
  {
    zds.max_entries = max_entries;
  }

  // List members in a data set
  vector<ZDSMem> members;
  rc = zds_list_members(&zds, dsn, members);

  // Note if results are truncated
  if (RTNCD_WARNING == rc)
  {
    if (ZDS_RSNCD_MAXED_ENTRIES_REACHED == zds.diag.detail_rc)
    {
      results_truncated = true;
    }
  }

  // Note failure if we can't list
  if (RTNCD_SUCCESS != rc && RTNCD_WARNING != rc)
  {
    cerr << "Error: could not read data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  // Perform dynalloc
  vector<string> dds;
  dds.reserve(3);
  dds.push_back("alloc dd(newdd) da('" + dsn + "') shr");
  dds.push_back("alloc dd(outdd)");
  dds.push_back("alloc dd(sysin)");

  rc = zut_loop_dynalloc(dds);
  if (0 != rc)
  {
    return RTNCD_FAILURE;
  }

  // Build super c selection criteria
  string data = " SRCHFOR '" + pattern + "'\n";

  for (vector<ZDSMem>::iterator it = members.begin(); it != members.end(); ++it)
  {
    data += " SELECT " + it->name + "\n";
  }

  // Write control statements
  zds_write_to_dd(&zds, "sysin", data);
  if (0 != rc)
  {
    cerr << "Error: could not write to dd: '" << "sysin" << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  // Perform search
  rc = zut_search("parms are unused for now but can be passed to super c, e.g. ANYC (any case)");
  if (RTNCD_SUCCESS != rc &&
      RTNCD_WARNING != rc &&
      ZUT_RTNCD_SEARCH_SUCCESS != rc &&
      ZUT_RTNCD_SEARCH_WARNING != rc)
  {
    cerr << "Error: could error invoking ISRSUPC rc: '" << rc << "'" << endl;
  }

  // Read output from super c
  string output;
  rc = zds_read_from_dd(&zds, "outdd", output);
  if (0 != rc)
  {
    cerr << "Error: could not read from dd: '" << "outdd" << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }
  cout << output << endl;

  if (results_truncated)
  {
    if (warn == "true")
    {
      cerr << "Warning: results truncated" << endl;
    }
  }

  // Free dynalloc dds
  zut_free_dynalloc_dds(dds);

  return RTNCD_SUCCESS;
}

int handle_tool_amblist(const ParseResult &result)
{
  int rc = 0;

  string dsn = result.get_value<std::string>("dsn", "");
  string statements = " " + result.get_value<std::string>("control-statements", "");

  // Perform dynalloc
  vector<string> dds;
  dds.reserve(3);
  dds.push_back("alloc dd(syslib) da('" + dsn + "') shr");
  dds.push_back("alloc dd(sysprint) lrecl(80) recfm(f,b) blksize(80)");
  dds.push_back("alloc dd(sysin) lrecl(80) recfm(f,b) blksize(80)");

  rc = zut_loop_dynalloc(dds);
  if (0 != rc)
  {
    return RTNCD_FAILURE;
  }

  transform(statements.begin(), statements.end(), statements.begin(), ::toupper);

  // Write control statements
  ZDS zds = {0};
  zds_write_to_dd(&zds, "sysin", statements);
  if (0 != rc)
  {
    cerr << "Error: could not write to dd: '" << "sysin" << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  // Perform search
  rc = zut_run("AMBLIST");
  if (RTNCD_SUCCESS != rc)
  {
    cerr << "Error: could error invoking AMBLIST rc: '" << rc << "'" << endl;
  }

  // Read output from amblist
  string output;
  rc = zds_read_from_dd(&zds, "sysprint", output);
  if (0 != rc)
  {
    cerr << "Error: could not read from dd: '" << "sysprint" << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }
  cout << output << endl;

  // Free dynalloc dds
  zut_free_dynalloc_dds(dds);

  return RTNCD_SUCCESS;
}

int handle_tool_run(const ParseResult &result)
{
  int rc = 0;
  string program = result.get_value<std::string>("program", "");
  string dynalloc_pre = result.get_value<std::string>("dynalloc-pre", "");
  string dynalloc_post = result.get_value<std::string>("dynalloc-post", "");

  // Allocate anything that was requested
  if (dynalloc_pre.length() > 0)
  {
    vector<string> dds;

    ifstream in(dynalloc_pre.c_str());
    if (!in.is_open())
    {
      cerr << "Error: could not open '" << dynalloc_pre << "'" << endl;
      return RTNCD_FAILURE;
    }

    string line;
    while (getline(in, line))
    {
      dds.push_back(line);
    }
    in.close();

    rc = zut_loop_dynalloc(dds);
    if (0 != rc)
    {
      return RTNCD_FAILURE;
    }
  }

  string indd = result.get_value<std::string>("in-dd", "");
  if (indd.length() > 0)
  {
    string ddname = "DD:" + indd;
    ofstream out(ddname.c_str());
    if (!out.is_open())
    {
      cerr << "Error: could not open input '" << ddname << "'" << endl;
      return RTNCD_FAILURE;
    }

    string input = result.get_value<std::string>("input", "");
    if (result.has("input"))
    {
      out << input << endl;
    }

    out.close();
  }

  transform(program.begin(), program.end(), program.begin(), ::toupper);

  rc = zut_run(program);

  if (0 != rc)
  {
    cerr << "Error: program '" << program << "' ended with rc: '" << rc << "'" << endl;
    rc = RTNCD_FAILURE;
  }

  string outdd = result.get_value<std::string>("out-dd", "");
  if (outdd.length() > 0)
  {
    string ddname = "DD:" + outdd;
    ifstream in(ddname.c_str());
    if (!in.is_open())
    {
      cerr << "Error: could not open output '" << ddname << "'" << endl;
      return RTNCD_FAILURE;
    }

    string line;
    while (getline(in, line))
    {
      cout << line << endl;
    }
    in.close();
  }

  // Optionally free everything that was allocated
  if (dynalloc_post.length() > 0)
  {
    vector<string> dds;

    ifstream in(dynalloc_post.c_str());
    if (!in.is_open())
    {
      cerr << "Error: could not open '" << dynalloc_post << "'" << endl;
    }

    string line;
    while (getline(in, line))
    {
      dds.push_back(line);
    }
    in.close();

    zut_loop_dynalloc(dds);
  }

  return rc;
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
  const auto is_interactive = result.get_value<bool>("interactive", false);
  if (result.get_value<bool>("version", false))
  {
    const auto version_rc = handle_version(result);
    if (!is_interactive)
    {
      return version_rc;
    }
  }

  if (is_interactive)
  {
    return run_interactive_mode(result.get_value<std::string>("shm-file", ""));
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
