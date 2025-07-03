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

static void e2a_inplace(std::string &s)
{
  if (s.empty())
    return;
  s.push_back('\0');
  __e2a_s(&s[0]);
  s.pop_back();
}

std::vector<ZJob> list_jobs_by_owner(std::string owner_name)
{
  std::vector<ZJob> jobs;
  ZJB zjb = {0};

  __a2e_s(&owner_name[0]);
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

    __a2e_s(&owner_name[0]);
    __a2e_s(&prefix[0]);
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