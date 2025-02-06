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
#include <algorithm>
#include "zcn.hpp"
#include "zut.hpp"
#include "zcli.hpp"
#include "zjb.hpp"
#include "unistd.h"
#include "zds.hpp"
#include "zusf.hpp"
#include "ztso.hpp"

#ifndef TO_STRING
#define TO_STRING(x) static_cast<std::ostringstream &>(           \
                         (std::ostringstream() << std::dec << x)) \
                         .str()
#endif

using namespace std;

int handle_job_list(ZCLIResult);
int handle_job_list_files(ZCLIResult);
int handle_job_view_status(ZCLIResult);
int handle_job_view_file(ZCLIResult);
int handle_job_view_jcl(ZCLIResult);
int handle_job_submit(ZCLIResult);
int handle_job_delete(ZCLIResult);

int handle_console_issue(ZCLIResult);

int handle_data_set_create_dsn(ZCLIResult);
int handle_data_set_create_dsn_vb(ZCLIResult);
int handle_data_set_create_dsn_adata(ZCLIResult);
int handle_data_set_restore(ZCLIResult);
int handle_data_set_view_dsn(ZCLIResult);
int handle_data_set_list(ZCLIResult);
int handle_data_set_list_members_dsn(ZCLIResult);
int handle_data_set_write_to_dsn(ZCLIResult);
int handle_data_set_delete_dsn(ZCLIResult);

int handle_log_view(ZCLIResult);

int handle_tool_convert_dsect(ZCLIResult);
int handle_tool_dynalloc(ZCLIResult);

// TODO(Kelosky):
// help w/verbose examples
// add simple examples to help

int handle_uss_create_file(ZCLIResult);
int handle_uss_create_dir(ZCLIResult);
int handle_uss_list(ZCLIResult);
int handle_uss_view(ZCLIResult);
int handle_uss_write(ZCLIResult);
int handle_uss_delete_file(ZCLIResult);
int handle_uss_delete_dir(ZCLIResult);
int handle_uss_chmod(ZCLIResult);
int handle_uss_chown(ZCLIResult);

int handle_tso_issue(ZCLIResult);

int main(int argc, char *argv[])
{
  // CLI
  ZCLI zcli(argv[PROCESS_NAME_ARG]);
  zcli.set_interactive_mode(true);

  ZCLIOption response_format_csv("response-format-csv");
  response_format_csv.set_description("returns the response in CSV format");
  response_format_csv.get_aliases().push_back("--rfc");
  response_format_csv.set_default("false");
  response_format_csv.set_required(false);

  ZCLIOption response_format_bytes("response-format-bytes");
  response_format_bytes.set_description("returns the response as raw bytes");
  response_format_bytes.get_aliases().push_back("--rfb");
  response_format_bytes.set_required(false);

  ZCLIGroup tso_group("tso");
  tso_group.set_description("TSO operations");

  ZCLIVerb tso_issue("issue");
  tso_issue.set_description("issue TSO command");
  tso_issue.set_zcli_verb_handler(handle_tso_issue);

  ZCLIPositional tso_command("command");
  tso_command.set_required(true);
  tso_command.set_description("command to issue");

  tso_issue.get_positionals().push_back(tso_command);

  tso_group.get_verbs().push_back(tso_issue);

  //
  // data set group
  //
  ZCLIGroup data_set_group("data-set");
  data_set_group.set_description("z/OS data set operations");

  ZCLIPositional data_set_dsn("dsn");
  data_set_dsn.set_description("data set name, optionally with member specified");
  data_set_dsn.set_required(true);

  ZCLIOption data_set_encoding("encoding");
  data_set_encoding.set_description("return data set contents in given encoding");

  // data set verbs
  ZCLIVerb data_set_create("create");
  data_set_create.set_description("create data set using defaults: DSORG=PO, RECFM=FB, LRECL=80");
  data_set_create.set_zcli_verb_handler(handle_data_set_create_dsn);
  data_set_create.get_positionals().push_back(data_set_dsn);
  data_set_group.get_verbs().push_back(data_set_create);

  ZCLIVerb data_set_create_vb("create-vb");
  data_set_create_vb.set_description("create VB data set using defaults: DSORG=PO, RECFM=VB, LRECL=255");
  data_set_create_vb.set_zcli_verb_handler(handle_data_set_create_dsn_vb);
  data_set_create_vb.get_positionals().push_back(data_set_dsn);
  data_set_group.get_verbs().push_back(data_set_create_vb);

  ZCLIVerb data_set_create_adata("create-adata");
  data_set_create_adata.set_description("create VB data set using defaults: DSORG=PO, RECFM=VB, LRECL=32756");
  data_set_create_adata.set_zcli_verb_handler(handle_data_set_create_dsn_adata);
  data_set_create_adata.get_positionals().push_back(data_set_dsn);
  data_set_group.get_verbs().push_back(data_set_create_adata);

  ZCLIVerb data_set_restore("restore");
  data_set_restore.set_description("restore/recall data set");
  data_set_restore.set_zcli_verb_handler(handle_data_set_restore);
  data_set_restore.get_positionals().push_back(data_set_dsn);
  data_set_group.get_verbs().push_back(data_set_restore);

  ZCLIVerb data_set_view("view");
  data_set_view.set_description("view data set");
  data_set_view.set_zcli_verb_handler(handle_data_set_view_dsn);
  data_set_view.get_positionals().push_back(data_set_dsn);
  data_set_view.get_options().push_back(data_set_encoding);
  data_set_view.get_options().push_back(response_format_bytes);
  data_set_group.get_verbs().push_back(data_set_view);

  ZCLIVerb data_set_list("list");
  ZCLIOption data_set_max_entries("max-entries");
  data_set_max_entries.set_description("max number of results to return before error generated");
  data_set_list.get_options().push_back(data_set_max_entries);

  ZCLIOption data_set_truncate_warn("warn");
  data_set_truncate_warn.set_description("warn if trucated or not found");
  data_set_truncate_warn.set_default("true");
  data_set_list.get_options().push_back(data_set_truncate_warn);

  data_set_list.set_description("list data sets");
  data_set_list.set_zcli_verb_handler(handle_data_set_list);
  data_set_list.get_positionals().push_back(data_set_dsn);
  data_set_list.get_options().push_back(response_format_csv);
  data_set_group.get_verbs().push_back(data_set_list);

  ZCLIVerb data_set_list_members("list-members");
  data_set_list_members.set_description("list data set members");
  data_set_list_members.set_zcli_verb_handler(handle_data_set_list_members_dsn);
  data_set_list_members.get_positionals().push_back(data_set_dsn);
  data_set_group.get_verbs().push_back(data_set_list_members);

  ZCLIVerb data_set_write("write");
  data_set_write.set_description("write to data set");
  data_set_write.set_zcli_verb_handler(handle_data_set_write_to_dsn);
  data_set_write.get_positionals().push_back(data_set_dsn);
  data_set_write.get_options().push_back(data_set_encoding);
  data_set_group.get_verbs().push_back(data_set_write);

  ZCLIVerb data_set_delete("delete");
  data_set_delete.set_description("delete data set");
  data_set_delete.set_zcli_verb_handler(handle_data_set_delete_dsn);
  data_set_delete.get_positionals().push_back(data_set_dsn);
  data_set_group.get_verbs().push_back(data_set_delete);

  //
  // jobs group
  //
  ZCLIGroup job_group("job");
  job_group.set_description("z/OS job operations");

  ZCLIOption spool_encoding("encoding");
  spool_encoding.set_description("return spool contents in given encoding");

  // jobs verbs
  ZCLIVerb job_list("list");
  job_list.set_description("list jobs");
  job_list.set_zcli_verb_handler(handle_job_list);
  ZCLIOption job_owner("owner");
  job_owner.set_description("filter by owner");
  job_list.get_options().push_back(job_owner);
  job_list.get_options().push_back(response_format_csv);
  job_group.get_verbs().push_back(job_list);

  ZCLIVerb job_list_files("list-files");
  job_list_files.set_description("list spool files for jobid");
  job_list_files.set_zcli_verb_handler(handle_job_list_files);
  ZCLIPositional job_jobid("jobid");
  job_jobid.set_required(true);
  job_jobid.set_description("valid jobid");
  job_list_files.get_positionals().push_back(job_jobid);
  job_list_files.get_options().push_back(response_format_csv);
  job_group.get_verbs().push_back(job_list_files);

  ZCLIVerb job_view_status("view-status");
  job_view_status.set_description("view job status");
  job_view_status.set_zcli_verb_handler(handle_job_view_status);
  job_view_status.get_positionals().push_back(job_jobid);
  job_view_status.get_options().push_back(response_format_csv);
  job_group.get_verbs().push_back(job_view_status);

  ZCLIVerb job_view_file("view-file");
  job_view_file.set_description("view job file output");
  job_view_file.set_zcli_verb_handler(handle_job_view_file);
  job_view_file.get_positionals().push_back(job_jobid);
  job_view_file.get_options().push_back(spool_encoding);
  job_view_file.get_options().push_back(response_format_bytes);

  ZCLIPositional job_dsn_key("key");
  job_dsn_key.set_required(true);
  job_dsn_key.set_description("valid job dsn key via 'job list-files'");
  job_view_file.get_positionals().push_back(job_dsn_key);
  job_group.get_verbs().push_back(job_view_file);

  ZCLIVerb job_view_jcl("view-jcl");
  job_view_jcl.set_description("view job jcl from input jobid");
  job_view_jcl.set_zcli_verb_handler(handle_job_view_jcl);
  job_view_jcl.get_positionals().push_back(job_jobid);
  job_group.get_verbs().push_back(job_view_jcl);

  ZCLIVerb job_submit("submit");
  job_submit.set_description("submit a job");
  job_submit.set_zcli_verb_handler(handle_job_submit);
  ZCLIOption job_jobid_only("only-jobid");
  job_jobid_only.set_description("show only job id on success");
  job_submit.get_options().push_back(job_jobid_only);
  ZCLIPositional job_dsn("dsn");
  job_dsn.set_required(true);
  job_dsn.set_description("dsn containing JCL");
  job_submit.get_positionals().push_back(job_dsn);
  job_group.get_verbs().push_back(job_submit);

  ZCLIVerb job_delete("delete");
  job_delete.set_description("delete a job");
  job_delete.set_zcli_verb_handler(handle_job_delete);
  job_delete.get_positionals().push_back(job_jobid);
  job_group.get_verbs().push_back(job_delete);

  //
  // console group
  //
  ZCLIGroup console_group("console");
  console_group.set_description("z/OS console operations");

  // console verbs
  ZCLIVerb console_issue("issue");
  console_issue.set_description("issue a console command");
  console_issue.set_zcli_verb_handler(handle_console_issue);
  ZCLIOption console_name("console-name");
  console_name.set_required(true);
  console_name.get_aliases().push_back("--cn");
  console_name.set_description("extended console name");
  console_issue.get_options().push_back(console_name);
  ZCLIPositional console_command("command");
  console_command.set_required(true);
  console_command.set_description("command to run, e.g. 'D IPLINFO'");
  console_issue.get_positionals().push_back(console_command);
  console_group.get_verbs().push_back(console_issue);

  //
  // uss group
  //
  ZCLIGroup uss_group("uss");
  uss_group.set_description("z/OS USS operations");

  // uss common options and positionals
  ZCLIPositional uss_file_path("file-path");
  uss_file_path.set_required(true);
  uss_file_path.set_description("file path");
  ZCLIOption uss_file_mode("mode");
  uss_file_mode.set_required(false);
  uss_file_mode.set_description("permissions");
  ZCLIOption uss_recursive("recursive");
  uss_recursive.set_required(false);
  uss_recursive.set_description("recursive");

  ZCLIVerb uss_create_file("create-file");
  uss_create_file.set_description("create a USS file");
  uss_create_file.set_zcli_verb_handler(handle_uss_create_file);
  uss_create_file.get_positionals().push_back(uss_file_path);
  uss_create_file.get_options().push_back(uss_file_mode);
  uss_group.get_verbs().push_back(uss_create_file);

  ZCLIVerb uss_create_dir("create-dir");
  uss_create_dir.set_description("create a USS directory");
  uss_create_dir.set_zcli_verb_handler(handle_uss_create_dir);
  uss_create_dir.get_positionals().push_back(uss_file_path);
  uss_create_dir.get_options().push_back(uss_file_mode);
  uss_group.get_verbs().push_back(uss_create_dir);

  ZCLIVerb uss_list("list");
  uss_list.set_description("list USS files and directories");
  uss_list.set_zcli_verb_handler(handle_uss_list);
  uss_list.get_positionals().push_back(uss_file_path);
  uss_group.get_verbs().push_back(uss_list);

  ZCLIOption uss_encoding("encoding");
  uss_encoding.set_description("return file contents in given encoding");

  ZCLIVerb uss_view("view");
  uss_view.set_description("view a USS file");
  uss_view.get_positionals().push_back(uss_file_path);
  uss_view.set_zcli_verb_handler(handle_uss_view);
  uss_view.get_options().push_back(uss_encoding);
  uss_view.get_options().push_back(response_format_bytes);
  uss_group.get_verbs().push_back(uss_view);

  ZCLIVerb uss_write("write");
  uss_write.set_description("write to a USS file");
  uss_write.set_zcli_verb_handler(handle_uss_write);
  uss_write.get_positionals().push_back(uss_file_path);
  uss_write.get_options().push_back(uss_encoding);
  uss_group.get_verbs().push_back(uss_write);

  ZCLIVerb uss_delete_file("delete-file");
  uss_delete_file.set_description("delete a USS file");
  uss_delete_file.set_zcli_verb_handler(handle_uss_delete_file);
  uss_delete_file.get_positionals().push_back(uss_file_path);
  uss_group.get_verbs().push_back(uss_delete_file);

  ZCLIVerb uss_delete_dir("delete-dir");
  uss_delete_dir.set_description("delete a USS directory");
  uss_delete_dir.set_zcli_verb_handler(handle_uss_delete_dir);
  uss_delete_dir.get_positionals().push_back(uss_file_path);
  uss_delete_dir.get_options().push_back(uss_recursive);
  uss_group.get_verbs().push_back(uss_delete_dir);

  ZCLIVerb uss_chmod("chmod");
  uss_chmod.set_description("change permissions on a USS file or directory");
  uss_chmod.set_zcli_verb_handler(handle_uss_chmod);
  uss_chmod.get_positionals().push_back(uss_file_path);
  uss_chmod.get_options().push_back(uss_file_mode);
  uss_chmod.get_options().push_back(uss_recursive);
  uss_group.get_verbs().push_back(uss_chmod);

  ZCLIVerb uss_chown("chown");
  uss_chown.set_description("change owner on a USS file or directory");
  uss_chown.set_zcli_verb_handler(handle_uss_chown);
  uss_chown.get_positionals().push_back(uss_file_path);
  uss_chown.get_options().push_back(uss_recursive);
  uss_group.get_verbs().push_back(uss_chown);
  // log group
  //
  ZCLIGroup log_group("log");
  log_group.set_description("log operations");
  ZCLIVerb log_view("view");
  log_view.set_description("view log");
  log_view.set_zcli_verb_handler(handle_log_view);
  ZCLIOption log_option_lines("lines");
  log_option_lines.set_default("100");
  log_option_lines.set_description("number of lines to print");
  log_view.get_options().push_back(log_option_lines);
  log_group.get_verbs().push_back(log_view);

  //
  // tool group
  //
  ZCLIGroup tool_group("tool");
  tool_group.set_description("tool operations");

  // console verbs
  ZCLIVerb tool_convert_dsect("ccnedsct");
  tool_convert_dsect.set_description("convert dsect to c struct");
  tool_convert_dsect.set_zcli_verb_handler(handle_tool_convert_dsect);
  ZCLIOption adata_dsn("adata-dsn");
  adata_dsn.set_description("input adata dsn");
  adata_dsn.set_required(true);
  adata_dsn.get_aliases().push_back("--ad");
  ZCLIOption chdr_name("chdr-dsn");
  chdr_name.get_aliases().push_back("--cd");
  chdr_name.set_description("output chdr dsn");
  chdr_name.set_required(true);
  ZCLIOption sysprint("sysprint");
  sysprint.get_aliases().push_back("--sp");
  sysprint.set_description("sysprint output");
  ZCLIOption sysout("sysout");
  sysout.set_description("sysout output");
  sysout.get_aliases().push_back("--so");
  tool_convert_dsect.get_options().push_back(adata_dsn);
  tool_convert_dsect.get_options().push_back(chdr_name);
  tool_convert_dsect.get_options().push_back(sysprint);
  tool_convert_dsect.get_options().push_back(sysout);
  tool_group.get_verbs().push_back(tool_convert_dsect);

  ZCLIVerb tool_dynalloc("bpxwdy2");
  tool_dynalloc.set_description("dynalloc command");
  tool_dynalloc.set_zcli_verb_handler(handle_tool_dynalloc);
  ZCLIPositional dynalloc_parm("parm");
  dynalloc_parm.set_description("dynalloc parm string");
  dynalloc_parm.set_required(true);
  tool_dynalloc.get_positionals().push_back(dynalloc_parm);
  tool_group.get_verbs().push_back(tool_dynalloc);

  // add all groups to the CLI
  zcli.get_groups().push_back(data_set_group);
  zcli.get_groups().push_back(console_group);
  zcli.get_groups().push_back(job_group);
  zcli.get_groups().push_back(uss_group);
  zcli.get_groups().push_back(tso_group);
  // zcli.get_groups().push_back(log_group);
  zcli.get_groups().push_back(tool_group);

  // parse
  return zcli.parse(argc, argv);
}

int handle_job_list(ZCLIResult result)
{
  int rc = 0;
  ZJB zjb = {0};
  string owner_name(result.get_option("--owner").get_value());

  vector<ZJob> jobs;
  rc = zjb_list_by_owner(&zjb, owner_name, jobs);

  if (0 != rc)
  {
    cout << "Error: could not list jobs for: '" << owner_name << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  const auto emit_csv = result.get_option("--response-format-csv").get_value() == "true";
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

  return RTNCD_SUCCESS;
}

int handle_job_list_files(ZCLIResult result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid(result.get_positional("jobid").get_value());

  vector<ZJobDD> job_dds;
  rc = zjb_list_dds_by_jobid(&zjb, jobid, job_dds);

  if (0 != rc)
  {
    cout << "Error: could not list jobs for: '" << jobid << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  const auto emit_csv = result.get_option("--response-format-csv").get_value() == "true";
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

  return RTNCD_SUCCESS;
}

int handle_job_view_status(ZCLIResult result)
{
  int rc = 0;
  ZJB zjb = {0};
  ZJob job = {0};
  string jobid(result.get_positional("jobid").get_value());

  const auto emit_csv = result.get_option("--response-format-csv").get_value() == "true";
  rc = zjb_view_by_jobid(&zjb, jobid, job);

  if (0 != rc)
  {
    cout << "Error: could not view job status for: '" << jobid << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zjb.diag.e_msg << endl;
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

int handle_job_view_file(ZCLIResult result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid(result.get_positional("jobid").get_value());
  string key(result.get_positional("key").get_value());

  const auto hasEncoding = result.get_option("--encoding").is_found() && zut_prepare_encoding(result.get_option("--encoding").get_value(), &zjb.encoding_opts);

  string resp;
  rc = zjb_read_jobs_output_by_jobid_and_key(&zjb, jobid, atoi(key.c_str()), resp);

  if (0 != rc)
  {
    cout << "Error: could not view job file for: '" << jobid << "' with key '" << key << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  if (hasEncoding && result.get_option("--response-format-bytes").get_value() == "true")
  {
    zut_print_string_as_bytes(resp);
  }
  else
  {
    cout << resp;
  }

  return RTNCD_SUCCESS;
}

int handle_job_view_jcl(ZCLIResult result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid(result.get_positional("jobid").get_value());

  string resp;
  rc = zjb_read_job_jcl_by_jobid(&zjb, jobid, resp);

  if (0 != rc)
  {
    cout << "Error: could not view job file for: '" << jobid << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zjb.diag.e_msg << endl;
    return -1;
  }

  cout << resp;

  return 0;
}

int handle_job_submit(ZCLIResult result)
{
  int rc = 0;
  ZJB zjb = {0};
  string dsn(result.get_positional("dsn").get_value());

  vector<ZJob> jobs;
  string jobid;
  rc = zjb_submit(&zjb, dsn, jobid);

  if (0 != rc)
  {
    cout << "Error: could not submit JCL: '" << dsn << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  string only_jobid(result.get_option("--only-jobid").get_value());
  if ("true" == only_jobid)
    cout << jobid << endl;
  else
    cout << "Submitted " << dsn << ", " << jobid << endl;

  return RTNCD_SUCCESS;
}

int handle_job_delete(ZCLIResult result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid(result.get_positional("jobid").get_value());

  rc = zjb_delete_by_jobid(&zjb, jobid);

  if (0 != rc)
  {
    cout << "Error: could not delete job: '" << jobid << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "Job " << jobid << " deleted " << endl;

  return RTNCD_SUCCESS;
}

int handle_console_issue(ZCLIResult result)
{
  int rc = 0;
  ZCN zcn = {0};

  string console_name(result.get_option("--console-name").get_value());
  string command(result.get_positional("command").get_value());

  rc = zcn_activate(&zcn, string(console_name));
  if (0 != rc)
  {
    cout << "Error: could not activate console: '" << console_name << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zcn.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  printf("%.8s", zcn.console_name);

  rc = zcn_put(&zcn, command);
  if (0 != rc)
  {
    cout << "Error: could not write to console: '" << console_name << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zcn.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  string response = "";
  rc = zcn_get(&zcn, response);
  if (0 != rc)
  {
    cout << "Error: could not get from console: '" << console_name << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zcn.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << response << endl;

  // example issuing command which requires a reply
  // e.g. zowexx console issue --console-name DKELOSKX "SL SET,ID=DK00"
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
    cout << "Error: could not deactivate console: '" << console_name << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zcn.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }
  return rc;
}

int handle_data_set_create_dsn(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn").get_value();
  ZDS zds = {0};
  string response;
  rc = zds_create_dsn(&zds, dsn, response);
  if (0 != rc)
  {
    cout << "Error: could not create data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cout << "  Details:\n"
         << response << endl;
    return RTNCD_FAILURE;
  }

  cout << "Data set '" << dsn << "' created" << endl;

  return rc;
}

int handle_data_set_create_dsn_vb(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn").get_value();
  ZDS zds = {0};
  string response;
  rc = zds_create_dsn_vb(&zds, dsn, response);
  if (0 != rc)
  {
    cout << "Error: could not create data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cout << "  Details:\n"
         << response << endl;
    return -1;
  }

  cout << "Data set '" << dsn << "' created" << endl;

  return rc;
}

int handle_data_set_create_dsn_adata(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn").get_value();
  ZDS zds = {0};
  string response;
  rc = zds_create_dsn_adata(&zds, dsn, response);
  if (0 != rc)
  {
    cout << "Error: could not create data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cout << "  Details:\n"
         << response << endl;
    return -1;
  }

  cout << "Data set '" << dsn << "' created" << endl;

  return rc;
}

int handle_data_set_restore(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn").get_value();
  ZDS zds = {0};
  string response;
  unsigned int code = 0;

  string parm = "alloc da('" + dsn + "') shr";

  rc = zut_bpxwdyn(parm, &code, response);
  if (0 != rc)
  {
    cout << "Error: bpxwdyn with parm '" << parm << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << response << endl;
    return RTNCD_FAILURE;
  }

  cout << "Data set '" << dsn << "' restored" << endl;

  return rc;
}

int handle_data_set_view_dsn(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn").get_value();
  ZCLIOption &encoding = result.get_option("--encoding");
  ZDS zds = {0};
  string response;
  const auto hasEncoding = result.get_option("--encoding").is_found() && zut_prepare_encoding(result.get_option("--encoding").get_value(), &zds.encoding_opts);
  rc = zds_read_from_dsn(&zds, dsn, response);
  if (0 != rc)
  {
    cout << "Error: could not read data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  if (hasEncoding && result.get_option("--response-format-bytes").get_value() == "true")
  {
    zut_print_string_as_bytes(response);
  }
  else
  {
    cout << response;
  }

  return rc;
}

int handle_data_set_list(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn").get_value();
  string max_entries = result.get_option("--max-entries").get_value();
  string warn = result.get_option("--warn").get_value();

  ZDS zds = {0};
  if (max_entries.size() > 0)
  {
    zds.max_entries = atoi(max_entries.c_str());
  }
  vector<ZDSEntry> entries;

  const auto emit_csv = result.get_option("--response-format-csv").get_value() == "true";
  rc = zds_list_data_sets(&zds, dsn, entries);
  if (RTNCD_SUCCESS == rc)
  {
    vector<string> fields;
    for (vector<ZDSEntry>::iterator it = entries.begin(); it != entries.end(); ++it)
    {
      if (emit_csv)
      {
        fields.push_back(it->name);
        fields.push_back(it->dsorg);
        fields.push_back(it->volser);
        std::cout << zut_format_as_csv(fields) << std::endl;
        fields.clear();
      }
      else
      {
        std::cout << left << setw(44) << it->name << " " << it->volser << " " << it->dsorg << endl;
      }
    }
  }
  else if (RTNCD_WARNING == rc)
  {
    if ("true" == warn)
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
    vector<string> fields;
    for (vector<ZDSEntry>::iterator it = entries.begin(); it != entries.end(); ++it)
    {
      if (emit_csv)
      {
        fields.push_back(it->name);
        fields.push_back(it->dsorg);
        fields.push_back(it->volser);
        std::cout << zut_format_as_csv(fields) << std::endl;
        fields.clear();
      }
      else
      {
        std::cout << left << setw(44) << it->name << " " << it->volser << " " << it->dsorg << endl;
      }
    }
  }
  else
  {
    cout << "Error: could not list data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  return rc;
}

int handle_data_set_list_members_dsn(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn").get_value();
  ZDS zds = {0};
  vector<ZDSMem> members;
  rc = zds_list_members(&zds, dsn, members);
  if (0 != rc)
  {
    cout << "Error: could not read data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }
  for (vector<ZDSMem>::iterator it = members.begin(); it != members.end(); ++it)
  {
    std::cout << left << setw(12) << it->name << endl;
  }

  return rc;
}

int handle_data_set_write_to_dsn(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn").get_value();
  ZDS zds = {0};

  string data;
  string line;

  size_t byteSize = 0ul;
  const auto hasEncoding = result.get_option("--encoding").is_found() && zut_prepare_encoding(result.get_option("--encoding").get_value(), &zds.encoding_opts);
  if (hasEncoding)
  {
    std::istreambuf_iterator<char> begin(std::cin);
    std::istreambuf_iterator<char> end;

    std::vector<char> bytes(begin, end);
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

  rc = zds_write_to_dsn(&zds, dsn, data);

  if (0 != rc)
  {
    cout << "Error: could not write to data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }
  cout << "Wrote data to '" << dsn << "'" << endl;

  return rc;
}

int handle_data_set_delete_dsn(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn").get_value();
  ZDS zds = {0};
  rc = zds_delete_dsn(&zds, dsn);

  if (0 != rc)
  {
    cout << "Error: could not delete data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }
  cout << "Data set '" << dsn << "' deleted" << endl;

  return rc;
}

int handle_log_view(ZCLIResult result)
{
  int rc = 0;
  unsigned int code = 0;
  string resp;

  string lines = result.get_option("--lines").get_value();

  cout << "lines are " << lines << endl;
  return 0;
}
int handle_tool_dynalloc(ZCLIResult result)
{
  int rc = 0;
  unsigned int code = 0;
  string resp;

  string parm(result.get_positional("parm").get_value());

  // alloc da('DKELOSKY.TEMP.ADATA') DSORG(PO) SPACE(5,5) CYL LRECL(80) RECFM(F,b) NEW DIR(5) vol(USER01)
  rc = zut_bpxwdyn(parm, &code, resp);
  if (0 != rc)
  {
    cout << "Error: bpxwdyn with parm '" << parm << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << resp << endl;
    return RTNCD_FAILURE;
  }

  cout << resp << endl;

  return rc;
}

int handle_uss_create_file(ZCLIResult result)
{
  int rc = 0;
  string file_path = result.get_positional("file-path").get_value();
  string mode(result.get_option("--mode").get_value());
  if (mode == "")
    mode = "644";

  ZUSF zusf = {0};
  rc = zusf_create_uss_file_or_dir(&zusf, file_path, mode, false);
  if (0 != rc)
  {
    cout << "Error: could not create USS file: '" << file_path << "' rc: '" << rc << "'" << endl;
    cout << "  Details:\n"
         << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "USS file '" << file_path << "' created" << endl;

  return rc;
}

int handle_uss_create_dir(ZCLIResult result)
{
  int rc = 0;
  string file_path = result.get_positional("file-path").get_value();
  string mode(result.get_option("--mode").get_value());
  if (mode == "")
    mode = "755";

  ZUSF zusf = {0};
  rc = zusf_create_uss_file_or_dir(&zusf, file_path, mode, true);
  if (0 != rc)
  {
    cout << "Error: could not create USS directory: '" << file_path << "' rc: '" << rc << "'" << endl;
    cout << "  Details:\n"
         << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "USS directory '" << file_path << "' created" << endl;

  return rc;
}

int handle_uss_list(ZCLIResult result)
{
  int rc = 0;
  string uss_file = result.get_positional("file-path").get_value();

  ZUSF zusf = {0};
  string response;
  rc = zusf_list_uss_file_path(&zusf, uss_file, response);
  if (0 != rc)
  {
    cout << "Error: could not list USS files: '" << uss_file << "' rc: '" << rc << "'" << endl;
    cout << "  Details:\n"
         << zusf.diag.e_msg << endl
         << response << endl;
    return RTNCD_FAILURE;
  }

  cout << response;

  return rc;
}

int handle_uss_view(ZCLIResult result)
{
  int rc = 0;
  string uss_file = result.get_positional("file-path").get_value();

  ZUSF zusf = {0};
  const auto hasEncoding = result.get_option("--encoding").is_found() && zut_prepare_encoding(result.get_option("--encoding").get_value(), &zusf.encoding_opts);

  string response;
  rc = zusf_read_from_uss_file(&zusf, uss_file, response);
  if (0 != rc)
  {
    cout << "Error: could not view USS file: '" << uss_file << "' rc: '" << rc << "'" << endl;
    cout << "  Details:\n"
         << zusf.diag.e_msg << endl
         << response << endl;
    return RTNCD_FAILURE;
  }

  if (hasEncoding && result.get_option("--response-format-bytes").get_value() == "true")
  {
    zut_print_string_as_bytes(response);
  }
  else
  {
    cout << response << endl;
  }

  return rc;
}

int handle_uss_write(ZCLIResult result)
{
  int rc = 0;
  string file = result.get_positional("file-path").get_value();
  ZUSF zusf = {0};

  string data;
  string line;
  size_t byteSize = 0ul;

  // Use Ctrl/Cmd + D to stop writing data manually
  const auto hasEncoding = result.get_option("--encoding").is_found() && zut_prepare_encoding(result.get_option("--encoding").get_value(), &zusf.encoding_opts);
  if (hasEncoding)
  {
    std::istreambuf_iterator<char> begin(std::cin);
    std::istreambuf_iterator<char> end;

    std::vector<char> bytes(begin, end);
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

  rc = zusf_write_to_uss_file(&zusf, file, data);
  if (0 != rc)
  {
    cout << "Error: could not write to USS file: '" << file << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }
  cout << "Wrote data to '" << file << "'" << endl;

  return rc;
}

int handle_uss_delete_file(ZCLIResult result)
{
  printf("method not implemented\n");
  return 1;
}

int handle_uss_delete_dir(ZCLIResult result)
{
  printf("method not implemented\n");
  return 1;
}

int handle_uss_chmod(ZCLIResult result)
{
  int rc = 0;
  string file_path = result.get_positional("file-path").get_value();
  string mode(result.get_option("--mode").get_value());
  if (mode == "")
    mode = "755";

  ZUSF zusf = {0};
  rc = zusf_chmod_uss_file_or_dir(&zusf, file_path, mode);
  if (0 != rc)
  {
    cout << "Error: could not create USS path: '" << file_path << "' rc: '" << rc << "'" << endl;
    cout << "  Details:\n"
         << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "USS path '" << file_path << "' modified: '" << mode << "'" << endl;

  return rc;
}

int handle_uss_chown(ZCLIResult result)
{
  printf("method not implemented\n");
  return 1;
}

int handle_tso_issue(ZCLIResult result)
{
  int rc = 0;
  string command = result.get_positional("command").get_value();
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

int handle_tool_convert_dsect(ZCLIResult result)
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

  string adata_dsn(result.get_option("--adata-dsn").get_value());
  string chdr_dsn(result.get_option("--chdr-dsn").get_value());
  string sysprint(result.get_option("--sysprint").get_value());
  string sysout(result.get_option("--sysout").get_value());

  const char *user = getlogin();
  string struser(user);
  transform(struser.begin(), struser.end(), struser.begin(), ::tolower); // upper case

  if (!result.get_option("--sysprint").is_found())
    sysprint = "/tmp/" + struser + "_sysprint.txt";
  if (!result.get_option("--sysout").is_found())
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

  for (vector<string>::iterator it = dds.begin(); it != dds.end(); it++)
  {
    rc = zut_bpxwdyn(*it, &code, resp);

    if (0 != rc)
    {
      cout << "Error: bpxwdyn failed with '" << *it << "' rc: '" << rc << "'" << endl;
      cout << "  Details: " << resp << endl;
      return -1;
    }
  }

  rc = zut_convert_dsect();
  if (0 != rc)
  {
    cout << "Error: convert failed with rc: '" << rc << "'" << endl;
    cout << "  See '" << sysprint << "' and '" << sysout << "' for more details" << endl;
    return -1;
  }

  cout << "DSECT converted to '" << chdr_dsn << "'" << endl;
  cout << "Copy it via `cp \"//'" + chdr_dsn + "'\" <member>.h`" << endl;

  return rc;
}
