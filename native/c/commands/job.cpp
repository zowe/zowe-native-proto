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

#include "job.hpp"
#include "common_args.hpp"
#include "../zds.hpp"
#include "../zjb.hpp"
#include "../zusf.hpp"
#include "../zut.hpp"
#include <unistd.h>

using namespace parser;
using namespace std;
using namespace commands::common;

namespace job
{
int handle_job_list(const ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  string owner_name = result.get_value<string>("owner", "*");
  string prefix_name = result.get_value<string>("prefix", "*");
  long long max_entries = result.get_value<long long>("max-entries", 0);
  bool warn = result.get_value<bool>("warn", true);

  if (max_entries > 0)
  {
    zjb.jobs_max = max_entries;
  }

  vector<ZJob> jobs;
  rc = zjb_list_by_owner(&zjb, owner_name, prefix_name, jobs);

  if (RTNCD_SUCCESS == rc || RTNCD_WARNING == rc)
  {
    bool emit_csv = result.get_value<bool>("response-format-csv", false);
    for (vector<ZJob>::iterator it = jobs.begin(); it != jobs.end(); it++)
    {
      if (emit_csv)
      {
        vector<string> fields;
        fields.reserve(5);
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
  string jobid = result.get_value<std::string>("jobid", "");
  long long max_entries = result.get_value<long long>("max-entries", 0);
  bool warn = result.get_value<bool>("warn", true);

  if (max_entries > 0)
  {
    zjb.dds_max = max_entries;
  }

  vector<ZJobDD> job_dds;
  rc = zjb_list_dds(&zjb, jobid, job_dds);
  if (RTNCD_SUCCESS == rc || RTNCD_WARNING == rc)
  {
    bool emit_csv = result.get_value<bool>("response-format-csv", false);
    std::vector<string> fields;
    fields.reserve(5);
    for (vector<ZJobDD>::iterator it = job_dds.begin(); it != job_dds.end(); ++it)
    {
      fields.push_back(it->ddn);
      fields.push_back(it->dsn);
      fields.push_back(zut_int_to_string(it->key));
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
  string jobid = result.get_value<std::string>("jobid", "");

  bool emit_csv = result.get_value<bool>("response-format-csv", false);

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
    fields.reserve(6);
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
  string jobid = result.get_value<std::string>("jobid", "");
  long long key = result.get_value<long long>("key", 0);

  if (result.has("encoding"))
  {
    zut_prepare_encoding(result.get_value<std::string>("encoding", ""), &zjb.encoding_opts);
  }
  if (result.has("local-encoding"))
  {
    const auto source_encoding = result.get_value<std::string>("local-encoding", "");
    if (!source_encoding.empty() && source_encoding.size() < sizeof(zjb.encoding_opts.source_codepage))
    {
      memcpy(zjb.encoding_opts.source_codepage, source_encoding.data(), source_encoding.length() + 1);
    }
  }

  string resp;
  rc = zjb_read_jobs_output_by_key(&zjb, jobid, key, resp);

  if (0 != rc)
  {
    cerr << "Error: could not view job file for: '" << jobid << "' with key '" << key << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zjb.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  bool has_encoding = result.has("encoding");
  bool response_format_bytes = result.get_value<bool>("response-format-bytes", false);

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
  string jobid = result.get_value<std::string>("jobid", "");

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
  string dsn = result.get_value<std::string>("dsn", "");
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
  string file = result.get_value<std::string>("file-path", "");

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
  ZJB zjb = {0};
  string jobid;
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
  bool encoding_prepared = result.has("encoding") && zut_prepare_encoding(result.get_value<std::string>("encoding", ""), &encoding_opts);

  if (result.has("local-encoding"))
  {
    const auto source_encoding = result.get_value<std::string>("local-encoding", "");
    if (!source_encoding.empty() && source_encoding.size() < sizeof(encoding_opts.source_codepage))
    {
      memcpy(encoding_opts.source_codepage, source_encoding.data(), source_encoding.length() + 1);
    }
  }

  if (encoding_prepared && encoding_opts.data_type != eDataTypeBinary)
  {
    const auto source_encoding = strlen(encoding_opts.source_codepage) > 0 ? string(encoding_opts.source_codepage) : "UTF-8";
    data = zut_encode(data, source_encoding, string(encoding_opts.codepage), zjb.diag);
  }

  return job_submit_common(result, data, jobid, "JCL");
}

int handle_job_delete(const ParseResult &result)
{
  int rc = 0;
  ZJB zjb = {0};
  string jobid = result.get_value<std::string>("jobid", "");

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
  string jobid = result.get_value<std::string>("jobid", "");

  // Note: Cancel options (dump, force, purge, restart) are currently not used by the backend
  // but are defined for future compatibility
  bool option_dump = result.get_value<bool>("dump", false);
  bool option_force = result.get_value<bool>("force", false);
  bool option_purge = result.get_value<bool>("purge", false);
  bool option_restart = result.get_value<bool>("restart", false);

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
  string jobid = result.get_value<std::string>("jobid", "");

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
  string jobid = result.get_value<std::string>("jobid", "");

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

  bool only_jobid = result.get_value<bool>("only-jobid", false);
  bool only_correlator = result.get_value<bool>("only-correlator", false);
  string wait = result.get_value<std::string>("wait", "");
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

void register_commands(parser::Command &root_command)
{
  auto encoding_option = make_aliases("--encoding", "--ec");
  auto source_encoding_option = make_aliases("--local-encoding", "--lec");
  auto response_format_csv_option = make_aliases("--response-format-csv", "--rfc");
  auto response_format_bytes_option = make_aliases("--response-format-bytes", "--rfb");

  // Job command group
  auto job_group = command_ptr(new Command("job", "z/OS job operations"));

  // List subcommand
  auto job_list_cmd = command_ptr(new Command("list", "list jobs"));
  job_list_cmd->add_keyword_arg("owner", make_aliases("--owner", "-o"), "filter by owner", ArgType_Single, false);
  job_list_cmd->add_keyword_arg("prefix", make_aliases("--prefix", "-p"), "filter by prefix", ArgType_Single, false);
  job_list_cmd->add_keyword_arg(MAX_ENTRIES);
  job_list_cmd->add_keyword_arg(WARN);
  job_list_cmd->add_keyword_arg(RESPONSE_FORMAT_CSV);
  job_list_cmd->set_handler(handle_job_list);
  job_group->add_command(job_list_cmd);

  // List-files subcommand
  auto job_list_files_cmd = command_ptr(new Command("list-files", "list spool files for jobid"));
  job_list_files_cmd->add_alias("lf");
  job_list_files_cmd->add_positional_arg(JOB_ID);
  job_list_files_cmd->add_keyword_arg(MAX_ENTRIES);
  job_list_files_cmd->add_keyword_arg(WARN);
  job_list_files_cmd->add_keyword_arg(RESPONSE_FORMAT_CSV);
  job_list_files_cmd->set_handler(handle_job_list_files);
  job_group->add_command(job_list_files_cmd);

  // View-status subcommand
  auto job_view_status_cmd = command_ptr(new Command("view-status", "view job status"));
  job_view_status_cmd->add_alias("vs");
  job_view_status_cmd->add_positional_arg(JOB_ID);
  job_view_status_cmd->add_keyword_arg(RESPONSE_FORMAT_CSV);
  job_view_status_cmd->set_handler(handle_job_view_status);
  job_group->add_command(job_view_status_cmd);

  // View-file subcommand
  auto job_view_file_cmd = command_ptr(new Command("view-file", "view job file output"));
  job_view_file_cmd->add_alias("vf");
  job_view_file_cmd->add_positional_arg(JOB_ID);
  job_view_file_cmd->add_positional_arg("key", "valid job dsn key via 'job list-files'", ArgType_Single, true);
  job_view_file_cmd->add_keyword_arg(ENCODING);
  job_view_file_cmd->add_keyword_arg(LOCAL_ENCODING);
  job_view_file_cmd->add_keyword_arg(RESPONSE_FORMAT_BYTES);
  job_view_file_cmd->set_handler(handle_job_view_file);
  job_group->add_command(job_view_file_cmd);

  // View-jcl subcommand
  auto job_view_jcl_cmd = command_ptr(new Command("view-jcl", "view job jcl from input jobid"));
  job_view_jcl_cmd->add_alias("vj");
  job_view_jcl_cmd->add_positional_arg(JOB_ID);
  job_view_jcl_cmd->set_handler(handle_job_view_jcl);
  job_group->add_command(job_view_jcl_cmd);

  // Submit subcommand
  auto job_submit_cmd = command_ptr(new Command("submit", "submit a job"));
  job_submit_cmd->add_alias("sub");
  job_submit_cmd->add_positional_arg(DSN);
  job_submit_cmd->add_keyword_arg("wait", make_aliases("--wait"), "wait for job status", ArgType_Single, false);
  job_submit_cmd->add_keyword_arg("only-jobid", make_aliases("--only-jobid", "--oj"), "show only job id on success", ArgType_Flag, false, ArgValue(false));
  job_submit_cmd->add_keyword_arg("only-correlator", make_aliases("--only-correlator", "--oc"), "show only job correlator on success", ArgType_Flag, false, ArgValue(false));
  job_submit_cmd->set_handler(handle_job_submit);
  job_group->add_command(job_submit_cmd);

  // Submit-jcl subcommand
  auto job_submit_jcl_cmd = command_ptr(new Command("submit-jcl", "submit JCL contents directly"));
  job_submit_jcl_cmd->add_alias("subj");
  job_submit_jcl_cmd->add_keyword_arg("wait", make_aliases("--wait"), "wait for job status", ArgType_Single, false);
  job_submit_jcl_cmd->add_keyword_arg("only-jobid", make_aliases("--only-jobid", "--oj"), "show only job id on success", ArgType_Flag, false, ArgValue(false));
  job_submit_jcl_cmd->add_keyword_arg("only-correlator", make_aliases("--only-correlator", "--oc"), "show only job correlator on success", ArgType_Flag, false, ArgValue(false));
  job_submit_jcl_cmd->add_keyword_arg(ENCODING);
  job_submit_jcl_cmd->add_keyword_arg(LOCAL_ENCODING);
  job_submit_jcl_cmd->set_handler(handle_job_submit_jcl);
  job_group->add_command(job_submit_jcl_cmd);

  // Submit-uss subcommand
  auto job_submit_uss_cmd = command_ptr(new Command("submit-uss", "submit a job from USS files"));
  job_submit_uss_cmd->add_alias("sub-u");
  job_submit_uss_cmd->add_positional_arg(FILE_PATH);
  job_submit_uss_cmd->add_keyword_arg("wait", make_aliases("--wait"), "wait for job status", ArgType_Single, false);
  job_submit_uss_cmd->add_keyword_arg("only-jobid", make_aliases("--only-jobid", "--oj"), "show only job id on success", ArgType_Flag, false, ArgValue(false));
  job_submit_uss_cmd->add_keyword_arg("only-correlator", make_aliases("--only-correlator", "--oc"), "show only job correlator on success", ArgType_Flag, false, ArgValue(false));
  job_submit_uss_cmd->set_handler(handle_job_submit_uss);
  job_group->add_command(job_submit_uss_cmd);

  // Delete subcommand
  auto job_delete_cmd = command_ptr(new Command("delete", "delete a job"));
  job_delete_cmd->add_alias("del");
  job_delete_cmd->add_positional_arg(JOB_ID);
  job_delete_cmd->set_handler(handle_job_delete);
  job_group->add_command(job_delete_cmd);

  // Cancel subcommand
  auto job_cancel_cmd = command_ptr(new Command("cancel", "cancel a job"));
  job_cancel_cmd->add_alias("cnl");
  job_cancel_cmd->add_positional_arg(JOB_ID);
  job_cancel_cmd->add_keyword_arg("dump", make_aliases("--dump", "-d"), "Dump the cancelled jobs if waiting for conversion, in conversion, or in execution", ArgType_Flag, false, ArgValue(false));
  job_cancel_cmd->add_keyword_arg("force", make_aliases("--force", "-f"), "Force cancel the jobs, even if marked", ArgType_Flag, false, ArgValue(false));
  job_cancel_cmd->add_keyword_arg("purge", make_aliases("--purge", "-p"), "Purge output of the cancelled jobs", ArgType_Flag, false, ArgValue(false));
  job_cancel_cmd->add_keyword_arg("restart", make_aliases("--restart", "-r"), "Request that automatic restart management automatically restart the selected jobs after they are cancelled", ArgType_Flag, false, ArgValue(false));
  job_cancel_cmd->set_handler(handle_job_cancel);
  job_group->add_command(job_cancel_cmd);

  // Hold subcommand
  auto job_hold_cmd = command_ptr(new Command("hold", "hold a job"));
  job_hold_cmd->add_alias("hld");
  job_hold_cmd->add_positional_arg(JOB_ID);
  job_hold_cmd->set_handler(handle_job_hold);
  job_group->add_command(job_hold_cmd);

  // Release subcommand
  auto job_release_cmd = command_ptr(new Command("release", "release a job"));
  job_release_cmd->add_alias("rel");
  job_release_cmd->add_positional_arg(JOB_ID);
  job_release_cmd->set_handler(handle_job_release);
  job_group->add_command(job_release_cmd);

  root_command.add_command(job_group);
}
} // namespace job