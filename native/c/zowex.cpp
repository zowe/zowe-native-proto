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

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include "zcn.hpp"
#include "zut.hpp"
#include "zjb.hpp"
#include "zds.hpp"
#include "zusf.hpp"
#include "ztso.hpp"
#include "zuttype.h"

#ifndef TO_STRING
#define TO_STRING(x) static_cast<ostringstream &>(      \
                         (ostringstream() << dec << x)) \
                         .str()
#endif

using namespace pparser;
using namespace std;

int handle_job_list(ParseResult &);
int handle_job_list_files(ParseResult &);
int handle_job_view_status(ParseResult &);
int handle_job_view_file(ParseResult &);
int handle_job_view_jcl(ParseResult &);
int handle_job_submit(ParseResult &);
int handle_job_submit_jcl(ParseResult &);
int handle_job_submit_uss(ParseResult &);
int handle_job_delete(ParseResult &);
int handle_job_cancel(ParseResult &);
int handle_job_hold(ParseResult &);
int handle_job_release(ParseResult &);

int handle_console_issue(ParseResult &);

int handle_data_set_create_dsn(ParseResult &);
int handle_data_set_create_dsn_vb(ParseResult &);
int handle_data_set_create_dsn_adata(ParseResult &);
int handle_data_set_restore(ParseResult &);
int handle_data_set_view_dsn(ParseResult &);
int handle_data_set_list(ParseResult &);
int handle_data_set_list_members_dsn(ParseResult &);
int handle_data_set_write_to_dsn(ParseResult &);
int handle_data_set_delete_dsn(ParseResult &);
int handle_data_set_create_member_dsn(ParseResult &);

int handle_log_view(ParseResult &);

int handle_tool_convert_dsect(ParseResult &);
int handle_tool_dynalloc(ParseResult &);
int handle_tool_display_symbol(ParseResult &);
int handle_tool_search(ParseResult &);
int handle_tool_run(ParseResult &);

// TODO(Kelosky):
// help w/verbose examples
// add simple examples to help

int handle_uss_create_file(ParseResult &);
int handle_uss_create_dir(ParseResult &);
int handle_uss_list(ParseResult &);
int handle_uss_view(ParseResult &);
int handle_uss_write(ParseResult &);
int handle_uss_delete(ParseResult &);
int handle_uss_chmod(ParseResult &);
int handle_uss_chown(ParseResult &);
int handle_uss_chtag(ParseResult &);
int handle_tso_issue(ParseResult &);

int main(int argc, char *argv[])
{
  ArgumentParser parser(argv[PROCESS_NAME_ARG], "C++ CLI for z/OS resources");
  Command &rootCommand = parser.getRootCommand();

  // --- Global options ---
  root_command.add_keyword_arg("interactive", "--it", "--interactive", "interactive (REPL) mode", ArgType_Flag);

  // --- tso group ---
  auto tso_cmd = make_shared<Command>("tso", "TSO operations");
  {
    auto issue_cmd = make_shared<Command>("issue", "issue TSO command");
    issue_cmd->add_positional_arg("command", "command to issue", ArgType_Single, true);
    issue_cmd->set_handler(handle_tso_issue);
    tso_cmd->add_command(issue_cmd);
  }
  root_command.add_command(tso_cmd);

  // --- data set group (ds) ---
  auto data_set_cmd = make_shared<Command>("data-set", "z/OS data set operations");
  data_set_cmd->add_alias("ds");
  {
    const string dsn_help = "data set name, optionally with member specified";
    const string max_entries_help = "max number of results to return before error generated";
    const string warn_help = "warn if truncated or not found";

    auto create_cmd = make_shared<Command>("create", "create data set using defaults: dsorg=PO, recfm=FB, lrecl=80");
    create_cmd->add_alias("cre");
    create_cmd->add_positional_arg("dsn", dsn_help, ArgType_Single, true);
    create_cmd->set_handler(handle_data_set_create_dsn);
    data_set_cmd->add_command(create_cmd);

    auto create_vb_cmd = make_shared<Command>("create-vb", "create VB data set using defaults: dsorg=PO, recfm=VB, lrecl=255");
    create_vb_cmd->add_alias("cre-vb");
    create_vb_cmd->add_positional_arg("dsn", dsn_help, ArgType_Single, true);
    create_vb_cmd->set_handler(handle_data_set_create_dsn_vb);
    data_set_cmd->add_command(create_vb_cmd);

    auto create_adata_cmd = make_shared<Command>("create-adata", "create VB data set using defaults: dsorg=PO, recfm=VB, lrecl=32756");
    create_adata_cmd->add_alias("cre-a");
    create_adata_cmd->add_positional_arg("dsn", dsn_help, ArgType_Single, true);
    create_adata_cmd->set_handler(handle_data_set_create_dsn_adata);
    data_set_cmd->add_command(create_adata_cmd);

    auto create_member_cmd = make_shared<Command>("create-member", "create member in data set");
    create_member_cmd->add_alias("cre-m");
    create_member_cmd->add_positional_arg("dsn", dsn_help, ArgType_Single, true);
    create_member_cmd->set_handler(handle_data_set_create_member_dsn);
    data_set_cmd->add_command(create_member_cmd);

    auto restore_cmd = make_shared<Command>("restore", "restore/recall data set");
    restore_cmd->add_positional_arg("dsn", dsn_help, ArgType_Single, true);
    restore_cmd->set_handler(handle_data_set_restore);
    data_set_cmd->add_command(restore_cmd);

    auto view_cmd = make_shared<Command>("view", "view data set");
    view_cmd->add_positional_arg("dsn", dsn_help, ArgType_Single, true);
    view_cmd->add_keyword_arg("encoding", "--ec", "--encoding", "return contents in given encoding", ArgType_Single);
    view_cmd->add_keyword_arg("response-format-bytes", "", "--rfb", "returns the response as raw bytes", ArgType_Flag);
    view_cmd->set_handler(handle_data_set_view_dsn);
    data_set_cmd->add_command(view_cmd);

    auto list_cmd = make_shared<Command>("list", "list data sets");
    list_cmd->add_alias("ls");
    list_cmd->add_positional_arg("dsn", dsn_help, ArgType_Single, true);
    list_cmd->add_keyword_arg("attributes", "-a", "--attributes", "display data set attributes", ArgType_Flag);
    list_cmd->add_keyword_arg("max-entries", "--me", "--max-entries", max_entries_help, ArgType_Single);
    list_cmd->add_keyword_arg("warn", "", "--warn", warn_help, ArgType_Flag, false, ArgValue(true));
    list_cmd->add_keyword_arg("response-format-csv", "", "--rfc", "returns the response in CSV format", ArgType_Flag);
    list_cmd->set_handler(handle_data_set_list);
    data_set_cmd->add_command(list_cmd);

    auto list_members_cmd = make_shared<Command>("list-members", "list data set members");
    list_members_cmd->add_alias("lm");
    list_members_cmd->add_positional_arg("dsn", dsn_help, ArgType_Single, true);
    list_members_cmd->add_keyword_arg("max-entries", "--me", "--max-entries", max_entries_help, ArgType_Single);
    list_members_cmd->add_keyword_arg("warn", "", "--warn", warn_help, ArgType_Flag, false, ArgValue(true));
    list_members_cmd->set_handler(handle_data_set_list_members_dsn);
    data_set_cmd->add_command(list_members_cmd);

    auto write_cmd = make_shared<Command>("write", "write to data set");
    write_cmd->add_positional_arg("dsn", dsn_help, ArgType_Single, true);
    write_cmd->add_keyword_arg("encoding", "--ec", "--encoding", "return contents in given encoding", ArgType_Single);
    write_cmd->add_keyword_arg("etag", "", "--etag", "provide the e-tag for a write response to detect conflicts before save", ArgType_Single, false);
    write_cmd->add_keyword_arg("etag-only", "", "--etag-only", "only print the e-tag for a write response (when successful)", ArgType_Flag, false);
    write_cmd->set_handler(handle_data_set_write_to_dsn);
    data_set_cmd->add_command(write_cmd);

    auto delete_cmd = make_shared<Command>("delete", "delete data set");
    delete_cmd->add_alias("del");
    delete_cmd->add_positional_arg("dsn", dsn_help, ArgType_Single, true);
    delete_cmd->set_handler(handle_data_set_delete_dsn);
    data_set_cmd->add_command(delete_cmd);
  }
  root_command.add_command(data_set_cmd);

  // --- jobs group ---
  auto job_cmd = make_shared<Command>("job", "z/OS job operations");
  {
    const string max_entries_help = "max number of results to return before error generated";
    const string warn_help = "warn if truncated or not found";
    const string jobid_help = "valid job ID";

    auto list_cmd = make_shared<Command>("list", "list jobs");
    list_cmd->add_keyword_arg("owner", "-o", "--owner", "filter by owner", ArgType_Single);
    list_cmd->add_keyword_arg("prefix", "-p", "--prefix", "filter by prefix", ArgType_Single);
    list_cmd->add_keyword_arg("max-entries", "--me", "--max-entries", max_entries_help, ArgType_Single);
    list_cmd->add_keyword_arg("warn", "", "--warn", warn_help, ArgType_Flag, false, ArgValue(true));
    list_cmd->add_keyword_arg("response-format-csv", "", "--rfc", "returns the response in CSV format", ArgType_Flag);
    list_cmd->set_handler(handle_job_list);
    job_cmd->add_command(list_cmd);

    auto list_files_cmd = make_shared<Command>("list-files", "list spool files for jobid");
    list_files_cmd->add_alias("lf");
    list_files_cmd->add_positional_arg("jobid", jobid_help, ArgType_Single, true);
    list_files_cmd->add_keyword_arg("response-format-csv", "", "--rfc", "returns the response in CSV format", ArgType_Flag);
    list_files_cmd->set_handler(handle_job_list_files);
    job_cmd->add_command(list_files_cmd);

    auto view_status_cmd = make_shared<Command>("view-status", "view job status");
    view_status_cmd->add_alias("vs");
    view_status_cmd->add_positional_arg("jobid", jobid_help, ArgType_Single, true);
    view_status_cmd->add_keyword_arg("response-format-csv", "", "--rfc", "returns the response in CSV format", ArgType_Flag);
    view_status_cmd->set_handler(handle_job_view_status);
    job_cmd->add_command(view_status_cmd);

    auto view_file_cmd = make_shared<Command>("view-file", "view job file output");
    view_file_cmd->add_alias("vf");
    view_file_cmd->add_positional_arg("jobid", jobid_help, ArgType_Single, true);
    view_file_cmd->add_positional_arg("key", "valid job dsn key via 'job list-files'", ArgType_Single, true);
    view_file_cmd->add_keyword_arg("encoding", "--ec", "--encoding", "return contents in given encoding", ArgType_Single);
    view_file_cmd->add_keyword_arg("response-format-bytes", "", "--rfb", "returns the response as raw bytes", ArgType_Flag);
    view_file_cmd->set_handler(handle_job_view_file);
    job_cmd->add_command(view_file_cmd);

    auto view_jcl_cmd = make_shared<Command>("view-jcl", "view job JCL from input job ID");
    view_jcl_cmd->add_alias("vj");
    view_jcl_cmd->add_positional_arg("jobid", jobid_help, ArgType_Single, true);
    view_jcl_cmd->set_handler(handle_job_view_jcl);
    job_cmd->add_command(view_jcl_cmd);

    auto submit_cmd = make_shared<Command>("submit", "submit a job");
    submit_cmd->add_alias("sub");
    submit_cmd->add_positional_arg("dsn", "dsn containing JCL", ArgType_Single, true);
    submit_cmd->add_keyword_arg("only-jobid", "--oj", "--only-jobid", "show only job ID on success", ArgType_Flag);
    submit_cmd->set_handler(handle_job_submit);
    job_cmd->add_command(submit_cmd);

    auto submit_jcl_cmd = make_shared<Command>("submit-jcl", "submit JCL contents directly");
    submit_jcl_cmd->add_alias("subj");
    submit_jcl_cmd->add_keyword_arg("only-jobid", "--oj", "--only-jobid", "show only job ID on success", ArgType_Flag);
    submit_jcl_cmd->add_keyword_arg("encoding", "--ec", "--encoding", "jcl encoding", ArgType_Single);
    submit_jcl_cmd->set_handler(handle_job_submit_jcl);
    job_cmd->add_command(submit_jcl_cmd);

    auto submit_uss_cmd = make_shared<Command>("submit-uss", "submit a job from uss files");
    submit_uss_cmd->add_alias("sub-u");
    submit_uss_cmd->add_positional_arg("file-path", "uss file containing JCL", ArgType_Single, true);
    submit_uss_cmd->add_keyword_arg("only-jobid", "--oj", "--only-jobid", "show only job ID on success", ArgType_Flag);
    submit_uss_cmd->set_handler(handle_job_submit_uss);
    job_cmd->add_command(submit_uss_cmd);

    auto delete_cmd = make_shared<Command>("delete", "delete a job");
    delete_cmd->add_alias("del");
    delete_cmd->add_positional_arg("jobid", jobid_help, ArgType_Single, true);
    delete_cmd->set_handler(handle_job_delete);
    job_cmd->add_command(delete_cmd);

    auto cancel_cmd = make_shared<Command>("cancel", "cancel a job");
    cancel_cmd->add_alias("cnl");
    cancel_cmd->add_positional_arg("jobid", jobid_help, ArgType_Single, true);
    cancel_cmd->add_keyword_arg("dump", "-d", "--dump", "dump the cancelled jobs if waiting for conversion, in conversion, or in execution.", ArgType_Flag);
    cancel_cmd->add_keyword_arg("force", "-f", "--force", "force cancel the jobs, even if marked.", ArgType_Flag);
    cancel_cmd->add_keyword_arg("purge", "-p", "--purge", "purge output of the cancelled jobs.", ArgType_Flag);
    cancel_cmd->add_keyword_arg("restart", "-r", "--restart", "request that automatic restart management automatically restart the selected jobs after they are cancelled.", ArgType_Flag);
    cancel_cmd->set_handler(handle_job_cancel);
    job_cmd->add_command(cancel_cmd);

    auto hold_cmd = make_shared<Command>("hold", "hold a job");
    hold_cmd->add_alias("hld");
    hold_cmd->add_positional_arg("jobid", jobid_help, ArgType_Single, true);
    hold_cmd->set_handler(handle_job_hold);
    job_cmd->add_command(hold_cmd);

    auto release_cmd = make_shared<Command>("release", "release a job");
    release_cmd->add_alias("rel");
    release_cmd->add_positional_arg("jobid", jobid_help, ArgType_Single, true);
    release_cmd->set_handler(handle_job_release);
    job_cmd->add_command(release_cmd);
  }
  root_command.add_command(job_cmd);

  // --- console group (cn) ---
  auto console_cmd = make_shared<Command>("console", "z/os console operations");
  console_cmd->add_alias("cn");
  {
    auto issue_cmd = make_shared<Command>("issue", "issue a console command");
    issue_cmd->add_positional_arg("command", "command to run, e.g. 'd iplinfo'", ArgType_Single, true);
    issue_cmd->add_keyword_arg("console-name", "--cn", "--console-name", "extended console name", ArgType_Single, false, ArgValue("zowex"));
    issue_cmd->set_handler(handle_console_issue);
    console_cmd->add_command(issue_cmd);
  }
  root_command.add_command(console_cmd);

  // --- uss group ---
  auto uss_cmd = make_shared<Command>("uss", "z/OS USS operations");
  {
    const string file_path_help = "file path";
    const string mode_help = "permissions";
    const string recursive_help = "applies the operation recursively (e.g. for folders w/ inner files)";

    auto create_file_cmd = make_shared<Command>("create-file", "create a USS file");
    create_file_cmd->add_positional_arg("file-path", file_path_help, ArgType_Single, true);
    create_file_cmd->add_keyword_arg("mode", "", "--mode", mode_help, ArgType_Single, false);
    create_file_cmd->set_handler(handle_uss_create_file);
    uss_cmd->add_command(create_file_cmd);

    auto create_dir_cmd = make_shared<Command>("create-dir", "create a USS directory");
    create_dir_cmd->add_positional_arg("file-path", file_path_help, ArgType_Single, true);
    create_dir_cmd->add_keyword_arg("mode", "", "--mode", mode_help, ArgType_Single, false);
    create_dir_cmd->set_handler(handle_uss_create_dir);
    uss_cmd->add_command(create_dir_cmd);

    auto list_cmd = make_shared<Command>("list", "list USS files and directories");
    list_cmd->add_positional_arg("file-path", file_path_help, ArgType_Single, true);
    list_cmd->set_handler(handle_uss_list);
    uss_cmd->add_command(list_cmd);

    auto view_cmd = make_shared<Command>("view", "view a USS file");
    view_cmd->add_positional_arg("file-path", file_path_help, ArgType_Single, true);
    view_cmd->add_keyword_arg("encoding", "--ec", "--encoding", "return contents in given encoding", ArgType_Single);
    view_cmd->add_keyword_arg("response-format-bytes", "", "--rfb", "returns the response as raw bytes", ArgType_Flag);
    view_cmd->set_handler(handle_uss_view);
    uss_cmd->add_command(view_cmd);

    auto write_cmd = make_shared<Command>("write", "write to a USS file");
    write_cmd->add_positional_arg("file-path", file_path_help, ArgType_Single, true);
    write_cmd->add_keyword_arg("encoding", "--ec", "--encoding", "return contents in given encoding", ArgType_Single);
    write_cmd->add_keyword_arg("etag", "", "--etag", "provide the e-tag for a write response to detect conflicts before save", ArgType_Single, false);
    write_cmd->add_keyword_arg("etag-only", "", "--etag-only", "only print the e-tag for a write response (when successful)", ArgType_Flag, false);
    write_cmd->set_handler(handle_uss_write);
    uss_cmd->add_command(write_cmd);

    auto delete_cmd = make_shared<Command>("delete", "delete a USS item");
    delete_cmd->add_positional_arg("file-path", file_path_help, ArgType_Single, true);
    delete_cmd->add_keyword_arg("recursive", "-r", "--recursive", recursive_help, ArgType_Flag, false);
    delete_cmd->set_handler(handle_uss_delete);
    uss_cmd->add_command(delete_cmd);

    auto chmod_cmd = make_shared<Command>("chmod", "change permissions on a USS file or directory");
    chmod_cmd->add_positional_arg("mode", "new permissions for the file or directory", ArgType_Single, true);
    chmod_cmd->add_positional_arg("file-path", file_path_help, ArgType_Single, true);
    chmod_cmd->add_keyword_arg("recursive", "-r", "--recursive", recursive_help, ArgType_Flag, false);
    chmod_cmd->set_handler(handle_uss_chmod);
    uss_cmd->add_command(chmod_cmd);

    auto chown_cmd = make_shared<Command>("chown", "change owner on a USS file or directory");
    chown_cmd->add_positional_arg("owner", "new owner (or owner:group) for the file or directory", ArgType_Single, true);
    chown_cmd->add_positional_arg("file-path", file_path_help, ArgType_Single, true);
    chown_cmd->add_keyword_arg("recursive", "-r", "--recursive", recursive_help, ArgType_Flag, false);
    chown_cmd->set_handler(handle_uss_chown);
    uss_cmd->add_command(chown_cmd);

    auto chtag_cmd = make_shared<Command>("chtag", "change tags on a USS file");
    chtag_cmd->add_positional_arg("codeset", "new codeset for the file", ArgType_Single, true);
    chtag_cmd->add_positional_arg("file-path", file_path_help, ArgType_Single, true);
    chtag_cmd->add_keyword_arg("recursive", "-r", "--recursive", recursive_help, ArgType_Flag, false);
    chtag_cmd->set_handler(handle_uss_chtag);
    uss_cmd->add_command(chtag_cmd);
  }
  root_command.add_command(uss_cmd);

  // --- log group ---
  // note: log group was commented out in the original zcli setup
  // auto log_cmd = make_shared<Command>("log", "log operations");
  // {
  //     auto view_cmd = make_shared<Command>("view", "view log");
  //     view_cmd->add_keyword_arg("lines", "", "--lines", "number of lines to print", ArgType_Single);
  //     view_cmd->set_handler(handle_log_view);
  //     log_cmd->add_command(view_cmd);
  // }
  // root_command.add_command(log_cmd);

  // --- tool group ---
  auto tool_cmd = make_shared<Command>("tool", "tool operations");
  {
    auto convert_dsect_cmd = make_shared<Command>("ccnedsct", "convert dsect to c struct");
    convert_dsect_cmd->add_keyword_arg("adata-dsn", "--ad", "--adata-dsn", "input adata dsn", ArgType_Single, true);
    convert_dsect_cmd->add_keyword_arg("chdr-dsn", "--cd", "--chdr-dsn", "output chdr dsn", ArgType_Single, true);
    convert_dsect_cmd->add_keyword_arg("sysprint", "--sp", "--sysprint", "sysprint output", ArgType_Single, false);
    convert_dsect_cmd->add_keyword_arg("sysout", "--so", "--sysout", "sysout output", ArgType_Single, false);
    convert_dsect_cmd->set_handler(handle_tool_convert_dsect);
    tool_cmd->add_command(convert_dsect_cmd);

    auto dynalloc_cmd = make_shared<Command>("bpxwdy2", "dynalloc command");
    dynalloc_cmd->add_positional_arg("parm", "dynalloc parm string", ArgType_Single, true);
    dynalloc_cmd->set_handler(handle_tool_dynalloc);
    tool_cmd->add_command(dynalloc_cmd);

    auto display_symbol_cmd = make_shared<Command>("display-symbol", "display system symbol");
    display_symbol_cmd->add_positional_arg("symbol", "symbol to display", ArgType_Single, true);
    display_symbol_cmd->set_handler(handle_tool_display_symbol);
    tool_cmd->add_command(display_symbol_cmd);

    auto search_cmd = make_shared<Command>("search", "search members for string");
    search_cmd->add_positional_arg("dsn", "data set to search", ArgType_Single, true);
    search_cmd->add_positional_arg("string", "string to search for", ArgType_Single, true);
    search_cmd->add_keyword_arg("max-entries", "--me", "--max-entries", "max number of results to return", ArgType_Single);
    search_cmd->add_keyword_arg("warn", "", "--warn", "warn if truncated or not found", ArgType_Flag, false, ArgValue(true)); // default true
    search_cmd->set_handler(handle_tool_search);
    tool_cmd->add_command(search_cmd);

    auto run_cmd = make_shared<Command>("run", "run a program");
    run_cmd->add_positional_arg("program", "name of program to run", ArgType_Single, true);
    run_cmd->add_keyword_arg("dynalloc-pre", "--dp", "--dynalloc-pre", "dynalloc pre run statements", ArgType_Single);
    run_cmd->add_keyword_arg("dynalloc-post", "--dt", "--dynalloc-post", "dynalloc post run statements", ArgType_Single);
    run_cmd->add_keyword_arg("in-dd", "--idd", "--in-dd", "input ddname", ArgType_Single);
    run_cmd->add_keyword_arg("input", "--in", "--input", "input string", ArgType_Single);
    run_cmd->add_keyword_arg("out-dd", "--odd", "--out-dd", "output ddname", ArgType_Single);
    run_cmd->set_handler(handle_tool_run);
    tool_cmd->add_command(run_cmd);
  }
  root_command.add_command(tool_cmd);

  // use parser to parse given arguments
  parse_result result = parser.parse(argc, argv);

  if (result.status != parse_result::pparser_status_success)
  {
    // help was shown by the parser, or error was already printed to stderr by the parser
    return result.exit_code;
  }

  // handle interactive mode if requested
  if (result.find_kw_arg_bool("interactive"))
  {
    cout << "Started, enter command or 'quit' to quit..." << endl;

    string command;
    int rc = 0;
    int is_tty = isatty(fileno(stdout));
    do
    {
      if (is_tty)
        cout << "\r> " << flush;

      getline(cin, command);

      // TODO: if (should_quit(command))
      // break;

      parser.parse(command);

      if (!is_tty)
      {
        cout << "[" << rc << "]" << endl;
        // EBCDIC \x37 = ASCII \x04 = End of Transmission (Ctrl+D)
        cout << '\x37' << flush;
        cerr << '\x37' << flush;
      }

    } while (!should_quit(command));

    cout << "...terminated" << endl;

    return rc;
  }

  return result.exitCode;
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

int handle_job_list(ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  string owner_name(result.find_kw_arg_string("owner"));
  string prefix_name(result.find_kw_arg_string("prefix"));
  string max_entries = result.find_kw_arg_string("max-entries");
  string warn = result.find_kw_arg_string("warn");

  if (max_entries.size() > 0)
  {
    zjb.jobs_max = atoi(max_entries.c_str());
  }

  vector<ZJob> jobs;
  rc = zjb_list_by_owner(&zjb, owner_name, prefix_name, jobs);

  if (RTNCD_SUCCESS == rc || RTNCD_WARNING == rc)
  {
    const auto emit_csv = result.find_kw_arg_bool("response-format-csv");
    for (vector<ZJob>::iterator it = jobs.begin(); it != jobs.end(); it++)
    {
      if (emit_csv)
      {
        vector<string> fields;
        fields.push_back(it->jobid);
        fields.push_back(it->retcode);
        fields.push_back(it->jobname);
        fields.push_back(it->status);
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
    if ("true" == warn)
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

  return "false" == warn && rc == RTNCD_WARNING ? RTNCD_SUCCESS : rc;
}

int handle_job_list_files(ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid(result.find_pos_arg_string("jobid"));

  vector<ZJobDD> job_dds;
  rc = zjb_list_dds_by_jobid(&zjb, jobid, job_dds);

  if (0 != rc)
  {
    cerr << "Error: could not list jobs for: '" << jobid << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  const auto emit_csv = result.find_kw_arg_bool("response-format-csv");
  for (vector<ZJobDD>::iterator it = job_dds.begin(); it != job_dds.end(); ++it)
  {
    vector<string> fields;
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

  return RTNCD_SUCCESS;
}

int handle_job_view_status(ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  ZJob job = {0};
  string jobid(result.find_pos_arg_string("jobid"));

  const auto emit_csv = result.find_kw_arg_bool("response-format-csv");
  rc = zjb_view_by_jobid(&zjb, jobid, job);

  if (0 != rc)
  {
    cerr << "Error: could not view job status for: '" << jobid << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zjb.diag.e_msg << endl;
    return -1;
  }

  if (emit_csv)
  {
    vector<string> fields;
    fields.push_back(job.jobid);
    fields.push_back(job.retcode);
    fields.push_back(job.jobname);
    fields.push_back(job.status);
    cout << zut_format_as_csv(fields) << endl;
  }
  else
  {
    cout << job.jobid << " " << left << setw(10) << job.retcode << " " << job.jobname << " " << job.status << endl;
  }
  return 0;
}

int handle_job_view_file(ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid(result.find_pos_arg_string("jobid"));
  string key(result.find_pos_arg_string("key"));

  const auto hasEncoding = zut_prepare_encoding(result.find_kw_arg_string("encoding"), &zjb.encoding_opts);

  string resp;
  rc = zjb_read_jobs_output_by_jobid_and_key(&zjb, jobid, atoi(key.c_str()), resp);

  if (0 != rc)
  {
    cerr << "Error: could not view job file for: '" << jobid << "' with key '" << key << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  if (hasEncoding && result.find_kw_arg_bool("response-format-bytes"))
  {
    zut_print_string_as_bytes(resp);
  }
  else
  {
    cout << resp;
  }

  return RTNCD_SUCCESS;
}

int handle_job_view_jcl(ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid(result.find_pos_arg_string("jobid"));

  string resp;
  rc = zjb_read_job_jcl_by_jobid(&zjb, jobid, resp);

  if (0 != rc)
  {
    cerr << "Error: could not view job file for: '" << jobid << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zjb.diag.e_msg << endl;
    return -1;
  }

  cout << resp;

  return 0;
}

int handle_job_submit(ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  string dsn(result.find_pos_arg_string("dsn"));

  vector<ZJob> jobs;
  string jobid;
  rc = zjb_submit_dsn(&zjb, dsn, jobid);

  if (0 != rc)
  {
    cerr << "Error: could not submit JCL: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  if (result.find_kw_arg_bool("only-jobid"))
    cout << jobid << endl;
  else
    cout << "Submitted " << dsn << ", " << jobid << endl;

  return RTNCD_SUCCESS;
}

int handle_job_submit_uss(ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  string file(result.find_pos_arg_string("file-path"));

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

  vector<ZJob> jobs;
  string jobid;
  rc = zjb_submit(&zjb, response, jobid);

  if (0 != rc)
  {
    cerr << "Error: could not submit JCL: '" << file << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  if (result.find_kw_arg_bool("only-jobid"))
    cout << jobid << endl;
  else
    cout << "Submitted " << file << ", " << jobid << endl;

  return RTNCD_SUCCESS;
}

int handle_job_submit_jcl(ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};

  string data;
  string line;

  istreambuf_iterator<char> begin(cin);
  istreambuf_iterator<char> end;

  vector<char> raw_bytes(begin, end);
  data.assign(raw_bytes.begin(), raw_bytes.end());

  if (!isatty(fileno(stdout)))
  {
    const auto bytes = zut_get_contents_as_bytes(data);
    data.assign(bytes.begin(), bytes.end());
  }
  raw_bytes.clear();

  ZEncode encoding_opts = {0};
  const auto encoding_prepared = result.get_kw_arg_string("encoding") != nullptr && zut_prepare_encoding(*result.get_kw_arg_string("encoding"), &encoding_opts);

  if (encoding_prepared && encoding_opts.data_type != eDataTypeBinary)
  {
    data = zut_encode(data, "UTF-8", string(encoding_opts.codepage), zjb.diag);
  }

  vector<ZJob> jobs;
  string jobid;
  rc = zjb_submit(&zjb, data, jobid);

  if (0 != rc)
  {
    cerr << "Error: could not submit JCL: '" << data << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  if (result.find_kw_arg_bool("only-jobid"))
    cout << jobid << endl;
  else
    cout << "Submitted, " << jobid << endl;

  return RTNCD_SUCCESS;
}

int handle_job_delete(ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid(result.find_pos_arg_string("jobid"));

  rc = zjb_delete_by_jobid(&zjb, jobid);

  if (0 != rc)
  {
    cerr << "Error: could not delete job: '" << jobid << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "Job " << jobid << " deleted " << endl;

  return RTNCD_SUCCESS;
}

int handle_job_cancel(ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid(result.find_pos_arg_string("jobid"));

  string option_dump(result.find_kw_arg_string("dump"));
  string option_force(result.find_kw_arg_string("force"));
  string option_purge(result.find_kw_arg_string("purge"));
  string option_restart(result.find_kw_arg_string("restart"));

  rc = zjb_cancel_by_jobid(&zjb, jobid);

  if (0 != rc)
  {
    cout << "Error: could not cancel job: '" << jobid << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "Job " << jobid << " cancelled " << endl;

  return RTNCD_SUCCESS;
}

int handle_job_hold(ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid(result.find_pos_arg_string("jobid"));

  rc = zjb_hold_by_jobid(&zjb, jobid);

  if (0 != rc)
  {
    cout << "Error: could not hold job: '" << jobid << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "Job " << jobid << " held " << endl;

  return RTNCD_SUCCESS;
}

int handle_job_release(ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid(result.find_pos_arg_string("jobid"));

  rc = zjb_release_by_jobid(&zjb, jobid);

  if (0 != rc)
  {
    cout << "Error: could not release job: '" << jobid << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "Job " << jobid << " released " << endl;

  return RTNCD_SUCCESS;
}

int handle_console_issue(ParseResult &result)
{
  int rc = 0;
  ZCN zcn = {0};

  string console_name(result.find_kw_arg_string("console-name"));
  string command(result.find_pos_arg_string("command"));

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

  string response = "";
  rc = zcn_get(&zcn, response);
  if (0 != rc)
  {
    cerr << "Error: could not get from console: '" << console_name << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zcn.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << response << endl;

  // example issuing command which requires a reply
  // e.g. zoweax console issue --console-name DKELOSKX "SL SET,ID=DK00"
  // rc = zcn_get(&zcn, response);
  // cout << response << endl;
  // char reply[24] = {0};
  // sprintf(reply, "R %.*s,CANCEL", zcn.reply_id_len, zcn.reply_id);
  // rc = zcn_put(&zcn, reply.c_str());
  // rc = zcn_get(&zcn, response);
  // cout << response << endl;

  rc = zcn_deactivate(&zcn);
  if (0 != rc)
  {
    cerr << "Error: could not deactivate console: '" << console_name << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zcn.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }
  return rc;
}

int handle_data_set_create_member(ZDS zds, string dsn)
{
  int rc = 0;
  size_t start = dsn.find_first_of('(');
  size_t end = dsn.find_last_of(')');

  if (start != string::npos && end != string::npos && end > start)
  {
    string member_name = dsn.substr(start + 1, end - start - 1);
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
    cout << "Data set created: '" << dsn << "'" << endl;
  }
  return rc;
}

int handle_data_set_create_member_dsn(ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};
  string response;
  vector<ZDSEntry> entries;

  size_t start = dsn.find_first_of('(');
  size_t end = dsn.find_last_of(')');
  string member_name = nullptr;
  if (start != string::npos && end != string::npos && end > start)
  {
    member_name = dsn.substr(start + 1, end - start - 1);
    dsn.erase(start, end - start + 1);
  }
  else
  {
    cout << "Error: could not find member name in dsn: '" << dsn << "'" << endl;
    return RTNCD_FAILURE;
  }

  rc = zds_list_data_sets(&zds, dsn, entries);
  if (0 != rc || entries.size() == 0)
  {
    cout << "Error: could not create data set member: '" << dsn << "' rc: '" << rc << "'" << endl;
    cout << "  Details:\n"
         << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }
  return handle_data_set_create_member(zds, dsn + "(" + member_name + ")");
}

int handle_data_set_create_dsn(ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};
  string response;
  rc = zds_create_dsn(&zds, dsn, response);
  if (0 != rc)
  {
    cerr << "Error: could not create data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << response << endl;
    return RTNCD_FAILURE;
  }
  return handle_data_set_create_member(zds, dsn);
}

int handle_data_set_create_dsn_vb(ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};
  string response;
  rc = zds_create_dsn_vb(&zds, dsn, response);
  if (0 != rc)
  {
    cerr << "Error: could not create data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << response << endl;
    return -1;
  }
  return handle_data_set_create_member(zds, dsn);
}

int handle_data_set_create_dsn_adata(ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};
  string response;
  rc = zds_create_dsn_adata(&zds, dsn, response);
  if (0 != rc)
  {
    cerr << "Error: could not create data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << response << endl;
    return -1;
  }
  return handle_data_set_create_member(zds, dsn);
}

int handle_data_set_restore(ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};
  string response;
  unsigned int code = 0;

  string parm = "alloc da('" + dsn + "') shr";

  rc = zut_bpxwdyn(parm, &code, response);
  if (0 != rc)
  {
    cerr << "Error: bpxwdyn with parm '" << parm << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << response << endl;
    return RTNCD_FAILURE;
  }

  cout << "Data set '" << dsn << "' restored" << endl;

  return rc;
}

int handle_data_set_view_dsn(ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};
  string response;

  const auto hasEncoding = zut_prepare_encoding(result.find_kw_arg_string("encoding"), &zds.encoding_opts);
  rc = zds_read_from_dsn(&zds, dsn, response);
  if (0 != rc)
  {
    cerr << "Error: could not read data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  const auto etag = zut_calc_adler32_checksum(response);
  cout << "etag: " << hex << etag << endl;
  cout << "data: ";
  if (hasEncoding && result.find_kw_arg_bool("response-format-bytes"))
  {
    zut_print_string_as_bytes(response);
  }
  else
  {
    cout << response << endl;
  }

  return rc;
}

int handle_data_set_list(ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");

  if (dsn.length() > MAX_DS_LENGTH)
  {
    cerr << "Error: data set pattern exceeds 44 character length limit" << endl;
    return RTNCD_FAILURE;
  }

  dsn += ".**";

  const auto max_entries = result.find_kw_arg_string("max-entries");
  const auto warn = result.find_kw_arg_bool("warn");
  const auto attributes = result.find_kw_arg_string("attributes");

  ZDS zds = {0};
  if (max_entries.size() > 0)
  {
    zds.max_entries = atoi(max_entries.c_str());
  }
  vector<ZDSEntry> entries;

  const auto emit_csv = result.find_kw_arg_bool("response-format-csv");
  rc = zds_list_data_sets(&zds, dsn, entries);
  if (RTNCD_SUCCESS == rc || RTNCD_WARNING == rc)
  {
    vector<string> fields;
    for (vector<ZDSEntry>::iterator it = entries.begin(); it != entries.end(); ++it)
    {
      if (emit_csv)
      {
        fields.push_back(it->name);
        fields.push_back(it->dsorg);
        fields.push_back(it->volser);
        fields.push_back(it->migr ? "true" : "false");
        cout << zut_format_as_csv(fields) << endl;
        fields.clear();
      }
      else
      {
        if (attributes)
        {
          cout << left << setw(44) << it->name << " " << it->volser << " " << setw(4) << it->dsorg << endl;
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

  return !warn && rc == RTNCD_WARNING ? RTNCD_SUCCESS : rc;
}

int handle_data_set_list_members_dsn(ParseResult &result)
{
  int rc = 0;
  const auto dsn = result.find_pos_arg_string("dsn");
  const auto max_entries = result.get_kw_arg_string("max-entries");
  const auto warn = result.find_kw_arg_bool("--warn");
  ZDS zds = {0};
  if (max_entries != nullptr)
  {
    zds.max_entries = atoi(max_entries->c_str());
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

  return !warn && rc == RTNCD_WARNING ? RTNCD_SUCCESS : rc;
}

int handle_data_set_write_to_dsn(ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};
  const auto hasEncoding = result.get_kw_arg_string("encoding") != nullptr && zut_prepare_encoding(*result.get_kw_arg_string("encoding"), &zusf.encoding_opts);

  string data = "";
  string line = "";
  size_t byteSize = 0ul;

  if (!isatty(fileno(stdout)))
  {
    istreambuf_iterator<char> begin(cin);
    istreambuf_iterator<char> end;

    vector<char> input(begin, end);
    const auto temp = string(input.begin(), input.end());
    input.clear();
    const auto bytes = zut_get_contents_as_bytes(temp);

    data.assign(bytes.begin(), bytes.end());
    byteSize = bytes.size();
  }
  else
  {
    while (getline(cin, line))
    {
      data += line;
      data.push_back('\n');
    }
    byteSize = data.size();
  }

  rc = zds_write_to_dsn(&zds, dsn, data, result.find_kw_arg_string("etag"));

  if (0 != rc)
  {
    cerr << "Error: could not write to data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  if (!result.find_kw_arg_bool("etag-only"))
  {
    cout << "Wrote data to '" << dsn << "'" << endl;
  }

  return rc;
}

int handle_data_set_delete_dsn(ParseResult &result)
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

int handle_log_view(ParseResult &result)
{
  int rc = 0;
  unsigned int code = 0;
  string resp;

  string lines = result.find_kw_arg_string("lines");

  cout << "lines are " << lines << endl;
  return 0;
}

int handle_uss_create_file(ParseResult &result)
{
  int rc = 0;
  const auto file_path = result.find_pos_arg_string("file-path");
  auto mode = result.find_pos_arg_string("mode");
  if (mode == "")
    mode = "644";

  ZUSF zusf = {0};
  rc = zusf_create_uss_file_or_dir(&zusf, file_path, mode, false);
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

int handle_uss_create_dir(ParseResult &result)
{
  int rc = 0;
  const auto file_path = result.find_pos_arg_string("file-path");
  auto mode = result.find_pos_arg_string("mode");
  if (mode == "")
    mode = "755";

  ZUSF zusf = {0};
  rc = zusf_create_uss_file_or_dir(&zusf, file_path, mode, true);
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

int handle_uss_list(ParseResult &result)
{
  int rc = 0;
  const auto uss_file = result.find_pos_arg_string("file-path");

  ZUSF zusf = {0};
  string response;
  rc = zusf_list_uss_file_path(&zusf, uss_file, response);
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

int handle_uss_view(ParseResult &result)
{
  int rc = 0;
  string uss_file = result.find_pos_arg_string("file-path");

  ZUSF zusf = {0};
  const auto hasEncoding = result.get_kw_arg_string("encoding") != nullptr && zut_prepare_encoding(*result.get_kw_arg_string("encoding"), &zusf.encoding_opts);

  struct stat file_stats = {0};
  if (stat(uss_file.c_str(), &file_stats) == -1)
  {
    cerr << "Error: Path " << uss_file << " does not exist";
    return RTNCD_FAILURE;
  }

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

  cout << "etag: " << zut_build_etag(file_stats.st_mtime, file_stats.st_size) << endl;
  cout << "data: ";
  if (hasEncoding && result.find_kw_arg_bool("response-format-bytes"))
  {
    zut_print_string_as_bytes(response);
  }
  else
  {
    cout << response << endl;
  }

  return rc;
}

int handle_uss_write(ParseResult &result)
{
  int rc = 0;
  string file = result.find_pos_arg_string("file-path");
  ZUSF zusf = {0};
  const auto encoding_prepared = result.get_kw_arg_string("encoding") != nullptr && zut_prepare_encoding(*result.get_kw_arg_string("encoding"), &encoding_opts);

  string data = "";
  string line = "";
  size_t byteSize = 0ul;

  // Use Ctrl/Cmd + D to stop writing data manually
  if (!isatty(fileno(stdout)))
  {
    istreambuf_iterator<char> begin(cin);
    istreambuf_iterator<char> end;

    vector<char> input(begin, end);
    const auto temp = string(input.begin(), input.end());
    input.clear();
    const auto bytes = zut_get_contents_as_bytes(temp);

    data.assign(bytes.begin(), bytes.end());
    byteSize = bytes.size();
  }
  else
  {
    while (getline(cin, line))
    {
      data += line;
      data.push_back('\n');
    }
    byteSize = data.size();
  }

  const auto etag = result.find_kw_arg_string("etag");
  rc = zusf_write_to_uss_file(&zusf, file, data, etag);
  if (0 != rc)
  {
    cerr << "Error: could not write to USS file: '" << file << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }
  if (!result.find_kw_arg_bool("etag-only"))
  {
    cout << "Wrote data to '" << file << "'" << endl;
  }

  return rc;
}

int handle_uss_delete(ParseResult &result)
{
  auto file_path = result.find_pos_arg_string("file-path");
  const auto recursive = result.find_kw_arg_bool("recursive");

  ZUSF zusf = {0};
  const auto rc = zusf_delete_uss_item(&zusf, file_path, recursive);

  if (rc != 0)
  {
    cerr << "Failed to delete USS item " << file_path << ":\n " << zusf.diag.e_msg << endl;
  }

  return rc;
}

int handle_uss_chmod(ParseResult &result)
{
  int rc = 0;
  const auto mode = result.find_pos_arg_string("mode");
  auto file_path = result.find_pos_arg_string("file-path");

  ZUSF zusf = {0};
  rc = zusf_chmod_uss_file_or_dir(&zusf, file_path, mode, result.find_kw_arg_bool("recursive"));
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

int handle_uss_chown(ParseResult &result)
{
  auto path = result.find_pos_arg_string("file-path");
  const auto owner = result.find_pos_arg_string("owner");

  ZUSF zusf = {0};

  const auto rc = zusf_chown_uss_file_or_dir(&zusf, path, owner, result.find_kw_arg_bool("recursive"));
  if (rc != 0)
  {
    cerr << "Error: could not chown USS path: '" << path << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  return rc;
}

int handle_uss_chtag(ParseResult &result)
{
  auto path = result.find_pos_arg_string("file-path");
  const auto tag = result.find_pos_arg_string("tag");

  ZUSF zusf = {0};
  const auto rc = zusf_chtag_uss_file_or_dir(&zusf, path, tag, result.find_kw_arg_bool("recursive"));

  if (rc != 0)
  {
    cerr << "Error: could not chtag USS path: '" << path << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  return rc;
}

int handle_tso_issue(ParseResult &result)
{
  int rc = 0;
  const auto command = result.find_pos_arg_string("command");
  string response = "";

  rc = ztso_issue(command, response);

  if (0 != rc)
  {
    cerr << "Error running command, rc '" << rc << "'" << endl;
    cerr << "  Details: " << response << endl;
  }

  cout << response;

  return rc;
}

int handle_tool_convert_dsect(ParseResult &result)
{
  int rc = 0;
  ZCN zcn = {0};
  unsigned int code = 0;
  string resp;

  // create a lrecl 255, vb for chdr output
  // create a lrecl 32756, vb for adata output
  // z/os unix .s file
  // as -madata --gadata="//'DKELOSKY.TEMP.ADATA(IHAECB)'" ihaecb.s
  // convert --adata (dsn) --out-chdr (dsn) --sysout /tmp/user/sysout.txt --sysprint /tmp/user/sysprint.txt

  string adata_dsn(result.find_kw_arg_string("adata-dsn"));
  string chdr_dsn(result.find_kw_arg_string("chdr-dsn"));
  string sysprint(result.find_kw_arg_string("sysprint"));
  string sysout(result.find_kw_arg_string("sysout"));

  const char *user = getlogin();
  string struser(user);
  transform(struser.begin(), struser.end(), struser.begin(), ::tolower); // upper case

  if (result.get_kw_arg_string("sysprint") == nullptr)
    sysprint = "/tmp/" + struser + "_sysprint.txt";
  if (result.get_kw_arg_string("sysout") == nullptr)
    sysout = "/tmp/" + struser + "_sysout.txt";

  cout << adata_dsn << " " << chdr_dsn << " " << sysprint << " " << sysout << endl;

  vector<string> dds;
  char buffer[256] = {0};
  // https://www.ibm.com/docs/en/zos/3.1.0?topic=definition-status-group
  // https://www.ibm.com/docs/en/zos/3.1.0?topic=pp-syntax-2
  dds.push_back("alloc fi(sysprint) path('" + sysprint + "') pathopts(owronly,ocreat,otrunc) pathmode(sirusr,siwusr,sirgrp) filedata(text) msg(2)");
  dds.push_back("alloc fi(sysout) path('" + sysout + "') pathopts(owronly,ocreat,otrunc) pathmode(sirusr,siwusr,sirgrp) filedata(text) msg(2)");
  dds.push_back("alloc fi(sysadata) da('" + adata_dsn + "') shr msg(2)");
  dds.push_back("alloc fi(edcdsect) da('" + chdr_dsn + "') shr msg(2)");

  rc = loop_dynalloc(dds);
  if (RTNCD_SUCCESS != rc)
  {
    return RTNCD_FAILURE;
  }

  rc = zut_convert_dsect();
  if (0 != rc)
  {
    cerr << "Error: convert failed with rc: '" << rc << "'" << endl;
    cout << "  See '" << sysprint << "' and '" << sysout << "' for more details" << endl;
    return -1;
  }

  cout << "DSECT converted to '" << chdr_dsn << "'" << endl;
  cout << "Copy it via `cp \"//'" + chdr_dsn + "'\" <member>.h`" << endl;

  return rc;
}

int handle_tool_dynalloc(ParseResult &result)
{
  int rc = 0;
  unsigned int code = 0;
  string resp;

  string parm(result.find_kw_arg_string("parm"));

  // alloc da('DKELOSKY.TEMP.ADATA') DSORG(PO) SPACE(5,5) CYL LRECL(80) RECFM(F,b) NEW DIR(5) vol(USER01)
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

int handle_tool_display_symbol(ParseResult &result)
{
  int rc = 0;
  string symbol(result.find_pos_arg_string("symbol"));
  transform(symbol.begin(), symbol.end(), symbol.begin(), ::toupper); // upper case
  symbol = "&" + symbol;
  string value;
  rc = zut_substitute_sybmol(symbol, value);
  if (0 != rc)
  {
    cerr << "Error: asasymbf with parm '" << symbol << "' rc: '" << rc << "'" << endl;
    return RTNCD_FAILURE;
  }
  cout << value << endl;

  return RTNCD_SUCCESS;
}

int handle_tool_run(ParseResult &result)
{
  int rc = 0;
  string program(result.find_pos_arg_string("program"));
  string dynalloc_pre(result.find_kw_arg_string("dynalloc-pre"));
  string dynalloc_post(result.find_kw_arg_string("dynalloc-post"));

  // allocate anything that was requested
  if (result.get_kw_arg_string("dynalloc-pre") != nullptr)
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
    if (RTNCD_SUCCESS != rc)
    {
      return RTNCD_FAILURE;
    }
  }

  const auto indd = result.find_kw_arg_string("in-dd");
  if (result.get_kw_arg_string("in-dd") != nullptr)
  {
    string ddname = "DD:" + indd;
    ofstream out(ddname.c_str());
    if (!out.is_open())
    {
      cerr << "Error: could not open input '" << ddname << "'" << endl;
      return RTNCD_FAILURE;
    }

    string input(result.find_kw_arg_string("input"));
    if (result.get_kw_arg_string("input"))
    {
      out << input << endl;
    }

    out.close();
  }

  transform(program.begin(), program.end(), program.begin(), ::toupper); // upper case

  rc = zut_run(program);

  if (0 != rc)
  {
    cerr << "Error: program '" << program << "' ended with rc: '" << rc << "'" << endl;
    rc = RTNCD_FAILURE; // continue to obtain output
  }

  const auto outdd = result.find_kw_arg_string("out-dd");
  if (result.get_kw_arg_string("out-dd") != nullptr)
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

  // optionally free everything that was allocated
  if (result.get_kw_arg_string("dynalloc-post") != nullptr)
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

int handle_tool_search(ParseResult &result)
{
  int rc = 0;

  string pattern(result.find_pos_arg_string("string"));
  const auto warn = result.find_kw_arg_bool("warn");
  string max_entries = result.find_kw_arg_string("max-entries");
  string dsn(result.find_pos_arg_string("dsn"));

  ZDS zds = {0};
  bool results_truncated = false;

  if (max_entries.size() > 0)
  {
    zds.max_entries = atoi(max_entries.c_str());
  }

  // list members in a data set
  vector<ZDSMem> members;
  rc = zds_list_members(&zds, dsn, members);

  // note if results are truncated
  if (RTNCD_WARNING == rc)
  {

    if (ZDS_RSNCD_MAXED_ENTRIES_REACHED == zds.diag.detail_rc)
    {
      results_truncated = true;
    }
  }

  // note failure if we can't list
  if (RTNCD_SUCCESS != rc && RTNCD_WARNING != rc)
  {
    cerr << "Error: could not read data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  // perform dynalloc
  vector<string> dds;
  dds.push_back("alloc dd(newdd) da('" + dsn + "') shr");
  dds.push_back("alloc dd(outdd)");
  dds.push_back("alloc dd(sysin)");

  rc = loop_dynalloc(dds);
  if (RTNCD_SUCCESS != rc)
  {
    return RTNCD_FAILURE;
  }

  // build super c selection criteria
  string data = " SRCHFOR '" + pattern + "'\n";

  for (vector<ZDSMem>::iterator it = members.begin(); it != members.end(); ++it)
  {
    data += " SELECT " + it->name + "\n";
  }

  // write control statements
  zds_write_to_dd(&zds, "sysin", data);
  if (0 != rc)
  {
    cerr << "Error: could not write to dd: '" << "sysin" << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  // perform search
  rc = zut_search("parms are unused for now but can be passed to super c, e.g. ANYC (any case)");
  if (rc != RTNCD_SUCCESS ||
      rc != ZUT_RTNCD_SEARCH_SUCCESS ||
      rc != RTNCD_WARNING ||
      rc != ZUT_RTNCD_SEARCH_WARNING)
  {
    cerr << "Error: could error invoking ISRSUPC rc: '" << rc << "'" << endl;
    // NOTE(Kelosky): don't exit here, but proceed to print errors
  }

  // read output from super c
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
    if (warn)
    {
      cerr << "Warning: results truncated" << endl;
    }
  }

  return !warn && rc == RTNCD_WARNING ? RTNCD_SUCCESS : rc;
}