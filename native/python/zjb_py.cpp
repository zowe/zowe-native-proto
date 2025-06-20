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
        e2a_inplace(job.job_correlator);
    }

    return jobs;
}