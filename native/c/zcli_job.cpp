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

#include "parser.hpp"
#include "zjb.hpp"
#include "zds.hpp"
#include "zusf.hpp"
#include "zut.hpp"
#include "zcli.hpp"

#ifndef TO_STRING
#define TO_STRING(x) static_cast<std::ostringstream &>(           \
                         (std::ostringstream() << std::dec << x)) \
                         .str()
#endif

using namespace parser;
using namespace std;

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
  bool encoding_prepared = result.has_kw_arg("encoding") && zut_prepare_encoding(result.find_kw_arg_string("encoding"), &encoding_opts);

  if (encoding_prepared && encoding_opts.data_type != eDataTypeBinary)
  {
    data = zut_encode(data, "UTF-8", string(encoding_opts.codepage), zjb.diag);
  }

  return job_submit_common(result, data, jobid, "JCL");
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
