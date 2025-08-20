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

int loop_dynalloc(vector<string> &list);
int free_dynalloc_dds(vector<string> &list);

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

  // Data set command group
  auto data_set_cmd = command_ptr(new Command("data-set", "z/OS data set operations"));
  data_set_cmd->add_alias("ds");

  // Common data set options that are reused
  auto encoding_option = make_aliases("--encoding", "--ec");
  auto etag_option = make_aliases("--etag");
  auto etag_only_option = make_aliases("--etag-only");
  auto return_etag_option = make_aliases("--return-etag");
  auto pipe_path_option = make_aliases("--pipe-path");
  auto response_format_csv_option = make_aliases("--response-format-csv", "--rfc");
  auto response_format_bytes_option = make_aliases("--response-format-bytes", "--rfb");

  // Create subcommand
  auto ds_create_cmd = command_ptr(new Command("create", "create data set"));
  ds_create_cmd->add_alias("cre");
  ds_create_cmd->add_positional_arg("dsn", "data set name, optionally with member specified", ArgType_Single, true);

  // Data set creation attributes
  ds_create_cmd->add_keyword_arg("alcunit", make_aliases("--alcunit"), "Allocation unit", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("blksize", make_aliases("--blksize"), "Block size", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("dirblk", make_aliases("--dirblk"), "Directory blocks", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("dsorg", make_aliases("--dsorg"), "Data set organization", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("primary", make_aliases("--primary"), "Primary space", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("recfm", make_aliases("--recfm"), "Record format", ArgType_Single, false, ArgValue(std::string("FB")));
  ds_create_cmd->add_keyword_arg("lrecl", make_aliases("--lrecl"), "Record length", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("dataclass", make_aliases("--dataclass"), "Data class", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("unit", make_aliases("--unit"), "Device type", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("dsntype", make_aliases("--dsntype"), "Data set type", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("mgntclass", make_aliases("--mgntclass"), "Management class", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("dsname", make_aliases("--dsname"), "Data set name", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("avgblk", make_aliases("--avgblk"), "Average block length", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("secondary", make_aliases("--secondary"), "Secondary space", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("size", make_aliases("--size"), "Size", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("storclass", make_aliases("--storclass"), "Storage class", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("vol", make_aliases("--vol"), "Volume serial", ArgType_Single, false);
  ds_create_cmd->set_handler(handle_data_set_create);
  data_set_cmd->add_command(ds_create_cmd);

  // Create-fb subcommand
  auto ds_create_fb_cmd = command_ptr(new Command("create-fb", "create FB data set using defaults: DSORG=PO, RECFM=FB, LRECL=80 "));
  ds_create_fb_cmd->add_alias("cre-fb");
  ds_create_fb_cmd->add_positional_arg("dsn", "data set name, optionally with member specified", ArgType_Single, true);
  ds_create_fb_cmd->set_handler(handle_data_set_create_fb);
  data_set_cmd->add_command(ds_create_fb_cmd);

  // Create-vb subcommand
  auto ds_create_vb_cmd = command_ptr(new Command("create-vb", "create VB data set using defaults: DSORG=PO, RECFM=VB, LRECL=255"));
  ds_create_vb_cmd->add_alias("cre-vb");
  ds_create_vb_cmd->add_positional_arg("dsn", "data set name, optionally with member specified", ArgType_Single, true);
  ds_create_vb_cmd->set_handler(handle_data_set_create_vb);
  data_set_cmd->add_command(ds_create_vb_cmd);

  // Create-adata subcommand
  auto ds_create_adata_cmd = command_ptr(new Command("create-adata", "create VB data set using defaults: DSORG=PO, RECFM=VB, LRECL=32756"));
  ds_create_adata_cmd->add_alias("cre-a");
  ds_create_adata_cmd->add_positional_arg("dsn", "data set name, optionally with member specified", ArgType_Single, true);
  ds_create_adata_cmd->set_handler(handle_data_set_create_adata);
  data_set_cmd->add_command(ds_create_adata_cmd);

  // Create-loadlib subcommand
  auto ds_create_loadlib_cmd = command_ptr(new Command("create-loadlib", "create loadlib data set using defaults: DSORG=PO, RECFM=U, LRECL=0"));
  ds_create_loadlib_cmd->add_alias("cre-u");
  ds_create_loadlib_cmd->add_positional_arg("dsn", "data set name, optionally with member specified", ArgType_Single, true);
  ds_create_loadlib_cmd->set_handler(handle_data_set_create_loadlib);
  data_set_cmd->add_command(ds_create_loadlib_cmd);

  // Create-member subcommand
  auto ds_create_member_cmd = command_ptr(new Command("create-member", "create member in data set"));
  ds_create_member_cmd->add_alias("cre-m");
  ds_create_member_cmd->add_positional_arg("dsn", "data set name with member specified", ArgType_Single, true);
  ds_create_member_cmd->set_handler(handle_data_set_create_member);
  data_set_cmd->add_command(ds_create_member_cmd);

  // View subcommand
  auto ds_view_cmd = command_ptr(new Command("view", "view data set"));
  ds_view_cmd->add_positional_arg("dsn", "data set name, optionally with member specified", ArgType_Single, true);
  ds_view_cmd->add_keyword_arg("encoding", encoding_option, "return contents in given encoding", ArgType_Single, false);
  ds_view_cmd->add_keyword_arg("response-format-bytes", response_format_bytes_option, "returns the response as raw bytes", ArgType_Flag, false, ArgValue(false));
  ds_view_cmd->add_keyword_arg("return-etag", return_etag_option, "Display the e-tag for a read response in addition to data", ArgType_Flag, false, ArgValue(false));
  ds_view_cmd->add_keyword_arg("pipe-path", pipe_path_option, "Specify a FIFO pipe path for transferring binary data", ArgType_Single, false);
  ds_view_cmd->set_handler(handle_data_set_view);
  data_set_cmd->add_command(ds_view_cmd);

  // List subcommand
  auto ds_list_cmd = command_ptr(new Command("list", "list data sets"));
  ds_list_cmd->add_alias("ls");
  ds_list_cmd->add_positional_arg("dsn", "data set name pattern", ArgType_Single, true);
  ds_list_cmd->add_keyword_arg("attributes", make_aliases("--attributes", "-a"), "display data set attributes", ArgType_Flag, false, ArgValue(false));
  ds_list_cmd->add_keyword_arg("max-entries", make_aliases("--max-entries", "--me"), "max number of results to return before warning generated", ArgType_Single, false);
  ds_list_cmd->add_keyword_arg("warn", make_aliases("--warn"), "warn if truncated or not found", ArgType_Flag, false, ArgValue(true));
  ds_list_cmd->add_keyword_arg("response-format-csv", response_format_csv_option, "returns the response in CSV format", ArgType_Flag, false, ArgValue(false));
  ds_list_cmd->set_handler(handle_data_set_list);
  data_set_cmd->add_command(ds_list_cmd);

  // List-members subcommand
  auto ds_list_members_cmd = command_ptr(new Command("list-members", "list data set members"));
  ds_list_members_cmd->add_alias("lm");
  ds_list_members_cmd->add_positional_arg("dsn", "data set name", ArgType_Single, true);
  ds_list_members_cmd->add_keyword_arg("max-entries", make_aliases("--max-entries", "--me"), "max number of results to return before warning generated", ArgType_Single, false);
  ds_list_members_cmd->add_keyword_arg("warn", make_aliases("--warn"), "warn if truncated or not found", ArgType_Flag, false, ArgValue(true));
  ds_list_members_cmd->set_handler(handle_data_set_list_members);
  data_set_cmd->add_command(ds_list_members_cmd);

  // Write subcommand
  auto ds_write_cmd = command_ptr(new Command("write", "write to data set"));
  ds_write_cmd->add_positional_arg("dsn", "data set name, optionally with member specified", ArgType_Single, true);
  ds_write_cmd->add_keyword_arg("encoding", encoding_option, "encoding for input data", ArgType_Single, false);
  ds_write_cmd->add_keyword_arg("etag", etag_option, "Provide the e-tag for a write response to detect conflicts before save", ArgType_Single, false);
  ds_write_cmd->add_keyword_arg("etag-only", etag_only_option, "Only print the e-tag for a write response (when successful)", ArgType_Flag, false, ArgValue(false));
  ds_write_cmd->add_keyword_arg("pipe-path", pipe_path_option, "Specify a FIFO pipe path for transferring binary data", ArgType_Single, false);
  ds_write_cmd->set_handler(handle_data_set_write);
  data_set_cmd->add_command(ds_write_cmd);

  // Delete subcommand
  auto ds_delete_cmd = command_ptr(new Command("delete", "delete data set"));
  ds_delete_cmd->add_alias("del");
  ds_delete_cmd->add_positional_arg("dsn", "data set name, optionally with member specified", ArgType_Single, true);
  ds_delete_cmd->set_handler(handle_data_set_delete);
  data_set_cmd->add_command(ds_delete_cmd);

  // Restore subcommand
  auto ds_restore_cmd = command_ptr(new Command("restore", "restore/recall data set"));
  ds_restore_cmd->add_positional_arg("dsn", "data set name", ArgType_Single, true);
  ds_restore_cmd->set_handler(handle_data_set_restore);
  data_set_cmd->add_command(ds_restore_cmd);

  // Compress subcommand
  auto ds_compress_cmd = command_ptr(new Command("compress", "compress data set"));
  ds_compress_cmd->add_positional_arg("dsn", "data set to compress", ArgType_Single, true);
  ds_compress_cmd->set_handler(handle_data_set_compress);
  data_set_cmd->add_command(ds_compress_cmd);

  arg_parser->get_root_command().add_command(data_set_cmd);

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

  // USS command group
  auto uss_cmd = command_ptr(new Command("uss", "z/OS USS operations"));

  // Common encoding/etag/pipe-path option helpers (reuse from data-set group)
  auto uss_encoding_option = make_aliases("--encoding", "--ec");
  auto uss_etag_option = make_aliases("--etag");
  auto uss_etag_only_option = make_aliases("--etag-only");
  auto uss_return_etag_option = make_aliases("--return-etag");
  auto uss_pipe_path_option = make_aliases("--pipe-path");
  auto uss_response_format_bytes_option = make_aliases("--response-format-bytes", "--rfb");

  // Create-file subcommand
  auto uss_create_file_cmd = command_ptr(new Command("create-file", "create a USS file"));
  uss_create_file_cmd->add_positional_arg("file-path", "file path", ArgType_Single, true);
  uss_create_file_cmd->add_keyword_arg("mode", make_aliases("--mode"), "permissions", ArgType_Single, false);
  uss_create_file_cmd->set_handler(handle_uss_create_file);
  uss_cmd->add_command(uss_create_file_cmd);

  // Create-dir subcommand
  auto uss_create_dir_cmd = command_ptr(new Command("create-dir", "create a USS directory"));
  uss_create_dir_cmd->add_positional_arg("file-path", "file path", ArgType_Single, true);
  uss_create_dir_cmd->add_keyword_arg("mode", make_aliases("--mode"), "permissions", ArgType_Single, false);
  uss_create_dir_cmd->set_handler(handle_uss_create_dir);
  uss_cmd->add_command(uss_create_dir_cmd);

  // List subcommand
  auto uss_list_cmd = command_ptr(new Command("list", "list USS files and directories"));
  uss_list_cmd->add_positional_arg("file-path", "file path", ArgType_Single, true);
  uss_list_cmd->add_keyword_arg("all", make_aliases("--all", "-a"), "list all files and directories", ArgType_Flag, false, ArgValue(false));
  uss_list_cmd->add_keyword_arg("long", make_aliases("--long", "-l"), "list long format", ArgType_Flag, false, ArgValue(false));
  uss_list_cmd->add_keyword_arg("response-format-csv", response_format_csv_option, "returns the response in CSV format", ArgType_Flag, false, ArgValue(false));
  uss_list_cmd->set_handler(handle_uss_list);
  uss_cmd->add_command(uss_list_cmd);

  // View subcommand
  auto uss_view_cmd = command_ptr(new Command("view", "view a USS file"));
  uss_view_cmd->add_positional_arg("file-path", "file path", ArgType_Single, true);
  uss_view_cmd->add_keyword_arg("encoding", uss_encoding_option, "return contents in given encoding", ArgType_Single, false);
  uss_view_cmd->add_keyword_arg("response-format-bytes", uss_response_format_bytes_option, "returns the response as raw bytes", ArgType_Flag, false, ArgValue(false));
  uss_view_cmd->add_keyword_arg("return-etag", uss_return_etag_option, "Display the e-tag for a read response in addition to data", ArgType_Flag, false, ArgValue(false));
  uss_view_cmd->add_keyword_arg("pipe-path", uss_pipe_path_option, "Specify a FIFO pipe path for transferring binary data", ArgType_Single, false);
  uss_view_cmd->set_handler(handle_uss_view);
  uss_cmd->add_command(uss_view_cmd);

  // Write subcommand
  auto uss_write_cmd = command_ptr(new Command("write", "write to a USS file"));
  uss_write_cmd->add_positional_arg("file-path", "file path", ArgType_Single, true);
  uss_write_cmd->add_keyword_arg("encoding", uss_encoding_option, "encoding for input data", ArgType_Single, false);
  uss_write_cmd->add_keyword_arg("etag", uss_etag_option, "Provide the e-tag for a write response to detect conflicts before save", ArgType_Single, false);
  uss_write_cmd->add_keyword_arg("etag-only", uss_etag_only_option, "Only print the e-tag for a write response (when successful)", ArgType_Flag, false, ArgValue(false));
  uss_write_cmd->add_keyword_arg("pipe-path", uss_pipe_path_option, "Specify a FIFO pipe path for transferring binary data", ArgType_Single, false);
  uss_write_cmd->set_handler(handle_uss_write);
  uss_cmd->add_command(uss_write_cmd);

  // Delete subcommand
  auto uss_delete_cmd = command_ptr(new Command("delete", "delete a USS item"));
  uss_delete_cmd->add_positional_arg("file-path", "file path", ArgType_Single, true);
  uss_delete_cmd->add_keyword_arg("recursive", make_aliases("--recursive", "-r"), "Applies the operation recursively (e.g. for folders w/ inner files)", ArgType_Flag, false, ArgValue(false));
  uss_delete_cmd->set_handler(handle_uss_delete);
  uss_cmd->add_command(uss_delete_cmd);

  // Chmod subcommand
  auto uss_chmod_cmd = command_ptr(new Command("chmod", "change permissions on a USS file or directory"));
  uss_chmod_cmd->add_positional_arg("mode", "new permissions for the file or directory", ArgType_Single, true);
  uss_chmod_cmd->add_positional_arg("file-path", "file path", ArgType_Single, true);
  uss_chmod_cmd->add_keyword_arg("recursive", make_aliases("--recursive", "-r"), "Applies the operation recursively (e.g. for folders w/ inner files)", ArgType_Flag, false, ArgValue(false));
  uss_chmod_cmd->set_handler(handle_uss_chmod);
  uss_cmd->add_command(uss_chmod_cmd);

  // Chown subcommand
  auto uss_chown_cmd = command_ptr(new Command("chown", "change owner on a USS file or directory"));
  uss_chown_cmd->add_positional_arg("owner", "New owner (or owner:group) for the file or directory", ArgType_Single, true);
  uss_chown_cmd->add_positional_arg("file-path", "file path", ArgType_Single, true);
  uss_chown_cmd->add_keyword_arg("recursive", make_aliases("--recursive", "-r"), "Applies the operation recursively (e.g. for folders w/ inner files)", ArgType_Flag, false, ArgValue(false));
  uss_chown_cmd->set_handler(handle_uss_chown);
  uss_cmd->add_command(uss_chown_cmd);

  // Chtag subcommand
  auto uss_chtag_cmd = command_ptr(new Command("chtag", "change tags on a USS file"));
  uss_chtag_cmd->add_positional_arg("file-path", "file path", ArgType_Single, true);
  uss_chtag_cmd->add_positional_arg("tag", "new tag for the file", ArgType_Single, true);
  uss_chtag_cmd->add_keyword_arg("recursive", make_aliases("--recursive", "-r"), "Applies the operation recursively (e.g. for folders w/ inner files)", ArgType_Flag, false, ArgValue(false));
  uss_chtag_cmd->set_handler(handle_uss_chtag);
  uss_cmd->add_command(uss_chtag_cmd);

  arg_parser->get_root_command().add_command(uss_cmd);

  // Job command group
  auto job_cmd = command_ptr(new Command("job", "z/OS job operations"));

  // List subcommand
  auto job_list_cmd = command_ptr(new Command("list", "list jobs"));
  job_list_cmd->add_keyword_arg("owner", make_aliases("--owner", "-o"), "filter by owner", ArgType_Single, false);
  job_list_cmd->add_keyword_arg("prefix", make_aliases("--prefix", "-p"), "filter by prefix", ArgType_Single, false);
  job_list_cmd->add_keyword_arg("max-entries", make_aliases("--max-entries", "--me"), "max number of results to return before warning generated", ArgType_Single, false);
  job_list_cmd->add_keyword_arg("warn", make_aliases("--warn"), "warn if truncated or not found", ArgType_Flag, false, ArgValue(true));
  job_list_cmd->add_keyword_arg("response-format-csv", response_format_csv_option, "returns the response in CSV format", ArgType_Flag, false, ArgValue(false));
  job_list_cmd->set_handler(handle_job_list);
  job_cmd->add_command(job_list_cmd);

  // List-files subcommand
  auto job_list_files_cmd = command_ptr(new Command("list-files", "list spool files for jobid"));
  job_list_files_cmd->add_alias("lf");
  job_list_files_cmd->add_positional_arg("jobid", "valid jobid or job correlator", ArgType_Single, true);
  job_list_files_cmd->add_keyword_arg("max-entries", make_aliases("--max-entries", "--me"), "max number of files to return before warning generated", ArgType_Single, false);
  job_list_files_cmd->add_keyword_arg("warn", make_aliases("--warn"), "warn if truncated or not found", ArgType_Flag, false, ArgValue(true));
  job_list_files_cmd->add_keyword_arg("response-format-csv", response_format_csv_option, "returns the response in CSV format", ArgType_Flag, false, ArgValue(false));
  job_list_files_cmd->set_handler(handle_job_list_files);
  job_cmd->add_command(job_list_files_cmd);

  // View-status subcommand
  auto job_view_status_cmd = command_ptr(new Command("view-status", "view job status"));
  job_view_status_cmd->add_alias("vs");
  job_view_status_cmd->add_positional_arg("jobid", "valid jobid or job correlator", ArgType_Single, true);
  job_view_status_cmd->add_keyword_arg("response-format-csv", response_format_csv_option, "returns the response in CSV format", ArgType_Flag, false, ArgValue(false));
  job_view_status_cmd->set_handler(handle_job_view_status);
  job_cmd->add_command(job_view_status_cmd);

  // View-file subcommand
  auto job_view_file_cmd = command_ptr(new Command("view-file", "view job file output"));
  job_view_file_cmd->add_alias("vf");
  job_view_file_cmd->add_positional_arg("jobid", "valid jobid or job correlator", ArgType_Single, true);
  job_view_file_cmd->add_positional_arg("key", "valid job dsn key via 'job list-files'", ArgType_Single, true);
  job_view_file_cmd->add_keyword_arg("encoding", encoding_option, "return contents in given encoding", ArgType_Single, false);
  job_view_file_cmd->add_keyword_arg("response-format-bytes", response_format_bytes_option, "returns the response as raw bytes", ArgType_Flag, false, ArgValue(false));
  job_view_file_cmd->set_handler(handle_job_view_file);
  job_cmd->add_command(job_view_file_cmd);

  // View-jcl subcommand
  auto job_view_jcl_cmd = command_ptr(new Command("view-jcl", "view job jcl from input jobid"));
  job_view_jcl_cmd->add_alias("vj");
  job_view_jcl_cmd->add_positional_arg("jobid", "valid jobid or job correlator", ArgType_Single, true);
  job_view_jcl_cmd->set_handler(handle_job_view_jcl);
  job_cmd->add_command(job_view_jcl_cmd);

  // Submit subcommand
  auto job_submit_cmd = command_ptr(new Command("submit", "submit a job"));
  job_submit_cmd->add_alias("sub");
  job_submit_cmd->add_positional_arg("dsn", "dsn containing JCL", ArgType_Single, true);
  job_submit_cmd->add_keyword_arg("wait", make_aliases("--wait"), "wait for job status", ArgType_Single, false);
  job_submit_cmd->add_keyword_arg("only-jobid", make_aliases("--only-jobid", "--oj"), "show only job id on success", ArgType_Flag, false, ArgValue(false));
  job_submit_cmd->add_keyword_arg("only-correlator", make_aliases("--only-correlator", "--oc"), "show only job correlator on success", ArgType_Flag, false, ArgValue(false));
  job_submit_cmd->set_handler(handle_job_submit);
  job_cmd->add_command(job_submit_cmd);

  // Submit-jcl subcommand
  auto job_submit_jcl_cmd = command_ptr(new Command("submit-jcl", "submit JCL contents directly"));
  job_submit_jcl_cmd->add_alias("subj");
  job_submit_jcl_cmd->add_keyword_arg("wait", make_aliases("--wait"), "wait for job status", ArgType_Single, false);
  job_submit_jcl_cmd->add_keyword_arg("only-jobid", make_aliases("--only-jobid", "--oj"), "show only job id on success", ArgType_Flag, false, ArgValue(false));
  job_submit_jcl_cmd->add_keyword_arg("only-correlator", make_aliases("--only-correlator", "--oc"), "show only job correlator on success", ArgType_Flag, false, ArgValue(false));
  job_submit_jcl_cmd->add_keyword_arg("encoding", encoding_option, "encoding for input data", ArgType_Single, false);
  job_submit_jcl_cmd->set_handler(handle_job_submit_jcl);
  job_cmd->add_command(job_submit_jcl_cmd);

  // Submit-uss subcommand
  auto job_submit_uss_cmd = command_ptr(new Command("submit-uss", "submit a job from USS files"));
  job_submit_uss_cmd->add_alias("sub-u");
  job_submit_uss_cmd->add_positional_arg("file-path", "USS file containing JCL", ArgType_Single, true);
  job_submit_uss_cmd->add_keyword_arg("wait", make_aliases("--wait"), "wait for job status", ArgType_Single, false);
  job_submit_uss_cmd->add_keyword_arg("only-jobid", make_aliases("--only-jobid", "--oj"), "show only job id on success", ArgType_Flag, false, ArgValue(false));
  job_submit_uss_cmd->add_keyword_arg("only-correlator", make_aliases("--only-correlator", "--oc"), "show only job correlator on success", ArgType_Flag, false, ArgValue(false));
  job_submit_uss_cmd->set_handler(handle_job_submit_uss);
  job_cmd->add_command(job_submit_uss_cmd);

  // Delete subcommand
  auto job_delete_cmd = command_ptr(new Command("delete", "delete a job"));
  job_delete_cmd->add_alias("del");
  job_delete_cmd->add_positional_arg("jobid", "valid jobid or job correlator", ArgType_Single, true);
  job_delete_cmd->set_handler(handle_job_delete);
  job_cmd->add_command(job_delete_cmd);

  // Cancel subcommand
  auto job_cancel_cmd = command_ptr(new Command("cancel", "cancel a job"));
  job_cancel_cmd->add_alias("cnl");
  job_cancel_cmd->add_positional_arg("jobid", "valid jobid or job correlator", ArgType_Single, true);
  job_cancel_cmd->add_keyword_arg("dump", make_aliases("--dump", "-d"), "Dump the cancelled jobs if waiting for conversion, in conversion, or in execution", ArgType_Flag, false, ArgValue(false));
  job_cancel_cmd->add_keyword_arg("force", make_aliases("--force", "-f"), "Force cancel the jobs, even if marked", ArgType_Flag, false, ArgValue(false));
  job_cancel_cmd->add_keyword_arg("purge", make_aliases("--purge", "-p"), "Purge output of the cancelled jobs", ArgType_Flag, false, ArgValue(false));
  job_cancel_cmd->add_keyword_arg("restart", make_aliases("--restart", "-r"), "Request that automatic restart management automatically restart the selected jobs after they are cancelled", ArgType_Flag, false, ArgValue(false));
  job_cancel_cmd->set_handler(handle_job_cancel);
  job_cmd->add_command(job_cancel_cmd);

  // Hold subcommand
  auto job_hold_cmd = command_ptr(new Command("hold", "hold a job"));
  job_hold_cmd->add_alias("hld");
  job_hold_cmd->add_positional_arg("jobid", "valid jobid or job correlator", ArgType_Single, true);
  job_hold_cmd->set_handler(handle_job_hold);
  job_cmd->add_command(job_hold_cmd);

  // Release subcommand
  auto job_release_cmd = command_ptr(new Command("release", "release a job"));
  job_release_cmd->add_alias("rel");
  job_release_cmd->add_positional_arg("jobid", "valid jobid or job correlator", ArgType_Single, true);
  job_release_cmd->set_handler(handle_job_release);
  job_cmd->add_command(job_release_cmd);

  arg_parser->get_root_command().add_command(job_cmd);

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
