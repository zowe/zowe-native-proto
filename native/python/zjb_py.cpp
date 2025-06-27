#include "zjb_py.hpp"
#include <iostream>

std::vector<ZJob> list_jobs_by_owner(std::string owner_name)
{
    std::vector<ZJob> jobs;
    ZJB zjb = {0};

    a2e_inplace(owner_name);
    int rc = zjb_list_by_owner(&zjb, owner_name, "", jobs);

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

ZJob get_job_status(std::string jobid)
{
    ZJob job = {0};
    ZJB zjb = {0};

    a2e_inplace(jobid);
    int rc = zjb_view(&zjb, jobid, job);

    e2a_inplace(job.jobname);
    e2a_inplace(job.jobid);
    e2a_inplace(job.owner);
    e2a_inplace(job.status);
    e2a_inplace(job.full_status);
    e2a_inplace(job.retcode);
    e2a_inplace(job.job_correlator);

    return job;
}

std::vector<ZJobDD> list_spool_files(std::string jobid)
{
    std::vector<ZJobDD> jobDDs;
    ZJB zjb = {0};

    a2e_inplace(jobid);
    int rc = zjb_list_dds(&zjb, jobid, jobDDs);

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

    e2a_inplace(response);

    return response;
}

std::string get_job_jcl(std::string jobid)
{
    std::string response;
    ZJB zjb = {0};

    a2e_inplace(jobid);
    int rc = zjb_read_job_jcl(&zjb, jobid, response);

    e2a_inplace(response);

    return response;
}

std::string submit_job(std::string jcl_content)
{
    std::string jobid;
    ZJB zjb = {0};

    a2e_inplace(jcl_content);
    int rc = zjb_submit(&zjb, jcl_content, jobid);
    e2a_inplace(jobid);
    return jobid;
}

bool delete_job(std::string jobid)
{
    ZJB zjb = {0};

    a2e_inplace(jobid);
    int rc = zjb_delete(&zjb, jobid);

    return true;
}