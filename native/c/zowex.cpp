/*
  This program and the accompanying materials are
  made available under the terms of the Eclipse Public License v2.0 which accompanies
  this distribution, and is available at https://www.eclipse.org/legal/epl-v20.html

  SPDX-License-Identifier: EPL-2.0

  Copyright Contributors to the Zowe Project.
*/

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <string>
#include <cstdlib>
#include <stdio.h>
#include <algorithm>
#include "zcn.hpp"
#include "zut.hpp"
#include "zcli.hpp"
#include "zjb.hpp"
#include "unistd.h"
#include "zds.hpp"

using namespace std;

int handle_job_list(ZCLIResult);
int handle_job_list_files(ZCLIResult);
int handle_job_view_file(ZCLIResult);
int handle_job_view_jcl(ZCLIResult);
int handle_job_submit(ZCLIResult);
int handle_job_delete(ZCLIResult);

int handle_console_issue(ZCLIResult);

int handle_data_set_create_dsn(ZCLIResult);
int handle_data_set_create_dsn_vb(ZCLIResult);
int handle_data_set_create_dsn_adata(ZCLIResult);
int handle_data_set_view_dsn(ZCLIResult);
int handle_data_set_list(ZCLIResult);
int handle_data_set_list_members_dsn(ZCLIResult);
int handle_data_set_write_to_dsn(ZCLIResult);
int handle_data_set_delete_dsn(ZCLIResult);

int handle_tool_convert_dsect(ZCLIResult);

int handle_test_command(ZCLIResult);
int handle_test_dynalloc(ZCLIResult);
int handle_test_run(ZCLIResult);

// TODO(Kelosky):
// help w/verbose examples
// add simple examples to help

int main(int argc, char *argv[])
{
  // CLI
  ZCLI zcli(argv[PROCESS_NAME_ARG]);
  zcli.set_interactive_mode(true);

  //
  // test group
  //
  ZCLIGroup test_group("test");
  test_group.set_description("test other operations");

  // test verbs
  ZCLIVerb test_command("command");
  test_command.set_description("test command");
  test_command.set_zcli_verb_handler(handle_test_command);
  test_group.get_verbs().push_back(test_command);

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

  ZCLIVerb data_set_view("view");
  data_set_view.set_description("view data set");
  data_set_view.set_zcli_verb_handler(handle_data_set_view_dsn);
  data_set_view.get_positionals().push_back(data_set_dsn);
  data_set_view.get_options().push_back(data_set_encoding);
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

  ZCLIOption dslist_rfc("response-format-csv");
  dslist_rfc.set_description("returns the response in CSV format");
  dslist_rfc.get_aliases().push_back("--rfc");
  data_set_list.get_options().push_back(dslist_rfc);

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

  // jobs verbs
  ZCLIVerb job_list("list");
  job_list.set_description("list jobs");
  job_list.set_zcli_verb_handler(handle_job_list);
  ZCLIOption job_owner("owner");
  job_owner.set_description("filter by owner");
  job_list.get_options().push_back(job_owner);
  ZCLIOption job_list_rfc("response-format-csv");
  job_list_rfc.set_description("returns the response in CSV format");
  job_list_rfc.get_aliases().push_back("--rfc");
  job_list.get_options().push_back(job_list_rfc);
  job_group.get_verbs().push_back(job_list);

  ZCLIVerb job_list_files("list-files");
  job_list_files.set_description("list spool files for jobid");
  job_list_files.set_zcli_verb_handler(handle_job_list_files);
  ZCLIPositional job_jobid("jobid");
  job_jobid.set_required(true);
  job_jobid.set_description("valid jobid");
  job_list_files.get_positionals().push_back(job_jobid);
  job_group.get_verbs().push_back(job_list_files);

  ZCLIVerb job_view_file("view-file");
  job_view_file.set_description("view job file output");
  job_view_file.set_zcli_verb_handler(handle_job_view_file);
  job_view_file.get_positionals().push_back(job_jobid);

  ZCLIPositional job_dsn_key("key");
  job_dsn_key.set_required(true);
  job_dsn_key.set_description("valid job dsn key via 'job list-files'");
  job_view_file.get_positionals().push_back(job_dsn_key);
  job_group.get_verbs().push_back(job_view_file);

  ZCLIVerb job_view_jcl("view-jcl");
  job_view_jcl.set_description("view job jcl from inbput jobid");
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
  // tool group
  //
  ZCLIGroup tool_group("tool");
  tool_group.set_description("tool operations");

  // console verbs
  ZCLIVerb tool_convert_dsect("convert-dsect");
  tool_convert_dsect.set_description("conver dsect to c struct");
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

  ZCLIVerb tool_dynalloc("dynalloc");
  tool_dynalloc.set_description("dynalloc command");
  tool_dynalloc.set_zcli_verb_handler(handle_test_dynalloc);
  ZCLIPositional dynalloc_parm("parm");
  dynalloc_parm.set_description("dynalloc test parm string");
  dynalloc_parm.set_required(true);
  tool_dynalloc.get_positionals().push_back(dynalloc_parm);
  tool_group.get_verbs().push_back(tool_dynalloc);

  // add all groups to the CLI
  zcli.get_groups().push_back(test_group);
  zcli.get_groups().push_back(data_set_group);
  zcli.get_groups().push_back(console_group);
  zcli.get_groups().push_back(job_group);
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
    return -1;
  }

  const bool emit_csv = result.get_option("--response-format-csv").is_found();
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

  return 0;
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
    return -1;
  }

  for (vector<ZJobDD>::iterator it = job_dds.begin(); it != job_dds.end(); ++it)
  {
    cout << left << setw(9) << it->ddn << " " << it->dsn << " " << setw(4) << it->key << " " << it->stepname << " " << it->procstep << endl;
  }

  return 0;
}

int handle_job_view_file(ZCLIResult result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid(result.get_positional("jobid").get_value());
  string key(result.get_positional("key").get_value());

  string resp;
  rc = zjb_read_jobs_output_by_jobid_and_key(&zjb, jobid, atoi(key.c_str()), resp);

  if (0 != rc)
  {
    cout << "Error: could not view job file for: '" << jobid << "' with key '" << key << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zjb.diag.e_msg << endl;
    return -1;
  }

  cout << resp;

  return 0;
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
    return -1;
  }

  string only_jobid(result.get_option("--only-jobid").get_value());
  if ("true" == only_jobid)
    cout << jobid << endl;
  else
    cout << "Submitted " << dsn << ", " << jobid << endl;

  return 0;
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
    return -1;
  }

  cout << "Job " << jobid << " deleted " << endl;

  return 0;
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
    return -1;
  }

  printf("%.8s", zcn.console_name);

  rc = zcn_put(&zcn, command);
  if (0 != rc)
  {
    cout << "Error: could not write to console: '" << console_name << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zcn.diag.e_msg << endl;
    return -1;
  }

  string response = "";
  rc = zcn_get(&zcn, response);
  if (0 != rc)
  {
    cout << "Error: could not get from console: '" << console_name << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zcn.diag.e_msg << endl;
    return -1;
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
    return -1;
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
    return -1;
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

int handle_data_set_view_dsn(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn").get_value();
  ZCLIOption &encoding = result.get_option("--encoding");
  ZDS zds = {0};
  string response;
  string encodingValue = encoding.get_value();
  const bool hasEncoding = !encodingValue.empty();
  rc = zds_read_from_dsn(&zds, dsn, response, hasEncoding ? &encodingValue : NULL);
  if (0 != rc)
  {
    cout << "Error: could not read data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zds.diag.e_msg << endl;
    return -1;
  }
  if (hasEncoding)
  {
    for (char *p = (char *)response.data(); p < (response.data() + response.length()); p++)
    {
      printf("%02x ", (unsigned char)*p);
    }
    printf("\n");
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

  const bool emit_csv = result.get_option("--response-format-csv").is_found();
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
    return -1;
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
    return -1;
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

  while (getline(cin, line))
  {
    data += line;
    data.push_back('\n');
  }

  rc = zds_write_to_dsn(&zds, dsn, data);

  if (0 != rc)
  {
    cout << "Error: could not write to data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << zds.diag.e_msg << endl;
    return -1;
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
    return -1;
  }
  cout << "Data set '" << dsn << "' deleted" << endl;

  return rc;
}

int handle_test_command(ZCLIResult result)
{
  int rc = 0;
  rc = zut_test();

  cout << "test code called " << rc << endl;

  return 0;
}

int handle_test_dynalloc(ZCLIResult result)
{
  int rc = 0;
  unsigned int code = 0;
  string resp;

  string parm(result.get_positional("parm").get_value());

  cout << "parm is '" << parm << "'" << endl;

  // alloc da('DKELOSKY.TEMP.ADATA') DSORG(PO) SPACE(5,5) CYL LRECL(80) RECFM(F,b) NEW DIR(5) vol(USER01)
  // zowex test bpxwdyn "alloc da('ibmuser.temp') space(5,5) dsorg(po) dir(5) cyl lrecl(80) recfm(f,b) new"
  rc = zut_bpxwdyn(parm, &code, resp);
  if (0 != rc)
  {
    cout << "Error: bpxwdyn with parm '" << parm << "' rc: '" << rc << "'" << endl;
    cout << "  Details: " << resp << endl;
    return -1;
  }

  cout << resp << endl;

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