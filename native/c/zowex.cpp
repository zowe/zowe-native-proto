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
#include <unistd.h>
#include "zcn.hpp"
#include "zut.hpp"
#include "zcli.hpp"
#include "zjb.hpp"
#include "zds.hpp"
#include "zusf.hpp"
#include "ztso.hpp"
#include "zuttype.h"

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
int handle_job_submit_jcl(ZCLIResult);
int handle_job_submit_uss(ZCLIResult);
int handle_job_delete(ZCLIResult);
int handle_job_cancel(ZCLIResult);
int handle_job_hold(ZCLIResult);
int handle_job_release(ZCLIResult);

int handle_console_issue(ZCLIResult);

int handle_data_set_create_dsn(ZCLIResult);
int handle_data_set_create_dsn_vb(ZCLIResult);
int handle_data_set_create_dsn_adata(ZCLIResult);
int handle_data_set_create_dsn_loadlib(ZCLIResult);
int handle_data_set_restore(ZCLIResult);
int handle_data_set_view_dsn(ZCLIResult);
int handle_data_set_list(ZCLIResult);
int handle_data_set_list_members_dsn(ZCLIResult);
int handle_data_set_write_to_dsn(ZCLIResult);
int handle_data_set_delete_dsn(ZCLIResult);
int handle_data_set_create_member_dsn(ZCLIResult);

int handle_log_view(ZCLIResult);

int handle_tool_convert_dsect(ZCLIResult);
int handle_tool_dynalloc(ZCLIResult);
int handle_tool_display_symbol(ZCLIResult);
int handle_data_set_compress(ZCLIResult);
int handle_tool_search(ZCLIResult);
int handle_tool_amblist(ZCLIResult);
int handle_tool_run(ZCLIResult);

// TODO(Kelosky):
// help w/verbose examples
// add simple examples to help

int handle_uss_create_file(ZCLIResult);
int handle_uss_create_dir(ZCLIResult);
int handle_uss_list(ZCLIResult);
int handle_uss_view(ZCLIResult);
int handle_uss_write(ZCLIResult);
int handle_uss_delete(ZCLIResult);
int handle_uss_chmod(ZCLIResult);
int handle_uss_chown(ZCLIResult);
int handle_uss_chtag(ZCLIResult);
int handle_tso_issue(ZCLIResult);

int job_submit_common(ZCLIResult, string, string &, string);

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
  response_format_csv.set_is_bool(true);

  ZCLIOption response_format_bytes("response-format-bytes");
  response_format_bytes.set_description("returns the response as raw bytes");
  response_format_bytes.get_aliases().push_back("--rfb");
  response_format_bytes.set_required(false);
  response_format_bytes.set_is_bool(true);

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

  ZCLIOption encoding_option("encoding");
  encoding_option.get_aliases().push_back("--ec");
  encoding_option.set_description("return contents in given encoding");

  ZCLIOption etag("etag");
  etag.set_required(false);
  etag.set_description("Provide the e-tag for a write response to detect conflicts before save");

  ZCLIOption etag_only("etag-only");
  etag_only.set_required(false);
  etag_only.set_description("Only print the e-tag for a write response (when successful)");

  ZCLIOption return_etag("return-etag");
  return_etag.set_required(false);
  return_etag.set_description("Display the e-tag for a read response in addition to data");

  // data set attributes options
  ZCLIOption return_alcunit("alcunit");
  return_alcunit.set_required(false);
  return_alcunit.set_description("Allocation unit");

  ZCLIOption return_blksize("blksize");
  return_blksize.set_required(false);
  return_blksize.set_description("Block size");

  ZCLIOption return_dirblk("dirblk");
  return_dirblk.set_required(false);
  return_dirblk.set_description("Directory blocks");

  ZCLIOption return_dsorg("dsorg");
  return_dsorg.set_required(false);
  return_dsorg.set_description("Data set organization");

  ZCLIOption return_primary("primary");
  return_primary.set_required(false);
  return_primary.set_description("Primary space");

  ZCLIOption return_recfm("recfm");
  return_recfm.set_required(false);
  return_recfm.set_description("Record format");

  ZCLIOption return_lrecl("lrecl");
  return_lrecl.set_required(false);
  return_lrecl.set_description("Record length");

  ZCLIOption return_dataclass("dataclass");
  return_dataclass.set_required(false);
  return_dataclass.set_description("Data class");

  ZCLIOption return_unit("unit");
  return_unit.set_required(false);
  return_unit.set_description("Device type");

  ZCLIOption return_dsntype("dsntype");
  return_dsntype.set_required(false);
  return_dsntype.set_description("Data set type");

  ZCLIOption return_mgntclass("mgntclass");
  return_mgntclass.set_required(false);
  return_mgntclass.set_description("Management class");

  ZCLIOption return_dsname("dsname");
  return_dsname.set_required(false);
  return_dsname.set_description("Data set name");

  ZCLIOption return_avgblk("avgblk");
  return_avgblk.set_required(false);
  return_avgblk.set_description("Average block length");

  ZCLIOption return_secondary("secondary");
  return_secondary.set_required(false);
  return_secondary.set_description("Secondary space");

  ZCLIOption return_size("size");
  return_size.set_required(false);
  return_size.set_description("Size");

  ZCLIOption return_storclass("storclass");
  return_storclass.set_required(false);
  return_storclass.set_description("Storage class");

  ZCLIOption return_vol("vol");
  return_vol.set_required(false);
  return_vol.set_description("Volume serial");

  ZCLIOption pipe_path("pipe-path");
  pipe_path.set_required(false);
  pipe_path.set_description("Specify a FIFO pipe path for transferring binary data");

  //
  // data set group
  //
  ZCLIGroup data_set_group("data-set");
  data_set_group.get_aliases().push_back("ds");
  data_set_group.set_description("z/OS data set operations");

  ZCLIPositional data_set_dsn("dsn");
  data_set_dsn.set_description("data set name, optionally with member specified");
  data_set_dsn.set_required(true);

  // data set verbs
  ZCLIVerb data_set_create("create");
  data_set_create.get_aliases().push_back("cre");
  data_set_create.set_description("create data set");
  data_set_create.set_zcli_verb_handler(handle_data_set_create_dsn);
  data_set_create.get_options().push_back(return_alcunit);
  data_set_create.get_options().push_back(return_blksize);
  data_set_create.get_options().push_back(return_dirblk);
  data_set_create.get_options().push_back(return_dsorg);
  data_set_create.get_options().push_back(return_primary);
  data_set_create.get_options().push_back(return_recfm);
  data_set_create.get_options().push_back(return_lrecl);
  data_set_create.get_options().push_back(return_dataclass);
  data_set_create.get_options().push_back(return_unit);
  data_set_create.get_options().push_back(return_dsntype);
  data_set_create.get_options().push_back(return_mgntclass);
  data_set_create.get_options().push_back(return_dsname);
  data_set_create.get_options().push_back(return_avgblk);
  data_set_create.get_options().push_back(return_secondary);
  data_set_create.get_options().push_back(return_size);
  data_set_create.get_options().push_back(return_storclass);
  data_set_create.get_options().push_back(return_vol);
  data_set_create.get_positionals().push_back(data_set_dsn);
  data_set_group.get_verbs().push_back(data_set_create);

  ZCLIVerb data_set_create_vb("create-vb");
  data_set_create_vb.get_aliases().push_back("cre-vb");
  data_set_create_vb.set_description("create VB data set using defaults: DSORG=PO, RECFM=VB, LRECL=255");
  data_set_create_vb.set_zcli_verb_handler(handle_data_set_create_dsn_vb);
  data_set_create_vb.get_positionals().push_back(data_set_dsn);
  data_set_group.get_verbs().push_back(data_set_create_vb);

  ZCLIVerb data_set_create_adata("create-adata");
  data_set_create_adata.get_aliases().push_back("cre-a");
  data_set_create_adata.set_description("create VB data set using defaults: DSORG=PO, RECFM=VB, LRECL=32756");
  data_set_create_adata.set_zcli_verb_handler(handle_data_set_create_dsn_adata);
  data_set_create_adata.get_positionals().push_back(data_set_dsn);
  data_set_group.get_verbs().push_back(data_set_create_adata);

  ZCLIVerb data_set_create_loadlib("create-loadlib");
  data_set_create_loadlib.get_aliases().push_back("cre-u");
  data_set_create_loadlib.set_description("create loadlib data set using defaults: DSORG=PO, RECFM=U, LRECL=0");
  data_set_create_loadlib.set_zcli_verb_handler(handle_data_set_create_dsn_loadlib);
  data_set_create_loadlib.get_positionals().push_back(data_set_dsn);
  data_set_group.get_verbs().push_back(data_set_create_loadlib);

  ZCLIVerb data_set_create_member("create-member");
  data_set_create_member.get_aliases().push_back("cre-m");
  data_set_create_member.set_description("create member in data set");
  data_set_create_member.set_zcli_verb_handler(handle_data_set_create_member_dsn);
  data_set_create_member.get_positionals().push_back(data_set_dsn);
  data_set_group.get_verbs().push_back(data_set_create_member);

  ZCLIVerb data_set_restore("restore");
  data_set_restore.set_description("restore/recall data set");
  data_set_restore.set_zcli_verb_handler(handle_data_set_restore);
  data_set_restore.get_positionals().push_back(data_set_dsn);
  data_set_group.get_verbs().push_back(data_set_restore);

  ZCLIVerb data_set_view("view");
  data_set_view.set_description("view data set");
  data_set_view.set_zcli_verb_handler(handle_data_set_view_dsn);
  data_set_view.get_positionals().push_back(data_set_dsn);
  data_set_view.get_options().push_back(encoding_option);
  data_set_view.get_options().push_back(response_format_bytes);
  data_set_view.get_options().push_back(return_etag);
  data_set_view.get_options().push_back(pipe_path);
  data_set_group.get_verbs().push_back(data_set_view);

  ZCLIVerb data_set_list("list");
  ZCLIOption data_set_max_entries("max-entries");
  data_set_max_entries.get_aliases().push_back("--me");
  data_set_max_entries.set_description("max number of results to return before warning generated");
  data_set_list.get_options().push_back(data_set_max_entries);

  ZCLIOption data_set_truncate_warn("warn");
  data_set_truncate_warn.set_description("warn if truncated or not found");
  data_set_truncate_warn.set_default("true");
  data_set_truncate_warn.set_is_bool(true);
  data_set_list.get_options().push_back(data_set_truncate_warn);

  data_set_list.set_description("list data sets");
  data_set_list.get_aliases().push_back("ls");
  ZCLIOption data_set_attributes("attributes");
  data_set_attributes.set_description("display data set attributes");
  data_set_attributes.get_aliases().push_back("-a");
  data_set_attributes.set_default("false");
  data_set_list.get_options().push_back(data_set_attributes);
  data_set_list.set_zcli_verb_handler(handle_data_set_list);
  data_set_list.get_positionals().push_back(data_set_dsn);
  data_set_list.get_options().push_back(response_format_csv);
  data_set_group.get_verbs().push_back(data_set_list);

  ZCLIVerb data_set_list_members("list-members");
  data_set_list_members.get_aliases().push_back("lm");
  data_set_list_members.set_description("list data set members");
  data_set_list_members.set_zcli_verb_handler(handle_data_set_list_members_dsn);
  data_set_list_members.get_positionals().push_back(data_set_dsn);
  data_set_list_members.get_options().push_back(data_set_max_entries);
  data_set_list_members.get_options().push_back(data_set_truncate_warn);
  data_set_group.get_verbs().push_back(data_set_list_members);

  ZCLIVerb data_set_write("write");
  data_set_write.set_description("write to data set");
  data_set_write.set_zcli_verb_handler(handle_data_set_write_to_dsn);
  data_set_write.get_positionals().push_back(data_set_dsn);
  data_set_write.get_options().push_back(encoding_option);
  data_set_write.get_options().push_back(etag);
  data_set_write.get_options().push_back(etag_only);
  data_set_write.get_options().push_back(pipe_path);
  data_set_group.get_verbs().push_back(data_set_write);

  ZCLIVerb data_set_compress("compress");
  data_set_compress.set_description("compress data set");
  data_set_compress.set_zcli_verb_handler(handle_data_set_compress);
  ZCLIPositional compress_dsn("dsn");
  compress_dsn.set_description("data set to compress");
  compress_dsn.set_required(true);
  data_set_compress.get_positionals().push_back(compress_dsn);
  data_set_group.get_verbs().push_back(data_set_compress);

  ZCLIVerb data_set_delete("delete");
  data_set_delete.get_aliases().push_back("del");
  data_set_delete.set_description("delete data set");
  data_set_delete.set_zcli_verb_handler(handle_data_set_delete_dsn);
  data_set_delete.get_positionals().push_back(data_set_dsn);
  data_set_group.get_verbs().push_back(data_set_delete);

  //
  // jobs group
  //
  ZCLIGroup job_group("job");
  job_group.set_description("z/OS job operations");

  // jobs verbs
  ZCLIVerb job_list("list");
  job_list.set_description("list jobs");
  job_list.set_zcli_verb_handler(handle_job_list);
  ZCLIOption job_owner("owner");
  job_owner.get_aliases().push_back("-o");
  job_owner.set_description("filter by owner");
  job_list.get_options().push_back(job_owner);
  ZCLIOption job_prefix("prefix");
  job_prefix.get_aliases().push_back("-p");
  job_prefix.set_description("filter by prefix");
  job_list.get_options().push_back(job_prefix);

  ZCLIOption job_max_entries("max-entries");
  job_max_entries.get_aliases().push_back("--me");
  job_max_entries.set_description("max number of results to return before warning generated");
  job_list.get_options().push_back(job_max_entries);

  ZCLIOption job_truncate_warn("warn");
  job_truncate_warn.set_description("warn if trucated or not found");
  job_truncate_warn.set_default("true");
  job_truncate_warn.set_is_bool(true);
  job_list.get_options().push_back(job_truncate_warn);

  job_list.get_options().push_back(response_format_csv);
  job_group.get_verbs().push_back(job_list);

  ZCLIVerb job_list_files("list-files");
  job_list_files.get_aliases().push_back("lf");
  job_list_files.set_description("list spool files for jobid");
  job_list_files.set_zcli_verb_handler(handle_job_list_files);
  ZCLIPositional job_jobid("jobid");
  job_jobid.set_required(true);
  job_jobid.set_description("valid jobid or job correlator");
  job_list_files.get_positionals().push_back(job_jobid);
  job_list_files.get_options().push_back(response_format_csv);
  ZCLIOption job_list_files_max_entries("max-entries");
  job_list_files_max_entries.get_aliases().push_back("--me");
  job_list_files_max_entries.set_description("max number of files to return before warning generated");
  ZCLIOption job_list_files_warn("warn");
  job_list_files_warn.set_description("warn if trucated or not found");
  job_list_files_warn.set_default("true");
  job_list_files_warn.set_is_bool(true);
  job_list_files.get_options().push_back(job_list_files_warn);
  job_list_files.get_options().push_back(job_list_files_max_entries);

  job_group.get_verbs().push_back(job_list_files);

  ZCLIVerb job_view_status("view-status");
  job_view_status.get_aliases().push_back("vs");
  job_view_status.set_description("view job status");
  job_view_status.set_zcli_verb_handler(handle_job_view_status);
  job_view_status.get_positionals().push_back(job_jobid);
  job_view_status.get_options().push_back(response_format_csv);
  job_group.get_verbs().push_back(job_view_status);

  ZCLIVerb job_view_file("view-file");
  job_view_file.get_aliases().push_back("vf");
  job_view_file.set_description("view job file output");
  job_view_file.set_zcli_verb_handler(handle_job_view_file);
  job_view_file.get_positionals().push_back(job_jobid);
  job_view_file.get_options().push_back(encoding_option);
  job_view_file.get_options().push_back(response_format_bytes);

  ZCLIPositional job_dsn_key("key");
  job_dsn_key.set_required(true);
  job_dsn_key.set_description("valid job dsn key via 'job list-files'");
  job_view_file.get_positionals().push_back(job_dsn_key);
  job_group.get_verbs().push_back(job_view_file);

  ZCLIVerb job_view_jcl("view-jcl");
  job_view_jcl.get_aliases().push_back("vj");
  job_view_jcl.set_description("view job jcl from input jobid");
  job_view_jcl.set_zcli_verb_handler(handle_job_view_jcl);
  job_view_jcl.get_positionals().push_back(job_jobid);
  job_group.get_verbs().push_back(job_view_jcl);

  ZCLIVerb job_submit("submit");
  job_submit.get_aliases().push_back("sub");
  job_submit.set_description("submit a job");
  job_submit.set_zcli_verb_handler(handle_job_submit);

  ZCLIOption job_wait("wait");
  job_wait.set_description("wait for job status");
  job_submit.get_options().push_back(job_wait);

  ZCLIOption job_jobid_only("only-jobid");
  job_jobid_only.get_aliases().push_back("--oj");
  job_jobid_only.set_description("show only job id on success");
  job_jobid_only.set_is_bool(true);
  job_submit.get_options().push_back(job_jobid_only);

  ZCLIOption job_job_correlator_only("only-correlator");
  job_job_correlator_only.get_aliases().push_back("--oc");
  job_job_correlator_only.set_description("show only job correlator on success");
  job_job_correlator_only.set_is_bool(true);
  job_submit.get_options().push_back(job_job_correlator_only);

  job_submit.get_exclusive_options().push_back(job_jobid_only);
  job_submit.get_exclusive_options().push_back(job_job_correlator_only);

  ZCLIPositional job_dsn("dsn");
  job_dsn.set_required(true);
  job_dsn.set_description("dsn containing JCL");
  job_submit.get_positionals().push_back(job_dsn);
  job_group.get_verbs().push_back(job_submit);

  ZCLIVerb job_submit_jcl("submit-jcl");
  job_submit_jcl.get_aliases().push_back("subj");
  job_submit_jcl.set_description("submit JCL contents directly");
  job_submit_jcl.set_zcli_verb_handler(handle_job_submit_jcl);
  job_submit_jcl.get_options().push_back(job_jobid_only);
  job_submit_jcl.get_options().push_back(encoding_option);
  job_submit_jcl.get_options().push_back(job_job_correlator_only);
  job_submit_jcl.get_exclusive_options().push_back(job_jobid_only);
  job_submit_jcl.get_exclusive_options().push_back(job_job_correlator_only);
  job_submit_jcl.get_options().push_back(job_wait);
  job_group.get_verbs().push_back(job_submit_jcl);

  ZCLIVerb job_submit_uss("submit-uss");
  job_submit_uss.get_aliases().push_back("sub-u");
  job_submit_uss.set_description("submit a job from USS files");
  job_submit_uss.set_zcli_verb_handler(handle_job_submit_uss);
  ZCLIPositional job_uss_file("file-path");
  job_uss_file.set_required(true);
  job_uss_file.set_description("USS file containing JCL");
  job_submit_uss.get_positionals().push_back(job_uss_file);

  job_submit_uss.get_options().push_back(job_jobid_only);
  job_submit_uss.get_options().push_back(job_job_correlator_only);
  job_submit_uss.get_exclusive_options().push_back(job_jobid_only);
  job_submit_uss.get_exclusive_options().push_back(job_job_correlator_only);
  job_submit_uss.get_options().push_back(job_wait);
  job_group.get_verbs().push_back(job_submit_uss);

  ZCLIVerb job_delete("delete");
  job_delete.get_aliases().push_back("del");
  job_delete.set_description("delete a job");
  job_delete.set_zcli_verb_handler(handle_job_delete);
  job_delete.get_positionals().push_back(job_jobid);
  job_group.get_verbs().push_back(job_delete);

  ZCLIVerb job_cancel("cancel");
  job_cancel.get_aliases().push_back("cnl");
  job_cancel.set_description("cancel a job");
  job_cancel.set_zcli_verb_handler(handle_job_cancel);
  job_cancel.get_positionals().push_back(job_jobid);

  ZCLIOption job_cancel_dump("dump");
  job_cancel_dump.get_aliases().push_back("-d");
  job_cancel_dump.set_description("Dump the cancelled jobs if waiting for conversion, in conversion, or in execution.");
  job_cancel.get_options().push_back(job_cancel_dump);
  ZCLIOption job_cancel_force("force");
  job_cancel_force.get_aliases().push_back("-f");
  job_cancel_force.set_description("Force cancel the jobs, even if marked.");
  job_cancel.get_options().push_back(job_cancel_force);
  ZCLIOption job_cancel_purge("purge");
  job_cancel_purge.get_aliases().push_back("-p");
  job_cancel_purge.set_description("Purge output of the cancelled jobs.");
  job_cancel.get_options().push_back(job_cancel_purge);
  ZCLIOption job_cancel_restart("restart");
  job_cancel_restart.get_aliases().push_back("-r");
  job_cancel_restart.set_description("Request that automatic restart management automatically restart the selected jobs after they are cancelled.");
  job_cancel.get_options().push_back(job_cancel_restart);
  job_group.get_verbs().push_back(job_cancel);

  ZCLIVerb job_hold("hold");
  job_hold.get_aliases().push_back("hld");
  job_hold.set_description("hold a job");
  job_hold.set_zcli_verb_handler(handle_job_hold);
  job_hold.get_positionals().push_back(job_jobid);
  job_group.get_verbs().push_back(job_hold);

  ZCLIVerb job_release("release");
  job_release.get_aliases().push_back("rel");
  job_release.set_description("release a job");
  job_release.set_zcli_verb_handler(handle_job_release);
  job_release.get_positionals().push_back(job_jobid);
  job_group.get_verbs().push_back(job_release);

  //
  // console group
  //
  ZCLIGroup console_group("console");
  console_group.get_aliases().push_back("cn");
  console_group.set_description("z/OS console operations");

  // console verbs
  ZCLIVerb console_issue("issue");
  console_issue.set_description("issue a console command");
  console_issue.set_zcli_verb_handler(handle_console_issue);
  ZCLIOption console_name("console-name");
  console_name.set_default("zowex");
  console_name.set_required(true);
  console_name.get_aliases().push_back("--cn");
  console_name.set_description("extended console name");
  console_issue.get_options().push_back(console_name);
  ZCLIOption console_wait("wait");
  console_wait.set_default("true");
  console_wait.set_is_bool(true);
  console_wait.set_description("wait for responses");
  console_issue.get_options().push_back(console_wait);
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
  uss_recursive.get_aliases().push_back("-r");
  uss_recursive.set_required(false);
  uss_recursive.set_description("Applies the operation recursively (e.g. for folders w/ inner files)");

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

  ZCLIVerb uss_view("view");
  uss_view.set_description("view a USS file");
  uss_view.get_positionals().push_back(uss_file_path);
  uss_view.set_zcli_verb_handler(handle_uss_view);
  uss_view.get_options().push_back(encoding_option);
  uss_view.get_options().push_back(response_format_bytes);
  uss_view.get_options().push_back(return_etag);
  uss_view.get_options().push_back(pipe_path);
  uss_group.get_verbs().push_back(uss_view);

  ZCLIVerb uss_write("write");
  uss_write.set_description("write to a USS file");
  uss_write.set_zcli_verb_handler(handle_uss_write);
  uss_write.get_positionals().push_back(uss_file_path);
  uss_write.get_options().push_back(encoding_option);
  uss_write.get_options().push_back(etag);
  uss_write.get_options().push_back(etag_only);
  uss_write.get_options().push_back(pipe_path);
  uss_group.get_verbs().push_back(uss_write);

  ZCLIVerb uss_delete("delete");
  uss_delete.set_description("delete a USS item");
  uss_delete.set_zcli_verb_handler(handle_uss_delete);
  uss_delete.get_positionals().push_back(uss_file_path);
  uss_delete.get_options().push_back(uss_recursive);
  uss_group.get_verbs().push_back(uss_delete);

  ZCLIPositional uss_owner("owner");
  uss_owner.set_required(true);
  uss_owner.set_description("New owner (or owner:group) for the file or directory");

  ZCLIPositional uss_mode_positional("mode");
  uss_mode_positional.set_required(true);
  uss_mode_positional.set_description("new permissions for the file or directory");

  ZCLIVerb uss_chmod("chmod");
  uss_chmod.set_description("change permissions on a USS file or directory");
  uss_chmod.set_zcli_verb_handler(handle_uss_chmod);
  uss_chmod.get_positionals().push_back(uss_mode_positional);
  uss_chmod.get_positionals().push_back(uss_file_path);
  uss_chmod.get_options().push_back(uss_recursive);
  uss_group.get_verbs().push_back(uss_chmod);

  ZCLIVerb uss_chown("chown");
  uss_chown.set_description("change owner on a USS file or directory");
  uss_chown.set_zcli_verb_handler(handle_uss_chown);
  uss_chown.get_positionals().push_back(uss_owner);
  uss_chown.get_positionals().push_back(uss_file_path);
  uss_chown.get_options().push_back(uss_recursive);
  uss_group.get_verbs().push_back(uss_chown);

  ZCLIPositional uss_tag("tag");
  uss_tag.set_required(true);
  uss_tag.set_description("new tag for the file");

  ZCLIVerb uss_chtag("chtag");
  uss_chtag.set_description("change tags on a USS file");
  uss_chtag.set_zcli_verb_handler(handle_uss_chtag);
  uss_chtag.get_positionals().push_back(uss_file_path);
  uss_chtag.get_positionals().push_back(uss_tag);
  uss_chtag.get_options().push_back(uss_recursive);
  uss_group.get_verbs().push_back(uss_chtag);

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

  ZCLIVerb tool_display_symbol("display-symbol");
  tool_display_symbol.set_description("display system symbol");
  tool_display_symbol.set_zcli_verb_handler(handle_tool_display_symbol);
  ZCLIPositional symbol_value("symbol");
  symbol_value.set_description("symbol to display");
  symbol_value.set_required(true);
  tool_display_symbol.get_positionals().push_back(symbol_value);
  tool_group.get_verbs().push_back(tool_display_symbol);

  ZCLIVerb tool_search("search");
  tool_search.set_description("search members for string");
  tool_search.set_zcli_verb_handler(handle_tool_search);
  ZCLIPositional search_dsn("dsn");
  search_dsn.set_description("data set to search");
  search_dsn.set_required(true);
  tool_search.get_positionals().push_back(search_dsn);
  ZCLIPositional search_value("string");
  search_value.set_description("string to search for");
  search_value.set_required(true);
  tool_search.get_positionals().push_back(search_value);

  tool_search.get_options().push_back(data_set_max_entries);
  tool_search.get_options().push_back(data_set_truncate_warn);

  tool_group.get_verbs().push_back(tool_search);

  ZCLIVerb tool_amblist("amblist");
  tool_amblist.set_description("invoke amblist");
  tool_amblist.set_zcli_verb_handler(handle_tool_amblist);
  ZCLIPositional amblist_dsn("dsn");
  amblist_dsn.set_description("data containing input load modules");
  amblist_dsn.set_required(true);
  tool_amblist.get_positionals().push_back(amblist_dsn);
  ZCLIOption ablist_control("control-statements");
  ablist_control.set_description("amblist control statements, e.g. listload output=map,member=testprog");
  ablist_control.set_required(true);
  ablist_control.get_aliases().push_back("--cs");
  tool_amblist.get_options().push_back(ablist_control);
  tool_group.get_verbs().push_back(tool_amblist);

  ZCLIVerb tool_run("run");

  ZCLIOption dynalloc_pre("dynalloc-pre");
  dynalloc_pre.set_description("dynalloc pre run statements");
  dynalloc_pre.get_aliases().push_back("--dp");
  tool_run.get_options().push_back(dynalloc_pre);

  ZCLIOption dynalloc_post("dynalloc-post");
  dynalloc_post.set_description("dynalloc post run statements");
  dynalloc_post.get_aliases().push_back("--dt");
  tool_run.get_options().push_back(dynalloc_post);

  ZCLIOption indd("in-dd");
  indd.set_description("input ddname");
  indd.get_aliases().push_back("--idd");
  tool_run.get_options().push_back(indd);

  ZCLIOption input("input");
  input.set_description("input");
  input.get_aliases().push_back("--in");
  tool_run.get_options().push_back(input);

  ZCLIOption outdd("out-dd");
  outdd.set_description("output ddname");
  outdd.get_aliases().push_back("--odd");
  tool_run.get_options().push_back(outdd);

  tool_run.set_description("run a program");
  tool_run.set_zcli_verb_handler(handle_tool_run);
  ZCLIPositional run_name("program");
  run_name.set_description("name of program to run");
  run_name.set_required(true);
  tool_run.get_positionals().push_back(run_name);
  tool_group.get_verbs().push_back(tool_run);

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

int handle_job_list(ZCLIResult result)
{
  int rc = 0;
  ZJB zjb = {0};
  string owner_name(result.get_option_value("--owner"));
  string prefix_name(result.get_option_value("--prefix"));
  string max_entries = result.get_option_value("--max-entries");
  string warn = result.get_option_value("--warn");

  if (max_entries.size() > 0)
  {
    zjb.jobs_max = atoi(max_entries.c_str());
  }

  vector<ZJob> jobs;
  rc = zjb_list_by_owner(&zjb, owner_name, prefix_name, jobs);

  if (RTNCD_SUCCESS == rc || RTNCD_WARNING == rc)
  {
    const auto emit_csv = result.get_option_value("--response-format-csv") == "true";
    for (vector<ZJob>::iterator it = jobs.begin(); it != jobs.end(); it++)
    {
      if (emit_csv)
      {
        vector<string> fields;
        fields.push_back(it->jobid);
        fields.push_back(it->retcode);
        fields.push_back(it->jobname);
        fields.push_back(it->status);
        fields.push_back(it->job_correlator);
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

int handle_job_list_files(ZCLIResult result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid(result.get_positional("jobid")->get_value());

  string max_entries = result.get_option_value("--max-entries");
  string warn = result.get_option_value("--warn");

  if (max_entries.size() > 0)
  {
    zjb.dds_max = atoi(max_entries.c_str());
  }

  vector<ZJobDD> job_dds;
  rc = zjb_list_dds(&zjb, jobid, job_dds);
  if (RTNCD_SUCCESS == rc || RTNCD_WARNING == rc)
  {
    const auto emit_csv = result.get_option_value("--response-format-csv") == "true";
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
    if ("true" == warn)
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

  return "false" == warn && rc == RTNCD_WARNING ? RTNCD_SUCCESS : rc;
}

int handle_job_view_status(ZCLIResult result)
{
  int rc = 0;
  ZJB zjb = {0};
  ZJob job = {0};
  string jobid(result.get_positional("jobid")->get_value());

  const auto emit_csv = result.get_option_value("--response-format-csv") == "true";

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
    fields.push_back(job.job_correlator);
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
  string jobid(result.get_positional("jobid")->get_value());
  string key(result.get_positional("key")->get_value());

  const auto hasEncoding = zut_prepare_encoding(result.get_option_value("--encoding"), &zjb.encoding_opts);

  string resp;
  rc = zjb_read_jobs_output_by_key(&zjb, jobid, atoi(key.c_str()), resp);

  if (0 != rc)
  {
    cerr << "Error: could not view job file for: '" << jobid << "' with key '" << key << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  if (hasEncoding && result.get_option_value("--response-format-bytes") == "true")
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
  string jobid(result.get_positional("jobid")->get_value());

  string resp;
  rc = zjb_read_job_jcl(&zjb, jobid, resp);

  if (0 != rc)
  {
    cerr << "Error: could not view job file for: '" << jobid << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zjb.diag.e_msg << endl;
    return -1;
  }

  cout << resp;

  return 0;
}

int job_submit_common(ZCLIResult result, string jcl, string &jobid, string identifier)
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

  string only_jobid(result.get_option_value("--only-jobid"));
  string only_correlator(result.get_option_value("--only-correlator"));
  string wait(result.get_option_value("--wait"));
  transform(wait.begin(), wait.end(), wait.begin(), ::toupper);

  if ("true" == only_jobid)
    cout << jobid << endl;
  else if ("true" == only_correlator)
    cout << string(zjb.job_correlator, sizeof(zjb.job_correlator)) << endl;
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

int handle_job_submit(ZCLIResult result)
{
  int rc = 0;
  ZJB zjb = {0};
  string dsn(result.get_positional("dsn")->get_value());
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

int handle_job_submit_uss(ZCLIResult result)
{
  int rc = 0;
  ZJB zjb = {0};
  string file(result.get_positional("file-path")->get_value());

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

int handle_job_submit_jcl(ZCLIResult result)
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
  const auto encoding_prepared = result.get_option("--encoding") != nullptr && zut_prepare_encoding(result.get_option_value("--encoding"), &encoding_opts);

  if (encoding_prepared && encoding_opts.data_type != eDataTypeBinary)
  {
    data = zut_encode(data, "UTF-8", string(encoding_opts.codepage), zjb.diag);
  }

  string jobid;
  rc = zjb_submit(&zjb, data, jobid);

  return job_submit_common(result, data, jobid, data);
}

int handle_job_delete(ZCLIResult result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid(result.get_positional("jobid")->get_value());

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

int handle_job_cancel(ZCLIResult result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid(result.get_positional("jobid")->get_value());

  string option_dump(result.get_option_value("--dump"));
  string option_force(result.get_option_value("--force"));
  string option_purge(result.get_option_value("--purge"));
  string option_restart(result.get_option_value("--restart"));

  rc = zjb_cancel(&zjb, jobid);

  if (0 != rc)
  {
    cout << "Error: could not cancel job: '" << jobid << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "Job " << jobid << " cancelled " << endl;

  return RTNCD_SUCCESS;
}

int handle_job_hold(ZCLIResult result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid(result.get_positional("jobid")->get_value());

  rc = zjb_hold(&zjb, jobid);

  if (0 != rc)
  {
    cout << "Error: could not hold job: '" << jobid << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "Job " << jobid << " held " << endl;

  return RTNCD_SUCCESS;
}

int handle_job_release(ZCLIResult result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid(result.get_positional("jobid")->get_value());

  rc = zjb_release(&zjb, jobid);

  if (0 != rc)
  {
    cout << "Error: could not release job: '" << jobid << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "Job " << jobid << " released " << endl;

  return RTNCD_SUCCESS;
}

int handle_console_issue(ZCLIResult result)
{
  int rc = 0;
  ZCN zcn = {0};

  string console_name(result.get_option_value("--console-name"));
  string command(result.get_positional("command")->get_value());
  string wait = result.get_option_value("--wait");

  rc = zcn_activate(&zcn, string(console_name));
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

  if ("true" == wait)
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

int handle_data_set_create_member_dsn(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn")->get_value();
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
  if (RTNCD_WARNING < rc || entries.size() == 0)
  {
    cout << "Error: could not create data set member: '" << dsn << "' rc: '" << rc << "'" << endl;
    cout << "  Details:\n"
         << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }
  return handle_data_set_create_member(zds, dsn + "(" + member_name + ")");
}

int handle_data_set_create_dsn(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn")->get_value();
  ZDS zds = {0};
  DS_ATTRIBUTES attributes = {0};

  if (result.get_option("--alcunit"))
  {
    attributes.alcunit = result.get_option("--alcunit")->get_value();
  }
  if (result.get_option("--blksize"))
  {
    attributes.blksize = std::strtoul(result.get_option("--blksize")->get_value().c_str(), nullptr, 10);
  }
  if (result.get_option("--dirblk"))
  {
    attributes.dirblk = std::strtoul(result.get_option("--dirblk")->get_value().c_str(), nullptr, 10);
  }
  if (result.get_option("--dsorg"))
  {
    attributes.dsorg = result.get_option("--dsorg")->get_value();
  }
  if (result.get_option("--primary"))
  {
    attributes.primary = std::strtoul(result.get_option("--primary")->get_value().c_str(), nullptr, 10);
  }
  if (result.get_option("--recfm"))
  {
    attributes.recfm = result.get_option("--recfm")->get_value();
  }
  if (result.get_option("--lrecl"))
  {
    attributes.lrecl = std::strtoul(result.get_option("--lrecl")->get_value().c_str(), nullptr, 10);
  }
  if (result.get_option("--dataclass"))
  {
    attributes.dataclass = result.get_option("--dataclass")->get_value();
  }
  if (result.get_option("--unit"))
  {
    attributes.unit = result.get_option("--unit")->get_value();
  }
  if (result.get_option("--dsntype"))
  {
    attributes.dsntype = result.get_option("--dsntype")->get_value();
  }
  if (result.get_option("--mgntclass"))
  {
    attributes.mgntclass = result.get_option("--mgntclass")->get_value();
  }
  if (result.get_option("--dsname"))
  {
    attributes.dsname = result.get_option("--dsname")->get_value();
  }
  if (result.get_option("--avgblk"))
  {
    attributes.avgblk = std::strtoul(result.get_option("--avgblk")->get_value().c_str(), nullptr, 10);
  }
  if (result.get_option("--secondary"))
  {
    attributes.secondary = std::strtoul(result.get_option("--secondary")->get_value().c_str(), nullptr, 10);
  }
  if (result.get_option("--size"))
  {
    attributes.size = std::strtoul(result.get_option("--size")->get_value().c_str(), nullptr, 10);
  }
  if (result.get_option("--storclass"))
  {
    attributes.storclass = result.get_option("--storclass")->get_value();
  }
  if (result.get_option("--vol"))
  {
    attributes.vol = result.get_option("--vol")->get_value();
  }

  string response;
  rc = zds_create_dsn(&zds, dsn, attributes, response);
  if (0 != rc)
  {
    cerr << "Error: could not create data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << response << endl;
    return RTNCD_FAILURE;
  }
  return handle_data_set_create_member(zds, dsn);
}

int handle_data_set_create_dsn_vb(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn")->get_value();
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

int handle_data_set_create_dsn_adata(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn")->get_value();
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

int handle_data_set_create_dsn_loadlib(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn")->get_value();
  ZDS zds = {0};
  string response;
  rc = zds_create_dsn_loadlib(&zds, dsn, response);
  if (0 != rc)
  {
    cerr << "Error: could not create data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << response << endl;
    return -1;
  }
  return handle_data_set_create_member(zds, dsn);
}

int handle_data_set_restore(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn")->get_value();
  ZDS zds = {0};
  string response;
  unsigned int code = 0;

  // perform dynalloc
  vector<string> dds;
  dds.push_back("alloc da('" + dsn + "') shr");
  dds.push_back("free da('" + dsn + "')");

  rc = loop_dynalloc(dds);
  if (RTNCD_SUCCESS != rc)
  {
    return RTNCD_FAILURE;
  }

  cout << "Data set '" << dsn << "' restored" << endl;

  return rc;
}

int handle_data_set_view_dsn(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn")->get_value();
  ZDS zds = {0};

  const auto hasEncoding = zut_prepare_encoding(result.get_option_value("--encoding"), &zds.encoding_opts);

  auto *pipe_path = result.get_option("--pipe-path");
  if (pipe_path != nullptr && pipe_path->is_found())
  {
    rc = zds_read_from_dsn_streamed(&zds, dsn, pipe_path->get_value());

    if (result.get_option_value("--return-etag") == "true")
    {
      string temp_content;
      // Temporary read to get etag value
      auto read_rc = zds_read_from_dsn(&zds, dsn, temp_content);
      if (read_rc == 0)
      {
        const auto etag = zut_calc_adler32_checksum(temp_content);
        cout << std::hex << etag << endl;
      }
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

    if (result.get_option_value("--return-etag") == "true")
    {
      const auto etag = zut_calc_adler32_checksum(response);
      cout << "etag: " << std::hex << etag << endl;
      cout << "data: ";
    }
    if (hasEncoding && result.get_option_value("--response-format-bytes") == "true")
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

int handle_data_set_list(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn")->get_value();

  if (dsn.length() > MAX_DS_LENGTH)
  {
    cerr << "Error: data set pattern exceeds 44 character length limit" << endl;
    return RTNCD_FAILURE;
  }

  dsn += ".**";

  string max_entries = result.get_option_value("--max-entries");
  string warn = result.get_option_value("--warn");
  string attributes = result.get_option_value("--attributes");

  ZDS zds = {0};
  if (max_entries.size() > 0)
  {
    zds.max_entries = atoi(max_entries.c_str());
  }
  vector<ZDSEntry> entries;

  const auto emit_csv = result.get_option_value("--response-format-csv") == "true";
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
        fields.push_back(it->recfm);
        std::cout << zut_format_as_csv(fields) << std::endl;
        fields.clear();
      }
      else
      {
        if ("true" == attributes)
        {
          std::cout << left << setw(44) << it->name << " " << it->volser << " " << setw(4) << it->dsorg << " " << setw(6) << it->recfm << endl;
        }
        else
        {
          std::cout << left << setw(44) << it->name << endl;
        }
      }
    }
  }
  if (RTNCD_WARNING == rc)
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
  }

  if (RTNCD_SUCCESS != rc && RTNCD_WARNING != rc)
  {
    cerr << "Error: could not list data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  return warn == "false" && rc == RTNCD_WARNING ? RTNCD_SUCCESS : rc;
}

int handle_data_set_list_members_dsn(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn")->get_value();
  string max_entries = result.get_option_value("--max-entries");
  string warn = result.get_option_value("--warn");
  ZDS zds = {0};
  if (max_entries.size() > 0)
  {
    zds.max_entries = atoi(max_entries.c_str());
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
    if ("true" == warn)
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

  return rc;
}

int handle_data_set_write_to_dsn(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn")->get_value();
  ZDS zds = {0};
  if (result.get_option("--encoding") != nullptr)
  {
    zut_prepare_encoding(result.get_option_value("--encoding"), &zds.encoding_opts);
  }

  auto *etag_opt = result.get_option("--etag");
  if (etag_opt != nullptr && etag_opt->is_found())
  {
    strcpy(zds.etag, etag_opt->get_value().c_str());
  }

  auto *pipe_path = result.get_option("--pipe-path");
  if (pipe_path != nullptr && pipe_path->is_found())
  {
    rc = zds_write_to_dsn_streamed(&zds, dsn, pipe_path->get_value());
  }
  else
  {
    string data;
    string line;
    size_t byteSize = 0ul;

    if (!isatty(fileno(stdout)))
    {
      std::istreambuf_iterator<char> begin(std::cin);
      std::istreambuf_iterator<char> end;

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

    rc = zds_write_to_dsn(&zds, dsn, data);
  }

  if (0 != rc)
  {
    cerr << "Error: could not write to data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  auto *etag_opt2 = result.get_option("--etag-only");
  if (etag_opt2 != nullptr && etag_opt2->get_value() == "true")
  {
    cout << zds.etag << endl;
  }
  else
  {
    cout << "Wrote data to '" << dsn << "'" << endl;
  }

  return rc;
}

int handle_data_set_delete_dsn(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn")->get_value();
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

int handle_log_view(ZCLIResult result)
{
  int rc = 0;
  unsigned int code = 0;
  string resp;

  string lines = result.get_option_value("--lines");

  cout << "lines are " << lines << endl;
  return 0;
}

int handle_uss_create_file(ZCLIResult result)
{
  int rc = 0;
  string file_path = result.get_positional("file-path")->get_value();
  string mode(result.get_option_value("--mode"));
  if ("" == mode)
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

int handle_uss_create_dir(ZCLIResult result)
{
  int rc = 0;
  string file_path = result.get_positional("file-path")->get_value();
  string mode(result.get_option_value("--mode"));
  if ("" == mode)
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

int handle_uss_list(ZCLIResult result)
{
  int rc = 0;
  string uss_file = result.get_positional("file-path")->get_value();

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

int handle_uss_view(ZCLIResult result)
{
  int rc = 0;
  string uss_file = result.get_positional("file-path")->get_value();

  ZUSF zusf = {0};
  const auto hasEncoding = result.get_option("--encoding") != nullptr && zut_prepare_encoding(result.get_option_value("--encoding"), &zusf.encoding_opts);

  struct stat file_stats;
  if (stat(uss_file.c_str(), &file_stats) == -1)
  {
    cerr << "Error: Path " << uss_file << " does not exist";
    return RTNCD_FAILURE;
  }

  auto *pipe_path = result.get_option("--pipe-path");
  if (pipe_path != nullptr && pipe_path->is_found())
  {
    rc = zusf_read_from_uss_file_streamed(&zusf, uss_file, pipe_path->get_value());

    if (result.get_option_value("--return-etag") == "true")
    {
      cout << zut_build_etag(file_stats.st_mtime, file_stats.st_size) << endl;
    }
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

    if (result.get_option_value("--return-etag") == "true")
    {
      cout << "etag: " << zut_build_etag(file_stats.st_mtime, file_stats.st_size) << endl;
      cout << "data: ";
    }
    if (hasEncoding && result.get_option_value("--response-format-bytes") == "true")
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

int handle_uss_write(ZCLIResult result)
{
  int rc = 0;
  string file = result.get_positional("file-path")->get_value();
  ZUSF zusf = {0};
  if (result.get_option("--encoding") != nullptr)
  {
    zut_prepare_encoding(result.get_option_value("--encoding"), &zusf.encoding_opts);
  }

  auto *etag_opt = result.get_option("--etag");
  if (etag_opt != nullptr && etag_opt->is_found())
  {
    strcpy(zusf.etag, etag_opt->get_value().c_str());
  }

  auto *pipe_path = result.get_option("--pipe-path");
  if (pipe_path != nullptr && pipe_path->is_found())
  {
    rc = zusf_write_to_uss_file_streamed(&zusf, file, pipe_path->get_value());
  }
  else
  {
    string data = "";
    string line = "";
    size_t byteSize = 0ul;

    // Use Ctrl/Cmd + D to stop writing data manually
    if (!isatty(fileno(stdout)))
    {
      std::istreambuf_iterator<char> begin(std::cin);
      std::istreambuf_iterator<char> end;

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
    rc = zusf_write_to_uss_file(&zusf, file, data);
  }

  if (0 != rc)
  {
    cerr << "Error: could not write to USS file: '" << file << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  auto *etag_opt2 = result.get_option("--etag-only");
  if (etag_opt2 != nullptr && etag_opt2->get_value() == "true")
  {
    cout << "etag: " << zusf.etag << '\n'
         << "created: " << (zusf.created ? "true" : "false") << '\n';
  }
  else
  {
    cout << "Wrote data to '" << file << "'" << (zusf.created ? " (created new file)" : " (overwrote existing)") << endl;
  }

  return rc;
}

int handle_uss_delete(ZCLIResult result)
{
  string file_path = result.get_positional("file-path")->get_value();
  bool recursive = result.get_option_value("--recursive") == "true";

  ZUSF zusf = {0};
  const auto rc = zusf_delete_uss_item(&zusf, file_path, recursive);

  if (rc != 0)
  {
    cerr << "Failed to delete USS item " << file_path << ":\n " << zusf.diag.e_msg << endl;
  }

  return rc;
}

int handle_uss_chmod(ZCLIResult result)
{
  int rc = 0;
  string mode(result.get_positional("mode")->get_value());
  string file_path = result.get_positional("file-path")->get_value();

  ZUSF zusf = {0};
  rc = zusf_chmod_uss_file_or_dir(&zusf, file_path, mode, result.get_option_value("--recursive") == "true");
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

int handle_uss_chown(ZCLIResult result)
{
  string path = result.get_positional("file-path")->get_value();
  string owner = result.get_positional("owner")->get_value();

  ZUSF zusf = {0};

  const auto rc = zusf_chown_uss_file_or_dir(&zusf, path, owner, result.get_option_value("--recursive") == "true");
  if (rc != 0)
  {
    cerr << "Error: could not chown USS path: '" << path << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  return rc;
}

int handle_uss_chtag(ZCLIResult result)
{
  string path = result.get_positional("file-path")->get_value();
  string tag = result.get_positional("tag")->get_value();

  ZUSF zusf = {0};
  const auto rc = zusf_chtag_uss_file_or_dir(&zusf, path, tag, result.get_option_value("--recursive") == "true");

  if (rc != 0)
  {
    cerr << "Error: could not chtag USS path: '" << path << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  return rc;
}

int handle_tso_issue(ZCLIResult result)
{
  int rc = 0;
  string command = result.get_positional("command")->get_value();
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

  string adata_dsn(result.get_option_value("--adata-dsn"));
  string chdr_dsn(result.get_option_value("--chdr-dsn"));
  string sysprint(result.get_option_value("--sysprint"));
  string sysout(result.get_option_value("--sysout"));

  const char *user = getlogin();
  string struser(user);
  transform(struser.begin(), struser.end(), struser.begin(), ::tolower); // upper case

  if (!result.get_option("--sysprint"))
    sysprint = "/tmp/" + struser + "_sysprint.txt";
  if (!result.get_option("--sysout"))
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

  rc = free_dynalloc_dds(dds);
  if (RTNCD_SUCCESS != rc)
  {
    return RTNCD_FAILURE;
  }

  return rc;
}

int handle_tool_dynalloc(ZCLIResult result)
{
  int rc = 0;
  unsigned int code = 0;
  string resp;

  string parm(result.get_positional("parm")->get_value());

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

int handle_tool_display_symbol(ZCLIResult result)
{
  int rc = 0;
  string symbol(result.get_positional("symbol")->get_value());
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

int handle_tool_run(ZCLIResult result)
{
  int rc = 0;
  string program(result.get_positional("program")->get_value());
  string dynalloc_pre(result.get_option_value("--dynalloc-pre"));
  string dynalloc_post(result.get_option_value("--dynalloc-post"));

  // allocate anything that was requested
  if (result.get_option("--dynalloc-pre"))
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

  string indd(result.get_option_value("--in-dd"));
  if (result.get_option("--in-dd"))
  {
    string ddname = "DD:" + indd;
    ofstream out(ddname.c_str());
    if (!out.is_open())
    {
      cerr << "Error: could not open input '" << ddname << "'" << endl;
      return RTNCD_FAILURE;
    }

    string input(result.get_option_value("--input"));
    if (result.get_option("--input"))
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

  string outdd(result.get_option_value("--out-dd"));
  if (result.get_option("--out-dd"))
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
  if (result.get_option("--dynalloc-post"))
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

int handle_tool_search(ZCLIResult result)
{
  int rc = 0;

  string pattern(result.get_positional("string")->get_value());
  string warn = result.get_option_value("--warn");
  string max_entries = result.get_option_value("--max-entries");
  string dsn(result.get_positional("dsn")->get_value());

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
    if ("true" == warn)
    {
      cerr << "Warning: results truncated" << endl;
    }
  }

  rc = free_dynalloc_dds(dds);
  if (RTNCD_SUCCESS != rc)
  {
    return RTNCD_FAILURE;
  }

  return RTNCD_SUCCESS;
}

int handle_tool_amblist(ZCLIResult result)
{
  int rc = 0;

  string dsn(result.get_positional("dsn")->get_value());
  string statements = " " + result.get_option_value("--control-statements");

  // perform dynalloc
  vector<string> dds;
  dds.push_back("alloc dd(syslib) da('" + dsn + "') shr");
  dds.push_back("alloc dd(sysprint) lrecl(80) recfm(f,b) blksize(80)");
  dds.push_back("alloc dd(sysin) lrecl(80) recfm(f,b) blksize(80)");

  rc = loop_dynalloc(dds);
  if (RTNCD_SUCCESS != rc)
  {
    return RTNCD_FAILURE;
  }

  transform(statements.begin(), statements.end(), statements.begin(), ::toupper); // upper case

  // write control statements
  ZDS zds = {0};
  zds_write_to_dd(&zds, "sysin", statements);
  if (0 != rc)
  {
    cerr << "Error: could not write to dd: '" << "sysin" << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  // perform search
  rc = zut_run("AMBLIST");
  if (RTNCD_SUCCESS != rc)
  {
    cerr << "Error: could error invoking AMBLIST rc: '" << rc << "'" << endl;
    // NOTE(Kelosky): don't exit here, but proceed to print errors
  }

  // read output from amblist
  string output;
  rc = zds_read_from_dd(&zds, "sysprint", output);
  if (0 != rc)
  {
    cerr << "Error: could not read from dd: '" << "sysprint" << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }
  cout << output << endl;

  rc = free_dynalloc_dds(dds);
  if (RTNCD_SUCCESS != rc)
  {
    return RTNCD_FAILURE;
  }

  return RTNCD_SUCCESS;
}

int handle_data_set_compress(ZCLIResult result)
{
  int rc = 0;

  string dsn(result.get_positional("dsn")->get_value());

  transform(dsn.begin(), dsn.end(), dsn.begin(), ::toupper); // upper case

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
  if (RTNCD_SUCCESS != rc)
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

  // https://www.ibm.com/docs/en/zos/3.1.0?topic=reference-examples-application-program-user-interaction-module-uim
  // https://www.ibm.com/docs/en/zos/3.1.0?topic=commands-compress-command-dfsmsdss
  // https://www.ibm.com/docs/en/zos/3.1.0?topic=interface-calling-block-structure
  // ADRXMAIA

  // //STEP0001 EXEC PGM=ADRDSSU
  // //SYSPRINT DD SYSOUT=*
  // //SYSIN DD *
  // COPY DS(INC( -
  //   USER.PDS  -
  //  )) -
  // REPLACEU -
  // COMPRESS
  // /*

  // perform compress
  rc = zut_run("IEBCOPY");
  if (RTNCD_SUCCESS != rc)
  {
    cerr << "Error: could error invoking IEBCOPY rc: '" << rc << "'" << endl;
  }

  // read output from amblist
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

  rc = free_dynalloc_dds(dds);
  if (RTNCD_SUCCESS != rc)
  {
    return RTNCD_FAILURE;
  }

  return RTNCD_SUCCESS;
}