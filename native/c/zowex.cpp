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
#include <algorithm>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include "zcn.hpp"
#include "zut.hpp"
#include "parser.hpp"
#include "zjb.hpp"
#include "zds.hpp"
#include "zusf.hpp"
#include "ztso.hpp"
#include "zshmem.hpp"
#include "zuttype.h"

#ifndef TO_STRING
#define TO_STRING(x) static_cast<std::ostringstream &>(           \
                         (std::ostringstream() << std::dec << x)) \
                         .str()
#endif

// Version information
#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "unknown"
#endif

#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

using namespace parser;
using namespace std;

int free_dynalloc_dds(vector<string> &list);

int handle_console_issue(const ParseResult &result);

int handle_tso_issue(const ParseResult &result);

int handle_data_set_create(const ParseResult &result);
int handle_data_set_create_fb(const ParseResult &result);
int handle_data_set_create_vb(const ParseResult &result);
int handle_data_set_create_adata(const ParseResult &result);
int handle_data_set_create_loadlib(const ParseResult &result);
int handle_data_set_view(const ParseResult &result);
int handle_data_set_list(const ParseResult &result);
int handle_data_set_list_members(const ParseResult &result);
int handle_data_set_write(const ParseResult &result);
int handle_data_set_delete(const ParseResult &result);
int handle_data_set_restore(const ParseResult &result);
int handle_data_set_compress(const ParseResult &result);
int handle_data_set_create_member(const ParseResult &result);

int handle_tool_convert_dsect(const ParseResult &result);
int handle_tool_dynalloc(const ParseResult &result);
int handle_tool_display_symbol(const ParseResult &result);
int handle_tool_search(const ParseResult &result);
int handle_tool_amblist(const ParseResult &result);
int handle_tool_run(const ParseResult &result);

int handle_uss_create_file(const ParseResult &result);
int handle_uss_create_dir(const ParseResult &result);
int handle_uss_list(const ParseResult &result);
int handle_uss_view(const ParseResult &result);
int handle_uss_write(const ParseResult &result);
int handle_uss_delete(const ParseResult &result);
int handle_uss_chmod(const ParseResult &result);
int handle_uss_chown(const ParseResult &result);
int handle_uss_chtag(const ParseResult &result);

int handle_version(const ParseResult &result);

int handle_root_command(const ParseResult &result);

int handle_job_list(const ParseResult &result);
int handle_job_list_files(const ParseResult &result);
int handle_job_view_status(const ParseResult &result);
int handle_job_view_file(const ParseResult &result);
int handle_job_view_jcl(const ParseResult &result);
int handle_job_submit(const ParseResult &result);
int handle_job_submit_jcl(const ParseResult &result);
int handle_job_submit_uss(const ParseResult &result);
int handle_job_delete(const ParseResult &result);
int handle_job_cancel(const ParseResult &result);
int handle_job_hold(const ParseResult &result);
int handle_job_release(const ParseResult &result);

int loop_dynalloc(vector<string> &list);

bool should_quit(const std::string &input);
int run_interactive_mode(const std::string &shm_file_path);

std::tr1::shared_ptr<ArgumentParser> arg_parser;
int main(int argc, char *argv[])
{
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

int handle_console_issue(const ParseResult &result)
{
  int rc = 0;
  ZCN zcn = {0};

  string console_name = result.find_kw_arg_string("console-name");
  int timeout = result.find_kw_arg_int("timeout");

  string command = result.find_pos_arg_string("command");
  bool wait = result.find_kw_arg_bool("wait");

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
  string command = result.find_pos_arg_string("command");
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

int process_data_set_create_result(ZDS *zds, int rc, string dsn, string response)
{
  if (0 != rc)
  {
    cerr << "Error: could not create data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << response << endl;
    return RTNCD_FAILURE;
  }

  // Handle member creation if specified
  size_t start = dsn.find_first_of('(');
  size_t end = dsn.find_last_of(')');
  if (start != string::npos && end != string::npos && end > start)
  {
    string member_name = dsn.substr(start + 1, end - start - 1);
    string data = "";
    rc = zds_write_to_dsn(zds, dsn, data);
    if (0 != rc)
    {
      cout << "Error: could not write to data set: '" << dsn << "' rc: '" << rc << "'" << endl;
      cout << "  Details: " << zds->diag.e_msg << endl;
      return RTNCD_FAILURE;
    }
    cout << "Data set and/or member created: '" << dsn << "'" << endl;
  }
  else
  {
    cout << "Data set created: '" << dsn << "'" << endl;
  }

  return rc;
}

int handle_data_set_create(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};
  DS_ATTRIBUTES attributes = {0};

  // Extract all the optional creation attributes
  if (result.has_kw_arg("alcunit"))
  {
    attributes.alcunit = result.find_kw_arg_string("alcunit");
  }
  if (result.has_kw_arg("blksize"))
  {
    string blksize_str = result.find_kw_arg_string("blksize");
    if (!blksize_str.empty())
    {
      attributes.blksize = std::strtoul(blksize_str.c_str(), nullptr, 10);
    }
  }
  if (result.has_kw_arg("dirblk"))
  {
    string dirblk_str = result.find_kw_arg_string("dirblk");
    if (!dirblk_str.empty())
    {
      attributes.dirblk = std::strtoul(dirblk_str.c_str(), nullptr, 10);
    }
  }
  if (result.has_kw_arg("dsorg"))
  {
    attributes.dsorg = result.find_kw_arg_string("dsorg");
  }
  if (result.has_kw_arg("primary"))
  {
    string primary_str = result.find_kw_arg_string("primary");
    if (!primary_str.empty())
    {
      attributes.primary = std::strtoul(primary_str.c_str(), nullptr, 10);
    }
  }
  if (result.has_kw_arg("recfm"))
  {
    attributes.recfm = result.find_kw_arg_string("recfm");
  }
  if (result.has_kw_arg("lrecl"))
  {
    string lrecl_str = result.find_kw_arg_string("lrecl");
    if (!lrecl_str.empty())
    {
      attributes.lrecl = std::strtoul(lrecl_str.c_str(), nullptr, 10);
    }
  }
  if (result.has_kw_arg("dataclass"))
  {
    attributes.dataclass = result.find_kw_arg_string("dataclass");
  }
  if (result.has_kw_arg("unit"))
  {
    attributes.unit = result.find_kw_arg_string("unit");
  }
  if (result.has_kw_arg("dsntype"))
  {
    attributes.dsntype = result.find_kw_arg_string("dsntype");
  }
  if (result.has_kw_arg("mgntclass"))
  {
    attributes.mgntclass = result.find_kw_arg_string("mgntclass");
  }
  if (result.has_kw_arg("dsname"))
  {
    attributes.dsname = result.find_kw_arg_string("dsname");
  }
  if (result.has_kw_arg("avgblk"))
  {
    string avgblk_str = result.find_kw_arg_string("avgblk");
    if (!avgblk_str.empty())
    {
      attributes.avgblk = std::strtoul(avgblk_str.c_str(), nullptr, 10);
    }
  }
  if (result.has_kw_arg("secondary"))
  {
    string secondary_str = result.find_kw_arg_string("secondary");
    if (!secondary_str.empty())
    {
      attributes.secondary = std::strtoul(secondary_str.c_str(), nullptr, 10);
    }
  }
  if (result.has_kw_arg("size"))
  {
    string size_str = result.find_kw_arg_string("size");
    if (!size_str.empty())
    {
      attributes.size = std::strtoul(size_str.c_str(), nullptr, 10);
    }
  }
  if (result.has_kw_arg("storclass"))
  {
    attributes.storclass = result.find_kw_arg_string("storclass");
  }
  if (result.has_kw_arg("vol"))
  {
    attributes.vol = result.find_kw_arg_string("vol");
  }

  string response;
  rc = zds_create_dsn(&zds, dsn, attributes, response);
  return process_data_set_create_result(&zds, rc, dsn, response);
}

int handle_data_set_create_fb(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};
  string response;
  rc = zds_create_dsn_fb(&zds, dsn, response);
  return process_data_set_create_result(&zds, rc, dsn, response);
}

int handle_data_set_create_vb(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};
  string response;
  rc = zds_create_dsn_vb(&zds, dsn, response);
  return process_data_set_create_result(&zds, rc, dsn, response);
}

int handle_data_set_create_adata(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};
  string response;
  rc = zds_create_dsn_adata(&zds, dsn, response);
  return process_data_set_create_result(&zds, rc, dsn, response);
}

int handle_data_set_create_loadlib(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};
  string response;
  rc = zds_create_dsn_loadlib(&zds, dsn, response);
  return process_data_set_create_result(&zds, rc, dsn, response);
}

int handle_data_set_create_member(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};
  string response;
  vector<ZDSEntry> entries;

  size_t start = dsn.find_first_of('(');
  size_t end = dsn.find_last_of(')');
  string member_name;
  if (start != string::npos && end != string::npos && end > start)
  {
    member_name = dsn.substr(start + 1, end - start - 1);
    string dataset_name = dsn.substr(0, start);

    rc = zds_list_data_sets(&zds, dataset_name, entries);
    if (RTNCD_WARNING < rc || entries.size() == 0)
    {
      cout << "Error: could not create data set member: '" << dataset_name << "' rc: '" << rc << "'" << endl;
      cout << "  Details:\n"
           << zds.diag.e_msg << endl;
      return RTNCD_FAILURE;
    }

    string data = "";
    rc = zds_write_to_dsn(&zds, dsn, data);
    if (0 != rc)
    {
      cout << "Error: could not write to data set: '" << dsn << "' rc: '" << rc << "'" << endl;
      cout << "  Details: " << zds.diag.e_msg << endl;
      return RTNCD_FAILURE;
    }
    cout << "Data set and/or member created: '" << dsn << "'" << endl;
  }
  else
  {
    cout << "Error: could not find member name in dsn: '" << dsn << "'" << endl;
    return RTNCD_FAILURE;
  }

  return rc;
}

int handle_data_set_view(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};

  if (result.has_kw_arg("encoding"))
  {
    zut_prepare_encoding(result.find_kw_arg_string("encoding"), &zds.encoding_opts);
  }

  bool has_pipe_path = result.has_kw_arg("pipe-path");
  string pipe_path = result.find_kw_arg_string("pipe-path");

  if (has_pipe_path && !pipe_path.empty())
  {
    size_t content_len = 0;
    rc = zds_read_from_dsn_streamed(&zds, dsn, pipe_path, &content_len);

    if (result.find_kw_arg_bool("return-etag"))
    {
      string temp_content;
      auto read_rc = zds_read_from_dsn(&zds, dsn, temp_content);
      if (read_rc == 0)
      {
        const auto etag = zut_calc_adler32_checksum(temp_content);
        cout << "etag: " << std::hex << etag << std::dec << endl;
      }
      cout << "size: " << content_len << endl;
    }
  }
  else
  {
    string response;
    rc = zds_read_from_dsn(&zds, dsn, response);
    if (0 != rc)
    {
      cerr << "Error: could not read data set: '" << dsn << "' rc: '" << rc << "'" << endl;
      cerr << "  Details: " << zds.diag.e_msg << endl;
      return RTNCD_FAILURE;
    }

    if (result.find_kw_arg_bool("return-etag"))
    {
      const auto etag = zut_calc_adler32_checksum(response);
      cout << "etag: " << std::hex << etag << std::dec << endl;
      cout << "data: ";
    }

    bool has_encoding = result.has_kw_arg("encoding");
    bool response_format_bytes = result.find_kw_arg_bool("response-format-bytes");

    if (has_encoding && response_format_bytes)
    {
      zut_print_string_as_bytes(response);
    }
    else
    {
      cout << response;
    }
  }

  return rc;
}

int handle_data_set_list(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");

  if (dsn.length() > MAX_DS_LENGTH)
  {
    cerr << "Error: data set pattern exceeds 44 character length limit" << endl;
    return RTNCD_FAILURE;
  }

  dsn += ".**";

  int max_entries = result.find_kw_arg_int("max-entries");
  bool warn = result.find_kw_arg_bool("warn", true);
  bool attributes = result.find_kw_arg_bool("attributes");

  ZDS zds = {0};
  if (max_entries > 0)
  {
    zds.max_entries = max_entries;
  }
  vector<ZDSEntry> entries;

  bool emit_csv = result.find_kw_arg_bool("response-format-csv");
  rc = zds_list_data_sets(&zds, dsn, entries);
  if (RTNCD_SUCCESS == rc || RTNCD_WARNING == rc)
  {
    vector<string> fields;
    for (vector<ZDSEntry>::iterator it = entries.begin(); it != entries.end(); ++it)
    {
      if (emit_csv)
      {
        fields.push_back(it->name);
        if (attributes)
        {
          fields.push_back(it->dsorg);
          fields.push_back(it->volser);
          fields.push_back(it->migr ? "true" : "false");
          fields.push_back(it->recfm);
        }
        cout << zut_format_as_csv(fields) << endl;
        fields.clear();
      }
      else
      {
        if (attributes)
        {
          cout << left << setw(44) << it->name << " " << it->volser << " " << setw(4) << it->dsorg << " " << setw(6) << it->recfm << endl;
        }
        else
        {
          cout << left << setw(44) << it->name << endl;
        }
      }
    }
  }
  if (RTNCD_WARNING == rc)
  {
    if (warn)
    {
      if (ZDS_RSNCD_MAXED_ENTRIES_REACHED == zds.diag.detail_rc)
      {
        cerr << "Warning: results truncated" << endl;
      }
      else if (ZDS_RSNCD_NOT_FOUND == zds.diag.detail_rc)
      {
        cerr << "Warning: no matching results found" << endl;
      }
    }
  }

  if (RTNCD_SUCCESS != rc && RTNCD_WARNING != rc)
  {
    cerr << "Error: could not list data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  return (!warn && rc == RTNCD_WARNING) ? RTNCD_SUCCESS : rc;
}

int handle_data_set_list_members(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  int max_entries = result.find_kw_arg_int("max-entries");
  bool warn = result.find_kw_arg_bool("warn", true);

  ZDS zds = {0};
  if (max_entries > 0)
  {
    zds.max_entries = max_entries;
  }
  vector<ZDSMem> members;
  rc = zds_list_members(&zds, dsn, members);

  if (RTNCD_SUCCESS == rc || RTNCD_WARNING == rc)
  {
    for (vector<ZDSMem>::iterator it = members.begin(); it != members.end(); ++it)
    {
      cout << left << setw(12) << it->name << endl;
    }
  }
  if (RTNCD_WARNING == rc)
  {
    if (warn)
    {
      if (ZDS_RSNCD_MAXED_ENTRIES_REACHED == zds.diag.detail_rc)
      {
        cerr << "Warning: results truncated" << endl;
      }
    }
  }
  if (RTNCD_SUCCESS != rc && RTNCD_WARNING != rc)
  {
    cerr << "Error: could not read data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  return (!warn && rc == RTNCD_WARNING) ? RTNCD_SUCCESS : rc;
}

int handle_data_set_write(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};

  if (result.has_kw_arg("encoding"))
  {
    zut_prepare_encoding(result.find_kw_arg_string("encoding"), &zds.encoding_opts);
  }

  if (result.has_kw_arg("etag"))
  {
    string etag_value = result.find_kw_arg_string("etag");
    if (!etag_value.empty())
    {
      strcpy(zds.etag, etag_value.c_str());
    }
  }

  bool has_pipe_path = result.has_kw_arg("pipe-path");
  string pipe_path = result.find_kw_arg_string("pipe-path");
  size_t content_len = 0;

  if (has_pipe_path && !pipe_path.empty())
  {
    rc = zds_write_to_dsn_streamed(&zds, dsn, pipe_path, &content_len);
  }
  else
  {
    string data;
    string line;

    if (!isatty(fileno(stdout)))
    {
      std::istreambuf_iterator<char> begin(std::cin);
      std::istreambuf_iterator<char> end;

      vector<char> input(begin, end);
      const auto temp = string(input.begin(), input.end());
      input.clear();
      const auto bytes = zut_get_contents_as_bytes(temp);

      data.assign(bytes.begin(), bytes.end());
    }
    else
    {
      while (getline(cin, line))
      {
        data += line;
        data.push_back('\n');
      }
    }

    rc = zds_write_to_dsn(&zds, dsn, data);
  }

  if (0 != rc)
  {
    cerr << "Error: could not write to data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  if (result.find_kw_arg_bool("etag-only"))
  {
    cout << "etag: " << zds.etag << endl;
    if (content_len > 0)
      cout << "size: " << content_len << endl;
  }
  else
  {
    cout << "Wrote data to '" << dsn << "'" << endl;
  }

  return rc;
}

int handle_data_set_delete(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};
  rc = zds_delete_dsn(&zds, dsn);

  if (0 != rc)
  {
    cerr << "Error: could not delete data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }
  cout << "Data set '" << dsn << "' deleted" << endl;

  return rc;
}

int handle_data_set_restore(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};
  string response;
  unsigned int code = 0;

  // perform dynalloc
  vector<string> dds;
  dds.push_back("alloc da('" + dsn + "') shr");
  dds.push_back("free da('" + dsn + "')");

  rc = loop_dynalloc(dds);
  if (0 != rc)
  {
    return RTNCD_FAILURE;
  }

  cout << "Data set '" << dsn << "' restored" << endl;

  return rc;
}

int handle_data_set_compress(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");

  transform(dsn.begin(), dsn.end(), dsn.begin(), ::toupper);

  bool is_pds = false;

  string dsn_formatted = "//'" + dsn + "'";
  FILE *dir = fopen(dsn_formatted.c_str(), "r");
  if (dir)
  {
    fldata_t file_info = {0};
    char file_name[64] = {0};
    if (0 == fldata(dir, file_name, &file_info))
    {
      if (file_info.__dsorgPDSdir && !file_info.__dsorgPDSE)
      {
        is_pds = true;
      }
      fclose(dir);
    }
  }

  if (!is_pds)
  {
    cerr << "Error: data set'" << dsn << "' is not a PDS'" << endl;
    return RTNCD_FAILURE;
  }

  // perform dynalloc
  vector<string> dds;
  dds.push_back("alloc dd(a) da('" + dsn + "') shr");
  dds.push_back("alloc dd(b) da('" + dsn + "') shr");
  dds.push_back("alloc dd(sysprint) lrecl(80) recfm(f,b) blksize(80)");
  dds.push_back("alloc dd(sysin) lrecl(80) recfm(f,b) blksize(80)");

  rc = loop_dynalloc(dds);
  if (0 != rc)
  {
    return RTNCD_FAILURE;
  }

  // write control statements
  ZDS zds = {0};
  zds_write_to_dd(&zds, "sysin", "        COPY OUTDD=B,INDD=A");
  if (0 != rc)
  {
    cerr << "Error: could not write to dd: '" << "sysin" << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  // perform compress
  rc = zut_run("IEBCOPY");
  if (RTNCD_SUCCESS != rc)
  {
    cerr << "Error: could error invoking IEBCOPY rc: '" << rc << "'" << endl;
  }

  // read output from iebcopy
  string output;
  rc = zds_read_from_dd(&zds, "sysprint", output);
  if (0 != rc)
  {
    cerr << "Error: could not read from dd: '" << "sysprint" << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    cerr << output << endl;
    return RTNCD_FAILURE;
  }
  cout << "Data set '" << dsn << "' compressed" << endl;

  // free dynalloc dds
  free_dynalloc_dds(dds);

  return RTNCD_SUCCESS;
}

int handle_tool_convert_dsect(const ParseResult &result)
{
  int rc = 0;
  ZCN zcn = {0};
  unsigned int code = 0;
  string resp;

  string adata_dsn = result.find_kw_arg_string("adata-dsn");
  string chdr_dsn = result.find_kw_arg_string("chdr-dsn");
  string sysprint = result.find_kw_arg_string("sysprint");
  string sysout = result.find_kw_arg_string("sysout");

  const char *user = getlogin();
  string struser(user);
  transform(struser.begin(), struser.end(), struser.begin(), ::tolower);

  if (sysprint.empty())
    sysprint = "/tmp/" + struser + "_sysprint.txt";
  if (sysout.empty())
    sysout = "/tmp/" + struser + "_sysout.txt";

  cout << adata_dsn << " " << chdr_dsn << " " << sysprint << " " << sysout << endl;

  vector<string> dds;
  dds.push_back("alloc fi(sysprint) path('" + sysprint + "') pathopts(owronly,ocreat,otrunc) pathmode(sirusr,siwusr,sirgrp) filedata(text) msg(2)");
  dds.push_back("alloc fi(sysout) path('" + sysout + "') pathopts(owronly,ocreat,otrunc) pathmode(sirusr,siwusr,sirgrp) filedata(text) msg(2)");
  dds.push_back("alloc fi(sysadata) da('" + adata_dsn + "') shr msg(2)");
  dds.push_back("alloc fi(edcdsect) da('" + chdr_dsn + "') shr msg(2)");

  rc = loop_dynalloc(dds);
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
  free_dynalloc_dds(dds);

  return rc;
}

int handle_tool_dynalloc(const ParseResult &result)
{
  int rc = 0;
  unsigned int code = 0;
  string resp;

  string parm = result.find_pos_arg_string("parm");

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
  string symbol = result.find_pos_arg_string("symbol");
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

  string pattern = result.find_pos_arg_string("string");
  string warn = result.find_kw_arg_string("warn");
  int max_entries = result.find_kw_arg_int("max-entries");
  string dsn = result.find_pos_arg_string("dsn");

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
  dds.push_back("alloc dd(newdd) da('" + dsn + "') shr");
  dds.push_back("alloc dd(outdd)");
  dds.push_back("alloc dd(sysin)");

  rc = loop_dynalloc(dds);
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
  free_dynalloc_dds(dds);

  return RTNCD_SUCCESS;
}

int handle_tool_amblist(const ParseResult &result)
{
  int rc = 0;

  string dsn = result.find_pos_arg_string("dsn");
  string statements = " " + result.find_kw_arg_string("control-statements");

  // Perform dynalloc
  vector<string> dds;
  dds.push_back("alloc dd(syslib) da('" + dsn + "') shr");
  dds.push_back("alloc dd(sysprint) lrecl(80) recfm(f,b) blksize(80)");
  dds.push_back("alloc dd(sysin) lrecl(80) recfm(f,b) blksize(80)");

  rc = loop_dynalloc(dds);
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
  free_dynalloc_dds(dds);

  return RTNCD_SUCCESS;
}

int handle_tool_run(const ParseResult &result)
{
  int rc = 0;
  string program = result.find_pos_arg_string("program");
  string dynalloc_pre = result.find_kw_arg_string("dynalloc-pre");
  string dynalloc_post = result.find_kw_arg_string("dynalloc-post");

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

    rc = loop_dynalloc(dds);
    if (0 != rc)
    {
      return RTNCD_FAILURE;
    }
  }

  string indd = result.find_kw_arg_string("in-dd");
  if (indd.length() > 0)
  {
    string ddname = "DD:" + indd;
    ofstream out(ddname.c_str());
    if (!out.is_open())
    {
      cerr << "Error: could not open input '" << ddname << "'" << endl;
      return RTNCD_FAILURE;
    }

    string input = result.find_kw_arg_string("input");
    if (result.has_kw_arg("input"))
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

  string outdd = result.find_kw_arg_string("out-dd");
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

    loop_dynalloc(dds);
  }

  return rc;
}

int handle_uss_create_file(const ParseResult &result)
{
  int rc = 0;
  string file_path = result.find_pos_arg_string("file-path");

  int mode = result.find_kw_arg_int("mode");
  if (result.find_kw_arg_string("mode").empty())
  {
    mode = 644;
  }
  else if (mode == 0 && result.find_kw_arg_string("mode") != "0")
  {
    cerr << "Error: invalid mode provided.\nExamples of valid modes: 777, 0644" << endl;
    return RTNCD_FAILURE;
  }

  // Convert mode from decimal to octal
  mode_t cf_mode = 0;
  int temp_mode = mode;
  int multiplier = 1;

  // Convert decimal representation of octal to actual octal value
  // e.g. user inputs 777 -> converted to correct value for chmod
  while (temp_mode > 0)
  {
    cf_mode += (temp_mode % 10) * multiplier;
    temp_mode /= 10;
    multiplier *= 8;
  }

  ZUSF zusf = {0};
  rc = zusf_create_uss_file_or_dir(&zusf, file_path, cf_mode, false);
  if (0 != rc)
  {
    cerr << "Error: could not create USS file: '" << file_path << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "USS file '" << file_path << "' created" << endl;

  return rc;
}

int handle_uss_create_dir(const ParseResult &result)
{
  int rc = 0;
  string file_path = result.find_pos_arg_string("file-path");

  int mode = result.find_kw_arg_int("mode");
  if (result.find_kw_arg_string("mode").empty())
  {
    mode = 755;
  }
  else if (mode == 0 && result.find_kw_arg_string("mode") != "0")
  {
    cerr << "Error: invalid mode provided.\nExamples of valid modes: 777, 0644" << endl;
    return RTNCD_FAILURE;
  }

  // Convert mode from decimal to octal
  mode_t cf_mode = 0;
  int temp_mode = mode;
  int multiplier = 1;

  // Convert decimal representation of octal to actual octal value
  // e.g. user inputs 777 -> converted to correct value for chmod
  while (temp_mode > 0)
  {
    cf_mode += (temp_mode % 10) * multiplier;
    temp_mode /= 10;
    multiplier *= 8;
  }

  ZUSF zusf = {0};
  rc = zusf_create_uss_file_or_dir(&zusf, file_path, cf_mode, true);
  if (0 != rc)
  {
    cerr << "Error: could not create USS directory: '" << file_path << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "USS directory '" << file_path << "' created" << endl;

  return rc;
}

int handle_uss_list(const ParseResult &result)
{
  int rc = 0;
  string uss_file = result.find_pos_arg_string("file-path");

  ListOptions list_options = {0};
  list_options.all_files = result.find_kw_arg_bool("all");
  list_options.long_format = result.find_kw_arg_bool("long");

  const auto use_csv_format = result.find_kw_arg_bool("response-format-csv");

  ZUSF zusf = {0};
  string response;
  rc = zusf_list_uss_file_path(&zusf, uss_file, response, list_options, use_csv_format);
  if (0 != rc)
  {
    cerr << "Error: could not list USS files: '" << uss_file << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << zusf.diag.e_msg << endl
         << response << endl;
    return RTNCD_FAILURE;
  }

  cout << response;

  return rc;
}

int handle_uss_view(const ParseResult &result)
{
  int rc = 0;
  string uss_file = result.find_pos_arg_string("file-path");

  ZUSF zusf = {0};
  if (result.has_kw_arg("encoding"))
  {
    zut_prepare_encoding(result.find_kw_arg_string("encoding"), &zusf.encoding_opts);
  }

  struct stat file_stats;
  if (stat(uss_file.c_str(), &file_stats) == -1)
  {
    cerr << "Error: Path " << uss_file << " does not exist" << endl;
    return RTNCD_FAILURE;
  }

  bool has_pipe_path = result.has_kw_arg("pipe-path");
  string pipe_path = result.find_kw_arg_string("pipe-path");

  if (has_pipe_path && !pipe_path.empty())
  {
    size_t content_len = 0;
    rc = zusf_read_from_uss_file_streamed(&zusf, uss_file, pipe_path, &content_len);

    if (result.find_kw_arg_bool("return-etag"))
    {
      cout << "etag: " << zut_build_etag(file_stats.st_mtime, file_stats.st_size) << endl;
    }
    cout << "size: " << content_len << endl;
  }
  else
  {
    string response;
    rc = zusf_read_from_uss_file(&zusf, uss_file, response);
    if (0 != rc)
    {
      cerr << "Error: could not view USS file: '" << uss_file << "' rc: '" << rc << "'" << endl;
      cerr << "  Details:\n"
           << zusf.diag.e_msg << endl
           << response << endl;
      return RTNCD_FAILURE;
    }

    if (result.find_kw_arg_bool("return-etag"))
    {
      cout << "etag: " << zut_build_etag(file_stats.st_mtime, file_stats.st_size) << endl;
      cout << "data: ";
    }

    bool has_encoding = result.has_kw_arg("encoding");
    bool response_format_bytes = result.find_kw_arg_bool("response-format-bytes");

    if (has_encoding && response_format_bytes)
    {
      zut_print_string_as_bytes(response);
    }
    else
    {
      cout << response << endl;
    }
  }

  return rc;
}

int handle_uss_write(const ParseResult &result)
{
  int rc = 0;
  string file = result.find_pos_arg_string("file-path");
  ZUSF zusf = {0};

  if (result.has_kw_arg("encoding"))
  {
    zut_prepare_encoding(result.find_kw_arg_string("encoding"), &zusf.encoding_opts);
  }

  if (result.has_kw_arg("etag"))
  {
    string etag_value = result.find_kw_arg_string("etag");
    if (!etag_value.empty())
    {
      strcpy(zusf.etag, etag_value.c_str());
    }
  }

  bool has_pipe_path = result.has_kw_arg("pipe-path");
  string pipe_path = result.find_kw_arg_string("pipe-path");
  size_t content_len = 0;

  if (has_pipe_path && !pipe_path.empty())
  {
    rc = zusf_write_to_uss_file_streamed(&zusf, file, pipe_path, &content_len);
  }
  else
  {
    string data = "";
    string line = "";

    if (!isatty(fileno(stdout)))
    {
      std::istreambuf_iterator<char> begin(std::cin);
      std::istreambuf_iterator<char> end;

      vector<char> input(begin, end);
      const auto temp = string(input.begin(), input.end());
      input.clear();
      const auto bytes = zut_get_contents_as_bytes(temp);

      data.assign(bytes.begin(), bytes.end());
    }
    else
    {
      while (getline(cin, line))
      {
        data += line;
        data.push_back('\n');
      }
    }
    rc = zusf_write_to_uss_file(&zusf, file, data);
  }

  if (0 != rc)
  {
    cerr << "Error: could not write to USS file: '" << file << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  if (result.find_kw_arg_bool("etag-only"))
  {
    cout << "etag: " << zusf.etag << endl
         << "created: " << (zusf.created ? "true" : "false") << endl;
    if (content_len > 0)
      cout << "size: " << content_len << endl;
  }
  else
  {
    cout << "Wrote data to '" << file << "'" << (zusf.created ? " (created new file)" : " (overwrote existing)") << endl;
  }

  return rc;
}

int handle_uss_delete(const ParseResult &result)
{
  string file_path = result.find_pos_arg_string("file-path");
  bool recursive = result.find_kw_arg_bool("recursive");

  ZUSF zusf = {0};
  const auto rc = zusf_delete_uss_item(&zusf, file_path, recursive);

  if (0 != rc)
  {
    cerr << "Failed to delete USS item " << file_path << ":\n " << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "USS item '" << file_path << "' deleted" << endl;

  return rc;
}

int handle_uss_chmod(const ParseResult &result)
{
  int rc = 0;
  int mode = result.find_pos_arg_int("mode");
  if (mode == 0 && !result.find_pos_arg_string("mode").empty())
  {
    cerr << "Error: invalid mode provided.\nExamples of valid modes: 777, 0644" << endl;
    return RTNCD_FAILURE;
  }

  string file_path = result.find_pos_arg_string("file-path");
  bool recursive = result.find_kw_arg_bool("recursive");

  // Convert mode from decimal to octal
  mode_t chmod_mode = 0;
  int temp_mode = mode;
  int multiplier = 1;

  // Convert decimal representation of octal to actual octal value
  // e.g. user inputs 777 -> converted to correct value for chmod
  while (temp_mode > 0)
  {
    chmod_mode += (temp_mode % 10) * multiplier;
    temp_mode /= 10;
    multiplier *= 8;
  }

  ZUSF zusf = {0};
  rc = zusf_chmod_uss_file_or_dir(&zusf, file_path, chmod_mode, recursive);
  if (0 != rc)
  {
    cerr << "Error: could not chmod USS path: '" << file_path << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "USS path '" << file_path << "' modified: '" << mode << "'" << endl;

  return rc;
}

int handle_uss_chown(const ParseResult &result)
{
  string path = result.find_pos_arg_string("file-path");
  string owner = result.find_pos_arg_string("owner");
  bool recursive = result.find_kw_arg_bool("recursive");

  ZUSF zusf = {0};

  const auto rc = zusf_chown_uss_file_or_dir(&zusf, path, owner, recursive);
  if (0 != rc)
  {
    cerr << "Error: could not chown USS path: '" << path << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "USS path '" << path << "' owner changed to '" << owner << "'" << endl;

  return rc;
}

int handle_uss_chtag(const ParseResult &result)
{
  string path = result.find_pos_arg_string("file-path");
  string tag = result.find_pos_arg_string("tag");
  if (tag.empty())
  {
    tag = zut_int_to_string(result.find_pos_arg_int("tag"));
  }

  if (tag.empty())
  {
    cerr << "Error: no tag provided" << endl;
    return RTNCD_FAILURE;
  }

  bool recursive = result.find_kw_arg_bool("recursive");

  ZUSF zusf = {0};
  const auto rc = zusf_chtag_uss_file_or_dir(&zusf, path, tag, recursive);

  if (0 != rc)
  {
    cerr << "Error: could not chtag USS path: '" << path << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "USS path '" << path << "' tag changed to '" << tag << "'" << endl;

  return rc;
}

int job_submit_common(const ParseResult &result, string jcl, string &jobid, string identifier)
{
  int rc = 0;
  ZJB zjb = {0};
  rc = zjb_submit(&zjb, jcl, jobid);

  if (0 != rc)
  {
    cerr << "Error: could not submit JCL: '" << identifier << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  bool only_jobid = result.find_kw_arg_bool("only-jobid");
  bool only_correlator = result.find_kw_arg_bool("only-correlator");
  string wait = result.find_kw_arg_string("wait");
  transform(wait.begin(), wait.end(), wait.begin(), ::toupper);

  if (only_jobid)
    cout << jobid << endl;
  else if (only_correlator)
    cout << string(zjb.correlator, sizeof(zjb.correlator)) << endl;
  else
    cout << "Submitted " << identifier << ", " << jobid << endl;

#define JOB_STATUS_OUTPUT "OUTPUT"
#define JOB_STATUS_INPUT "ACTIVE"

  if (JOB_STATUS_OUTPUT == wait || JOB_STATUS_INPUT == wait)
  {
    rc = zjb_wait(&zjb, wait);
    if (0 != rc)
    {
      cerr << "Error: could not wait for job status: '" << wait << "' rc: '" << rc << "'" << endl;
      cerr << "  Details: " << zjb.diag.e_msg << endl;
      return RTNCD_FAILURE;
    }
  }
  else if ("" != wait)
  {
    cerr << "Error: cannot wait for unknown status '" << wait << "'" << endl;
    return RTNCD_FAILURE;
  }

  return rc;
}

int handle_job_list(const ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  string owner_name = result.find_kw_arg_string("owner");
  string prefix_name = result.find_kw_arg_string("prefix");
  int max_entries = result.find_kw_arg_int("max-entries");
  bool warn = result.find_kw_arg_bool("warn", true);

  if (max_entries > 0)
  {
    zjb.jobs_max = max_entries;
  }

  vector<ZJob> jobs;
  rc = zjb_list_by_owner(&zjb, owner_name, prefix_name, jobs);

  if (RTNCD_SUCCESS == rc || RTNCD_WARNING == rc)
  {
    bool emit_csv = result.find_kw_arg_bool("response-format-csv");
    for (vector<ZJob>::iterator it = jobs.begin(); it != jobs.end(); it++)
    {
      if (emit_csv)
      {
        vector<string> fields;
        fields.push_back(it->jobid);
        fields.push_back(it->retcode);
        fields.push_back(it->jobname);
        fields.push_back(it->status);
        fields.push_back(it->correlator);
        cout << zut_format_as_csv(fields) << endl;
      }
      else
      {
        cout << it->jobid << " " << left << setw(10) << it->retcode << " " << it->jobname << " " << it->status << endl;
      }
    }
  }
  if (RTNCD_WARNING == rc)
  {
    if (warn)
    {
      cerr << "Warning: results truncated" << endl;
    }
  }
  if (RTNCD_SUCCESS != rc && RTNCD_WARNING != rc)
  {
    cerr << "Error: could not list jobs for: '" << owner_name << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  return (!warn && rc == RTNCD_WARNING) ? RTNCD_SUCCESS : rc;
}

int handle_job_list_files(const ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid = result.find_pos_arg_string("jobid");
  int max_entries = result.find_kw_arg_int("max-entries");
  bool warn = result.find_kw_arg_bool("warn", true);

  if (max_entries > 0)
  {
    zjb.dds_max = max_entries;
  }

  vector<ZJobDD> job_dds;
  rc = zjb_list_dds(&zjb, jobid, job_dds);
  if (RTNCD_SUCCESS == rc || RTNCD_WARNING == rc)
  {
    bool emit_csv = result.find_kw_arg_bool("response-format-csv");
    for (vector<ZJobDD>::iterator it = job_dds.begin(); it != job_dds.end(); ++it)
    {
      std::vector<string> fields;
      fields.push_back(it->ddn);
      fields.push_back(it->dsn);
      fields.push_back(TO_STRING(it->key));
      fields.push_back(it->stepname);
      fields.push_back(it->procstep);
      if (emit_csv)
      {
        cout << zut_format_as_csv(fields) << endl;
      }
      else
      {
        cout << left << setw(9) << it->ddn << " " << it->dsn << " " << setw(4) << it->key << " " << it->stepname << " " << it->procstep << endl;
      }
    }
  }

  if (RTNCD_WARNING == rc)
  {
    if (warn)
    {
      cerr << "Warning: " << zjb.diag.e_msg << endl;
    }
  }

  if (RTNCD_SUCCESS != rc && RTNCD_WARNING != rc)
  {
    cerr << "Error: could not list files for: '" << jobid << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  return (!warn && rc == RTNCD_WARNING) ? RTNCD_SUCCESS : rc;
}

int handle_job_view_status(const ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  ZJob job = {0};
  string jobid = result.find_pos_arg_string("jobid");

  bool emit_csv = result.find_kw_arg_bool("response-format-csv");

  rc = zjb_view(&zjb, jobid, job);

  if (0 != rc)
  {
    cerr << "Error: could not view job status for: '" << jobid << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  if (emit_csv)
  {
    vector<string> fields;
    fields.push_back(job.jobid);
    fields.push_back(job.retcode);
    fields.push_back(job.jobname);
    fields.push_back(job.status);
    fields.push_back(job.correlator);
    fields.push_back(job.full_status);
    cout << zut_format_as_csv(fields) << endl;
  }
  else
  {
    cout << job.jobid << " " << left << setw(10) << job.retcode << " " << job.jobname << " " << job.status << endl;
  }
  return 0;
}

int handle_job_view_file(const ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid = result.find_pos_arg_string("jobid");
  int key = result.find_pos_arg_int("key");

  if (result.has_kw_arg("encoding"))
  {
    zut_prepare_encoding(result.find_kw_arg_string("encoding"), &zjb.encoding_opts);
  }

  string resp;
  rc = zjb_read_jobs_output_by_key(&zjb, jobid, key, resp);

  if (0 != rc)
  {
    cerr << "Error: could not view job file for: '" << jobid << "' with key '" << key << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  bool has_encoding = result.has_kw_arg("encoding");
  bool response_format_bytes = result.find_kw_arg_bool("response-format-bytes");

  if (has_encoding && response_format_bytes)
  {
    zut_print_string_as_bytes(resp);
  }
  else
  {
    cout << resp;
  }

  return RTNCD_SUCCESS;
}

int handle_job_view_jcl(const ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid = result.find_pos_arg_string("jobid");

  string resp;
  rc = zjb_read_job_jcl(&zjb, jobid, resp);

  if (0 != rc)
  {
    cerr << "Error: could not view job file for: '" << jobid << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << resp;

  return 0;
}

int handle_job_submit(const ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  string dsn = result.find_pos_arg_string("dsn");
  string jobid;

  ZDS zds = {0};
  string contents;
  rc = zds_read_from_dsn(&zds, dsn, contents);
  if (0 != rc)
  {
    cerr << "Error: could not read data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  return job_submit_common(result, contents, jobid, dsn);
}

int handle_job_submit_uss(const ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  string file = result.find_pos_arg_string("file-path");

  ZUSF zusf = {0};
  string response;
  rc = zusf_read_from_uss_file(&zusf, file, response);
  if (0 != rc)
  {
    cerr << "Error: could not view USS file: '" << file << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << zusf.diag.e_msg << endl
         << response << endl;
    return RTNCD_FAILURE;
  }

  string jobid;

  return job_submit_common(result, response, jobid, file);
}

int handle_job_submit_jcl(const ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};

  string data;
  string line;

  std::istreambuf_iterator<char> begin(std::cin);
  std::istreambuf_iterator<char> end;

  std::vector<char> raw_bytes(begin, end);
  data.assign(raw_bytes.begin(), raw_bytes.end());

  if (!isatty(fileno(stdout)))
  {
    const auto bytes = zut_get_contents_as_bytes(data);
    data.assign(bytes.begin(), bytes.end());
  }
  raw_bytes.clear();

  ZEncode encoding_opts = {0};
  bool encoding_prepared = result.has_kw_arg("encoding") && zut_prepare_encoding(result.find_kw_arg_string("encoding"), &encoding_opts);

  if (encoding_prepared && encoding_opts.data_type != eDataTypeBinary)
  {
    data = zut_encode(data, "UTF-8", string(encoding_opts.codepage), zjb.diag);
  }

  string jobid;
  rc = zjb_submit(&zjb, data, jobid);

  return job_submit_common(result, data, jobid, data);
}

int handle_job_delete(const ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid = result.find_pos_arg_string("jobid");

  rc = zjb_delete(&zjb, jobid);

  if (0 != rc)
  {
    cerr << "Error: could not delete job: '" << jobid << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "Job " << jobid << " deleted " << endl;

  return RTNCD_SUCCESS;
}

int handle_job_cancel(const ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid = result.find_pos_arg_string("jobid");

  // Note: Cancel options (dump, force, purge, restart) are currently not used by the backend
  // but are defined for future compatibility
  bool option_dump = result.find_kw_arg_bool("dump");
  bool option_force = result.find_kw_arg_bool("force");
  bool option_purge = result.find_kw_arg_bool("purge");
  bool option_restart = result.find_kw_arg_bool("restart");

  rc = zjb_cancel(&zjb, jobid);

  if (0 != rc)
  {
    cerr << "Error: could not cancel job: '" << jobid << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "Job " << jobid << " cancelled " << endl;

  return RTNCD_SUCCESS;
}

int handle_job_hold(const ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid = result.find_pos_arg_string("jobid");

  rc = zjb_hold(&zjb, jobid);

  if (0 != rc)
  {
    cerr << "Error: could not hold job: '" << jobid << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "Job " << jobid << " held " << endl;

  return RTNCD_SUCCESS;
}

int handle_job_release(const ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid = result.find_pos_arg_string("jobid");

  rc = zjb_release(&zjb, jobid);

  if (0 != rc)
  {
    cerr << "Error: could not release job: '" << jobid << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "Job " << jobid << " released " << endl;

  return RTNCD_SUCCESS;
}

int loop_dynalloc(vector<string> &list)
{
  int rc = 0;
  unsigned int code = 0;
  string response;

  for (vector<string>::iterator it = list.begin(); it != list.end(); it++)
  {
    rc = zut_bpxwdyn(*it, &code, response);

    if (0 != rc)
    {
      cerr << "Error: bpxwdyn failed with '" << *it << "' rc: '" << rc << "'" << endl;
      cerr << "  Details: " << response << endl;
      return -1;
    }
  }

  return rc;
}

int free_dynalloc_dds(vector<string> &list)
{
  vector<string> free_dds;

  for (vector<string>::iterator it = list.begin(); it != list.end(); it++)
  {
    string alloc_dd = *it;
    size_t start = alloc_dd.find(" ");
    size_t end = alloc_dd.find(")", start);
    if (start == string::npos || end == string::npos)
    {
      cerr << "Error: Invalid format in DD alloc string: " << alloc_dd << endl;
    }
    else
    {
      free_dds.push_back("free " + alloc_dd.substr(start + 1, end - start));
    }
  }

  return loop_dynalloc(free_dds);
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
