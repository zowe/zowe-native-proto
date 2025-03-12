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
#include <cstdlib>
#include <stdio.h>
#include "zcli.hpp"

using namespace std;

int handle_job_list(ZCLIResult);
int handle_job_list_files(ZCLIResult);
int handle_job_view_file(ZCLIResult);
int handle_job_submit(ZCLIResult);
int handle_job_delete(ZCLIResult);

int handle_console_issue(ZCLIResult);

int handle_data_set_create_dsn(ZCLIResult);
int handle_data_set_view_dsn(ZCLIResult);
int handle_data_set_list(ZCLIResult);
int handle_data_set_list_members_dsn(ZCLIResult);
int handle_data_set_write_to_dsn(ZCLIResult);
int handle_data_set_delete_dsn(ZCLIResult);

int handle_test_command(ZCLIResult);
int handle_test_bpxwdyn(ZCLIResult);

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

  ZCLIVerb test_bpxwdyn("bpxwdyn");
  test_bpxwdyn.set_description("test dynalloc command");
  test_bpxwdyn.set_zcli_verb_handler(handle_test_bpxwdyn);
  ZCLIPositional test_parm("parm");
  test_parm.set_description("dynalloc test parm string");
  test_parm.set_required(true);
  test_bpxwdyn.get_positionals().push_back(test_parm);
  test_group.get_verbs().push_back(test_bpxwdyn);

  //
  // data set group
  //
  ZCLIGroup data_set_group("data-set");
  data_set_group.set_description("z/OS data set operations");

  ZCLIPositional data_set_dsn("dsn");
  data_set_dsn.set_description("data set name, optionally with member specified");
  data_set_dsn.set_required(true);

  // data set verbs
  ZCLIVerb data_set_create("create");
  data_set_create.set_description("create data set using defaults: DSORG=PO, RECFM=FB, LRECL=80");
  data_set_create.set_zcli_verb_handler(handle_data_set_create_dsn);
  data_set_create.get_positionals().push_back(data_set_dsn);
  data_set_group.get_verbs().push_back(data_set_create);

  ZCLIVerb data_set_view("view");
  data_set_view.set_description("view data set");
  data_set_view.set_zcli_verb_handler(handle_data_set_view_dsn);
  data_set_view.get_positionals().push_back(data_set_dsn);
  data_set_group.get_verbs().push_back(data_set_view);

  ZCLIVerb data_set_list("list");
  data_set_list.set_description("list data sets");
  data_set_list.set_zcli_verb_handler(handle_data_set_list);
  data_set_list.get_positionals().push_back(data_set_dsn);
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
  job_group.get_verbs().push_back(job_list);

  ZCLIVerb job_list_files("list-files");
  job_list_files.set_description("list spool files");
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
  console_name.get_aliases().push_back("--cn");
  console_name.set_required(true);
  console_name.set_description("extended console name");
  console_issue.get_options().push_back(console_name);
  ZCLIPositional console_command("command");
  console_command.set_required(true);
  console_command.set_description("command to run, e.g. 'D IPLINFO'");
  console_issue.get_positionals().push_back(console_command);
  console_group.get_verbs().push_back(console_issue);

  // add all groups to the CLI
  zcli.get_groups().push_back(test_group);
  zcli.get_groups().push_back(data_set_group);
  zcli.get_groups().push_back(console_group);
  zcli.get_groups().push_back(job_group);

  // parse
  return zcli.parse(argc, argv);
}

int handle_job_list(ZCLIResult result)
{
  int rc = 0;
  string owner_name(result.get_option_value("--owner"));

  cout << "handle_job_list " << owner_name << endl;

  return 0;
}

int handle_job_list_files(ZCLIResult result)
{
  int rc = 0;
  string jobid(result.get_positional("jobid").get_value());

  cout << "handle_job_list_files " << jobid << endl;

  return 0;
}

int handle_job_view_file(ZCLIResult result)
{
  int rc = 0;
  string jobid(result.get_positional("jobid").get_value());
  string key(result.get_positional("key").get_value());

  cout << "handle_job_view_file " << jobid << " and " << key << endl;

  return 0;
}

int handle_job_submit(ZCLIResult result)
{
  int rc = 0;
  string dsn(result.get_positional("dsn").get_value());

  cout << "handle_job_submit " << dsn << endl;

  return 0;
}

int handle_job_delete(ZCLIResult result)
{

  int rc = 0;
  string jobid(result.get_positional("jobid").get_value());

  cout << "handle_job_delete " << jobid << endl;

  return 0;
}

int handle_console_issue(ZCLIResult result)
{
  int rc = 0;

  string console_name(result.get_option("--console-name").get_value());
  string command(result.get_positional("command").get_value());

  cout << "handle_console_issue " << console_name << " and " << command << endl;

  return 0;
}

int handle_data_set_create_dsn(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn").get_value();

  cout << "handle_data_set_create_dsn " << dsn << endl;

  return 0;
}

int handle_data_set_view_dsn(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn").get_value();

  cout << "handle_data_set_view_dsn " << dsn << endl;

  return 0;
}

int handle_data_set_list(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn").get_value();

  cout << "handle_data_set_list " << dsn << endl;

  return 0;
}

int handle_data_set_list_members_dsn(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn").get_value();

  cout << "handle_data_set_list_members_dsn " << dsn << endl;

  return 0;
}

int handle_data_set_write_to_dsn(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn").get_value();

  cout << "handle_data_set_write_to_dsn " << dsn << endl;

  return 0;
}

int handle_data_set_delete_dsn(ZCLIResult result)
{
  int rc = 0;
  string dsn = result.get_positional("dsn").get_value();

  cout << "handle_data_set_delete_dsn " << dsn << endl;

  return 0;
}

int handle_test_command(ZCLIResult result)
{
  int rc = 0;

  cout << "handle_test_command " << endl;

  return 0;
}

int handle_test_bpxwdyn(ZCLIResult result)
{
  int rc = 0;
  string parm = result.get_positional("parm").get_value();

  cout << "handle_test_bpxwdyn " << parm << endl;

  return 0;
}
