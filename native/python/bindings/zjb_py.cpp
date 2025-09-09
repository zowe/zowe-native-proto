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

#include "zjb_py.hpp"
#include <iostream>

std::vector<ZJob> list_jobs_by_owner(std::string owner_name)
{
  std::vector<ZJob> jobs;
  ZJB zjb = {0};

  a2e_inplace(owner_name);
  int rc = zjb_list_by_owner(&zjb, owner_name, "", jobs);

  if (rc != 0 && rc != RTNCD_WARNING)
  {
    std::string diag(zjb.diag.e_msg, zjb.diag.e_msg_len);
    diag.push_back('\0');
    e2a_inplace(diag);
    diag.pop_back();
    std::cerr << "ZJB list_jobs_by_owner failed, rc=" << rc
              << ", diag=\"" << diag << "\"\n";
  }

  for (auto &job : jobs)
  {
    e2a_inplace(job.jobname);
    e2a_inplace(job.jobid);
    e2a_inplace(job.owner);
    e2a_inplace(job.status);
    e2a_inplace(job.full_status);
    e2a_inplace(job.retcode);
    e2a_inplace(job.correlator);
  }

  return jobs;
}

std::vector<ZJob> list_jobs_by_owner(std::string owner_name, std::string prefix)
{
  std::vector<ZJob> jobs;
  ZJB zjb = {0};

  a2e_inplace(owner_name);
  a2e_inplace(prefix);
  int rc = zjb_list_by_owner(&zjb, owner_name, prefix, jobs);

  if (rc != 0)
  {
    std::string diag(zjb.diag.e_msg, zjb.diag.e_msg_len);
    diag.push_back('\0');
    e2a_inplace(diag);
    diag.pop_back();
    throw std::runtime_error(diag);
  }

  for (auto &job : jobs)
  {
    e2a_inplace(job.jobname);
    e2a_inplace(job.jobid);
    e2a_inplace(job.owner);
    e2a_inplace(job.status);
    e2a_inplace(job.full_status);
    e2a_inplace(job.retcode);
    e2a_inplace(job.correlator);
  }

  return jobs;
}

ZJob get_job_status(std::string jobid)
{
  ZJob job = {0};
  ZJB zjb = {0};

  a2e_inplace(jobid);
  int rc = zjb_view(&zjb, jobid, job);

  if (rc != 0)
  {
    std::string diag(zjb.diag.e_msg, zjb.diag.e_msg_len);
    diag.push_back('\0');
    e2a_inplace(diag);
    diag.pop_back();
    throw std::runtime_error(diag);
  }

  e2a_inplace(job.jobname);
  e2a_inplace(job.jobid);
  e2a_inplace(job.owner);
  e2a_inplace(job.status);
  e2a_inplace(job.full_status);
  e2a_inplace(job.retcode);
  e2a_inplace(job.correlator);

  return job;
}

std::vector<ZJobDD> list_spool_files(std::string jobid)
{
  std::vector<ZJobDD> jobDDs;
  ZJB zjb = {0};

  a2e_inplace(jobid);
  int rc = zjb_list_dds(&zjb, jobid, jobDDs);

  if (rc != 0)
  {
    std::string diag(zjb.diag.e_msg, zjb.diag.e_msg_len);
    diag.push_back('\0');
    e2a_inplace(diag);
    diag.pop_back();
    throw std::runtime_error(diag);
  }

  for (auto &dd : jobDDs)
  {
    e2a_inplace(dd.jobid);
    e2a_inplace(dd.ddn);
    e2a_inplace(dd.dsn);
    e2a_inplace(dd.stepname);
    e2a_inplace(dd.procstep);
  }

  return jobDDs;
}

std::string read_spool_file(std::string jobid, int key)
{
  std::string response;
  ZJB zjb = {0};

  a2e_inplace(jobid);
  int rc = zjb_read_jobs_output_by_key(&zjb, jobid, key, response);

  if (rc != 0)
  {
    std::string diag(zjb.diag.e_msg, zjb.diag.e_msg_len);
    diag.push_back('\0');
    e2a_inplace(diag);
    diag.pop_back();
    throw std::runtime_error(diag);
  }

  e2a_inplace(response);

  return response;
}

std::string get_job_jcl(std::string jobid)
{
  std::string response;
  ZJB zjb = {0};

  a2e_inplace(jobid);
  int rc = zjb_read_job_jcl(&zjb, jobid, response);

  if (rc != 0)
  {
    std::string diag(zjb.diag.e_msg, zjb.diag.e_msg_len);
    diag.push_back('\0');
    e2a_inplace(diag);
    diag.pop_back();
    throw std::runtime_error(diag);
  }

  e2a_inplace(response);

  return response;
}

std::string submit_job(std::string jcl_content)
{
  std::string jobid;
  ZJB zjb = {0};

  a2e_inplace(jcl_content);
  int rc = zjb_submit(&zjb, jcl_content, jobid);

  if (rc != 0)
  {
    std::string diag(zjb.diag.e_msg, zjb.diag.e_msg_len);
    diag.push_back('\0');
    e2a_inplace(diag);
    diag.pop_back();
    throw std::runtime_error(diag);
  }
  e2a_inplace(jobid);
  return jobid;
}

bool delete_job(std::string jobid)
{
  ZJB zjb = {0};

  a2e_inplace(jobid);
  int rc = zjb_delete(&zjb, jobid);

  if (rc != 0)
  {
    std::string diag(zjb.diag.e_msg, zjb.diag.e_msg_len);
    diag.push_back('\0');
    e2a_inplace(diag);
    diag.pop_back();
    throw std::runtime_error(diag);
  }

  return true;
}