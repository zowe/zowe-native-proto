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
#include "../native/c/parser.hpp"

using namespace std;

int handle_job_list(const parser::ParseResult &);
int handle_job_list_files(const parser::ParseResult &);
int handle_job_view_file(const parser::ParseResult &);
int handle_job_submit(const parser::ParseResult &);
int handle_job_delete(const parser::ParseResult &);

int handle_console_issue(const parser::ParseResult &);

int handle_data_set_create_dsn(const parser::ParseResult &);
int handle_data_set_view_dsn(const parser::ParseResult &);
int handle_data_set_list(const parser::ParseResult &);
int handle_data_set_list_members_dsn(const parser::ParseResult &);
int handle_data_set_write_to_dsn(const parser::ParseResult &);
int handle_data_set_delete_dsn(const parser::ParseResult &);

int handle_test_command(const parser::ParseResult &);
int handle_test_bpxwdyn(const parser::ParseResult &);

int main(int argc, char *argv[])
{
  // CLI
  parser::ArgumentParser arg_parser(argv[0]);

  //
  // test group
  //
  parser::command_ptr test_group(new parser::Command("test", "test other operations"));

  // test verbs
  parser::command_ptr test_command(new parser::Command("command", "test command"));
  test_command->set_handler(handle_test_command);
  test_group->add_command(test_command);

  parser::command_ptr test_bpxwdyn(new parser::Command("bpxwdyn", "test dynalloc command"));
  test_bpxwdyn->set_handler(handle_test_bpxwdyn);
  test_bpxwdyn->add_positional_arg("parm", "dynalloc test parm string", parser::ArgType_Single, true);
  test_group->add_command(test_bpxwdyn);

  //
  // data set group
  //
  parser::command_ptr data_set_group(new parser::Command("data-set", "z/OS data set operations"));

  // data set verbs
  parser::command_ptr data_set_create(new parser::Command("create", "create data set using defaults: DSORG=PO, RECFM=FB, LRECL=80"));
  data_set_create->set_handler(handle_data_set_create_dsn);
  data_set_create->add_positional_arg("dsn", "data set name, optionally with member specified", parser::ArgType_Single, true);
  data_set_group->add_command(data_set_create);

  parser::command_ptr data_set_view(new parser::Command("view", "view data set"));
  data_set_view->set_handler(handle_data_set_view_dsn);
  data_set_view->add_positional_arg("dsn", "data set name, optionally with member specified", parser::ArgType_Single, true);
  data_set_group->add_command(data_set_view);

  parser::command_ptr data_set_list(new parser::Command("list", "list data sets"));
  data_set_list->set_handler(handle_data_set_list);
  data_set_list->add_positional_arg("dsn", "data set name, optionally with member specified", parser::ArgType_Single, true);
  data_set_group->add_command(data_set_list);

  parser::command_ptr data_set_list_members(new parser::Command("list-members", "list data set members"));
  data_set_list_members->set_handler(handle_data_set_list_members_dsn);
  data_set_list_members->add_positional_arg("dsn", "data set name, optionally with member specified", parser::ArgType_Single, true);
  data_set_group->add_command(data_set_list_members);

  parser::command_ptr data_set_write(new parser::Command("write", "write to data set"));
  data_set_write->set_handler(handle_data_set_write_to_dsn);
  data_set_write->add_positional_arg("dsn", "data set name, optionally with member specified", parser::ArgType_Single, true);
  data_set_group->add_command(data_set_write);

  parser::command_ptr data_set_delete(new parser::Command("delete", "delete data set"));
  data_set_delete->set_handler(handle_data_set_delete_dsn);
  data_set_delete->add_positional_arg("dsn", "data set name, optionally with member specified", parser::ArgType_Single, true);
  data_set_group->add_command(data_set_delete);

  //
  // jobs group
  //
  parser::command_ptr job_group(new parser::Command("job", "z/OS job operations"));

  // jobs verbs
  parser::command_ptr job_list(new parser::Command("list", "list jobs"));
  job_list->set_handler(handle_job_list);
  job_list->add_keyword_arg("owner", parser::make_aliases("--owner"), "filter by owner", parser::ArgType_Single, false);
  job_group->add_command(job_list);

  parser::command_ptr job_list_files(new parser::Command("list-files", "list spool files"));
  job_list_files->set_handler(handle_job_list_files);
  job_list_files->add_positional_arg("jobid", "valid jobid", parser::ArgType_Single, true);
  job_group->add_command(job_list_files);

  parser::command_ptr job_view_file(new parser::Command("view-file", "view job file output"));
  job_view_file->set_handler(handle_job_view_file);
  job_view_file->add_positional_arg("jobid", "valid jobid", parser::ArgType_Single, true);
  job_view_file->add_positional_arg("key", "valid job dsn key via 'job list-files'", parser::ArgType_Single, true);
  job_group->add_command(job_view_file);

  parser::command_ptr job_submit(new parser::Command("submit", "submit a job"));
  job_submit->set_handler(handle_job_submit);
  job_submit->add_keyword_arg("only-jobid", parser::make_aliases("--only-jobid"), "show only job id on success", parser::ArgType_Flag, false);
  job_submit->add_positional_arg("dsn", "dsn containing JCL", parser::ArgType_Single, true);
  job_group->add_command(job_submit);

  parser::command_ptr job_delete(new parser::Command("delete", "delete a job"));
  job_delete->set_handler(handle_job_delete);
  job_delete->add_positional_arg("jobid", "valid jobid", parser::ArgType_Single, true);
  job_group->add_command(job_delete);

  //
  // console group
  //
  parser::command_ptr console_group(new parser::Command("console", "z/OS console operations"));

  // console verbs
  parser::command_ptr console_issue(new parser::Command("issue", "issue a console command"));
  console_issue->set_handler(handle_console_issue);
  console_issue->add_keyword_arg("console-name", parser::make_aliases("--console-name", "--cn"), "extended console name", parser::ArgType_Single, true);
  console_issue->add_positional_arg("command", "command to run, e.g. 'D IPLINFO'", parser::ArgType_Single, true);
  console_group->add_command(console_issue);

  // add all groups to the CLI
  arg_parser.get_root_command().add_command(test_group);
  arg_parser.get_root_command().add_command(data_set_group);
  arg_parser.get_root_command().add_command(console_group);
  arg_parser.get_root_command().add_command(job_group);

  // parse
  parser::ParseResult result = arg_parser.parse(argc, argv);
  return result.exit_code;
}

int handle_job_list(const parser::ParseResult &result)
{
  int rc = 0;
  string owner_name = result.find_kw_arg_string("owner");

  cout << "handle_job_list " << owner_name << endl;

  return 0;
}

int handle_job_list_files(const parser::ParseResult &result)
{
  int rc = 0;
  string jobid = result.find_pos_arg_string("jobid");

  cout << "handle_job_list_files " << jobid << endl;

  return 0;
}

int handle_job_view_file(const parser::ParseResult &result)
{
  int rc = 0;
  string jobid = result.find_pos_arg_string("jobid");
  string key = result.find_pos_arg_string("key");

  cout << "handle_job_view_file " << jobid << " and " << key << endl;

  return 0;
}

int handle_job_submit(const parser::ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");

  cout << "handle_job_submit " << dsn << endl;

  return 0;
}

int handle_job_delete(const parser::ParseResult &result)
{

  int rc = 0;
  string jobid = result.find_pos_arg_string("jobid");

  cout << "handle_job_delete " << jobid << endl;

  return 0;
}

int handle_console_issue(const parser::ParseResult &result)
{
  int rc = 0;

  string console_name = result.find_kw_arg_string("console-name");
  string command = result.find_pos_arg_string("command");

  cout << "handle_console_issue " << console_name << " and " << command << endl;

  return 0;
}

int handle_data_set_create_dsn(const parser::ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");

  cout << "handle_data_set_create_dsn " << dsn << endl;

  return 0;
}

int handle_data_set_view_dsn(const parser::ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");

  cout << "handle_data_set_view_dsn " << dsn << endl;

  return 0;
}

int handle_data_set_list(const parser::ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");

  cout << "handle_data_set_list " << dsn << endl;

  return 0;
}

int handle_data_set_list_members_dsn(const parser::ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");

  cout << "handle_data_set_list_members_dsn " << dsn << endl;

  return 0;
}

int handle_data_set_write_to_dsn(const parser::ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");

  cout << "handle_data_set_write_to_dsn " << dsn << endl;

  return 0;
}

int handle_data_set_delete_dsn(const parser::ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");

  cout << "handle_data_set_delete_dsn " << dsn << endl;

  return 0;
}

int handle_test_command(const parser::ParseResult &result)
{
  int rc = 0;

  cout << "handle_test_command " << endl;

  return 0;
}

int handle_test_bpxwdyn(const parser::ParseResult &result)
{
  int rc = 0;
  string parm = result.find_pos_arg_string("parm");

  cout << "handle_test_bpxwdyn " << parm << endl;

  return 0;
}
