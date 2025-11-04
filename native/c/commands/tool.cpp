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

#include "tool.hpp"
#include "common_args.hpp"
#include "../zcn.hpp"
#include "../zds.hpp"
#include "../zut.hpp"
#include "../zuttype.h"
#include <unistd.h>

using namespace parser;
using namespace std;
using namespace commands::common;

namespace tool
{

int handle_tool_convert_dsect(InvocationContext &context)
{
  int rc = 0;
  ZCN zcn = {0};
  unsigned int code = 0;
  string resp;

  string adata_dsn = context.get<std::string>("adata-dsn", "");
  string chdr_dsn = context.get<std::string>("chdr-dsn", "");
  string sysprint = context.get<std::string>("sysprint", "");
  string sysout = context.get<std::string>("sysout", "");

  const char *user = getlogin();
  string struser(user);
  transform(struser.begin(), struser.end(), struser.begin(), ::tolower);

  if (sysprint.empty())
    sysprint = "/tmp/" + struser + "_sysprint.txt";
  if (sysout.empty())
    sysout = "/tmp/" + struser + "_sysout.txt";

  context.output_stream() << adata_dsn << " " << chdr_dsn << " " << sysprint << " " << sysout << endl;

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
    context.error_stream() << "Error: convert failed with rc: '" << rc << "'" << endl;
    context.output_stream() << "  See '" << sysprint << "' and '" << sysout << "' for more details" << endl;
    return RTNCD_FAILURE;
  }

  context.output_stream() << "DSECT converted to '" << chdr_dsn << "'" << endl;
  context.output_stream() << "Copy it via `cp \"//'" + chdr_dsn + "'\" <member>.h`" << endl;

  // Free dynalloc dds
  zut_free_dynalloc_dds(dds, &context.error_stream());

  return rc;
}

int handle_tool_dynalloc(InvocationContext &context)
{
  int rc = 0;
  unsigned int code = 0;
  string resp;

  string parm = context.get<std::string>("parm", "");

  rc = zut_bpxwdyn(parm, &code, resp);
  if (0 != rc)
  {
    context.error_stream() << "Error: bpxwdyn with parm '" << parm << "' rc: '" << rc << "'" << endl;
    context.error_stream() << "  Details: " << resp << endl;
    return RTNCD_FAILURE;
  }

  context.output_stream() << resp << endl;

  return rc;
}

int handle_tool_display_symbol(InvocationContext &context)
{
  int rc = 0;
  string symbol = context.get<std::string>("symbol", "");
  transform(symbol.begin(), symbol.end(), symbol.begin(), ::toupper);
  symbol = "&" + symbol;
  string value;
  rc = zut_substitute_symbol(symbol, value);
  if (0 != rc)
  {
    context.error_stream() << "Error: asasymbf with parm '" << symbol << "' rc: '" << rc << "'" << endl;
    return RTNCD_FAILURE;
  }
  context.output_stream() << value << endl;

  return RTNCD_SUCCESS;
}

int handle_tool_list_parmlib(InvocationContext &context)
{
  int rc = 0;
  ZDIAG diag = {0};
  std::vector<std::string> parmlibs;
  rc = zut_list_parmlib(diag, parmlibs);
  if (0 != rc)
  {
    context.error_stream() << "Error: could not list parmlibs rc: '" << rc << "'" << endl;
    context.error_stream() << "  Details: " << diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  for (vector<string>::iterator it = parmlibs.begin(); it != parmlibs.end(); ++it)
  {
    context.output_stream() << *it << endl;
  }

  return rc;
}

int handle_tool_search(InvocationContext &context)
{
  int rc = 0;

  string pattern = context.get<std::string>("string", "");
  string parms = context.get<std::string>("parms", "");
  string dsn = context.get<std::string>("dsn", "");

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

  // Write control statements
  ZDS zds = {0};
  zds_write_to_dd(&zds, "sysin", data);
  if (0 != rc)
  {
    context.error_stream() << "Error: could not write to dd: '" << "sysin" << "' rc: '" << rc << "'" << endl;
    context.error_stream() << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  transform(parms.begin(), parms.end(), parms.begin(), ::toupper);

  // Perform search
  rc = zut_search(parms);
  if (RTNCD_SUCCESS != rc &&
      RTNCD_WARNING != rc &&
      ZUT_RTNCD_SEARCH_SUCCESS != rc &&
      ZUT_RTNCD_SEARCH_WARNING != rc)
  {
    context.error_stream() << "Error: could not invoke ISRSUPC rc: '" << rc << "'" << endl;
    zut_free_dynalloc_dds(dds, &context.error_stream());
    return RTNCD_FAILURE;
  }

  // Read output from super c
  string output;
  rc = zds_read_from_dd(&zds, "outdd", output);
  if (0 != rc)
  {
    context.error_stream() << "Error: could not read from dd: '" << "outdd" << "' rc: '" << rc << "'" << endl;
    context.error_stream() << "  Details: " << zds.diag.e_msg << endl;
    zut_free_dynalloc_dds(dds, &context.error_stream());
    return RTNCD_FAILURE;
  }
  context.output_stream() << output << endl;

  // Free dynalloc dds
  zut_free_dynalloc_dds(dds, &context.error_stream());

  return RTNCD_SUCCESS;
}

int handle_tool_amblist(InvocationContext &context)
{
  int rc = 0;

  string dsn = context.get<std::string>("dsn", "");
  string statements = " " + context.get<std::string>("control-statements", "");

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
    context.error_stream() << "Error: could not write to dd: '" << "sysin" << "' rc: '" << rc << "'" << endl;
    context.error_stream() << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  // Perform search
  rc = zut_run("AMBLIST");
  if (RTNCD_SUCCESS != rc)
  {
    context.error_stream() << "Error: could not invoke AMBLIST rc: '" << rc << "'" << endl;
    return RTNCD_FAILURE;
  }

  // Read output from amblist
  string output;
  rc = zds_read_from_dd(&zds, "sysprint", output);
  if (0 != rc)
  {
    context.error_stream() << "Error: could not read from dd: '" << "sysprint" << "' rc: '" << rc << "'" << endl;
    context.error_stream() << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }
  context.output_stream() << output << endl;

  // Free dynalloc dds
  zut_free_dynalloc_dds(dds, &context.error_stream());

  return RTNCD_SUCCESS;
}

int handle_tool_run(InvocationContext &context)
{
  int rc = 0;
  string program = context.get<std::string>("program", "");
  string dynalloc_pre = context.get<std::string>("dynalloc-pre", "");
  string dynalloc_post = context.get<std::string>("dynalloc-post", "");

  // Allocate anything that was requested
  if (dynalloc_pre.length() > 0)
  {
    vector<string> dds;

    ifstream in(dynalloc_pre.c_str());
    if (!in.is_open())
    {
      context.error_stream() << "Error: could not open '" << dynalloc_pre << "'" << endl;
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

  string indd = context.get<std::string>("in-dd", "");
  if (indd.length() > 0)
  {
    string ddname = "DD:" + indd;
    ofstream out(ddname.c_str());
    if (!out.is_open())
    {
      context.error_stream() << "Error: could not open input '" << ddname << "'" << endl;
      return RTNCD_FAILURE;
    }

    string input = context.get<std::string>("input", "");
    if (context.has("input"))
    {
      out << input << endl;
    }

    out.close();
  }

  transform(program.begin(), program.end(), program.begin(), ::toupper);

  rc = zut_run(program);

  if (0 != rc)
  {
    context.error_stream() << "Error: program '" << program << "' ended with rc: '" << rc << "'" << endl;
    rc = RTNCD_FAILURE;
  }

  string outdd = context.get<std::string>("out-dd", "");
  if (outdd.length() > 0)
  {
    string ddname = "DD:" + outdd;
    ifstream in(ddname.c_str());
    if (!in.is_open())
    {
      context.error_stream() << "Error: could not open output '" << ddname << "'" << endl;
      return RTNCD_FAILURE;
    }

    string line;
    while (getline(in, line))
    {
      context.output_stream() << line << endl;
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
      context.error_stream() << "Error: could not open '" << dynalloc_post << "'" << endl;
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

void register_commands(parser::Command &root_command)
{
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
  tool_dynalloc_cmd->add_positional_arg("parm", "dynalloc parm string", ArgType_Single, false);
  tool_dynalloc_cmd->set_handler(handle_tool_dynalloc);
  tool_cmd->add_command(tool_dynalloc_cmd);

  // Display symbol subcommand
  auto tool_display_symbol_cmd = command_ptr(new Command("display-symbol", "display system symbol"));
  tool_display_symbol_cmd->add_positional_arg("symbol", "symbol to display", ArgType_Single, true);
  tool_display_symbol_cmd->set_handler(handle_tool_display_symbol);
  tool_cmd->add_command(tool_display_symbol_cmd);

  // List-parmlib subcommand
  auto tool_list_parmlib_cmd = command_ptr(new Command("list-parmlib", "list parmlib"));
  tool_list_parmlib_cmd->set_handler(handle_tool_list_parmlib);
  tool_cmd->add_command(tool_list_parmlib_cmd);

  // Search subcommand
  auto tool_search_cmd = command_ptr(new Command("search", "search members for string with parms, e.g. --parms anyc"));
  tool_search_cmd->add_positional_arg(DSN);
  tool_search_cmd->add_positional_arg("string", "string to search for", ArgType_Single, true);
  tool_search_cmd->add_keyword_arg("parms",
                                   make_aliases("--parms", "--p"),
                                   "parms to pass to ISRSUPC", ArgType_Single, false);
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

  root_command.add_command(tool_cmd);
}
} // namespace tool